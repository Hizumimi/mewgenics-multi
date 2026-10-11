#pragma once
// mm_world_merge -- Mewgenics Multi : recopier le monde d'une save dans une
// autre. Le coeur de mm_world, sans Windows ni jeu : sqlite et la bibliotheque
// standard seulement, pour que tests/test_world_merge.cpp le verifie sur de
// vraies bases.
//
// La regle de chaque partie de la save est decrite dans mm_world.h.

#include <cstddef>
#include <cstdint>
#include <string>

struct sqlite3;

namespace mgmp {

struct WorldSummary {
    bool        npc       = false;   // npc_progress repris de l'hote
    int         npc_ready_cleared = 0; // recompenses pretes de l'hote effacees (-1 : format inconnu)
    int         unlocks   = 0;       // noms ajoutes a « unlocks »
    int         tutorials = 0;       // noms ajoutes a « tutorial_tokens »
    int         house_ups = 0;       // agrandissements ajoutes a « house_unlocks »
    std::string house_from, house_to;
    int         props     = 0;       // proprietes ecrites
};

// Des octets qui commencent comme une base sqlite.
bool world_is_sqlite(const uint8_t* p, size_t n);

// Une copie en memoire de ces octets, en lecture seule (sqlite3_deserialize).
// Nul si ce n'est pas une base lisible. A fermer avec sqlite3_close.
sqlite3* world_open_bytes(const uint8_t* p, size_t n);

// Une save de Mewgenics saine : tables files et properties, controle rapide ok.
bool world_looks_like_save(sqlite3* db);

// Cette propriete appartient-elle au monde partage ?
bool world_is_world_prop(const char* key);

// Le proprietaire de la save (propriete owner_steamid), en texte ; vide si
// absente. Deux saves du meme nom et du meme proprietaire sont le meme fichier
// sur un meme PC (deux instances du jeu partagent le dossier des saves).
std::string world_owner(sqlite3* db);

// Recopie le monde de `host` dans `own`, en une transaction. Faux = rien n'a
// ete change dans `own` (et la raison est au journal).
bool world_apply(sqlite3* host, sqlite3* own, WorldSummary& sum);

// Une empreinte du monde de cette save seulement (les blocs et les proprietes
// qu'on recopie) : l'hote ne renvoie sa save que si elle change -- pas a chaque
// case d'une aventure, ou la save s'ecrit mais le monde reste le meme.
uint64_t world_digest(sqlite3* db);

} // namespace mgmp
