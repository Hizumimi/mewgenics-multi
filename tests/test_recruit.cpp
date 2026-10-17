// Mewgenics Multi : a qui va une recrue (mm_recruit_core).
#include "mm_recruit_core.h"

#include <cstdio>
#include <vector>

static int g_fail = 0;
#define CHECK(c) do { if (!(c)) { ++g_fail; std::printf("  ECHEC ligne %d : %s\n", __LINE__, #c); } } while (0)

using namespace mgmp;

static uint32_t g_next = 0;
static uint32_t roll_fixed(uint32_t n, void*) { return g_next % n; }

int main() {
    // Personne.
    CHECK(recruit_owner({}, roll_fixed, nullptr) == -1);
    // Un spectateur : il l'a, meme si un autre a moins de chats que les autres.
    std::vector<RecruitPlayer> p = { {0, 2, 0}, {1, 0, 5000}, {2, 1, 0} };
    CHECK(recruit_owner(p, roll_fixed, nullptr) == 1);
    // Deux spectateurs : celui qui est sans chat depuis le plus longtemps.
    p = { {0, 2, 0}, {1, 0, 9000}, {2, 0, 4000} };
    CHECK(recruit_owner(p, roll_fixed, nullptr) == 2);
    // Egalite (venus sans chat au meme moment) : au sort.
    p = { {0, 1, 0}, {1, 0, 4000}, {2, 0, 4000}, {3, 0, 4000} };
    g_next = 0; CHECK(recruit_owner(p, roll_fixed, nullptr) == 1);
    g_next = 1; CHECK(recruit_owner(p, roll_fixed, nullptr) == 2);
    g_next = 2; CHECK(recruit_owner(p, roll_fixed, nullptr) == 3);
    g_next = 7; CHECK(recruit_owner(p, roll_fixed, nullptr) == 2);       // 7 % 3
    // Aucun spectateur : le moins de chats vivants ; egalite au sort.
    p = { {0, 2, 0}, {1, 1, 0}, {2, 3, 0} };
    CHECK(recruit_owner(p, roll_fixed, nullptr) == 1);
    p = { {0, 1, 0}, {1, 2, 0}, {2, 1, 0} };
    g_next = 0; CHECK(recruit_owner(p, roll_fixed, nullptr) == 0);
    g_next = 1; CHECK(recruit_owner(p, roll_fixed, nullptr) == 2);
    // Sans tirage : le premier.
    CHECK(recruit_owner(p, nullptr, nullptr) == 0);
    // Seul joueur : lui.
    p = { {0, 4, 0} };
    CHECK(recruit_owner(p, roll_fixed, nullptr) == 0);
    if (g_fail) { std::printf("%d echec(s)\n", g_fail); return 1; }
    std::printf("ok\n");
    return 0;
}
