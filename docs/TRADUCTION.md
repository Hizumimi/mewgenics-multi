# Traduire Mewgenics Multi

Le mod (ses fenêtres dans le jeu) et le lanceur « Mewgenics Multi.exe » parlent les
dix langues de Mewgenics : `fr` (la langue source), `en`, `sp`, `de`, `it`, `pt-br`,
`ru`, `ko`, `ja`, `zh-cn` — les codes du jeu (`settings.txt`, `data/text/combined.csv`).
Ce document dit comment écrire une traduction et quelles règles le test vérifie.
Restent en français : le LISEZ-MOI du zip, les journaux (`log_line`, `mgmp_trace`), le
panneau technique F1 (ImGui).

## 1. Le principe

- Chaque texte reste écrit **en français dans le code**, passé par `tr()`
  (`src/i18n/mm_lang.h`) : `tr(u8"Héberger")`. Ce texte exact est la **clé**.
- Une table par langue, `src/i18n/lang_<code>.cpp` (`lang_pt_br.cpp`, `lang_zh_cn.cpp`) :
  des paires `{u8"clé française exacte", u8"traduction"},` avant `{nullptr, nullptr}`.
  Une traduction absente (ou `""`) laisse le texte en français.
- **La langue** : la DLL suit le jeu (ligne `current_language` de son `settings.txt`,
  dans `%APPDATA%\Glaiel Games\Mewgenics\<id Steam>\`, à côté du dossier `saves`) — lue
  au chargement, à chaque retour au menu principal et dans les 2 s qui suivent un
  changement du fichier ; une ligne `langue du jeu : xx` au journal. Le lanceur prend
  `"ui": { "lang": "ja" }` de `mgmp.json` (`"auto"` ou absent : la langue du jeu, sinon
  celle de Windows, sinon le français) ; le choix « Langue » de ses Réglages l'écrit.
- Les outils : `tools/i18n_cles.py` écrit toutes les clés dans un CSV (où, contexte,
  largeur permise, traductions déjà faites, noms officiels des boutons du jeu cités) ;
  `tools/i18n_table.py` écrit une table depuis des fichiers JSON (`--extraire` : l'inverse) ;
  `tools/i18n_lignes.py` simule la mise en page du jeu (section 3) ; `tests/test_i18n.cpp`
  vérifie les tables (section 5).

## 2. Écrire une traduction

```cpp
const TrPair kTable_de[] = {
    {u8"Héberger", u8"Hosten"},
    {u8"Connexion à %s...", u8"Verbinde mit %s..."},
    {u8"Multijoueur\n\nHéberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"Mehrspieler\n\nHoste ein Spiel, damit deine Freunde beitreten,\n"
     u8"oder tritt dem Spiel eines Freundes bei."},
    {nullptr, nullptr}   // fin de la table
};
```

- La clé se recopie **à l'octet près** : mêmes `\n`, mêmes espaces, mêmes `%s`, mêmes
  balises `[c:red]…[/c]`, même ponctuation. Le plus sûr : ne pas écrire la table à la main,
  mais passer par `tools/i18n_table.py` :

  ```
  python tools/i18n_table.py de travail_de --extraire   # la table -> travail_de/00_table.json
  (corriger, ou ajouter un travail_de/50_ajouts.json : { "clé française": "traduction" })
  python tools/i18n_table.py de travail_de              # réécrit src/i18n/lang_de.cpp
  ```

  Dans le JSON, un retour à la ligne s'écrit comme dans le CSV de `tools/i18n_cles.py`
  (les deux caractères `\n`). L'outil refuse une clé qui n'est plus dans le code, écrit les
  caractères invisibles en `\u…` (U+200B et U+00A0 se perdent dans la plupart des éditeurs)
  et range la table dans l'ordre du code. Une clé dont le texte français a changé disparaît
  de l'extraction : la retraduire.
- Plusieurs `u8"…"` à la suite se collent (comme dans le code) : pratique pour les textes
  longs.
- Échappements : `\n` (retour à la ligne), `\"`, `\\`, `\u00A0` (espace insécable),
  `\u200B` (coupure permise, japonais et chinois). Les caractères s'écrivent tels quels
  en UTF-8 ; seuls les caractères invisibles s'écrivent en `\u…`.
- Pas d'entités HTML : `&nbsp;` (que `combined.csv` emploie) n'est remplacé que dans les
  textes du jeu (0x1409610A0, appelé par sa traduction), pas dans les nôtres (`SwfSetText`
  ne l'appelle pas) ; écrire `\u00A0`.
- Le même texte français à deux endroits du code n'a qu'une traduction. S'il en faut
  deux, le texte français doit différer.

## 3. Les retours à la ligne

### Dans le jeu (textes de `src/`)

Nos fenêtres sont la boîte de confirmation du jeu (`ConfirmationBox`, `swfs/ui.swf`,
caractère 2879) : son champ `prompt` (caractère 2877) est en **TikaFont 24 px**, centré,
**536 px de large** et 234 px de haut ; ses boutons `yes` / `no` (caractère 2748) en
TikaFont 28 px sur **223 px** de large et 105 px de haut.

**Le jeu coupe les lignes tout seul**, quoi que dise le SWF (le drapeau WordWrap du
champ `prompt` est faux, mais le moteur l'ignore) : `SwfSetText` (0x14098E8A0) met le
texte en page par 0x140990480 (texte simple) ou 0x140992440 (avec balises `[c:…]`). Les
deux font la même chose :

- ils ajoutent l'avance de chaque caractère ; quand la ligne dépasse la largeur du champ,
  ils coupent à la **dernière espace (U+0020) ou U+200B** de la ligne, qui devient le
  retour ;
- sans espace ni U+200B depuis le début de la ligne, ils coupent **juste avant le
  caractère qui déborde**, en plein mot (en japonais et en chinois sans U+200B : n'importe
  où, même devant 。 ou 」) ;
- U+200B a une **largeur nulle** et ne se dessine pas ; `\n` force un retour ; l'espace
  insécable U+00A0 ne coupe jamais (TikaFont l'a).

Ce que valent donc les `\n` : ce sont des **retours voulus** (un titre, la ligne vide
`\n\n`, une liste, une phrase qu'on veut voir commencer à la ligne) — pas des coupures de
largeur. Une ligne plus large que la boîte est recoupée par le jeu : c'est permis (près de
la moitié des lignes françaises le sont déjà), mais chaque ligne recoupée ajoute une ligne
à une boîte de hauteur fixe.

**Largeur d'une ligne : environ 36 colonnes.** Une colonne = une lettre latine,
cyrillique ou un signe ; **un idéogramme, un kana, un hangul ou un caractère pleine chasse
compte double** ; U+200B et les balises `[c:red]` `[/c]` ne comptent pas. (Mesures sur les
polices du jeu, à 24 px : une lettre de TikaFont fait en moyenne 0,58 em, soit 14 px —
536 px ≈ 38 lettres ; un idéogramme ou un kana 1 em = 24 px — 22 par ligne ; un hangul
0,76 em.) Un bouton tient environ 13 lettres par ligne, sur 3 lignes au plus.

Hauteur : 234 px, soit 6 lignes de TikaFont si l'interligne suit la police (1,41 em +
2 px), davantage s'il est plus serré — non mesuré en jeu. Des textes français ont déjà
8 lignes (le salon) : une traduction ne doit pas en ajouter. `tools/i18n_lignes.py` compte
les lignes affichées de chaque texte dans chaque langue (avances des glyphes des polices du
jeu, ordre de `data/font_remap.gon`, coupure du jeu) et liste ceux qui en prennent plus
qu'en français : `python tools/i18n_lignes.py cles.csv de` (une estimation : les `%s` ne
sont pas remplacés).

### Dans le lanceur (textes de `launcher/`)

Le lanceur coupe lui-même ses paragraphes à la largeur du papier (`launcher/ml_text`) : à
une espace ou à U+200B (qui disparaissent), après `\` et `/` ; jamais devant `: ; ! ? »`
ni après `«` ; un mot trop long est coupé où il déborde ; `\n` force un retour. Tailles :
13,5 à 21 px pour le texte, 44 à 48 px pour les titres. Les **boutons ont une largeur
fixe** (« Réparer » 112 points, « Parcourir… » 150, « Ouvrir le dossier » 230, « Copier
le dernier journal » 290, « Jouer » 280 à 38 px) : leurs libellés restent courts.

Polices : les polices du jeu (`swfs/fonts.swf`), et pour `ru`, `ko`, `ja`, `zh-cn` celles
de son alphabet (`swfs/international_fonts.swf` : TikaFontCyr et EdmundmCyr ; KoreanAll ;
TikaFontJP et EdmundmJP ; TikaFontCN et EdmundmCN, comme `data/font_remap.gon`), puis les
polices de Windows (Segoe UI, Malgun Gothic, Yu Gothic UI, Microsoft YaHei UI).

## 4. Japonais, chinois, coréen, russe

- **Japonais et chinois** : mettre `\u200B` aux coupures permises, comme `combined.csv`
  (entre les mots ou groupes de mots : `冒険を\u200Bやめますか？`), jamais devant une
  ponctuation fermante (。、！？」』）) ni après une ouvrante. Une ligne de plus de 36
  colonnes sans `\u200B` ni espace est refusée par le test.
- TikaFontJP n'a que **3449 glyphes** (les kanji des textes du jeu) : un kanji rare est
  dessiné par une police de secours (TikaFontCN, puis NotoSans dans le jeu ; Yu Gothic UI
  dans le lanceur) — préférer les kanji et kana courants. TikaFontCN a 8959 hanzi
  simplifiés.
- **Coréen** : des espaces entre les mots, comme le jeu (aucun U+200B dans sa colonne) ;
  KoreanAll a tous les hangul.
- **Russe** : TikaFontCyr (latin + cyrillique) ; guillemets « » comme en français.

## 5. Ce que vérifie `tests/test_i18n.cpp`

Le test lit les sources (`src/`, `launcher/`), y trouve chaque clé (`tr(` ou `trk(` suivi
de littéraux), puis contrôle chaque traduction. **Erreurs** (le test échoue) :

1. **Traduction orpheline** : sa clé n'est plus dans le code (le texte français a changé :
   recopier la nouvelle clé, revoir la traduction). Ou clé en double dans une table.
2. **Marqueurs printf** : exactement les mêmes, dans le même ordre et le même nombre
   (`%s`, `%d`, `%u`, `%02u`, `%lld`, `%ls`, `%%`…). Pas de marqueurs numérotés : si
   l'ordre des mots doit changer, reformuler la phrase.
3. **Même nombre de `\n`** que la clé (la structure du texte).
4. **Largeur de chaque ligne**, en colonnes (section 3) : au plus **max(36 ; 1,3 × la
   ligne française)** ; pour une ligne française de 16 colonnes ou moins (boutons, titres) :
   au plus **max(1,6 × ; + 6)**. Ex. : « Jouer » (5) → 11 ; « Réglages » (8) → 14 ; une
   ligne de 40 → 52.
5. **Japonais et chinois** : une ligne de plus de 36 colonnes sans U+200B ni espace.
6. **Bouton cité** : quand la clé cite entre « » un texte qui est lui-même une clé (un
   bouton du mod : « Pas encore », « Héberger »), la traduction doit contenir la
   traduction de ce bouton, telle quelle.

**Avertissement** : les clés encore sans traduction, comptées par langue. Avec la variable
d'environnement `MM_I18N_STRICT=1` (ou `-DMM_I18N_STRICT`), elles deviennent des erreurs.

## 6. Le ton

- Le français du mod **tutoie** : `de` « du », `sp` « tú », `it` « tu », `pt-br`
  « você », `ru` « ты » (comme les colonnes de ces langues dans `combined.csv`, qui
  tutoient aussi) ; `en` « you », simple et direct.
- `ko` et `ja` : la forme polie simple des jeux vidéo, comme `combined.csv`
  (`-시겠습니까` / `-세요`, `です` / `ます`). `zh-cn` : simple et direct, comme le jeu.
- Phrases courtes, le vocabulaire du jeu (section 7) ; pas de traduction mot à mot des
  tournures françaises.

## 7. Glossaire

### Les boutons et écrans du jeu

Quand le mod cite un bouton du jeu, la traduction reprend **le texte officiel du jeu dans
sa langue** (colonne de `combined.csv` dans `resources.gpak`, à lire sur sa machine ;
`tools/i18n_cles.py` le met dans sa dernière colonne pour chaque bouton cité) :

| clé de `combined.csv` | texte français du jeu | cité par le mod |
|---|---|---|
| `MAIN_MENU_PLAY` | Jouer | « Jouer » (recharger sa save) |
| `MAIN_MENU_MEOW` | Miauler | le bouton que le mod renomme « Multijoueur » |
| `MAIN_MENU_SETTINGS` | Réglages | |
| `MAIN_MENU_QUIT` | Quitter | |
| `PAUSE_MENU_BUTTON_QUITTOMENU` | Sauvegarder et retourner au menu | |
| `PAUSE_MENU_BUTTON_ABANDONRUN` | Abandonner l'aventure | le bouton qui devient « Échanger » |
| `PAUSE_MENU_BUTTON_ABANDONFIGHT` | Déclarer forfait | |
| `HOUSE_BUTTON_DEPART` | À l'aventure ! | « À l'aventure ! » |
| `HOUSE_BUTTON_DEFEND` | Défendre ! | « Défendre ! » |
| `HOUSE_BUTTON_ENDDAY` | Fin de journée | |
| `INVENTORY_LOCKIN_BUTTON` | Valider | « Valider » (colliers, objets) ; le bouton de l'échange est « Confirmer », pour ne pas le confondre |
| `POPUP_MODAL_YES`, `_NO`, `_OK`, `_CANCEL` | Oui, Non, OK, Annuler | |
| `NPC_NAME_BEANIES`, `NPC_NAME_JACK`, `NPC_NAME_BUTCH`… | Dr Beanies, Bébé Jack, Butch… | les PNJ à qui l'on donne un chat |
| `EVENT_ORGANGRINDER_NAME` | Broyeur d'organe | ce PNJ-là, que le joueur nomme lui-même |

### Les mots du mod

À traduire une fois, puis à reprendre **à l'identique** partout (le test le vérifie pour
les boutons cités) : Multijoueur, Héberger, Rejoindre, salon, hôte, invité, Échanger,
J'arrive, Pas encore, Rentrer, Recharger, Ne pas attendre, bourse, butin, Besoin,
Cupidité.

### À ne pas traduire

Mewgenics Multi, Mewgenics, Steam, Discord, les noms de fichiers et de réglages
(`mgmp.json`, `mgmp.dll`, `mgmp_loader.exe`, `Mewgenics.exe`, `resources.gpak`,
`steam_appid.txt`, `loader.txt`, `net.control`, `"auto"`), les adresses (127.0.0.1), les
noms des joueurs (`%s`). Les touches se traduisent comme sur un clavier du pays (« Échap »
→ Esc, Escape…), Ctrl+V aussi (Strg+V sur un clavier allemand, Ctrl+V ailleurs).

## 8. Pour les développeurs

- Un texte affiché au joueur : `tr(u8"…")`. Avec des valeurs : `fmt(tr(u8"Connexion à
  %s..."), cible)` — le format est traduit, les valeurs non.
- Une phrase entière par clé (avec `%s`), plutôt que des morceaux recollés : l'ordre des
  mots change d'une langue à l'autre.
- Un texte rangé dans une table et traduit plus tard : `trk(u8"…")` à l'endroit où il est
  écrit (le test et l'outil le trouvent), `tr(variable)` à l'affichage.
- Ne passent pas par `tr()` : les journaux, les noms internes (scènes, boutons du jeu
  comme `btn_feedback`, clés de `combined.csv`), et ce qui part sur le réseau (la raison
  d'un `MSG_REFUSE`, que le salon de l'autre reconnaît par ses mots ; le pseudo du HELLO) :
  celui qui reçoit traduit ce qu'il affiche.
- Un texte relu à l'écran (le libellé « Échanger » du menu pause) : garder la chaîne
  exacte qu'on y a écrite et comparer à elle, pas à un nouvel appel de `tr()` (la langue
  peut avoir changé entre-temps).
