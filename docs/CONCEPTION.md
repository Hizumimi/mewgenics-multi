# Mewgenics Multi — Document de conception

> Copie du document de conception en ligne
> (https://claude.ai/code/artifact/5e9195c8-3a1e-446a-a5a6-a43fea7459f7, révision 114,
> prise le 22 octobre 2026), pour qu'il se lise sans claude.ai. Ses trois schémas
> n'y sont pas. Le document en ligne est celui que Hizu lit : une règle qui change
> se note dans les deux. Ce qui a été fait build par build : `docs/PASSATION.md`.

Oct 11, 2026 · @Hizu

## Vision

Mewgenics Multi est un mod coop de 2 à 4 joueurs : chacun garde sa maison et ses chats, mais l'histoire, les PNJ et les aventures se vivent ensemble.

- Chaque joueur gère sa maison, ses chats et son élevage comme en solo.
- La progression du monde est commune : PNJ, histoire et déblocages avancent pour tout le monde.
- Les aventures se font ensemble, avec 4 chats au plus dans l'équipe, pour garder l'équilibrage du jeu.
- Le combat reste au tour par tour, et chaque joueur commande ses propres chats.

*(Schéma du document en ligne, non repris dans cette copie : boucle d'une journée · 5 étapes.)*

La nuit ferme la boucle : elle ne passe que quand tous les joueurs ont validé.

Les points marqués *(proposition)* sont des suggestions pour les cas pas encore tranchés, à valider ou à changer.

## Session et save multi

Chacun joue dans sa propre save, et tout le monde avance ensemble dans l'histoire. L'hôte choisit sa save en hébergeant : un emplacement vide lance une nouvelle partie, un emplacement plein reprend une partie. Celui qui rejoint choisit aussi son emplacement avant de rejoindre, vide ou déjà utilisé. Les saves se font souvent pendant la partie.

- Chaque save contient la maison du joueur et une copie de l'état partagé : PNJ, histoire, déblocages, jour en cours.
- Le joueur qui crée la partie est l'hôte, et sa copie de l'état partagé sert de référence. *(proposition)*
- Au lancement, les copies sont comparées ; si elles diffèrent, celle de l'hôte est reprise par tous. *(proposition)*
- La save multi ne se joue que quand tous les joueurs sont connectés, sinon l'état partagé divergerait. *(proposition)*
- À la connexion, le mod vérifie que tout le monde a la même version du jeu et du mod.
- Un petit fichier à côté de la save garde les infos de session : identifiant de partie, joueurs, hôte, version.

## Maisons et nuits

La nuit ne passe que si tous les joueurs sont d'accord ; tout le reste de la vie à la maison se joue en local.

| Élément | Où ça vit |
| --- | --- |
| Maison, meubles, chats, élevage | Local, chez chaque joueur |
| Jour en cours | Partagé |
| Passage de la nuit | Vote unanime |

Déroulement de la nuit :

1. Un joueur clique sur « finir la journée » : au lieu de lancer la nuit, le mod le marque « prêt ».
2. Tous les joueurs voient qui est prêt et qui ne l'est pas encore.
3. Un joueur prêt peut annuler tant que tout le monde n'a pas validé. *(proposition)*
4. Quand tous sont prêts, l'hôte donne le signal et chaque jeu fait sa nuit chez lui (accouplements, naissances).

L'élevage reste local : il n'a pas besoin d'être synchronisé, puisque les autres joueurs ne voient pas cette maison.

## PNJ et progression partagée

Toute action qui fait avancer un PNJ est envoyée aux autres joueurs : un chat donné à un PNJ apparaît chez ce PNJ pour tout le monde.

- Le chat donné quitte la maison de son propriétaire, comme en solo.
- La progression du PNJ (compteur, étapes, dialogues débloqués) est appliquée chez tous les joueurs.
- Les déblocages (classes, zones, histoire) sont communs à tous.
- Une récompense qui débloque quelque chose profite à tous ; un objet ou de l'or reçu en récompense va au joueur qui a fait le don* (décidé le 17 octobre, build 29).*

Trouvé dans le jeu : chaque PNJ a une jauge de faveur. On lui envoie des chats par le tuyau de la maison, et à certains paliers il donne une récompense : Frank agrandit la maison (grenier, pièces), Butch le rangement, Jack, Tracy et l'orgue améliorent leur boutique, Tink débloque des infos sur les chats et donne des objets, Beanies paie des quêtes en pièces. La save garde tout ça dans un bloc « npc\_progress », mêlé à l'histoire (intros des PNJ, classes, boss et zones débloqués). Build 11 : le monde est celui de l'hôte. Avant chaque chargement, la save de l'invité reprend la progression des PNJ de l'hôte telle quelle, et ses déblocages et agrandissements de maison en plus des siens. Chats, meubles, or, inventaire et jour restent ceux de l'invité. Limite connue : un invité plus avancé chez un PNJ reprend quand même le niveau de l'hôte. Build 12 : la faveur donnée par l'hôte apparaît tout de suite chez l'invité. Une récompense prête reste à l'hôte, et ce qu'elle débloque arrive chez l'invité à son prochain chargement. Build 29 : l'invité donne ses chats par le tuyau de sa maison, et leur faveur compte chez l'hôte. Si son don remplit la jauge, l'hôte prend la récompense pour lui dès qu'il est dans sa maison : les pièces, la nourriture et les objets partent chez l'invité, les déblocages restent à tous. Le PNJ passe aussi à sa récompense suivante chez l'invité, sans attendre son prochain chargement.

## Départ en aventure : la boîte et les classes

L'équipe a toujours 4 places, réparties selon le nombre de joueurs ; à 3, les joueurs décident entre eux qui prend la place libre.

| Joueurs | Chats par joueur | Place libre |
| --- | --- | --- |
| 2 | 2 chacun | Aucune |
| 3 | 1 chacun | 1 place pour un volontaire ; sans volontaire, départ à 3 chats |
| 4 | 1 chacun | Aucune |

Déroulement :

1. Un joueur propose de partir, et la boîte de départ s'ouvre chez tout le monde.
2. Chacun place ses chats depuis sa propre maison.
3. À 3 joueurs, le premier qui place un deuxième chat dans sa boîte de départ prend la place libre ; sans volontaire, l'équipe part à 3 chats. *(build 39, à tester)*
4. Chacun choisit la classe de ses chats : premier choisi, premier servi. Une classe prise devient grisée pour les autres.
5. Le départ se lance quand tous les joueurs ont cliqué « prêt ».

Build 13 (à tester) : chacun remplit sa boîte de départ chez lui, puis l'hôte clique « À l'aventure ! » et chaque invité répond « J'arrive ». À 3 joueurs, la place libre revenait alors à l'hôte (depuis la build 39, elle va au premier qui place un deuxième chat) ; les classes premier servi restent à faire. Si aucun chat ne peut partir, personne ne part.

Chaque chat placé est envoyé aux autres joueurs, qui en reçoivent une copie temporaire pour la durée de l'aventure : gènes, stats, mutations, capacités, objets.

## Votes en aventure

Le chemin sur la carte est choisi par un vote à la majorité.

- Chaque joueur vote pour une destination, et la majorité l'emporte.
- En cas d'égalité, un tirage au sort départage les options à égalité.
- Le premier vote lance un délai de 30 secondes, qui n'avance que quand tout le monde est sur la carte ; un joueur qui n'a pas voté à la fin compte comme abstention.
- Chacun peut changer son vote tant que tout le monde n'a pas voté.
- Le choix d'un événement est fait par le propriétaire du chat de l'événement, à chaque étape ; l'hôte choisit pour un événement sans chat, sur une recrue ou sur un de ses chats. Quand un événement demande quel chat y va, chacun ne peut envoyer que les siens, et le premier envoyé y va.
- La montée de niveau d'un chat est choisie par son propriétaire : l'option, la relance et la capacité à remplacer ; chez les autres, l'écran dit « À X de choisir ».
- Chaque joueur voit en direct les votes des autres.

## Combat

Chaque joueur commande uniquement ses propres chats ; les ennemis et tout le reste du combat sont simulés à l'identique sur chaque PC.

- Les combats du jeu sont déterministes : avec le même départ, ils se déroulent à l'identique sur chaque PC. L'auteur de mgmp l'a mesuré.
- Seules les actions des joueurs passent par le réseau, et chaque PC en calcule lui-même les conséquences.
- Chaque chat a son propre tour : son propriétaire le joue, les autres regardent.
- Les chats des autres joueurs sont visibles mais pas contrôlables.

Déconnexion d'un joueur :

- L'IA du jeu prend le contrôle de ses chats jusqu'à son retour. C'est fait dans les builds 32 à 34 (à tester) : l'IA tourne chez l'hôte, et sa décision part chez tous comme celle d'un joueur ; un combat n'attend plus un joueur absent.
- C'est l'IA que le jeu fait jouer à un chat charmé et au passif « Uncontrollable » (objet Brain Chip, Déjà Vu sévère).
- Le Déjà Vu de Steven, qui sanctionne les sorties forcées en pleine partie, est neutralisé pour une coupure réseau.
- À son retour, le joueur regarde le combat en cours, rattrapé jusqu'au tour en cours, et reprend ses chats au combat suivant (build 27).

## Mort d'un chat, mode spectateur et nouveaux chats

Un chat mort est perdu pour son propriétaire ; un joueur sans chat regarde la partie jusqu'à ce qu'un nouveau chat lui soit attribué.

- Un joueur qui n'a plus de chat vivant passe en mode spectateur : il voit tout mais ne commande rien.
- Un nouveau chat qui rejoint l'équipe en cours d'aventure est attribué automatiquement à un spectateur.
- Plusieurs spectateurs : le chat va à celui qui est sans chat depuis le plus longtemps* (build 28).*
- Aucun spectateur : le chat va au joueur qui a le moins de chats ; en cas d'égalité, tirage au sort* (build 28).*
- À la fin de l'aventure, chaque chat survivant rentre chez son propriétaire avec son état final.
- Un chat recruté en cours de route rentre chez le joueur à qui il a été attribué (build 28) ; si ce joueur n'est plus connecté à la fin, il reste chez l'hôte.

## Butin et or

Un objet équipé rentre avec son chat ; les autres objets passent un par un dans un défilé où chacun choisit Passer, Cupidité ou Besoin.

Le défilé, à la fin de l'aventure :

1. Tous les joueurs voient le même objet en même temps.
2. Chacun choisit **Passer** (je n'en veux pas), **Cupidité** (je le prends si personne n'en a besoin) ou **Besoin** (je le veux : jet de dé de 100).
3. Le résultat tombe quand tout le monde a choisi, puis on passe à l'objet suivant.

*(Schéma du document en ligne, non repris dans cette copie : attribution d'un objet · 2 décisions.)*

Un joueur qui a choisi Passer ne participe à aucun lancer.

- Égalité au dé : relance entre les ex æquo ; tout le monde voit le premier lancer (build 26).
- Les dés sont lancés par l'hôte, et tout le monde voit les résultats (build 26).
- Sans choix à la fin des 30 secondes, le joueur passe ; un objet que personne ne veut reste à l'hôte (build 26).

**Défilé** (build 26) : à la victoire d'une aventure mixte, les objets du sac rangés chez l'hôte défilent chez chaque joueur qui a amené des chats, 30 secondes par objet (« Je le veux » puis Besoin ou Cupidité, ou Passer), puis le résultat 4 secondes. Le premier objet attend que chacun ait sa fenêtre ouverte (3 minutes au plus, l'hôte peut commencer avant) ; ensuite, un joueur sans fenêtre passe son tour jusqu'à ce qu'elle s'ouvre, et quand l'hôte ne voit pas le défilé, le temps s'arrête pour tous. Un objet gagné par un invité quitte le rangement de l'hôte quand l'invité l'a pris, et rejoint le rangement de sa save à son prochain chargement, avec ses chats ; en attendant, il est gardé dans un fichier à côté de la save, avec sa bourse.

**Or** (build 21) : chaque joueur qui a amené des chats a sa bourse, que l'hôte tient à côté du compte de l'aventure ; le compte du jeu, lui, ne change pas. Ce qu'un chat trouve dans un événement ou ramasse en combat, et le prix d'un choix d'événement pour lui, vont à la bourse de son propriétaire. En boutique, chacun paie avec sa bourse (build 30). Le reste (provisions, récompenses des combats) est partagé à parts égales, et une bourse peut passer sous zéro. À la victoire, chaque invité encore connecté rapporte sa bourse dans sa maison et l'hôte le reste ; un joueur parti ne touche rien. La fenêtre du vote du chemin montre les trouvailles et la bourse de chacun.

En boutique pendant l'aventure, chacun paie avec sa bourse, et l'objet acheté va à celui qui paie. Avantage : pas de dispute autour d'une caisse commune, et tout le monde profite des combats gagnés ensemble.

## Échanges entre joueurs

Chacun peut donner à un autre joueur par un bouton « Échanger » ; celui qui reçoit n'a rien à accepter (décidé le 18 octobre, build 49).

- **À la maison** : des pièces, de la nourriture et des objets du rangement (16 au plus par don), à un joueur qui est lui aussi dans sa propre maison. Le don arrive tout de suite chez lui, et les deux saves sont enregistrées.
- **Rien ne se perd** : le don ne quitte la maison du donneur qu'une fois gardé par l'autre. S'il n'est pas dans sa maison, garde l'écran des objets ouvert plus de 20 secondes, part ou ne répond pas, le donneur garde tout et la fenêtre dit pourquoi.
- **En aventure à plusieurs**, sur la carte : des pièces et de la nourriture de sa bourse, à la bourse d'un autre joueur. Le compte de l'équipe ne change pas ; les objets sont déjà dans le sac commun.

## Architecture technique

Le mod est une DLL injectée dans Mewgenics.exe et bâtie sur mgmp : en combat, chaque PC calcule tout lui-même ; hors combat, l'hôte décide et envoie le résultat.

*(Schéma du document en ligne, non repris dans cette copie : architecture · hôte et joueurs, 3 couches chacun.)*

Les autres joueurs envoient leurs actions et leurs votes à l'hôte, qui renvoie à tous les résultats validés.

- **Chargement** : un lanceur démarre Mewgenics et y injecte la DLL, comme le mgmp\_loader.exe de mgmp. Le jeu n'est pas modifié sur le disque.
- **Hooks** : interceptions des fonctions du jeu avec MinHook ; mgmp couvre déjà le combat, la carte, les boutiques et les événements.
- **Réseau** : connexion TCP directe vers l'hôte (port 27600 dans mgmp) ; ou, depuis la build 39, par Steam : « Rejoindre la partie » dans la liste d'amis, sans adresse ni port (à tester).
- **Contrôle** : à chaque tour de combat et à chaque case de la carte, les PC comparent une empreinte de leur état. En cas d'écart, le jeu s'arrête et le journal indique la différence.
- **Transfert de chat** : un chat est converti en paquet de données (gènes, stats, mutations, capacités, objets), envoyé, puis recréé chez les autres.
- **Interface** : les fenêtres du jeu lui-même (sa boîte de confirmation, avec sa police et notre texte en Unicode) pour le salon, les votes, la boîte de départ et le défilé du butin ; leur texte se met à jour en direct. Le panneau technique de mgmp reste sur F1 pour le débogage.

Messages réseau à ajouter à ceux de mgmp :

| Message | Envoyé par | Rôle |
| --- | --- | --- |
| Prêt pour la nuit | Joueur | Valider ou annuler la fin de journée |
| Nuit | Hôte | Lancer la nuit chez tous |
| Événement PNJ | Joueur | Appliquer un don ou une progression chez tous |
| Monde | Hôte | Sa save entière, d'où chaque invité reprend les PNJ, l'histoire et les déblocages (build 11) |
| PNJ en direct | Hôte | L'état de ses PNJ (faveur, récompense visée) à chaque changement (build 12) |
| Équipe | Invité, puis hôte | Les chats d'un invité au départ, à qui est chaque chat, puis leur état final au retour (build 13) |
| Objets en aventure | Joueur, puis hôte | Chaque mouvement d'objet (équiper, rendre au sac, lire un livre), la réponse de l'hôte, et son état sur demande (build 22) |
| Choix d'un chat | Invité, puis hôte | L'option, la relance ou la capacité choisie par le propriétaire d'un chat (build 22), l'option d'un événement sur son chat ou le chat qu'il envoie dans un événement (build 23) : l'hôte la rejoue et la publie à tous |
| Place ou classe | Joueur | Réserver la place libre ou une classe |
| Vote | Joueur | Voter pour un chemin |
| Choix de butin | Joueur | Passer, Cupidité ou Besoin |
| Résultat de butin | Hôte | Gagnant et lancers de dés |
| Bourses | Hôte | Les bourses, chaque trouvaille, puis ce que chaque invité rapporte à la fin (build 21) |
| Attribution | Hôte | Nouveau chat donné à un joueur, chats confiés à l'IA |
| Don | Joueur, relayé par l'hôte | Un don à la maison, sa réponse et qui peut recevoir ; en aventure, un don de bourse vérifié par l'hôte (build 49) |

## Plan de développement

Le plus rapide est de partir de mgmp, un mod coop open source qui synchronise déjà les combats à 2, puis d'y ajouter les maisons séparées et les règles de ce document. Tout se construit d'abord à 2 joueurs.

1. **Partir de mgmp** : en grande partie fait. mgmp tourne sur la version installée, à condition d'ajouter steam\_appid.txt (686060) à côté de Mewgenics.exe. Le projet est copié dans le dépôt GitHub privé `mewgenics-multi`, qui le compile automatiquement à chaque modification ; la build 1 est installée depuis le 11 octobre.
   - Reste : une aventure complète à 2 avec ton ami.
2. **Interface dans le jeu** : fait (builds 2 à 4). Le bouton Miauler devient Multijoueur : Héberger ou Rejoindre, salon avec l'adresse et la liste des joueurs en direct, saisie de l'adresse au clavier. Tout passe par les fenêtres du jeu, avec sa police ; le panneau technique reste sur F1.
   - Terminé quand : validé en jeu le 11 octobre (build 4).
3. **Reconnaissance complémentaire** : ce que mgmp ne couvre pas, à savoir la fin de journée, le don aux PNJ, la boîte de départ, le choix des classes et l'écriture de la save de la maison. Piste pour la nuit : la fonction House::EndDay, qui n'est appelée qu'à un seul endroit.
   - Terminé quand : chaque fonction est trouvée et testée avec un hook qui écrit dans le journal.
4. **Nuit votée** : validée le 11 octobre avec la build 5 (deux jeux sur le même PC), reste à essayer à distance. Le clic sur le panneau de fin de journée (bouton EndDay\_Sign) est retenu jusqu'à ce que tout le monde soit prêt, puis rejoué chez chacun. Build 6 (validée le même jour) : les joueurs pas encore prêts voient « 1 joueur attend » sur leur panneau et une fenêtre avec « Passer la nuit » ; le panneau en mode « À l'aventure ! » n'est plus retenu.
   - Terminé quand : la nuit ne passe chez les 2 joueurs que si les 2 ont validé.
5. **Save multi et PNJ partagés** : chacun sa save codé dans la build 7 ; la build 8 corrige le départ (validé : le départ se vote et l'invité arrive dans l'aventure de l'hôte) ; la build 9 ramène l'invité chez lui quand l'aventure de l'hôte finit, même par un abandon en plein combat (validé ; la fin normale d'une aventure, victoire ou défaite, reste à tester). « Défendre ! » se joue en session depuis les builds 35 et 36 : chacun défend sa maison, seul (à tester). L'invité choisit son emplacement, le départ « À l'aventure ! » se vote, l'invité bascule tout seul dans l'aventure de l'hôte (avec ses propres chats depuis la build 13) puis rentre dans sa propre save. Build 11 : le monde partagé (validé : l'invité attend le choix de l'hôte, reprend ses PNJ et ses pièces, et le tuyau refuse ses chats). Build 12 : la faveur donnée par l'hôte en direct, à tester. Build 29 : les dons faits par l'invité, à tester.
   - Terminé quand : un don chez un joueur apparaît chez l'autre, et le reste après relance du jeu.
6. **Équipe mixte** : codée dans la build 13, testée le 12 octobre (départ et retour validés) ; la build 14 corrige la fin de journée de l'hôte et le retour après une aventure abandonnée. Chacun met ses chats dans sa boîte de départ (2 chacun à deux joueurs, 1 chacun à trois ou quatre, la place libre à l'hôte pour l'instant). Quand l'invité répond « J'arrive », ses chats partent avec lui dans l'aventure de l'hôte et il dirige les siens en combat ; à la fin, chaque chat rentre chez son propriétaire avec son état final, après une copie de sécurité de sa save : il rentre si le jeu le ramène à la maison, comme ceux de l'hôte, et personne ne revient d'une aventure abandonnée ou perdue. Une recrue va à un spectateur depuis la build 28 (voir « Mort d'un chat, mode spectateur et nouveaux chats »).
   - Terminé quand : les chats des 2 joueurs partent dans la même équipe, sur les 2 PC.
7. **Classes premier servi, au départ ensemble**.
   - Trouvé dans le jeu : après « À l'aventure ! » viennent l'écran des colliers de classe (un collier pris disparaît de la liste), puis celui des objets du rangement (une fois le désert débloqué). Le choix des colliers ne se reprend pas depuis une save : l'invité doit faire le départ lui-même.
   - Étape 1, build 15, testée : l'invité charge la partie de l'hôte après « J'arrive », part avec lui et voit les deux écrans. Les clics de l'hôte y sont rejoués, mais les colliers qu'il pose n'apparaissaient pas chez l'invité.
   - Étape 2, build 16, testée : chacun fait défiler les chats comme il veut. Seul le propriétaire d'un chat lui pose ou retire un collier ; sur le chat d'un autre, les colliers sont grisés. Ce que posent les autres apparaît tout de suite, et un collier pris n'est plus libre (l'hôte tranche). Seul « Valider » de l'hôte ferme l'écran pour tous.
   - Étape 3, build 17, testée le 13 octobre (le rangement de l'invité était vide : équiper ses chats reste à essayer) : au choix des objets, l'invité retrouve son propre rangement et équipe ses chats avec, comme chez lui. Chacun ne touche qu'à ses chats : les objets d'un chat qui n'est pas à lui sont grisés, et seul « Valider » de l'hôte ferme l'écran pour tous. Sur la carte, l'hôte reprend l'équipement des chats de l'invité avant de publier l'aventure ; les objets pris quittent le rangement de l'invité à son retour, avec ses chats. On ne voit pas en direct les objets choisis par les autres, et le butin de l'aventure va toujours dans le rangement de l'hôte.
8. **Combat avec des chats de maisons différentes**, en gardant la synchronisation de mgmp : validé le 13 octobre, un combat de 30 tours avec 2 chats chacun, sans désynchronisation.
   - Terminé quand : un combat entier se joue sans désynchronisation.
9. **Votes** : chemin, puis événements. Dans mgmp, l'hôte décide seul hors combat.
   - Chemin, build 18, à tester : un clic sur une case est un vote. La fenêtre du jeu montre en direct qui a voté quoi et le temps restant (« Même chemin », « Choisir », « Changer »). Le premier vote lance 30 secondes, qui n'avancent que quand tout le monde est sur la carte ; une égalité est tirée au sort par l'hôte. Le résultat s'affiche 2,5 secondes, puis le pion de l'hôte part et les invités suivent. Le pion d'un invité arrive aussi sur chaque case.
   - Événements et montées de niveau : chaque choix revient au propriétaire du chat concerné, pas à un vote. La montée de niveau et l'option d'un événement ont marché dans la partie du 17 octobre (build 23) ; le nom de celui qui décide s'affiche pendant un événement depuis la build 24, à tester. Depuis la build 38, la Némésis revient dans une aventure à plusieurs : elle avance chez chacun et son combat se joue ensemble, à tester.
10. **Butin et or** : l'or est fait, à tester (build 21, chacun sa bourse) ; le butin en objets se partage au défilé, à tester (build 26) ; en boutique, chacun paie avec sa bourse et garde ce qu'il achète, à tester (build 30).
11. **Mort, spectateur, nouveaux chats, déconnexion et IA** : les recrues vont aux spectateurs et rentrent chez leur joueur (build 28, à tester) ; un joueur qui part en plein combat laisse ses chats à l'hôte, et un combat n'attend plus un absent (builds 27 à 29 ; celui qui revient en plein combat construit le même combat que l'hôte depuis la build 39) ; l'IA du jeu joue les chats d'un absent (builds 32 à 34, à tester).
12. **Passage à 3 et 4 joueurs** : prévu dans le code de mgmp, mais jamais testé par son auteur ; à 3 joueurs, la place libre va au premier qui place un deuxième chat (build 39, à tester).
13. **Invitations Steam** : faites dans la build 39, à tester. L'hôte qui ouvre son salon apparaît « Rejoindre la partie » chez ses amis Steam, qui se connectent sans adresse ni port ; la connexion par adresse IP reste possible.

**Build 19, corrections, à tester.** La visée de l'autre joueur ne lit plus les chats sortis du combat (4 erreurs mémoire rattrapées par joueur dans les journaux du 13 octobre). Le retour anticipé à la maison joue la même musique chez l'invité. Il n'y a plus de fausse alerte à la première case d'une aventure, ni de lignes alarmantes hors aventure dans le journal. Une désynchronisation vue une seule fois, le 12 octobre (tour 9, un échange de place pendant une attaque ennemie), n'a pas de cause trouvée : la build 19 garde les tirages aléatoires de chaque tour de combat, pour la trouver si elle revient.

**Build 20, objets en aventure, à tester.** Le sac reste commun, mais chacun n'équipe que ses chats : sur l'écran des objets, ceux d'un chat qui n'est pas à toi sont grisés. Un invité change les objets de ses chats sur la carte ; à la fermeture de son sac, sa photo part chez l'hôte, qui la prend, ou la refuse s'il a changé quelque chose entre-temps. Le texte des messages part en octets : l'état du vote du chemin n'est plus coupé (l'invité voyait de faux votes et un faux gagnant).

**Build 21, chacun sa bourse, à tester.** Ce qu'un chat trouve (événement, combat) va à son propriétaire, le reste est partagé, et chacun rapporte sa bourse chez lui à la victoire (voir « Butin et or »). Les boîtes de meubles restent à l'hôte ; les bourses ne survivent pas à un redémarrage de l'hôte en pleine aventure.

**Build 22, chacun choisit pour ses chats et les objets en direct, à tester.** Pendant une montée de niveau, seul le propriétaire du chat choisit l'option, relance la liste et choisit la capacité à remplacer ; chez les autres, l'écran dit « À X de choisir » et leurs clics ne font rien. Chaque mouvement d'objet (équiper, rendre au sac, lire un livre) arrive aussitôt chez tous, même sac ouvert, et l'hôte départage deux prises du même objet. La duplication venait de la relecture d'un chat : un objet rendu au sac y restait porté ; ses emplacements sont maintenant vidés avant.

Build 23, les événements au propriétaire du chat et les objets sac ouvert, à tester. L'option d'un événement est choisie par le propriétaire du chat de l'événement ; chez les autres, le titre dit « À X de choisir » pendant les options. Le chat d'un événement où l'on choisit qui y va n'était pas répliqué (chacun envoyait le sien) : chacun ne peut maintenant envoyer que ses chats, et le premier envoyé y va chez tous. Un choix n'est plus joué pendant que l'autre lit encore la page du titre : il l'attend. Objets : l'écran des objets met la carte en pause, et le mod prenait le joueur sac ouvert pour un joueur dans une case ; les mouvements de l'hôte ne partaient donc jamais et ceux des invités étaient retenus. C'est corrigé : seules une boutique ou une montée de niveau comptent encore comme une case.

**Builds 24 et 25, les objets reconnus d'un joueur à l'autre et qui décide affiché pendant un événement, à tester.** Dans la partie du 17 octobre (build 23), quatre mouvements d'objets ont été refusés, puis réparés par une resynchronisation qui annulait le clic de l'invité. Pour reconnaître un objet chez l'autre joueur, le mod comparait aussi sa place dans l'écran des objets, renumérotée quand on le ferme, et sa marque « nouvel objet », effacée quand la souris passe dessus : deux détails qui changent chez un seul joueur. Ils ne comptent plus, et si un mouvement est encore refusé, les deux journaux disent quel détail diffère. « À X de choisir » ne s'affichait pas pendant un événement, car le titre de sa fenêtre ne se lisait pas : la phrase s'ajoute maintenant sous le texte de l'événement. La build 25, après une relecture du code, garantit qu'elle ne s'y ajoute qu'une fois et que le texte du jeu revient ensuite, même si le jeu reformate ce texte.

**Build 26, le défilé du butin, à tester.** À la victoire d'une aventure mixte, les objets du sac passent un par un chez les joueurs qui ont amené des chats : chacun choisit Besoin, Cupidité ou Passer, les dés de l'hôte décident, et ce qu'un invité remporte rejoint son rangement à son retour. Une relecture du code a fait ajouter une copie sur le disque de ce qui attend d'être ajouté à la save d'un invité (son butin et sa bourse), pour que rien ne se perde s'il ferme le jeu avant de rentrer, et une vérification que l'objet est encore chez l'hôte avant de le donner.

**Builds 27 et 28, les combats sans les absents et les recrues, livrées dans la build 29.** La partie du 17 octobre (build 25) a montré deux problèmes : « Abandonner l'aventure » dans la copie de l'hôte faisait perdre un chat à l'invité, et le combat suivant de l'hôte restait bloqué pour toujours en attendant l'invité rentré chez lui. L'hôte choisit maintenant qui joue chaque combat en entrant dans la case : lui et les invités de son aventure. Il prend les chats d'un joueur qui part en plein combat, et celui qui arrive regarde jusqu'au combat suivant. Chez un invité, « Abandonner » devient « Quitter l'aventure » : ses chats restent dans l'équipe et rentrent chez lui à la fin. Les recrues vont au spectateur, ou au joueur qui a le moins de chats, et rentrent chez lui. Deux relectures du code ont fait corriger, entre autres, une deuxième aventure qui prenait toute son équipe pour des recrues, et un invité désynchronisé en regardant un combat qui bloquait le combat suivant : il rentre maintenant chez lui, et peut revenir.

**Build 29, les dons de l'invité aux PNJ, à tester.** Le tuyau de l'invité prend ses chats ; leur faveur compte chez l'hôte, et une récompense remplie par son don lui revient : l'hôte la prend dès qu'il est dans sa maison, et les pièces, la nourriture et les objets rejoignent la save de l'invité à son prochain chargement, les déblocages restant à tous. Chaque décision de combat porte aussi son auteur et son rang, pour qu'aucune ne soit jouée deux fois quand un joueur arrive en plein combat. Livrée le 17 octobre au soir, après une relecture du code qui a fait corriger deux bogues : un joueur qui rejoignait un combat en cours pouvait jouer une décision du tour suivant avant celles d'avant, et un don fait pendant qu'une récompense attendait d'être prise était perdu (il compte maintenant pour la suivante).

**Builds 30 et 31, les achats en boutique avec sa bourse, à tester.** En boutique, l'invité achète comme seul, mais l'achat se fait chez l'hôte : le prix est pris sur sa bourse, et l'objet arrive dans le sac commun chez tout le monde. L'objet est à lui : à la victoire, il ne passe pas au défilé, il rentre chez lui (équipé sur un de ses chats, il rentre avec lui). L'hôte paie aussi avec sa bourse ; les bonbons et les familiers restent à l'hôte, qui choisit aussi la montée de niveau d'un bonbon donné au chat d'un invité. Chaque copie rejoue chaque achat par le jeu, si bien qu'un familier acheté rejoint maintenant l'équipe chez tous. Dans la boutique, le compteur de pièces montre la bourse de chacun, et l'hôte qui veut sortir pendant qu'un invité achète encore est prévenu par une fenêtre. Les salles au trésor, qui sont des boutiques pour le jeu, ne changent pas. Une relecture du code a fait corriger, entre autres, ces salles au trésor, qui seraient revenues à celui qui ouvrait le coffre, et un bonbon donné au chat d'un invité qui bloquait sa montée de niveau (build 31).

**Builds 32 à 34, l'IA du jeu pour les chats d'un absent, à tester.** Chez l'hôte, un chat dont le joueur est parti en plein combat, ou n'était pas dans l'aventure à l'entrée de la case, est joué par l'IA du jeu : un court instant avant chaque action, puis sa décision part chez tous comme celle d'un joueur. Pendant son tour, la barre des capacités de l'hôte est grisée. Le jeu fait choisir cette même IA à un chat charmé, et chaque joueur qui attendait sa décision la refaisait à chaque image, ce qui pouvait désynchroniser le combat : c'est corrigé aussi. Deux relectures du code ont fait corriger, entre autres, ce chat charmé chez celui qui le dirige, qui jouait la décision du jeu et non celle du message, et un pouvoir hors des cases du chat qui arrêtait le combat (builds 33 et 34).

**Builds 35 à 37, « Défendre ! » en session, à tester.** Un joueur dont la maison est attaquée la défend sans quitter la session, seul, avec les chats de sa boîte de départ. Les autres le voient dans une fenêtre et dans leurs listes d'attente (nuit, départ, arrivée dans l'aventure), même s'ils arrivent pendant le combat. La build 36 corrige une défense quittée par « Quitter vers le menu », que le jeu croyait encore en cours dans la partie suivante.

**Build 37, deux adresses du jeu enfin trouvées.** Le mod ne les trouvait jamais en jeu depuis leur ajout. La récompense d'un don d'invité à un PNJ revient maintenant au donneur, et le défilé du butin montre le nom des objets au lieu de leur clé.

**Build 38, la Némésis à plusieurs, à tester.** Elle avance sur la carte de chacun comme chez l'hôte. Quand elle rattrape l'équipe, son combat se joue ensemble, comme une case. Un joueur qui arrive en cours de carte la voit apparaître là où elle est chez l'hôte.

**Build 39, revenir en plein combat, la 4e place et Steam, à tester.** Un joueur revenu pendant un combat recevait les chats, l'inventaire et l'historique d'après l'entrée de la case ; or le jeu ne rejoue jamais un niveau déjà joué dans l'aventure, et lui construisait donc un autre combat (vu le 17 octobre). Chaque entrée de case garde maintenant une copie de cet état, envoyée à celui qui arrive, avec la signature du premier tour pour vérifier. À 3 joueurs, la 4e place va au premier qui met un deuxième chat dans sa boîte de départ, et une fenêtre le dit à chacun. L'hôte peut inviter ses amis par Steam : un ami qui clique « Rejoindre la partie » se connecte sans adresse ni port. L'écran des sauvegardes s'ouvre tout seul quand la partie de l'hôte arrive, et chaque ligne du journal porte l'heure.

**Build 40, vérification globale des synchronisations, à tester.** Une relecture indépendante de tous les chemins de synchronisation a trouvé quatre problèmes que les journaux n'avaient pas encore montrés, tous corrigés. Un joueur qui cliquait « Rejoindre » pendant un événement de l'hôte pouvait être servi comme si l'hôte était sur la carte, et ses écrans de choix restaient figés jusqu'à la fin de l'aventure. La vérification de l'état à chaque case ne comparait qu'un seul autre joueur : à trois, le second invité n'était jamais vérifié. Un invité en retard de cinq cases pouvait recevoir l'inventaire et l'historique d'une case trop tôt. Enfin, pendant un chargement, les curseurs des autres joueurs remplissaient la file de réception, et le message suivant (une décision de combat, une case, un choix) était perdu : la file grandit maintenant, et les curseurs ne gardent que leur dernière position. Une seconde relecture a fait corriger trois effets de bord de ces corrections.

**Build 41, le départ sans second chargement, vérifié le 18 octobre.** Avant, à chaque départ, l'invité chargeait deux fois : la maison de l'hôte pour faire le départ avec lui, puis, une fois arrivé sur sa propre carte, l'aventure de l'hôte, en repassant par le menu. Les saves du 17 octobre montrent que les deux cartes ont la même disposition et ne diffèrent que par la graine de chaque case. Au vote, l'hôte tire donc une graine que tout le monde utilise au moment où le jeu tire la carte. L'invité compare ensuite son aventure à celle de l'hôte (la carte, les chats, le sac, l'historique) : si elles sont pareilles, il la garde et ne recharge rien. Sinon, ou si l'hôte a déjà joué une case, l'aventure de l'hôte se charge comme avant. Le réglage net.same\_map = false remet l'ancien fonctionnement.

**Build 45, la seconde vérification globale.** Une relecture complète du mod (réseau, synchronisation de l'aventure, départ et saves), puis trois relectures indépendantes des corrections (les builds 42 à 44 n'ont pas été installées). Un joueur refusé parce qu'il a une autre version ne bloque plus le salon de l'hôte : il sort tout de suite de la liste des joueurs, garde 30 s pour lire la raison, puis sa connexion est coupée. Le salon de l'invité dit quoi faire selon la raison : partie complète, version du mod, version du jeu ou fichiers du jeu. Un combat arrêté par une désynchronisation ne fige plus les suivants, même quand l'hôte recharge la même aventure. Un invité dont la connexion tombe ne joue plus ses chats seul. Au départ, l'hôte n'attend plus un invité parti, et l'invité recharge tout de suite quand sa carte ne peut pas être celle de l'hôte. Les deux joueurs doivent avoir la build 45 (protocole 61).

**Build 49, les échanges.** Un bouton « Échanger » ouvre une fenêtre pour donner à un autre joueur, sans qu'il ait à accepter : à la maison, des pièces, de la nourriture et des objets du rangement ; en aventure, des pièces et de la nourriture de sa bourse (voir Échanges entre joueurs). Le donneur ne perd rien tant que l'autre n'a pas gardé le don, ce qu'il donne ne quitte que la save d'où le don est parti, et les deux saves sont enregistrées aussitôt. Trois relectures ont fait serrer le reste : un objet promis à un don en cours, au butin ou à un achat ne se redonne pas, et le rangement d'un invité ne bouge pas tant qu'il attend ce que ses chats rapportent. Protocole 63 : les deux joueurs mettent à jour.

**Build 52, l'orientation des chats en combat, à tester.** La partie du 18 octobre (build 49) s'est figée chez l'invité : le combat était fini chez l'hôte et continuait chez lui. Les échanges n'y étaient pour rien. Au tour 16, Katara a fini son tour tournée vers l'ouest chez l'hôte et vers le sud chez l'invité ; les deux jeux ne comparent pas l'orientation, et au tour 20 une IA ennemie, qui la lit, a joué autrement. En visant, le jeu tourne le chat vers la cible avec une animation de demi-tour qui écrit l'orientation après coup, et seulement chez celui qui vise : 5 parties à deux sur 10 avaient de tels écarts. En combat à plusieurs, le chat ne se tourne plus vers la visée ; il se tourne quand l'action part, pareil chez tous. Si deux copies d'un combat ne finissent quand même pas ensemble, le joueur dont la copie attend en vain rentre chez lui avec un message (un panneau de sa maison le ramène), et un hôte qui attend un joueur dont le combat est fini laisse l'IA du jeu jouer ses chats. Deux relectures ont fait serrer ce filet de secours. Protocole 66 : les deux joueurs mettent à jour.

## Questions ouvertes et sources

Un point reste à trancher avant ou pendant le développement.

- [ ] La save multi peut-elle se jouer si un joueur manque ? Proposition actuelle : non.
- [x] Comment échanger de l'or et des objets entre joueurs ? Un bouton « Échanger », à la maison et en aventure (avec sa bourse), en don direct (décidé le 18 octobre).
- [x] Les choix pendant les événements se votent-ils comme le chemin ? Oui, décidé le 15 octobre.
- [x] Ce que trouve un chat en aventure va-t-il à tous ou à son propriétaire ? À son propriétaire, chacun sa bourse (décidé le 16 octobre).
- [x] Le sac en aventure est-il commun ? Oui, mais chacun n'équipe que ses chats (décidé le 16 octobre).
- [x] Pendant l'aventure, qui choisit la montée de niveau d'un chat, sa relance et la capacité à remplacer ? Son propriétaire, et les objets se synchronisent à chaque mouvement (décidé le 16 octobre).
- [x] Les récompenses des PNJ vont-elles au donneur ou à tous ? Les déblocages à tous, les objets et l'or au donneur (décidé le 17 octobre).
- [x] Un objet que tout le monde passe est-il perdu, ou revendu avec l'or partagé ? Il reste à l'hôte (décidé le 17 octobre).
- [x] Quel délai pour le butin : 30 secondes comme pour les votes (décidé le 15 octobre), ou aucun délai comme dans mgmp ?
- [x] Partir de mgmp ou repartir de zéro ? Le mod est bâti sur mgmp.
- [x] Version du jeu : la version installée, 1.1.21239, est prise en charge par la release v1 de mgmp, publiée le 12 septembre 2026.

Sources :

- [mgmp sur GitHub](https://github.com/SanTertrust/mgmp) : mod coop open source pour 2 joueurs, licence MIT ; son CLAUDE.md documente les adresses et structures du jeu.
- [Releases de mgmp](https://github.com/SanTertrust/mgmp/releases) : release v1 pour la version 1.1.21239 du jeu.
- [mgmp sur VGTimes](https://vgtimes.com/games/mewgenics/files/98234-cooperative-mode.html) : installation et lancement.
- [Auto-Battler Mode sur Nexus Mods](https://www.nexusmods.com/mewgenics/mods/8) : l'IA des chats via le passif « Uncontrollable ».
- [MewgenicsModTool sur Nexus Mods](https://www.nexusmods.com/mewgenics/mods/128) : neutralisation du Déjà Vu de Steven.
