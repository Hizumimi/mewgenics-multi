// mm_recruit_core -- Mewgenics Multi : a qui va une recrue. Voir mm_recruit_core.h.

#include "mm_recruit_core.h"

namespace mgmp {

int recruit_owner(const std::vector<RecruitPlayer>& players, uint32_t (*roll)(uint32_t n, void* ctx), void* ctx) {
    if (players.empty()) return -1;
    std::vector<const RecruitPlayer*> pick;
    // Les spectateurs : sans chat depuis le plus longtemps.
    for (const RecruitPlayer& p : players) {
        if (p.living > 0) continue;
        if (pick.empty() || p.catless_since < pick[0]->catless_since) pick.assign(1, &p);
        else if (p.catless_since == pick[0]->catless_since) pick.push_back(&p);
    }
    // Aucun : le moins de chats vivants.
    if (pick.empty())
        for (const RecruitPlayer& p : players) {
            if (pick.empty() || p.living < pick[0]->living) pick.assign(1, &p);
            else if (p.living == pick[0]->living) pick.push_back(&p);
        }
    if (pick.size() == 1 || !roll) return pick[0]->peer;
    const uint32_t i = roll((uint32_t)pick.size(), ctx);
    return pick[i < pick.size() ? i : 0]->peer;
}

} // namespace mgmp
