// mm_ai.cpp -- Mewgenics Multi : l'IA du jeu pour les chats d'un absent. Voir
// mm_ai.h pour la regle et ce que l'appel ne doit pas laisser.

#include "mm_ai.h"

#include "mgmp_addresses.h"
#include "mgmp_resolve.h"
#include "mgmp_lockstep.h"   // la cloture d'etat
#include "mgmp_rng.h"
#include "mgmp_mem.h"
#include "mgmp_log.h"

#include <windows.h>
#include <cstring>

namespace mgmp {
namespace {

// Le Character du cerveau : kBrain_Character (mgmp_addresses.h, Brain+0x38).
constexpr uintptr_t kBrain_AutoOrient = 0x2B1;   // « auto_orient » (Brain init, 0x14013487D)
constexpr uintptr_t kPassive_Char     = 0x38;    // ce que l'entree 112 lit du passif
constexpr size_t    kTA_FuncBuf       = 0x40;    // la std::function de la TurnAction :
constexpr size_t    kTA_FuncImpl      = 0x78;    // son tampon, puis son _Impl (nul = vide)

using FnAiChoice   = void* (__fastcall*)(void* passive, void* out);
using FnDeleteThis = void  (__fastcall*)(void* impl, bool dealloc);

uint32_t g_calls   = 0;
uint32_t g_rewinds = 0;
uint32_t g_fence   = 0;
bool     g_said_missing = false;

// L'appel, puis la std::function de la reponse detruite comme le fait le jeu
// (0x140082BB1) : _Impl->_Delete_this(_Impl != tampon local), entree 4 de sa
// vtable. Une fonction a part, sans objet C++ : __try l'exige.
bool seh_choose(FnAiChoice fn, void* passive, uint8_t* out) {
    __try {
        fn(passive, out);
        void* impl = *(void**)(out + kTA_FuncImpl);
        if (impl) {
            void** vt = *(void***)impl;
            ((FnDeleteThis)vt[4])(impl, impl != (void*)(out + kTA_FuncBuf));
            *(void**)(out + kTA_FuncImpl) = nullptr;
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

} // namespace

bool ai_available() {
    const bool ok = addr_of_call(C_AiChoice) != 0;
    if (!ok && !g_said_missing) {
        g_said_missing = true;
        log_line("AI", "!! l'IA du jeu (sub_140082B00) est introuvable dans cet executable -- "
                       "le joueur de l'hote jouera les chats des absents, comme avant");
    }
    return ok;
}

bool ai_choose(void* brain, TurnAction& out, const char** why) {
    if (why) *why = nullptr;
    memset(&out, 0, sizeof(out));

    const FnAiChoice fn = (FnAiChoice)addr_of_call(C_AiChoice);
    if (!fn) {
        if (why) *why = "l'IA du jeu est introuvable";
        return false;
    }
    void* ch = nullptr;
    if (!brain || !mem_read((const uint8_t*)brain + kBrain_Character, &ch, sizeof(ch)) || !ch) {
        if (why) *why = "le cerveau n'a pas de chat";
        return false;
    }

    // Le faux passif : seul son Character est lu.
    alignas(16) uint8_t passive[0x40] = {};
    memcpy(passive + kPassive_Char, &ch, sizeof(ch));
    // La TurnAction rendue (0x88 octets ; la std::function y est construite par
    // le jeu, alignee comme sur sa pile).
    alignas(16) uint8_t buf[sizeof(TurnAction)] = {};

    // Pas d'orientation de fin de tour pendant l'appel.
    uint8_t* orient_at = (uint8_t*)brain + kBrain_AutoOrient;
    uint8_t  orient    = 0;
    const bool orient_off = mem_read(orient_at, &orient, 1) && orient != 0;
    if (orient_off) {
        const uint8_t zero = 0;
        mem_write(orient_at, &zero, 1);
    }

    // Le flux de la simulation epingle, l'etat des chats sous cloture.
    uint64_t* s = rng_global_stream();
    uint64_t  before[4] = {};
    if (s) memcpy(before, s, sizeof(before));
    const uint32_t flight = rng_flight_mark();
    lockstep_state_fence_begin();

    const bool ok = seh_choose(fn, passive, buf);
    ++g_calls;

    // La cloture ne le dit qu'une fois par session, et cette fois peut deja avoir
    // servi a l'apercu de la visee (mgmp_aim) : ici, a part.
    if (const uint32_t moved = lockstep_state_fence_end("l'IA du jeu (le chat d'un absent)")) {
        if (g_fence++ == 0)
            log_line_lvl(LogLevel::Error, "AI", "!! l'IA du jeu a change l'etat de %u chat(s) chez l'hote "
                                                "(PV, case, elements ou mort) -- les invites ne l'ont pas : "
                                                "le combat va se desynchroniser", moved);
    }
    if (s && memcmp(before, s, sizeof(before)) != 0) {
        memcpy(s, before, sizeof(before));
        rng_flight_rewind(flight);
        ++g_rewinds;
    }
    if (orient_off) mem_write(orient_at, &orient, 1);

    if (!ok) {
        if (why) *why = "l'IA du jeu a plante (exception attrapee)";
        return false;
    }
    memcpy(&out, buf, sizeof(out));
    return true;
}

uint32_t ai_calls()      { return g_calls; }
uint32_t ai_rewinds()    { return g_rewinds; }
uint32_t ai_fence_hits() { return g_fence; }

void ai_reset_counters() {
    g_calls   = 0;
    g_rewinds = 0;
    g_fence   = 0;
}

} // namespace mgmp
