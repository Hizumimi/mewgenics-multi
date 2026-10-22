# Passation — Mewgenics Multi

Note écrite le 22 octobre 2026 par Claude, dans la session claude.ai qui a fait les
builds 1 à 52, pour que le travail continue dans Claude Code. Elle dit ce qui a été
fait, comment on travaillait, où en est le projet et ce qui reste.

À lire avec : `CLAUDE.md` (les règles du dépôt), `docs/CONCEPTION.md` (les règles du
jeu à plusieurs, copie du document de conception en ligne) et `docs/NOTES_MGMP.md`
(ce que fait le binaire du jeu : adresses, structures, mesures).

---

## 1. En bref

- **Mewgenics Multi** est un mod coop de 2 à 4 joueurs pour Mewgenics (Steam,
  application 686060, version **1.1.21239**). Il est bâti sur **mgmp** de
  SanTertrust (licence MIT, https://github.com/SanTertrust/mgmp), qui synchronisait
  déjà les combats et l'aventure à deux. Chaque joueur garde sa save, sa maison et
  ses chats ; le monde (PNJ, histoire, déblocages) est celui de l'hôte ; les
  aventures se jouent ensemble et chacun dirige ses chats.
- **Le dépôt** : `Hizumimi/mewgenics-multi` (privé), une seule branche, `main`. Les
  commits 1 à 10 sont mgmp d'origine (27 août – 12 septembre 2026) ; Mewgenics
  Multi commence le 11 octobre 2026 (commit `7e18b7d`). Pour y travailler :
  `git clone https://github.com/Hizumimi/mewgenics-multi.git`, connecté au compte
  GitHub de Hizu. Cette note y est (`docs/PASSATION.md`), avec une copie dans
  `Bureau\mgmp\PASSATION.md`.
- **Une build par commit** : chaque push sur `main` est compilé par GitHub Actions
  (Windows, MSVC), testé, et publié en pré-release `build-N` (sauf un commit marqué
  `[skip ci]`, section 4). 52 builds du 11 au 18 octobre ; protocole réseau 27
  (mgmp) → **66**.
- **Où on en est (22 octobre)** : la **build 52** (commit `9606ec7`, protocole 66)
  est installée chez Hizu depuis le 18 octobre au soir, et le zip pour son ami est
  prêt dans le même dossier. **Elle n'a pas encore été jouée** : aucun journal du
  mod depuis le 18 octobre (le lancement du 20 octobre a échoué parce que Steam
  était fermé, section 10). Elle corrige le gel du 18 octobre (section 7).
- **Les joueurs** : Hizu (propriétaire du dépôt, l'hôte en général) et un ami
  (l'invité), chacun sur son PC, par Steam ou par adresse IP. Les deux doivent avoir
  le même protocole (en pratique, la même build) : un autre protocole est refusé à la
  connexion, avec la raison dans le salon. La connexion compare le protocole et les
  empreintes du jeu et de `resources.gpak`, pas le numéro de build.

## 2. Où sont les choses

| Quoi | Où |
|---|---|
| Le code | ce dépôt : `src/`, `loader/`, `tests/`, `tools/` |
| Les règles du jeu à plusieurs | document en ligne https://claude.ai/code/artifact/5e9195c8-3a1e-446a-a5a6-a43fea7459f7 ; copie du 22 octobre : `docs/CONCEPTION.md`. Dans « Plan de développement », les builds 1–18 sont dans les étapes numérotées, puis un paragraphe par build ou groupe de builds, les décisions de Hizu datées dans « Questions ouvertes » |
| Le binaire du jeu | `docs/NOTES_MGMP.md` (écrit par l'auteur de mgmp ; tenues à jour depuis la build 40 : sections « Protocol 59 » à « Protocols 64–66 », « Steam transport », et le paragraphe « protocol 58 » de « Reconnect and mid-fight join » ; avant, les trouvailles sur le binaire sont dans les messages de commit et les commentaires du code) |
| L'historique du protocole | le commentaire de `kProtoVersion`, `src/net/mgmp_proto.h` ligne 54 : une phrase par version, de 66 (en tête) à 28 (avec deux entrées « 48 »), puis l'historique de mgmp, 2 à 27, en anglais. **Une seule ligne de 24 Ko** : la lire par morceaux (`grep -n -a … \| cut -c1-200`) |
| Le pourquoi de chaque changement | les messages de commit (en français depuis le 11 octobre : la règle, le cas, le protocole, les tests) |
| Les builds | releases `build-1` à `build-52` du dépôt |
| Le zip de l'ami | modèle dans `paquet/` ; `tools/paquet.ps1` (Windows) ou `tools/linux/paquet.sh` le refont |
| Le mod chez Hizu | `C:\Users\Hizumi\Desktop\mgmp` : `mgmp.dll`, `mgmp_loader.exe`, `mgmp.json`, `Lancer Mewgenics coop.bat`, les journaux, `mgmp-pour-ton-ami.zip` ; `original-mgmp-v1\` (les binaires de mgmp v1, gardés à part) et `outils\` (des scripts des débuts, qui listent les fichiers du jeu) |
| Le jeu | `C:\Program Files (x86)\Steam\steamapps\common\Mewgenics`, avec `steam_appid.txt` (686060) à côté de `Mewgenics.exe` : sans lui, Steam relance le jeu sans le mod |
| Les saves | `%APPDATA%\Glaiel Games\Mewgenics\<id Steam>\saves` (des bases sqlite `.sav`). On n'y écrit jamais avec un outil : on analyse des copies (règle de Hizu : rien n'est modifié dans ce dossier) |

## 3. Comment on a travaillé

- **Une fonction par build.** Le plan, le code, les tests de logique pure
  (`tests/`), la vérification de compilation, le commit et le push, la CI, la
  livraison (section 4), puis un paragraphe « Build N » dans le document de
  conception, et le LISEZ-MOI de l'ami quand ce que voient les joueurs change.
- **Des relectures indépendantes.** Après chaque fonction importante, deux ou trois
  relectures du code par des agents qui ne voyaient pas le travail (bugs,
  synchronisation, cas limites) ; leurs points, vérifiés un par un, faisaient la
  build suivante (par exemple 42 → 44, 46 → 48, 50 → 52). Deux vérifications
  globales de tous les chemins de synchronisation : builds 40 et 42–45.
- **Le binaire fait foi.** Une adresse, un champ, un appel se vérifient dans
  `Mewgenics.exe` 1.1.21239 (22 019 072 octets, SHA-256
  `4127cd6a792ae528bca6f65a8873dd61789591937d87656c2b586a5e30eb77ea`), pas seulement
  dans des notes. Outils : Python avec `pefile` et `capstone` (désassembler, trouver
  les appels et les références d'une fonction), Ghidra en mode headless pour
  décompiler une fonction, `tools/gen_sig.py` pour la signature d'une nouvelle
  cible. Les tables du code (`mgmp_addresses.h`,
  `mgmp_sigs.generated.h`) et le journal (`Mewgenics.exe+0x…`) donnent des RVA ; les
  commentaires, les commits et les notes donnent souvent l'adresse virtuelle,
  base `0x140000000` (RVA = adresse − `0x140000000`).
- **Les journaux d'abord.** Un bug de partie se diagnostique dans les journaux des
  deux joueurs avant de toucher au code (section 9) ; la cause et la mesure sont
  écrites dans le commit et, pour ce qui touche au binaire, dans
  `docs/NOTES_MGMP.md` (depuis la build 40).
- **Le protocole.** Tout changement de ce que calcule ou envoie un pair :
  `kProtoVersion` + 1 et une phrase en tête de l'historique (ligne 54). Les deux
  joueurs mettent alors à jour.
- **Le style.** Textes affichés aux joueurs en français avec accents (`u8"…"`,
  sources en UTF-8, MSVC `/utf-8`). Commentaires des fichiers de Mewgenics Multi en
  français sans accents ; ceux de mgmp restent en anglais. Les règles pures (sans
  Windows ni jeu) vont dans un `*_core.cpp/.h` ou un en-tête testé par `tests/` ; le
  fichier sans `_core` fait le lien avec le jeu et le réseau.
- **Les décisions de règle** ont été demandées à Hizu sous forme de choix, avec une
  option recommandée ; ses réponses sont datées dans le document de conception.
  Hizu écrit en français et teste les builds en partie avec son ami.

## 4. Compiler, tester, livrer

### La compilation qui compte : GitHub Actions

`.github/workflows/build.yml`, à chaque push sur `main` :
`cmake -B build -A x64 -DMM_BUILD_ID=<numéro du run>`,
`cmake --build build --config Release --parallel`,
`ctest --test-dir build --build-config Release --output-on-failure` (25 tests), puis
la pré-release `build-<numéro du run>` avec `mgmp.dll`, `mgmp_loader.exe` et
`SHA256SUMS.txt`. Le numéro de build est dans la DLL : en tête de chaque journal
(`mod    : Mewgenics Multi, build N`) et dans le titre du panneau F1. Les builds
récentes compilaient sans aucun avertissement. Un commit dont le message contient
`[skip ci]` ne lance pas de build : utile quand il ne touche pas au mod
(documentation, scripts), comme celui de cette note.

Suivre une build : `gh run list -R Hizumimi/mewgenics-multi --commit <sha>`,
`gh run view <id> -R Hizumimi/mewgenics-multi --log` (chercher `warning C`, `error C`,
`FAIL`), `gh api "repos/Hizumimi/mewgenics-multi/releases?per_page=3"`.

### En local sous Windows

Visual Studio 2022 (C++ x64) et CMake 3.21 ou plus : `cmake -B build -A x64` puis
`cmake --build build --config Release` ; les tests :
`ctest --test-dir build -C Release`. Seul MSVC x64 convient pour la DLL : le mod
partage avec le jeu des objets de la bibliothèque standard (`std::string`,
`std::function`) dont la taille et la disposition sont celles de MSVC
(`src/ui/mm_menu.cpp` le vérifie lignes 82 à 84).

### Sous Linux, pour vérifier seulement

Les vérifications faites à la main dans le conteneur de claude.ai avant chaque push,
mises en scripts le 22 octobre (aucun des deux ne produit la DLL) :

- `tools/linux/tests.sh` : les 22 tests de logique pure, avec g++ (les 3 autres,
  `test_config`, `test_sigscan`, `test_resolve_live`, demandent Windows).
- `tools/linux/compiler.sh [fichiers.cpp]` : compile pour Windows avec zig
  (`pip install ziglang`), sans lier ; sans argument, tous les fichiers de `src/`.
  Les avertissements connus, sans conséquence, sont listés en tête du script.

### Ajouter un fichier, un test, un crochet

- Un nouveau `.cpp` du mod s'ajoute à la liste `add_library(mgmp SHARED …)` de
  `CMakeLists.txt` (pas de recherche automatique). Un nouveau test : un
  `add_executable`/`add_test` dans `tests/CMakeLists.txt` et une ligne dans
  `tools/linux/tests.sh`.
- Une fonction du jeu à intercepter :
  1. la trouver dans `Mewgenics.exe` ;
  2. `python tools/gen_sig.py <Mewgenics.exe> <rva_hex>:NOM` donne sa ligne pour
     `src/core/mgmp_sigs.generated.h` ; `python tools/verify_sigs.py <Mewgenics.exe>`
     revérifie toutes les signatures ;
  3. `src/core/mgmp_addresses.h` : une valeur dans l'enum (avant `T_COUNT`) et une
     ligne dans la table (RVA, nom court, symbole) ;
  4. rien à faire dans `src/core/mgmp_config.cpp` : toutes les cibles y sont
     allumées par défaut, et `hooks_implied_by(true)` est appelé à chaque chargement
     (la paire éteint / allumé de `T_CharFace` n'a pas d'effet) ;
  5. `src/core/mgmp_hooks.cpp` : le détour `h_…`, le pointeur `o_…` et la liaison
     `{ T_…, (void*)&h_…, (void**)&o_… }`.

  Au démarrage, le journal liste chaque crochet : `[+]` posé, `[!]` pas posé (et
  pourquoi).

### Livrer une build

1. Attendre la fin de la CI et la release `build-N`.
2. **Windows** :
   `powershell -ExecutionPolicy Bypass -File tools\paquet.ps1 -Build N -Installer`.
   Il faut `gh` (GitHub CLI) connecté ; sinon télécharger `mgmp.dll`,
   `mgmp_loader.exe` et `SHA256SUMS.txt` depuis la page de la release et ajouter
   `-Depuis <dossier>`. Le script vérifie les empreintes, écrit
   `mgmp-pour-ton-ami.zip` dans `Bureau\mgmp` et, avec `-Installer` et le jeu fermé,
   y remplace `mgmp.dll` et `mgmp_loader.exe`.
   **Linux** : `tools/linux/paquet.sh N` prépare la même chose dans
   `build_linux/paquet-N/`.
3. Hizu envoie le zip à son ami.
4. Si le protocole a changé, le dire : les deux joueurs doivent mettre à jour.
5. Le document de conception : un paragraphe « Build N » (ce qui change pour les
   joueurs, « à tester ») ; `paquet/LISEZ-MOI.txt` si ce que voient les joueurs
   change (UTF-8 avec BOM, fins de ligne CRLF : `.gitattributes` les garde).

Le zip contient un dossier `mgmp\` : `LISEZ-MOI.txt`, `Lancer Mewgenics coop.bat`,
`mgmp.json`, `steam_appid.txt`, `mgmp.dll`, `mgmp_loader.exe`. Celui de la build 52,
refait par les deux scripts, est identique à celui livré le 18 octobre, sauf le .bat,
remplacé le 22 octobre dans le zip et chez Hizu (section 10).

## 5. Comment marche le mod

### Le démarrage

`Lancer Mewgenics coop.bat` lance `mgmp_loader.exe`, qui démarre `Mewgenics.exe`
(chemin : `game` dans `mgmp.json`) et y injecte `mgmp.dll` ; le jeu n'est pas modifié
sur le disque. La DLL lit `mgmp.json` à côté d'elle, retrouve ses cibles par
signature (125 dans la build 52 ; `mgmp_sigscan`, `mgmp_resolve`,
`mgmp_sigs.generated.h`), vérifie leurs prologues et pose les crochets MinHook
(`src/core/mgmp_hooks.cpp`). Elle ne contient aucune donnée du jeu : ce qu'il lui en
faut est lu dans `resources.gpak` sur la machine du joueur (`mgmp_gpak`).

### Le réseau

En étoile : chaque invité n'a qu'une connexion, vers l'hôte, qui relaie
(`src/net/mgmp_net.cpp`, TCP, port 27600 par défaut). Par Steam (build 39,
`mgmp_steam`) : « Rejoindre la partie » dans la liste d'amis ; c'est un tuyau entre
une connexion Steam (ISteamNetworkingSockets, par le `steam_api64.dll` du jeu) et une
connexion TCP locale, le reste du mod ne le voit pas. Les messages sont dans
`src/net/mgmp_proto.h` : ceux de mgmp (`MSG_HELLO` = 1 … `MSG_HOSTLEFT` = 20) et ceux
de Mewgenics Multi : `MSG_MM` (petits messages, `MM_NIGHT_READY` = 1 …
`MM_BATTLE_END` = 15), `MSG_WORLD`, `MSG_NPC`, `MSG_TEAM` (sous-sortes
`kTeamDepart` = 1 … `kTeamGift` = 18, dans `src/session/mm_team_merge.h`). La
poignée de main compare la version du mod (protocole), celle du jeu et ses fichiers.

### Le combat : lockstep déterministe (mgmp)

Les combats du jeu sont déterministes : seules les décisions des joueurs circulent
(`TurnAction`, prises au crochet de `Brain::GetChoice`), chaque PC simule tout le
reste. `GetChoice` est interrogé à chaque image : tant que la décision d'un autre
joueur n'est pas arrivée, le mod répond « rien » et le jeu attend. À chaque fin de
tour, une empreinte de l'état (`MSG_HASH`) est comparée ; au premier écart, le
combat s'arrête (`debug.desync_halt`) ; chacun envoie à l'autre son tableau des champs
hachés par chat (`STATEDUMP`) et chaque journal écrit la différence champ par champ. `g.turn`
est le numéro du tour en cours ; l'empreinte du tour T part à sa fin. Un chat est
désigné par sa place dans la liste des personnages du combat (ordre d'apparition),
pas dans l'ordre des tours, rebattu à chaque manche. **L'orientation des chats
(`Character+0x388`) n'est pas dans l'empreinte** (section 7).

Ajouts de Mewgenics Multi au combat :

- qui joue (`mgmp_members.h`) : l'hôte choisit les membres d'un combat à l'entrée
  de la case ; les chats d'un absent ou d'un joueur parti sont dirigés par l'hôte et
  joués par l'IA du jeu (`mm_ai`), dont la décision part chez tous ;
- revenir en plein combat (build 39) : l'état d'entrée de la case, puis toutes les
  décisions déjà jouées, rejouées chez lui (et l'empreinte du tour 0) ; chaque
  décision porte son auteur et son rang (`mgmp_seen.h`) ;
- l'aperçu de visée ne tourne plus le chat (crochet `T_CharFace`, build 50) ;
- un combat fini d'un seul côté (`mgmp_orphan.h`, builds 50–52).

### Hors combat : l'hôte décide

- **La carte** : l'invité suit l'hôte (`mgmp_follow`) ; un clic sur une case est un
  vote (`mm_vote`) ; à l'entrée de chaque case, l'hôte envoie chats, inventaire et
  historique de l'aventure (`mgmp_catsync`, `mgmp_invsync`, `mgmp_runhist`, par les
  sérialiseurs du jeu) et une empreinte de l'aventure est comparée avec chaque invité
  (`mgmp_nodehash`).
- **Les saves** (`mgmp_savefile`, `mm_team`) : chacun joue dans sa propre save. Au
  départ, l'invité qui répond « J'arrive » fait le même départ que l'hôte ; depuis la
  build 41, l'hôte tire une graine commune et l'invité garde sa propre carte si elle
  est identique, sinon il charge l'aventure de l'hôte, rangée à part
  (`mgmp_coop.sav`). À la fin, il rentre chez lui : ses chats (état final), sa
  bourse, son butin et ses recrues sont écrits dans sa save, après une copie de
  sécurité ; ce qui attend d'y entrer est gardé dans `<save>.mgmp-attente`.
- **Le monde** (`mm_world`, `mm_world_merge`, `mm_npc`, `mm_npc_state`,
  `mm_npcgift`) : avant chaque chargement, la save de l'invité reprend les PNJ,
  l'histoire et les déblocages de l'hôte (les saves sont des bases sqlite :
  `third_party/sqlite`) ; la faveur donnée par l'hôte arrive en direct ; un don de
  l'invité compte chez l'hôte, qui prend la récompense et lui renvoie pièces,
  nourriture et objets.
- **Le départ** : `mm_embark` (le départ voté, « J'arrive »), `mm_collars` (les
  colliers de classe, chacun pour ses chats), `mm_gear` (les objets du départ, de son
  propre rangement), `mm_slot_core.h` (la 4e place à 3 joueurs).
- **En aventure** : `mm_rungear` (sac commun, chacun équipe ses chats, chaque
  mouvement en direct), `mgmp_choice` (montées de niveau et événements au
  propriétaire du chat), `mm_purse` (une bourse par joueur), `mm_shop` (achats avec
  sa bourse, faits chez l'hôte), `mm_loot` (le défilé du butin à la victoire),
  `mm_recruit_core` (les recrues), `mm_nemesis` (la Némésis).
- **À la maison** : la nuit votée, « Défendre ! » (chacun seul chez soi),
  « Échanger » (`mm_gift`, `src/ui/mm_trade_ui.cpp`).
- **L'interface** (`src/ui/mm_menu.cpp`) : les fenêtres du jeu lui-même (sa boîte
  de confirmation, ses boutons, sa police) avec nos textes ; le salon, les attentes,
  les retours à la maison. Le panneau technique de mgmp (ImGui) est sur F1.

### Les réglages (`mgmp.json`, à côté de la DLL)

`game` (chemin du jeu), `log`, `net` (`role` : `off`, `host` ou `client` ; `addr`,
`port`, `control` ; `steam`, vrai par défaut ; `same_map`, vrai par défaut — faux
remet le départ avec second chargement d'avant la build 41), `ui` (`enabled`,
`visible`, `key`), `debug` (`follow`, `follow_delay_ms`, `join_barrier`,
`desync_halt`, `record`, `record_note`, `replay`). Les valeurs fixes du mod sont dans
`src/core/mgmp_tuning.h`. Dans la pratique, le salon (« Multijoueur ») choisit le
rôle : le `mgmp.json` livré a `role: off`.

### Les fichiers que le mod écrit

- dans son dossier : `mgmp_trace_AAAAMMJJ-HHMMSS.log` (un par lancement du jeu),
  `mgmp_boot.log`, `loader.txt` (écrit par le .bat), `dernier_hote.txt` (la dernière
  adresse tapée) ;
- dans le dossier des saves : `mgmp_coop.sav` (l'aventure de l'hôte, chez un
  invité), `<save>.mgmp-attente`, et `mgmp_backups\` (copies horodatées, dont
  `<save>.avant-monde-hote.<date>.bak` et `<save>.avant-retour-chats.<date>.bak`).

## 6. Ce qui a été fait, build par build

Dates d'octobre 2026. « Proto » : la version du protocole de la build. La dernière
colonne dit ce que l'on sait de sa livraison et d'une partie jouée avec elle
(d'après le document de conception, sauf : 34 d'après le commit `81fe344`, 46
d'après `docs/NOTES_MGMP.md` et l'historique du protocole, 31 de mémoire de la
session) ; une case vide veut dire : pas de retour.

| Build | Date | Proto | Ce qui change | Livrée, jouée |
|---|---|---|---|---|
| 1 | 11 | 27 | Base du projet à partir de mgmp : dépôt, compilation GitHub Actions, releases `build-N` | installée le 11 |
| 2 | 11 | 27 | Le bouton « Miauler » du menu principal devient « Multijoueur » | |
| 3 | 11 | 27 | Fenêtre Multijoueur : Héberger ou Rejoindre, salon en direct (adresse, joueurs), saisie de l'adresse | |
| 4 | 11 | 27 | Correctif : la fenêtre Multijoueur faisait planter le jeu | validée |
| 5 | 11 | 28 | Nuit votée : le clic sur le panneau de fin de journée est retenu jusqu'à ce que tous soient prêts (`MSG_MM`) | validée, deux jeux sur un PC |
| 6 | 11 | 29 | « 1 joueur attend » sur le panneau, fenêtre « Passer la nuit » | validée |
| 7 | 11 | 30 | Chacun sa save : départ voté, l'invité joue l'aventure de l'hôte puis rentre dans sa save | |
| 8 | 11 | 31 | Le panneau de départ s'appelle « Depart_Sign » : le clic de départ n'était pas vu | départ validé |
| 9 | 11 | 32 | Fin de l'aventure de l'hôte : l'invité est ramené chez lui, même en plein combat | validée (abandon) |
| 10 | 11 | 32 | Plus d'Échap simulé après « Quitter vers le menu » | |
| 11 | 11 | 33 | Monde partagé : PNJ, histoire et déblocages de l'hôte dans la save de l'invité (`MSG_WORLD`, sqlite ajouté) | validée |
| 12 | 12 | 34 | PNJ en direct : la faveur donnée par l'hôte arrive tout de suite (`MSG_NPC`) | |
| 13 | 12 | 35 | Équipe mixte : chacun emmène ses chats, les dirige et les retrouve chez lui (`MSG_TEAM`) | départ et retour validés |
| 14 | 12 | 36 | Fin de journée de l'hôte rétablie ; un chat d'invité ne rentre que si le jeu le ramène | |
| 15 | 12 | 37 | Départ ensemble : l'invité voit colliers et objets du départ avec l'hôte | testée |
| 16 | 13 | 38 | Colliers choisis ensemble : chacun pour ses chats, en direct, l'hôte tranche | testée |
| 17 | 13 | 39 | Objets du départ : chacun équipe ses chats avec son propre rangement | testée (rangement de l'invité vide) ; combat de 30 tours sans désynchronisation |
| 18 | 15 | 40 | Le chemin se vote (`MM_VOTE`) : majorité, 30 s, égalité tirée par l'hôte | |
| 19 | 15 | 41 | Corrections (visée, Némésis écartée des aventures à plusieurs, retour anticipé, fausses alertes) ; enregistreur des tirages | |
| 20 | 16 | 42 | Objets en aventure : sac commun, chacun n'équipe que ses chats ; texte des messages en octets | |
| 21 | 16 | 43 | Chacun sa bourse (`MM_PURSE`) | |
| 22 | 17 | 44 | Montées de niveau au propriétaire du chat (option, relance, capacité) ; mouvements d'objets en direct | |
| 23 | 17 | 45 | Événements au propriétaire du chat ; objets en direct sac ouvert | jouée : montées de niveau et événements marchent |
| 24 | 17 | 46 | Objets reconnus d'un joueur à l'autre ; « À X de choisir » sous le texte d'un événement | |
| 25 | 17 | 46 | La ligne « À X de choisir » ne double jamais | jouée : deux problèmes, corrigés par la 27 |
| 26 | 17 | 47 | Le défilé du butin à la victoire (Besoin, Cupidité, Passer ; dés de l'hôte) | |
| 27 | 17 | 48 | Les combats n'attendent plus un absent ; chez un invité, « Abandonner » devient « Quitter l'aventure » | livrée avec la 29 |
| 28 | 17 | 49 | Recrues au spectateur (ou au joueur qui a le moins de chats) ; combats qui tiennent quand un joueur part | livrée avec la 29 |
| 29 | 17 | 50 | Dons de l'invité aux PNJ ; décisions de combat jamais jouées deux fois (auteur et rang) | livrée |
| 30 | 17 | 51 | Achats en boutique avec sa bourse, faits chez l'hôte | |
| 31 | 17 | 51 | Boutique : bonbon donné au chat d'un invité, « Sortir » retenu | livrée |
| 32–34 | 17 | 52–53 | L'IA du jeu joue les chats d'un absent (chez l'hôte, décision envoyée à tous) ; 33–34 : relectures (chat charmé, pouvoir hors des cases) | 34 livrée |
| 35–36 | 17 | 54–55 | « Défendre ! » en session : chacun défend sa maison, seul ; 36 : défense quittée par le menu | |
| 37 | 18 | 56 | Deux globales du jeu enfin trouvées : récompense du don d'un invité au donneur, noms des objets au défilé | |
| 38 | 18 | 57 | La Némésis à plusieurs | |
| 39 | 18 | 58 | Revenir en plein combat (état d'entrée de la case) ; 4e place à 3 joueurs ; rejoindre par Steam ; heure dans le journal | |
| 40 | 18 | 59 | Vérification globale : 4 problèmes corrigés (Rejoindre pendant un événement, empreinte des cases à 3, invité en retard, file de réception) | |
| 41 | 18 | 60 | Le départ sans second chargement : graine commune, l'invité garde sa carte | « vérifié le 18 octobre » (document) |
| 42–45 | 18 | 61 | Seconde vérification globale : un joueur refusé ne bloque plus le salon et voit la raison, un combat arrêté ne fige plus les suivants, un invité coupé ne joue plus seul, le départ n'attend plus un invité parti | 42–44 jamais installées ; 45 livrée |
| 46–49 | 18 | 62–63 | Échanges entre joueurs (« Échanger », don direct : à la maison pièces, nourriture et objets ; en aventure, de bourse à bourse) ; 47–48 : relectures ; 49 : un départ sans état final ne bloque plus les objets | 46 jamais livrée ; 49 livrée et jouée le 18 : gel en combat |
| 50–52 | 18 | 64–66 | L'aperçu de visée ne tourne plus le chat ; un combat fini d'un seul côté ne fige plus personne ; 51–52 : relectures | 52 livrée le 18, pas encore jouée |

## 7. Le dernier bug corrigé : le gel du 18 octobre (builds 50–52)

- **Le cas.** Partie du 18 octobre, build 49 (journaux `mgmp_trace_20261018-192336`
  et `-192345`). Combat `e15743b8b1852f54` : chez l'hôte, il se finit pendant le tour
  20 ; chez l'invité, il continue, le tour 21 commence (un chat de l'hôte) et sa
  décision ne vient jamais. Hizu croyait à un bug des échanges : ils n'y étaient
  pour rien.
- **La cause.** Au tour 16, Katara finit son tour tournée vers l'ouest chez l'hôte et
  vers le sud chez l'invité. En visant, le jeu tourne le chat vers la cible
  (`Character::Face`, appelé par l'aperçu de décision en `0x1401381E9`) avec une
  animation de demi-tour qui écrit l'orientation après coup, et seulement chez celui
  qui vise : les autres ne calculent pas d'aperçu pour un chat humain qu'ils ne
  dirigent pas. L'orientation n'est pas dans l'empreinte, l'écart est donc
  invisible ; au tour 20, l'IA d'un ennemi, qui la lit, choisit autrement. 5 parties
  à deux sur 10 dans les journaux avaient de tels écarts.
- **Le correctif (build 50, protocole 64).** En combat commun, cet appel de `Face`
  par l'aperçu est empêché (`h_CharFace` dans `mgmp_hooks.cpp`,
  `lockstep_face_blocked` dans `mgmp_lockstep.cpp`), comme tout `Face` pendant une
  clôture d'état : le chat se tourne quand l'action part, pareil chez tous.
- **Le filet de secours (builds 50–52, `mgmp_orphan.h`, `MM_BATTLE_END`).** Si deux
  copies d'un combat ne finissent quand même pas ensemble : un invité figé après la
  fin de l'hôte rentre chez lui avec un message (règle 1) ; un hôte qui attend la
  décision d'un invité dont le combat est fini le retire du combat et l'IA du jeu
  joue ses chats (règle 2) ; cet invité, voyant l'hôte continuer, rentre chez lui
  (règle 3). Le détail, avec les délais (2 s, 10 s au tour même) : en tête de
  `src/session/mgmp_orphan.h` et dans `docs/NOTES_MGMP.md`, « Protocols 64–66 ».
  Tests : `tests/test_orphan.cpp`.
- **À regarder dans les journaux de la prochaine partie :**
  - au démarrage :
    `[+] CHARFACE  l'apercu de visee appelle Face depuis Mewgenics.exe+0x1381E9` ;
  - en combat, une fois : `l'apercu de visee allait tourner un chat (…) -- appel de
    Character::Face empeche` ;
  - à la fin : `Character::Face : N appel(s) de l'apercu de visee et M pendant une
    cloture d'etat empeche(s)` ;
  - et plus jamais `!! combat … : l'hote l'a fini`, `!! combat … : ce joueur l'a
    fini pendant son tour` (chez un invité) ni `!! combat … : le joueur N l'a fini
    chez lui` (chez l'hôte). Si l'une revient, le filet a servi : une autre
    divergence existe, à chercher avec la méthode de la section 9. La variante
    `… ce joueur l'a fini a son tour … rien n'attend` signale une divergence sur
    laquelle le filet n'a pas eu à agir.

## 8. Ce qui reste

### À voir marcher en partie

Dans l'ordre : la **build 52** (des combats entiers à deux, sans gel). Puis tout ce
qui n'a encore aucun retour de partie (le document de conception en marque la
plupart « à tester ») : le vote du chemin (18), le nom de celui qui décide pendant
un événement (24), les bourses (21), le défilé du butin (26), la boutique (30), les
recrues (28), l'IA des absents (32–34), revenir en plein combat (39), « Défendre ! »
(35–36), la Némésis (38), la faveur en direct (12), les dons de l'invité aux PNJ
(29), la 4e place et le jeu à 3 ou 4 (39, jamais joué à plus de deux), Steam (39),
les échanges (49) ; et, toujours « à tester » dans le document : la fin normale
d'une aventure, victoire ou défaite (9), un invité qui équipe ses chats de son
rangement au départ (17), les objets en aventure et reconnus d'un joueur à l'autre
(20, 22, 24–25), les builds 19 et 40, la nuit votée à distance, et une aventure
complète à deux.

### Questions et propositions sans réponse

- Dans le document : « La save multi peut-elle se jouer si un joueur manque ?
  Proposition actuelle : non. »
- Le document dit encore, au départ, que « les classes premier servi restent à
  faire », alors que depuis la build 16 un collier « n'existe qu'une fois : le
  premier qui le prend l'a, l'hôte tranche » (message du commit `a4d766e`) : à
  faire confirmer par Hizu et à harmoniser.
- Les bonbons et les familiers achetés en boutique restent à l'hôte (build 30) ;
  j'avais proposé de laisser un invité acheter des bonbons : pas de réponse.
- Une question posée à Hizu sur les boutons « Oui » / « Non » de la confirmation du
  retour anticipé à la maison est restée sans réponse : la lui reposer.

### Limites connues (voulues ou acceptées)

- Un invité plus avancé que l'hôte chez un PNJ reprend quand même le niveau de
  l'hôte (build 11).
- Une récompense de PNJ prête reste à l'hôte ; ce qu'elle débloque arrive chez
  l'invité à son prochain chargement (build 12).
- Les boîtes de meubles restent à l'hôte ; les bourses ne survivent pas à un
  redémarrage de l'hôte en pleine aventure (build 21).
- Un objet que tout le monde passe au défilé reste à l'hôte (décidé le 17 octobre).
- Une recrue dont le joueur est parti avant la fin reste chez l'hôte.
- Un joueur qui revient en plein combat le regarde, et reprend ses chats au combat
  suivant.

### Risques connus

- Une désynchronisation vue une seule fois, le 12 octobre (tour 9, un échange de
  place pendant une attaque ennemie), n'a jamais trouvé sa cause. L'enregistreur de
  la build 19 garde les tirages de chaque tour de combat commun et les écrit si les
  flux se séparent : si elle revient, la réponse est dans les deux journaux.
- L'orientation n'est toujours pas dans l'empreinte : une écriture d'orientation
  différée par une autre animation pourrait encore diverger sans être vue (risque
  résiduel décrit dans `docs/NOTES_MGMP.md`, « Protocols 64–66 »). Le filet de
  secours évite le gel, pas la divergence.
- Une mise à jour du jeu par Steam change le binaire : des signatures ne se
  retrouvent plus (`[!]` au démarrage) et les crochets correspondants ne sont pas
  posés. La procédure mesurée le 12 septembre est dans `docs/NOTES_MGMP.md`
  (« Surviving an update »).

## 9. Déboguer une partie

- **Les journaux.** Un par lancement du jeu, dans le dossier du mod
  (`mgmp_trace_AAAAMMJJ-HHMMSS.log`) ; ceux de l'hôte et de l'invité d'une même partie
  ont des noms à quelques secondes d'écart. Ceux de l'ami sont dans son propre
  dossier `mgmp` : les lui demander. Un journal se lit sur une copie.
- **Le format.** En tête : la build (`mod    : Mewgenics Multi, build N`), la
  résolution des adresses (125 dans la build 52, toutes à l'adresse attendue),
  chaque crochet (`[+]` / `[!]`). Puis une ligne par événement : numéro de
  séquence, compteur de tours du processus (jamais remis à zéro ; ce n'est pas
  `g.turn`, qui est dans le texte, par exemple `turn N hash …`), secondes depuis la
  première ligne, étiquette (`LOCKSTEP`, `NODEHASH`,
  `TEAM`, `MULTI`, `CHOICE`, `SAC`, `BUTIN`, `PURSE`, `SHOP`, `PNJ`, `DON`, `VOTE`,
  `EMBARK`, `FOLLOW`, `SAVEFILE`, `WORLD`, `NET`, `STEAM`…), texte. `!!` marque une
  anomalie ; `->` et `<-` les messages envoyés et reçus.
- **La méthode (celle du 18 octobre).** Retrouver le combat (`battle id now …`) et
  ses membres ; aligner les empreintes de tour des deux côtés ; trouver le dernier
  tour comparé et le premier où quelque chose diffère (une décision, une
  orientation, un tirage) ; puis lire le binaire pour comprendre qui écrit la valeur
  qui diffère. Avec `debug.desync_halt` (vrai par défaut), un écart d'empreinte
  arrête le combat et chaque journal écrit `STATE DIFF at turn N` : les champs qui
  diffèrent, chat par chat (un seul journal suffit pour les nommer).
- **Les outils de `tools/`** : `gen_sig.py` et `verify_sigs.py` (signatures),
  `analyze_trace.py` (les `TurnAction` d'un journal), `decode_record.py` (un
  enregistrement `debug.record`), `gpak.py` (lire `resources.gpak` ; ne jamais
  mettre ce qu'il en sort dans le dépôt), `save_snapshot.ps1` et `net_test.ps1`
  (de mgmp).

## 10. Pièges connus

- **Steam doit être ouvert avant le .bat.** Sinon le jeu se ferme pendant son
  démarrage, avant le mod (code 3221226505, `0xC0000409`), et `loader.txt` parle à
  tort d'une relance par Steam et de `steam_appid.txt` : c'est arrivé le 20 octobre,
  Steam était fermé. Depuis le 22 octobre, le .bat vérifie que `steam.exe` tourne ;
  sinon il le dit et attend une touche avant de lancer. Le loader lancé seul parle
  encore de `steam_appid.txt` pour un code de sortie non nul
  (`loader/mgmp_loader.cpp`).
- `src/net/mgmp_proto.h`, ligne 54 : 24 Ko sur une ligne. Ne jamais l'afficher en
  entier ; une modification se fait par un remplacement exact.
- La DLL partage des `std::string` et `std::function` avec le jeu : MSVC x64
  seulement (`mm_menu.cpp` ligne 84). Le nom `mgmp_coop.sav` tient dans le petit
  tampon de `std::string` (15 caractères au plus) exprès : ne pas l'allonger.
- `g.turn` est le tour **en cours** : l'empreinte du tour T part à sa fin, puis T+1
  commence. Deux copies d'accord finissent un combat avec le même `g.turn`.
- Le numéro d'un combat est la graine de sa case : rejoué après un rechargement,
  c'est le même. Tout état gardé par combat doit être oublié à chaque nouvelle photo
  du combat (voir `ends_forget_all` dans `mgmp_lockstep.cpp`).
- La fin d'une case (`T_NodeEnd`) vient au retour sur la carte, avant l'écran des
  montées de niveau ; « Espace » sur un pion au repos en appelle une autre
  (`follow_node_end_counts` les trie).
- L'aperçu de décision d'un chat humain n'est calculé que chez son propriétaire
  (chez les autres, le mod répond « rien » à `GetChoice`) ; celui des ennemis l'est
  chez tous, au rythme de l'horloge. Le chat d'un absent est décidé par l'IA du jeu
  chez l'hôte seul (`mm_ai`), dans une clôture d'état où `Face` est empêché. Tout effet de bord d'un
  aperçu est une divergence en puissance.
- Les saves sont des bases sqlite, que le jeu peut tenir ouvertes (`-wal`,
  `-journal`) : ne les lire que sur des copies. Le mod écrit dans une save (le monde
  de l'hôte, le retour des chats) après une copie de sécurité dans
  `mgmp_backups\` ; garder cette règle pour toute nouvelle écriture.
- Ne jamais mettre dans le dépôt d'image, de son, de police ni de donnée du jeu
  (`CLAUDE.md`).
