# SQLite 3.49.2 (domaine public)

Les saves de Mewgenics sont des bases sqlite. Le mod s'en sert pour reprendre
le monde de l'hôte (PNJ, histoire, déblocages) dans la save d'un invité, juste
avant que le jeu ne la charge (`src/session/mm_world.cpp`).

- `sqlite3.c`, `sqlite3.h` : l'amalgamation officielle de SQLite 3.49.2
  (`SQLITE_SOURCE_ID` « 2025-05-07 10:39:52 17144570b0d9… »), domaine public
  (https://sqlite.org/copyright.html).
- Provenance : le paquet npm `better-sqlite3@11.10.0`, dossier `deps/sqlite3/`
  (intégrité sha512 du paquet vérifiée contre le registre npm :
  `EwhOpyXiOEL/lKzHz9AW1msWFNzGc/z+LzeB3/jnFJpxu+th2yqvzsSWas1v9jgs9+xiXJcD5A8CJxAG2TaghQ==`).
  Ce paquet applique un seul correctif, sans effet sur le comportement : deux
  constantes de date écrites en clair (`30.0*86400.0` → `2592000.0`) pour MSVC.
