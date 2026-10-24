# Mewgenics Multi

Mod coop de 2 à 4 joueurs pour Mewgenics, construit sur mgmp
(SanTertrust, licence MIT). Chaque joueur garde sa maison et ses chats ;
l'histoire, les PNJ et les aventures se vivent ensemble.

- Note de passation, à lire en premier (ce qui a été fait build par build,
  l'état, la méthode, ce qui reste) : `docs/PASSATION.md`.
- Règles du mod (document de conception) :
  https://claude.ai/code/artifact/5e9195c8-3a1e-446a-a5a6-a43fea7459f7 ;
  copie lisible sans claude.ai : `docs/CONCEPTION.md`.
- Notes de rétro-ingénierie de mgmp (adresses, structures, mesures) :
  `docs/NOTES_MGMP.md`. C'est la référence sur ce que fait le binaire du jeu ;
  ces notes sont de la documentation écrite par l'auteur de mgmp.
- README d'origine de mgmp : `docs/README_MGMP.md`.

## Compiler

- GitHub Actions (`.github/workflows/build.yml`, windows-latest, MSVC) compile
  chaque push sur `main` et publie une pré-release `build-N` contenant
  `mgmp.dll`, `mgmp_loader.exe` et `SHA256SUMS.txt`. Un commit dont le message
  contient `[skip ci]` ne lance pas de build (utile pour la documentation).
- En local : `cmake -B build -A x64` puis `cmake --build build --config Release`.
- Sous Linux, pour vérifier seulement (pas de DLL) : `tools/linux/compiler.sh`
  et `tools/linux/tests.sh`.
- Le zip de l'ami : modèle dans `paquet/`, refait par `tools/paquet.ps1`
  (Windows) ou `tools/linux/paquet.sh`.

## Règles du projet

- Ne jamais mettre d'images, de sons, de polices ou de données du jeu dans le
  dépôt : le mod lit `resources.gpak` sur la machine du joueur au lancement.
- Version du jeu visée : 1.1.21239. Les adresses de `docs/NOTES_MGMP.md` sont
  celles de ce build.
- Tout changement de ce que calcule un pair est un changement de protocole :
  incrémenter la version dans `src/net/mgmp_proto.h`.
- Les textes affichés aux joueurs s'écrivent en français dans le code, passés par
  `tr()` ; ils sont traduits dans les 9 autres langues du jeu (`src/i18n/`). Un texte
  ajouté ou changé se traduit dans chaque table : règles et outils dans
  `docs/TRADUCTION.md` (`tests/test_i18n.cpp` les vérifie).
