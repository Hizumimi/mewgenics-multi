#pragma once
// mgmp_orphan -- Mewgenics Multi (protocole 64) : un combat fini chez l'un et pas
// chez l'autre. Les regles et le message seuls (tests/test_orphan.cpp) ; le reste
// est dans mgmp_lockstep.cpp (lockstep_on_node_end, lockstep_on_battle_end) et
// mm_menu.cpp (le retour a la maison).
//
// LE CAS (journaux du 18 octobre 2026, 19:23). Combat e15743b8b1852f54 : au tour
// 20, l'IA d'un ennemi choisit autrement chez l'hote et chez l'invite -- une
// orientation differente depuis le tour 16 (mgmp_lockstep.cpp, « Character::Face
// empeche »). Chez l'hote, le combat se finit pendant ce tour ; chez l'invite il
// continue, et le tour 21 commence : celui d'un chat de l'hote, dont la decision
// ne viendra jamais. L'empreinte du tour 20 n'a jamais ete comparee (l'hote ne
// l'a jamais calculee) : rien ne s'est arrete, l'invite est reste fige devant un
// combat que plus personne ne jouait.
//
// LES TOURS. Chez chacun, g.turn est le numero du tour EN COURS : l'empreinte du
// tour T part a la fin de T, puis T+1 commence. Les fins de tour font partie de
// la simulation : deux copies d'accord en font autant, et finissent un combat avec
// le meme g.turn -- parfois une ou deux fins de tour viennent apres la derniere
// action, chez les deux. Mesure : dans les journaux du 13 au 18 octobre, sur 14
// combats a deux, 13 ont leur derniere empreinte au meme tour chez les deux
// joueurs ; le 14e est celui du 18 octobre : l'hote a fini apres l'empreinte du
// tour 19 (g.turn 20), l'invite a envoye celle du tour 20 (g.turn 21).
//
//   - L'hote a fini avec g.turn = H. Un invite qui en est au tour T > H a fini H
//     et en a commence un que l'hote n'a jamais eu : sa copie a diverge.
//   - Ce joueur a fini avec g.turn = M. Une empreinte de l'hote pour un tour >= M
//     dit que l'hote a fini M : il a continue apres la fin de ce joueur.
//
// Ni l'un ni l'autre n'arrive a deux copies d'accord ; un invite en retard (qui
// rattrape l'hote) a un tour plus petit, jamais plus grand. La fin d'une case
// (T_NodeEnd, au retour sur la carte) vient apres toutes les fins de tour.
//
// CE QUI N'EST PAS VU. Un hote fige dans le tour M a attendre la decision d'un
// chat de cet invite, dont le combat est fini : aucune empreinte ne part. Le
// joueur peut alors quitter l'aventure (l'IA du jeu reprend ses chats chez
// l'hote) ; la cause de ce cas-la est la meme divergence, que l'orientation
// corrigee ne devrait plus produire.

#include <cstdint>
#include <cstring>

#include "mgmp_proto.h"

namespace mgmp {

enum OrphanKind : uint8_t {
    kOrphanNone     = 0,
    kOrphanHostDone = 1,   // le combat de l'hote est fini, celui de ce joueur continuait
    kOrphanMineDone = 2,   // le combat de ce joueur est fini, celui de l'hote continuait
};

// L'hote a fini son combat avec g.turn = `host_turn` ; ce joueur en est au tour `my_turn`.
inline bool orphan_past_host_end(uint32_t host_turn, uint32_t my_turn) { return my_turn > host_turn; }

// Ce joueur a fini son combat avec g.turn = `my_end_turn` ; une empreinte de l'hote pour le
// tour `host_hash_turn` est la (l'hote a fini ce tour-la).
inline bool orphan_host_went_on(uint32_t my_end_turn, uint32_t host_hash_turn) {
    return host_hash_turn >= my_end_turn;
}

// MM_BATTLE_END : a = 1 (de l'hote), x / y = le combat, text[0..3] = g.turn chez l'hote a la fin.
struct BattleEnd {
    uint64_t battle = 0;
    uint32_t turn   = 0;
};

inline MmMsg battle_end_encode(const BattleEnd& e) {
    MmMsg m{};
    m.kind = MM_BATTLE_END;
    m.a    = 1;
    m.x    = (uint32_t)(e.battle & 0xFFFFFFFFull);
    m.y    = (uint32_t)(e.battle >> 32);
    static_assert(sizeof(m.text) >= sizeof(e.turn) + 1, "MmMsg::text trop petit");
    memcpy(m.text, &e.turn, sizeof(e.turn));
    return m;
}

// Faux pour un autre message, une autre forme, ou un combat nul.
inline bool battle_end_decode(const MmMsg& m, BattleEnd& e) {
    if (m.kind != MM_BATTLE_END || m.a != 1) return false;
    e.battle = (uint64_t)m.x | ((uint64_t)m.y << 32);
    memcpy(&e.turn, m.text, sizeof(e.turn));
    return e.battle != 0;
}

} // namespace mgmp
