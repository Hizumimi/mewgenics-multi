// test_orphan.cpp -- Mewgenics Multi (protocoles 64-66) : un combat fini chez l'un
// et pas chez l'autre (mgmp_orphan.h). Les trois regles sur les numeros de tour et
// l'attente, avec les nombres du 18 octobre 2026 (combat e15743b8b1852f54 : l'hote
// a fini a son tour 20, l'invite attendait au tour 21 la decision d'un chat de
// l'hote), et le message MM_BATTLE_END dans ses deux formes, y compris a travers le
// codec du fil (enc_mm coupe les zeros de fin du texte).
#include "mgmp_orphan.h"

#include <cstdio>
#include <initializer_list>

using namespace mgmp;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}

void rule1() {
    printf("regle 1 : l'hote a fini, ce joueur attend plus loin\n");
    check(orphan_stuck_past_host_end(20, 21, 5000), "18/10 : fini a 20 chez l'hote, attente au tour 21 -- diverge");
    check(!orphan_stuck_past_host_end(20, 21, kOrphanWaitMs - 1), "une attente plus courte que kOrphanWaitMs : rien encore");
    check(orphan_stuck_past_host_end(20, 21, kOrphanWaitMs), "a kOrphanWaitMs tout juste");
    check(!orphan_stuck_past_host_end(20, 20, 60000), "au tour de la fin de l'hote : pas plus loin, rien");
    check(!orphan_stuck_past_host_end(20, 7, 60000), "un invite en retard (il rattrape) : pas une divergence");
    check(!orphan_stuck_past_host_end(20, 21, 0), "sans attente (une copie qui finit d'elle-meme) : rien");
}

void rule2() {
    printf("regle 2 : un invite a fini, l'hote attend un de ses chats\n");
    check(orphan_peer_left_behind(20, 21, 3000), "l'hote attend un tour apres la fin de l'invite -- diverge");
    check(orphan_peer_left_behind(20, 25, kOrphanWaitMs), "et plus loin, a kOrphanWaitMs tout juste");
    check(!orphan_peer_left_behind(20, 21, kOrphanWaitMs - 1), "une attente courte : rien encore");
    check(!orphan_peer_left_behind(20, 20, 3000), "au tour meme de la fin : 3 s ne suffisent pas (une victoire lente)");
    check(!orphan_peer_left_behind(20, 20, kOrphanWaitSameTurnMs - 1), "ni presque 10 s");
    check(orphan_peer_left_behind(20, 20, kOrphanWaitSameTurnMs), "10 s au tour meme : diverge (l'hote fige)");
    check(!orphan_peer_left_behind(20, 19, 600000), "l'hote en retard sur l'invite : ses decisions sont dans la file");
}

void rule3() {
    printf("regle 3 : ce joueur a fini, l'hote a continue\n");
    check(!orphan_host_went_on(20, 19), "une empreinte de l'hote pour le tour 19 (arrivee en retard) : d'accord");
    check(!orphan_host_went_on(20, 20), "le tour 20 seulement : une fin de tour de plus, la marge");
    check(orphan_host_went_on(20, 21), "l'hote a fini 20 et 21 -- il a continue sans ce joueur");
    check(orphan_host_went_on(20, 31), "et bien apres");
    check(!orphan_host_went_on(1, 0), "l'empreinte du debut du combat (tour 0)");
}

bool wire_round_trip(const MmMsg& in, MmMsg& out) {
    uint8_t buf[256];
    const uint32_t n = enc_mm(buf, sizeof(buf), in);
    if (!n) return false;
    Reader r(buf, n);
    if (r.u8v() != MSG_MM) return false;
    return dec_mm(r, out);
}

void message() {
    printf("MM_BATTLE_END\n");
    const BattleEnd e{0xe15743b8b1852f54ull, 20, kBattleEndFromHost};
    const MmMsg m = battle_end_encode(e);
    check(m.kind == MM_BATTLE_END && m.a == kBattleEndFromHost, "la sorte et la forme (de l'hote)");
    check(m.x == 0xb1852f54u && m.y == 0xe15743b8u, "le combat en deux moities");
    BattleEnd d;
    check(battle_end_decode(m, d) && d.battle == e.battle && d.turn == 20 && d.from == kBattleEndFromHost,
          "relu tel quel");
    const MmMsg g2 = battle_end_encode(BattleEnd{0xe15743b8b1852f54ull, 21, kBattleEndFromGuest});
    check(g2.a == kBattleEndFromGuest && battle_end_decode(g2, d) && d.from == kBattleEndFromGuest && d.turn == 21,
          "la forme d'un invite (a = 2)");

    for (uint32_t turn : {0u, 1u, 20u, 0x100u, 0x01000000u, 0xFFFFFFFFu}) {
        for (uint8_t from : {kBattleEndFromHost, kBattleEndFromGuest}) {
            MmMsg back{};
            BattleEnd got;
            const bool ok = wire_round_trip(battle_end_encode(BattleEnd{0x9a06160e18e92b3eull, turn, from}), back) &&
                            battle_end_decode(back, got) && got.turn == turn &&
                            got.battle == 0x9a06160e18e92b3eull && got.from == from;
            char what[96];
            snprintf(what, sizeof(what), "a travers le fil, tour 0x%08X, forme %u", turn, (unsigned)from);
            check(ok, what);
        }
    }

    MmMsg other = m;
    other.kind  = MM_GIFT;
    check(!battle_end_decode(other, d), "une autre sorte de message : refuse");
    MmMsg shape = m;
    shape.a     = 3;
    check(!battle_end_decode(shape, d), "une autre forme : refusee");
    shape.a     = 0;
    check(!battle_end_decode(shape, d), "la forme 0 : refusee");
    check(!battle_end_decode(battle_end_encode(BattleEnd{0, 5, kBattleEndFromHost}), d), "un combat nul : refuse");
}

} // namespace

int main() {
    rule1();
    rule2();
    rule3();
    message();
    printf(g_fail ? "\n%d ECHEC(S)\n" : "\ntout est bon\n", g_fail);
    return g_fail ? 1 : 0;
}
