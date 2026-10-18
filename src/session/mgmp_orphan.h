#pragma once
// mgmp_orphan -- Mewgenics Multi (protocoles 64-65) : un combat fini chez l'un et
// pas chez l'autre. Les regles et le message seuls (tests/test_orphan.cpp) ; le
// reste est dans mgmp_lockstep.cpp (lockstep_on_node_end, lockstep_on_battle_end,
// l'attente d'une decision dans lockstep_fill_choice) et mm_menu.cpp (le retour a
// la maison).
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
// LA FIN D'UNE CASE. ReturnToMap (0x1403624A0, le rappel de l'ecran de fin de
// combat) appelle d'abord 0x1403B3D60, qui rend la scene « Map » et appelle la fin
// de case (T_NodeEnd), puis seulement 0x1403B7720, qui ouvre la scene « LevelUp ».
// Les montees de niveau, qui attendent l'autre joueur (mgmp_choice), viennent donc
// APRES l'annonce : elles ne la retardent pas.
//
// LES TOURS. Chez chacun, g.turn est le numero du tour EN COURS : l'empreinte du
// tour T part a la fin de T, puis T+1 commence. Les fins de tour font partie de
// la simulation : deux copies d'accord en font autant, et finissent un combat avec
// le meme g.turn -- parfois une ou deux fins de tour viennent apres la derniere
// action, chez les deux. Mesure : dans les journaux du 13 au 18 octobre, sur 14
// combats a deux, 13 ont leur derniere empreinte au meme tour chez les deux
// joueurs ; le 14e est celui du 18 octobre : l'hote a fini apres l'empreinte du
// tour 19 (g.turn 20), l'invite a envoye celle du tour 20 (g.turn 21). La fin
// d'une case (T_NodeEnd, au retour sur la carte) vient apres toutes les fins de
// tour ; seule compte celle de la case jouee (follow_node_end_counts : Espace sur
// un pion au repos en appelle une autre).
//
// LES TROIS REGLES. Chacune demande une copie qui ATTEND, pas seulement un
// numero : une copie divergente qui finit son combat d'elle-meme (un tour plus
// tard, par exemple) n'est pas figee, et la poussee de l'hote a la case suivante
// la recale (chats, inventaire, historique ; le flux repart de la graine de la
// case).
//   1. L'hote a fini avec g.turn = H (MM_BATTLE_END, a = 1). Un invite au tour
//      T > H qui attend depuis kOrphanWaitMs la decision d'un chat d'un autre
//      joueur : personne ne la lui enverra. Il rentre chez lui (kOrphanHostDone).
//   2. Un invite a fini avec g.turn = M (sa fin de case ; MM_BATTLE_END, a = 2,
//      vers l'hote). L'hote attend la decision d'un chat de cet invite, sans rien
//      de lui dans la file -- ses decisions passaient avant son annonce, sur le
//      meme lien -- depuis kOrphanWaitMs au tour T > M, ou kOrphanWaitSameTurnMs
//      au tour M meme. L'hote le retire des membres de ce combat, l'IA du jeu joue
//      ses chats.
//   3. Cet invite voit alors une empreinte de l'hote pour un tour > M : l'hote a
//      fini M et le tour d'apres, il a continue sans lui. Sa copie a diverge ; il
//      rentre chez lui (kOrphanMineDone) -- ses montees de niveau, son butin, ses
//      morts seraient ceux d'un autre combat, et l'hote attendrait ses choix.
//      Un tour de marge : une fin de tour de plus chez l'un apres la derniere
//      action ne suffit pas a renvoyer quelqu'un.
// Aucune n'arrive a deux copies d'accord ; un invite en retard (qui rattrape
// l'hote) a un tour plus petit, jamais plus grand. Une copie qui a diverge mais
// finit son combat d'elle-meme apres l'hote (regle 1 sans attente) n'est que
// notee au journal.
//
// LES PARTIES D'UN MEME COMBAT. Le numero d'un combat est la graine de sa case :
// rejoue apres un rechargement, c'est le meme. Les fins notees sont donc oubliees
// a chaque nouvelle photo de ce combat (celle de l'hote, et la sienne chez un
// invite), au retour de l'hote au menu (HOSTLEFT) et quand l'invite sort de
// l'aventure ; un invite qui (re)arrive en plein combat recoit de l'hote sa fin,
// si elle est deja passee (lockstep_catchup).

