// test_ai.cpp -- Mewgenics Multi : le rythme et les garde-fous de l'IA qui joue
// les chats d'un absent (mm_ai_core.h).
#include "mm_ai_core.h"

#include <cstdio>

using namespace mgmp;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}

} // namespace

int main() {
    printf("-- le rythme d'un tour --\n");
    {
        AiPacer p;
        const uint64_t B = 0x1234;
        check(p.poll(1000, B, 3, 2) == AiStep::Wait, "the first ask waits");
        check(p.poll(1000 + kAiThinkMs - 1, B, 3, 2) == AiStep::Wait, "still waiting just before the delay");
        check(p.poll(1000 + kAiThinkMs, B, 3, 2) == AiStep::Decide, "decides once the delay is over");
        p.took();
        check(p.taken == 1, "one decision counted");
        check(p.poll(5000, B, 3, 2) == AiStep::Wait, "the next decision waits again");
        check(p.poll(5000 + kAiThinkMs, B, 3, 2) == AiStep::Decide, "...then decides");
        p.took();
        // Un autre chat, ou un autre tour : on repart de zero.
        check(p.poll(9000, B, 4, 2) == AiStep::Wait && p.taken == 0, "a new turn starts from zero");
        check(p.poll(9000 + kAiThinkMs, B, 4, 2) == AiStep::Decide, "and decides after the delay");
        check(p.poll(9500, B, 4, 3) == AiStep::Wait && p.taken == 0, "another cat starts from zero");
        check(p.poll(9500, B + 1, 4, 3) == AiStep::Wait && p.battle == B + 1, "another battle starts from zero");
    }

    printf("\n-- le plafond de decisions par tour --\n");
    {
        AiPacer p;
        uint64_t now = 100;
        uint32_t decided = 0;
        bool ended = false;
        for (uint32_t i = 0; i < kAiMaxActions + 5 && !ended; ++i) {
            p.poll(now, 7, 1, 0);
            now += kAiThinkMs;
            const AiStep s = p.poll(now, 7, 1, 0);
            if (s == AiStep::Decide) { ++decided; p.took(); }
            else if (s == AiStep::EndTurn) ended = true;
            now += 10;
        }
        check(decided == kAiMaxActions, "exactly the cap's worth of decisions");
        check(ended, "then the turn ends");
    }

    printf("\n-- une horloge de 0 ne bloque pas --\n");
    {
        AiPacer p;
        check(p.poll(0, 1, 0, 0) == AiStep::Wait && p.since == 1, "a zero clock still starts the wait");
        check(p.poll(kAiThinkMs + 1, 1, 0, 0) == AiStep::Decide, "and still ends it");
    }

    printf("\n-- ce qui part --\n");
    {
        const char* why = nullptr;
        check(ai_verdict(3, SLOT_NONE, &why) == AiVerdict::Use && !why, "an end of turn goes as is");
        check(ai_verdict(2, SLOT_MOVE, &why) == AiVerdict::Use, "a move goes");
        check(ai_verdict(2, SLOT_ATTACK, &why) == AiVerdict::Use, "an attack goes");
        check(ai_verdict(2, SLOT_BONUS, &why) == AiVerdict::Use, "the bonus ability goes");
        check(ai_verdict(2, SLOT_SPELL, &why) == AiVerdict::Use, "a spell goes");
        check(ai_verdict(2, SLOT_UNKNOWN, &why) == AiVerdict::EndTurn && why, "an ability in no slot ends the turn");
        check(ai_verdict(2, SLOT_NONE, &why) == AiVerdict::EndTurn && why, "an ability-less action ends the turn");
        check(ai_verdict(0, SLOT_NONE, &why) == AiVerdict::EndTurn && why, "nothing chosen ends the turn");
        check(ai_verdict(7, SLOT_NONE, &why) == AiVerdict::EndTurn && why, "a std::function action ends the turn");
        check(ai_verdict(1, SLOT_NONE, nullptr) == AiVerdict::EndTurn, "no reason asked: still safe");
    }

    printf("\n%s (%d failure%s)\n", g_fail ? "FAILED" : "PASSED", g_fail, g_fail == 1 ? "" : "s");
    return g_fail ? 1 : 0;
}
