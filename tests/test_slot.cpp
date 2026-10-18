// test_slot.cpp -- Mewgenics Multi (protocole 58) : la 4e place a 3 joueurs
// (mm_slot_core.h). Le premier qui met un deuxieme chat dans sa boite la prend et
// la garde tant qu'il en a deux ; sans volontaire, l'equipe part a 3 chats ; a 2
// ou 4 joueurs, il n'y a pas de place libre.
#include "mm_slot_core.h"

#include <cstdio>

using namespace mgmp;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}

constexpr uint32_t mask(int a, int b = -1, int c = -1, int d = -1) {
    return (a >= 0 ? 1u << a : 0u) | (b >= 0 ? 1u << b : 0u) | (c >= 0 ? 1u << c : 0u) |
           (d >= 0 ? 1u << d : 0u);
}

void quotas() {
    printf("quotas\n");
    check(slot_quota(0, 1, kSlotNobody, 4) == 4, "seul : toute l'equipe");
    check(slot_quota(0, 2, kSlotNobody, 4) == 2 && slot_quota(1, 2, kSlotNobody, 4) == 2, "a 2 : 2 chacun");
    check(slot_quota(0, 3, kSlotNobody, 4) == 1 && slot_quota(2, 3, kSlotNobody, 4) == 1,
          "a 3 sans volontaire : 1 chacun (3 chats)");
    check(slot_quota(2, 3, 2, 4) == 2 && slot_quota(0, 3, 2, 4) == 1 && slot_quota(1, 3, 2, 4) == 1,
          "a 3 : 2 pour celui qui a la place, 1 pour les autres");
    check(slot_quota(1, 4, 1, 4) == 1, "a 4 : 1 chacun, meme avec une place notee");
    int total = 0;
    for (uint8_t p = 0; p < 3; ++p) total += slot_quota(p, 3, 1, 4);
    check(total == 4, "a 3 avec un volontaire : 4 places en tout");
}

void first_claim_wins() {
    printf("le premier qui met un 2e chat la prend\n");
    SlotArbiter a;
    check(!a.settle(3, mask(0, 1, 2)) && a.owner == kSlotNobody, "personne ne la demande : a personne");
    a.claim(2);
    check(a.settle(3, mask(0, 1, 2)) && a.owner == 2, "le joueur 2 la demande : elle est a lui");
    a.claim(1);
    check(!a.settle(3, mask(0, 1, 2)) && a.owner == 2, "le joueur 1 la demande ensuite : elle reste au 2");
    a.claim(2);
    check(a.count == 2 && a.order[0] == 2, "une demande refaite garde son rang");
    a.release(2);
    check(a.settle(3, mask(0, 1, 2)) && a.owner == 1, "le 2 retire son chat : elle passe au 1, qui attendait");
    a.release(1);
    check(a.settle(3, mask(0, 1, 2)) && a.owner == kSlotNobody, "plus personne : a personne");
}

void host_counts_too() {
    printf("l'hote est un joueur comme les autres\n");
    SlotArbiter a;
    a.claim(1);
    a.claim(0);
    a.settle(3, mask(0, 1, 2));
    check(a.owner == 1, "l'invite a demande avant l'hote : elle est a l'invite");
}

void players_change() {
    printf("le nombre de joueurs change\n");
    SlotArbiter a;
    a.claim(1);
    check(a.settle(2, mask(0, 1)) == false && a.owner == kSlotNobody, "a 2 : pas de place libre");
    check(a.settle(3, mask(0, 1, 2)) && a.owner == 1, "un 3e arrive : la demande d'avant compte");
    check(a.settle(4, mask(0, 1, 2, 3)) && a.owner == kSlotNobody, "un 4e arrive : plus de place libre");
    check(a.wants(1), "... la demande reste notee");
    check(a.settle(3, mask(0, 1, 2)) && a.owner == 1, "il repart : elle revient au 1");
}

void departed_player() {
    printf("un joueur part\n");
    SlotArbiter a;
    a.claim(2);
    a.claim(1);
    a.settle(3, mask(0, 1, 2));
    check(a.owner == 2, "la place est au 2");
    // Le 2 part, un autre (3) arrive : toujours 3 joueurs.
    check(a.settle(3, mask(0, 1, 3)) && a.owner == 1, "le 2 part : elle passe au 1");
    check(!a.wants(2), "... et la demande du 2 est oubliee");
    a.claim(2);
    check(!a.settle(3, mask(0, 1, 2)) && a.owner == 1, "le 2 revient et la redemande : en dernier");
}

void bounds() {
    printf("bornes\n");
    SlotArbiter a;
    a.claim(8);
    a.claim(200);
    check(a.count == 0, "un numero hors des 8 joueurs est ignore");
    for (uint8_t p = 0; p < 8; ++p) a.claim(p);
    a.claim(3);
    check(a.count == 8, "8 demandes au plus, sans doublon");
    a.reset();
    check(a.count == 0 && a.owner == kSlotNobody, "reset");
}

} // namespace

int main() {
    quotas();
    first_claim_wins();
    host_counts_too();
    players_change();
    departed_player();
    bounds();
    if (g_fail) {
        printf("%d FAILED\n", g_fail);
        return 1;
    }
    printf("all ok\n");
    return 0;
}