#include <cstdint>
#include <cstring>

#include "mgmp_proto.h"

namespace mgmp {

enum OrphanKind : uint8_t {
    kOrphanNone     = 0,
    kOrphanHostDone = 1,   // le combat de l'hote est fini, celui de ce joueur continuait
    kOrphanMineDone = 2,   // le combat de ce joueur est fini, celui de l'hote continuait
};

// Combien de temps une decision attendue doit manquer avant qu'une regle tranche.
// Une decision deja partie arrive en quelques dizaines de millisecondes ; la
// longueur d'une reflexion n'y change rien, puisque chaque regle demande aussi que
// le combat de l'autre soit fini. L'attente se compte d'une demande du jeu a la
// suivante : un trou de plus de kOrphanPollGapMs entre deux (une image figee, la
// fenetre en arriere-plan) la fait repartir de zero.
constexpr uint32_t kOrphanWaitMs    = 2000;
constexpr uint32_t kOrphanPollGapMs = 250;
// Regle 2 au tour meme de la fin de l'invite : le jeu peut demander la decision
// d'un chat au tour ou le combat se finit, avant de voir la victoire (une fin de
// tour apres la derniere action, une mort encore animee) -- chez un hote lent, plus
// longtemps que chez l'invite. Une attente bien plus longue y est demandee.
constexpr uint32_t kOrphanWaitSameTurnMs = 10000;

// Regle 1 (invite) : l'hote a fini avec g.turn = `host_turn` ; ce joueur, au tour
// `my_turn`, attend depuis `waited_ms` la decision d'un chat d'un autre joueur.
inline bool orphan_stuck_past_host_end(uint32_t host_turn, uint32_t my_turn, uint64_t waited_ms) {
    return my_turn > host_turn && waited_ms >= kOrphanWaitMs;
}

// Regle 2 (hote) : l'invite a fini avec g.turn = `peer_end_turn` ; l'hote, au tour
// `my_turn`, attend depuis `waited_ms` la decision d'un chat de cet invite (compte
// au plus tot depuis l'arrivee de l'annonce de l'invite).
inline bool orphan_peer_left_behind(uint32_t peer_end_turn, uint32_t my_turn, uint64_t waited_ms) {
    if (my_turn > peer_end_turn)  return waited_ms >= kOrphanWaitMs;
    if (my_turn == peer_end_turn) return waited_ms >= kOrphanWaitSameTurnMs;
    return false;
}

// Regle 3 (invite) : ce joueur a fini avec g.turn = `my_end_turn` ; une empreinte de
// l'hote pour le tour `host_hash_turn` est la (l'hote a fini ce tour-la).
inline bool orphan_host_went_on(uint32_t my_end_turn, uint32_t host_hash_turn) {
    return host_hash_turn > my_end_turn;
}

// MM_BATTLE_END : a = 1 de l'hote (a tous), 2 d'un invite (a l'hote) ; x / y = le
// combat, text[0..3] = g.turn a la fin chez celui qui l'envoie.
constexpr uint8_t kBattleEndFromHost  = 1;
constexpr uint8_t kBattleEndFromGuest = 2;

struct BattleEnd {
    uint64_t battle = 0;
    uint32_t turn   = 0;
    uint8_t  from   = kBattleEndFromHost;
};

inline MmMsg battle_end_encode(const BattleEnd& e) {
    MmMsg m{};
    m.kind = MM_BATTLE_END;
    m.a    = e.from;
    m.x    = (uint32_t)(e.battle & 0xFFFFFFFFull);
    m.y    = (uint32_t)(e.battle >> 32);
    static_assert(sizeof(m.text) >= sizeof(e.turn) + 1, "MmMsg::text trop petit");
    memcpy(m.text, &e.turn, sizeof(e.turn));
    return m;
}

// Faux pour un autre message, une autre forme, ou un combat nul.
inline bool battle_end_decode(const MmMsg& m, BattleEnd& e) {
    if (m.kind != MM_BATTLE_END || (m.a != kBattleEndFromHost && m.a != kBattleEndFromGuest)) return false;
    e.from   = m.a;
    e.battle = (uint64_t)m.x | ((uint64_t)m.y << 32);
    memcpy(&e.turn, m.text, sizeof(e.turn));
    return e.battle != 0;
}

} // namespace mgmp
