#pragma once
// mm_npc_state -- Mewgenics Multi : l'etat de chaque PNJ, tel que le jeu le
// garde (bloc npc_progress de la save, serialiseur sub_1401D3B90) et tel que le
// mod l'envoie en direct (MSG_NPC, mm_npc.cpp). Portable : la bibliotheque
// standard seulement, pour que tests/test_world_merge.cpp le verifie.
//
// Dans le jeu, l'objet de progression (MewDirector+0x5A8, le meme que celui du
// boss de la maison) porte 8 PNJ de 0x78 octets a partir de +0x48, dans l'ordre
// de la save. D'apres la recompense visee de chacun sur les saves du joueur :
//   0 Beanies, 1 Butch, 2 Tink, 3 Frank, 4 Jack, 5 Tracy, 6 l'orgue, 7 Steven.
//
// Un PNJ, E etant son debut (offsets du serialiseur, qui lit et ecrit chaque
// champ en place) :
//   E+0x00  vector<{string, u32}>  les recompenses deja prises
//   E+0x18  string                 la recompense visee
//   E+0x38  u32                    (sens inconnu ; enregistre)
//   E+0x40  u32                    la faveur : les chats donnes vers la
//                                  recompense visee (sub_1401C1650 l'augmente
//                                  a chaque don)
//   E+0x44  u32                    (enregistre)
//   E+0x48  u8                     recompense prete (sub_1401C1650 : faveur >=
//                                  palier de la recompense visee)
//   E+0x4C  u32                    (enregistre, version >= 4)
//   E+0x50  u8, E+0x51 u8          (enregistres, version >= 0x13)
//   E+0x54  u32                    (enregistre, version >= 0x20)
//   E+0x58  32 octets              (enregistres, version >= 0x17)
// E+0x3C et le bourrage ne sont pas enregistres : on n'y touche jamais.
//
// Dans la save, le bloc commence par la version (u32, 38 aujourd'hui), puis les
// 8 PNJ dans cet ordre : la liste (u64 nombre, puis pour chaque recompense u64
// longueur + octets et un u32), la recompense visee (u64 longueur + octets),
// puis les champs simples dans l'ordre ci-dessus (0x38, 0x40, 0x44, 0x48, 0x4C,
// 0x50, 0x51, 0x58, 0x54). Le reste du bloc (l'histoire) suit.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace mgmp {

constexpr int kNpcCount = 8;

struct NpcState {
    std::string cur;              // la recompense visee
    uint64_t    taken = 0;        // les recompenses prises : en memoire, la taille
                                  // du tableau en octets ; dans la save, leur nombre
    uint32_t    a38   = 0;
    uint32_t    favor = 0;        // E+0x40
    uint32_t    c44   = 0;
    uint8_t     ready = 0;        // E+0x48
    uint32_t    e4c   = 0;
    uint8_t     f50   = 0, g51 = 0;
    uint32_t    i54   = 0;
    uint8_t     h58[32] = {};
};

// Le nom d'un PNJ pour le journal (« Jack »), « ? » hors bornes.
const char* npc_name(int idx);

// Les 8 PNJ d'un bloc npc_progress. Faux si le format n'est pas celui attendu
// (version < 0x20, ou bloc tronque).
bool npc_progress_parse(const std::vector<uint8_t>& blob, NpcState (&out)[kNpcCount],
                        uint32_t* version = nullptr);

// Le npc_progress de l'hote, prepare pour la save d'un invite (`blob` est
// modifie en place). C'est l'hote, qui a fait les dons, qui prend une
// recompense prete ; chez l'invite, un PNJ dont la recompense est prete chez
// l'hote perd donc ce drapeau, et sa faveur redescend sous le palier -- sinon
// un seul point de faveur (un chat, une aventure finie pour l'orgue) suffirait
// a l'invite pour la prendre aussi (sub_1401C1650 remet le drapeau des que la
// faveur atteint le palier). La faveur qu'il garde : la sienne si sa save en est
// a la meme recompense, sans drapeau, avec moins que l'hote (sa copie d'avant
// le dernier don) ; sinon 0.
// Renvoie le nombre de PNJ ainsi rabaisses, ou -1 si le bloc de l'hote n'a pas
// le format attendu (il reste alors tel quel). `own` peut etre vide ou illisible.
int npc_progress_for_guest(std::vector<uint8_t>& blob, const std::vector<uint8_t>& own);

// Le message en direct de l'hote (MSG_NPC) : les 8 PNJ, champs simples, la
// recompense visee et la taille de la liste des recompenses prises.
std::vector<uint8_t> npc_state_encode(const NpcState (&npcs)[kNpcCount]);
bool npc_state_decode(const uint8_t* p, size_t n, NpcState (&out)[kNpcCount]);

// Les deux en sont a la meme recompense : meme recompense visee, meme liste de
// recompenses prises.
bool npc_same_step(const NpcState& host, const NpcState& mine);
// Un des champs simples differe (« recompense prete » exclue).
bool npc_simple_fields_differ(const NpcState& host, const NpcState& mine);

// L'etat que l'invite doit prendre pour un PNJ en direct, partant du sien.
// Pas la meme etape : le sien, tel quel (la recompense a ete prise chez l'hote :
// il faudrait changer des chaines et des listes du jeu ; sa save sera mise au
// monde de l'hote a son prochain chargement). Meme etape : les champs simples
// de l'hote, sauf la faveur quand la recompense est prete chez l'hote (meme
// raison que npc_progress_for_guest), et jamais « recompense prete ».
NpcState npc_guest_target(const NpcState& host, const NpcState& mine);

} // namespace mgmp
