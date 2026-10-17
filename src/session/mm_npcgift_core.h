#pragma once
// mm_npcgift_core -- Mewgenics Multi : les dons de l'invite aux PNJ, la partie
// portable (bibliotheque standard seulement, verifiee par tests/test_npcgift.cpp).
//
// LES REGLES (document de conception, « PNJ et progression partagee » ; decide le
// 17 octobre : « les deblocages a tous, les objets et l'or au donneur »)
//   - Le monde est celui de l'hote : ses PNJ comptent pour tous (mm_world, mm_npc).
//   - Un invite en session donne ses chats aux PNJ comme seul : le tuyau de sa
//     maison les prend, et la carte des PNJ les envoie. Le chat quitte sa maison ;
//     la faveur, elle, va au PNJ de l'hote (kNpcDonate), pas a sa copie.
//   - Quand ce don remplit la jauge du PNJ chez l'hote, la recompense est au
//     donneur. L'hote la prend pour lui des qu'il est dans sa maison (sans
//     fenetre ouverte) et que le donneur est la : les pieces, la nourriture et les
//     objets partent chez le donneur (kNpcGift) ; le reste -- agrandissements,
//     boutiques, drapeaux de l'histoire -- reste dans le monde de l'hote et arrive
//     chez chacun avec lui. Le donneur parti avant : tout reste a l'hote.
//   - Un objet ne quitte le rangement de l'hote qu'une fois garde par le donneur
//     (kNpcGot), et les pieces et la nourriture ne quittent sa maison qu'alors :
//     sans reponse, un doublon plutot qu'une perte (comme au defile du butin).
//
// LE MESSAGE (MSG_TEAM kTeamNpc, voir mm_team_merge) : la tete MSG_TEAM, puis
// u8 op, u8 PNJ (0 Beanies ... 7 Steven, voir mm_npc_state.h), puis selon op :
//   kNpcDonate  invite -> hote : u32 faveur, le nom du chat (u16 n + octets),
//               u8 n puis n capacites (u16 n + octets) : les 4 actives puis les
//               2 passives du chat, « None » pour une place vide (Beanies les
//               apprend, sub_1401C1650).
//   kNpcReady   hote -> donneur : sa recompense est prete ; elle partira quand
//               l'hote sera dans sa maison.
//   kNpcGift    hote -> donneur : u64 numero de la recompense, i32 pieces,
//               i32 nourriture, u8 n puis n objets : nom, texte, a, b, c, d (u32),
//               e (u8), f (u32), puis le nom affiche (UTF-8).
//   kNpcGot     donneur -> hote : u64 numero, u8 1 pieces et nourriture gardees,
//               u64 un bit par objet garde.

#include "mm_team_merge.h"   // ItemFields, kTeamMagic, kTeamFormat, kTeamNpc

#include <cstdint>
#include <string>
#include <vector>

namespace mgmp {

constexpr uint8_t kNpcGiftNpcs      = 8;    // les PNJ de l'objet de progression
constexpr size_t  kNpcGiftAbilities = 6;    // 4 actives, 2 passives
constexpr size_t  kNpcGiftMaxItems  = 32;
constexpr size_t  kNpcGiftMaxName   = 63;   // la cle GON d'un objet ; plus long : il n'est pas envoye
constexpr size_t  kNpcGiftMaxText   = 255;  // le texte d'un objet (un livre : sa capacite) ; idem
constexpr uint32_t kNpcGiftMaxFavor = 16;   // un don : 1 dans le jeu
// Le « vote » des cadeaux des PNJ dans le fichier en attente du donneur (mm_team) :
// 0, que n'a jamais un depart (les votes partent de 1).
constexpr uint8_t kNpcGiftSeq = 0;

enum : uint8_t {
    kNpcDonate = 1,
    kNpcReady  = 2,
    kNpcGift   = 3,
    kNpcGot    = 4,
};

struct NpcGiftItem {
    ItemFields  f;
    std::string label;       // le nom affiche (UTF-8), donne par le jeu de l'hote
};

struct NpcGiftMsg {
    uint8_t op  = 0;
    uint8_t npc = 0;
    // kNpcDonate
    uint32_t                 favor = 0;
    std::string              cat;           // le nom du chat (UTF-8)
    std::vector<std::string> abilities;
    // kNpcGift, kNpcGot
    uint64_t                 gift  = 0;
    int32_t                  coins = 0;
    int32_t                  food  = 0;
    std::vector<NpcGiftItem> items;         // kNpcGift
    uint8_t                  money = 0;     // kNpcGot : pieces et nourriture gardees
    uint64_t                 kept  = 0;     // kNpcGot : un bit par objet garde
};

// Le message complet (tete MSG_TEAM comprise). Faux : illisible ou pas un kTeamNpc.
std::vector<uint8_t> npcgift_encode(const NpcGiftMsg& m);
bool npcgift_decode(const uint8_t* p, size_t n, NpcGiftMsg& out);

// Les capacites d'un chat telles que l'hote les donne au jeu : exactement 6, une
// place vide ou illisible devient « None » (le jeu saute « None » ; une chaine
// vide, il l'apprendrait).
std::vector<std::string> npcgift_abilities(const std::vector<std::string>& in);

// Un objet du rangement, vu par l'hote : son id (Equipment+0x00) et ses champs.
struct NpcStored {
    uint64_t   id = 0;
    ItemFields f;
};
// Les objets de `after` dont l'id n'etait pas dans `before` (ce que la recompense
// a range), dans l'ordre de `after`.
std::vector<size_t> npcgift_new_items(const std::vector<NpcStored>& before, const std::vector<NpcStored>& after);

// « 50 pièces, 3 nourritures, Collier vierge et Aiguille » (UTF-8) ; « rien » si vide.
std::string npcgift_list_text(int64_t coins, int64_t food, const std::vector<std::string>& labels);

// Ce que l'hote retient d'une recompense prete grace au don d'un invite.
constexpr uint8_t kNpcNoDonor = 0xFF;
struct NpcDonor {
    uint8_t     peer = kNpcNoDonor;   // numero du donneur dans la session
    std::string name;                 // son nom (UTF-8)
    std::string reward;               // la recompense visee quand la jauge s'est remplie
    bool        told = false;         // kNpcReady envoye
};

// Hote, apres un don applique (le sien ou celui d'un invite, `from` = kNpcNoDonor
// pour le sien) : la jauge vient-elle de se remplir ? Alors la recompense est a
// ce donneur (personne : la sienne). Une jauge deja pleine ne change pas de
// donneur. Vrai si `d` a change.
bool npcgift_after_donation(NpcDonor& d, bool was_ready, bool now_ready, uint8_t from, const std::string& name,
                            const std::string& reward);

// Hote, au moment ou le jeu prend la recompense d'un PNJ : a qui va-t-elle ?
// kNpcNoDonor : a l'hote (pas de donneur, ou plus la meme recompense visee).
uint8_t npcgift_reward_owner(const NpcDonor& d, const std::string& reward_now);

} // namespace mgmp
