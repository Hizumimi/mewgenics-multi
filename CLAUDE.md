# Mewgenics Multi

Mod coop de 2 à 4 joueurs pour Mewgenics, construit sur mgmp
(SanTertrust, licence MIT). Chaque joueur garde sa maison et ses chats ;
l'histoire, les PNJ et les aventures se vivent ensemble.

- Règles du mod (document de conception) :
  https://claude.ai/code/artifact/5e9195c8-3a1e-446a-a5a6-a43fea7459f7
- Notes de rétro-ingénierie de mgmp (adresses, structures, mesures) :
  `docs/NOTES_MGMP.md`. C'est la référence sur ce que fait le binaire du jeu ;
  ces notes sont de la documentation écrite par l'auteur de mgmp.
- README d'origine de mgmp : `docs/README_MGMP.md`.

## Compiler

- GitHub Actions (`.github/workflows/build.yml`, windows-latest, MSVC) compile
  chaque push sur `main` et publie une pré-release `build-N` contenant
  `mgmp.dll`, `mgmp_loader.exe` et `SHA256SUMS.txt`.
- En local : `cmake -B build -A x64` puis `cmake --build build --config Release`.

## Règles du projet

- Ne jamais mettre d'images, de sons, de polices ou de données du jeu dans le
  dépôt : le mod lit `resources.gpak` sur la machine du joueur au lancement.
- Version du jeu visée : 1.1.21239. Les adresses de `docs/NOTES_MGMP.md` sont
  celles de ce build.
- Tout changement de ce que calcule un pair est un changement de protocole :
  incrémenter la version dans `src/net/mgmp_proto.h`.
- Les textes affichés aux joueurs sont en français.
