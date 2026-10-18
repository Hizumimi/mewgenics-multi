// test_orphan.cpp -- Mewgenics Multi (protocole 64) : un combat fini chez l'un et
// pas chez l'autre (mgmp_orphan.h). Les deux regles sur les numeros de tour, avec
// les nombres du 18 octobre 2026 (combat e15743b8b1852f54 : l'hote a fini pendant
// son tour 20, l'invite a commence le tour 21), et le message MM_BATTLE_END, y
// compris a travers le codec du fil (enc_mm coupe les zeros de fin du texte).
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

void host_done() {
    printf("l'hote a fini, ce joueur continue-t-il ?\n");
    check(orphan_past_host_end(20, 21), "18/10 : l'hote fini au tour 20, l'invite au tour 21 -- diverge");
    check(!orphan_past_host_end(20, 20), "fini pendant le meme tour : d'accord");
    check(!orphan_past_host_end(20, 7), "un invite en retard (il rattrape) : pas une divergence");
    check(!orphan_past_host_end(0, 0), "un combat fini avant tout tour");
    check(orphan_past_host_end(0, 1), "fini avant tout tour chez l'hote, un tour ici : diverge");
}

void mine_done() {
    printf("ce joueur a fini, l'hote a-t-il continue ?\n");
    check(!orphan_host_went_on(20, 19), "une empreinte de l'hote pour le tour 19 (arrivee en retard) : d'accord");
    check(orphan_host_went_on(20, 20), "l'hote a fini le tour 20, pendant lequel ce joueur avait fini : diverge");
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
    const BattleEnd e{0xe15743b8b1852f54ull, 20};
    const MmMsg m = battle_end_encode(e);
    check(m.kind == MM_BATTLE_END && m.a == 1, "la sorte et la forme");
    check(m.x == 0xb1852f54u && m.y == 0xe15743b8u, "le combat en deux moities");
    BattleEnd d;
    check(battle_end_decode(m, d) && d.battle == e.battle && d.turn == 20, "relu tel quel");

    for (uint32_t turn : {0u, 1u, 20u, 0x100u, 0x01000000u, 0xFFFFFFFFu}) {
        MmMsg back{};
        BattleEnd got;
        const bool ok = wire_round_trip(battle_end_encode(BattleEnd{0x9a06160e18e92b3eull, turn}), back) &&
                        battle_end_decode(back, got) && got.turn == turn && got.battle == 0x9a06160e18e92b3eull;
        char what[96];
        snprintf(what, sizeof(what), "a travers le fil, tour 0x%08X", turn);
        check(ok, what);
    }

    MmMsg other = m;
    other.kind  = MM_GIFT;
    check(!battle_end_decode(other, d), "une autre sorte de message : refuse");
    MmMsg shape = m;
    shape.a     = 2;
    check(!battle_end_decode(shape, d), "une autre forme : refusee");
    check(!battle_end_decode(battle_end_encode(BattleEnd{0, 5}), d), "un combat nul : refuse");
}

} // namespace

int main() {
    host_done();
    mine_done();
    message();
    printf(g_fail ? "\n%d ECHEC(S)\n" : "\ntout est bon\n", g_fail);
    return g_fail ? 1 : 0;
}
