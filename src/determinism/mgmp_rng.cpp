#include "mgmp_rng.h"
#include "mgmp_config.h"
#include "mgmp_tuning.h"
#include "mgmp_record.h"
#include "mgmp_log.h"

#include <windows.h>
#include <intrin.h>
#include <cstring>

namespace mgmp {
namespace {

typedef int     (__fastcall* fn_randint)  (int    n, uint64_t* state);
typedef double  (__fastcall* fn_randfloat)(double m, uint64_t* state);
typedef __m128  (__fastcall* fn_rand2)    (double m, uint64_t* state);
typedef bool    (__fastcall* fn_rollchance)(double p, double scale, uint64_t* state);

fn_randint   o_randint   = nullptr;
fn_randfloat o_randfloat = nullptr;
fn_rand2     o_rand2     = nullptr;
fn_rollchance o_rollchance = nullptr;

uintptr_t g_base = 0;

volatile LONG64 g_total  = 0;
volatile LONG64 g_global = 0;

// --- the flight recorder (Mewgenics Multi) ---------------------------------
//
// The battle hash says THAT the simulation streams parted and nothing about
// where. The one rng-only desync seen so far (2026-10-12, turn 9: identical
// actions, identical cat state on both peers, different stream) is exactly the
// case where a log line per action is no help -- every action matched. What
// names the culprit is the list of draws each peer took that turn, side by
// side: the first one that differs, or the one only one peer took, carries the
// call site.
//
// So, while a shared battle runs, every global-stream draw made ON THE GAME'S
// THREAD goes into a ring (the one thread both peers' simulations run on --
// a draw on another thread's TLS stream is not the simulation's), and lockstep
// prints a turn's worth of it when that turn's rng half disagrees. Written and
// read on that thread only, so no locking.
struct FlightDraw {
    uint32_t seq    = 0;
    uint32_t site   = 0;      // caller RVA
    uint64_t s0     = 0;      // state[0] before the draw
    uint64_t result = 0;
    uint8_t  fn     = 0;      // RngFn
};
constexpr uint32_t kFlightSize = 4096;          // a power of two
FlightDraw     g_flight[kFlightSize];
uint32_t       g_flight_seq = 0;                // the next draw's number
volatile DWORD g_flight_tid = 0;                // 0 = not armed

inline bool watching() { return g_flight_tid != 0 || record_active(); }

// Return address -> RVA. Absolute addresses would differ between runs under
// ASLR and make every record differ; the RVA is stable for a pinned build.
inline uint32_t site_rva(void* ret) {
    uintptr_t a = (uintptr_t)ret;
    uintptr_t b = g_base;
    if (!b || a < b) return 0;
    uintptr_t d = a - b;
    return d > 0xFFFFFFFFull ? 0 : (uint32_t)d;
}

// The common tail of all three detours: decide whether this draw matters,
// count it, and record it.
//
// Ordering note: `s0` must be sampled BEFORE the original runs, because the
// original mutates the state in place. That is the entire reason these detours
// cannot be written as a simple "call original, then log".
inline void note(uint8_t fn, void* ret, uint64_t s0, uint64_t result, bool global,
                 const void* state) {
    InterlockedIncrement64(&g_total);
    if (global) InterlockedIncrement64(&g_global);

    if (global && g_flight_tid && GetCurrentThreadId() == g_flight_tid) {
        FlightDraw& d = g_flight[g_flight_seq & (kFlightSize - 1)];
        d.seq    = g_flight_seq;
        d.site   = site_rva(ret);
        d.s0     = s0;
        d.result = result;
        d.fn     = fn;
        ++g_flight_seq;
    }

    if (!record_active()) return;
    if (!global && tune::kRngGlobalOnly) {
        record_note_skipped();
        return;
    }
    record_rng(fn, site_rva(ret), s0, result, global, state);
}

} // namespace

uint64_t* rng_global_stream() {
    // gs:[0x58] is the TEB's ThreadLocalStoragePointer; slot 0 is the main
    // module's static TLS block, and the shared stream sits at +0x178 in it.
    char** tls = (char**)__readgsqword(0x58);
    if (!tls) return nullptr;
    char* block = tls[0];
    if (!block) return nullptr;
    return (uint64_t*)(block + 0x178);
}

// ---- detours --------------------------------------------------------------
//
// Each one reads state[0] first, forwards, then records. `state` is supplied by
// the caller and is always a live 32-byte array, so the read needs no SEH
// guard -- if it were bad the game would already have crashed inside the
// original on the very next instruction.

// Mewgenics Multi : these four are hooked for the whole process now (the
// flight recorder), so the first line of each is the way out when nothing is
// listening -- one load and a branch on the hottest functions in the game,
// instead of the counting and the TLS lookup below.
int __fastcall h_randint(int n, uint64_t* state) {
    if (!watching()) return o_randint(n, state);
    void*    ret = _ReturnAddress();
    bool     glb = (state == rng_global_stream());
    uint64_t s0  = state ? state[0] : 0;
    int      r   = o_randint(n, state);
    note(RNG_INT, ret, s0, (uint64_t)(uint32_t)r, glb, state);
    return r;
}

double __fastcall h_randfloat(double m, uint64_t* state) {
    if (!watching()) return o_randfloat(m, state);
    void*    ret = _ReturnAddress();
    bool     glb = (state == rng_global_stream());
    uint64_t s0  = state ? state[0] : 0;
    double   r   = o_randfloat(m, state);
    uint64_t bits;
    memcpy(&bits, &r, sizeof(bits));
    note(RNG_FLOAT, ret, s0, bits, glb, state);
    return r;
}

__m128 __fastcall h_rand2(double m, uint64_t* state) {
    if (!watching()) return o_rand2(m, state);
    void*    ret = _ReturnAddress();
    bool     glb = (state == rng_global_stream());
    uint64_t s0  = state ? state[0] : 0;
    __m128   r   = o_rand2(m, state);
    uint64_t bits;
    memcpy(&bits, &r, sizeof(bits));       // low half is enough as a checksum
    note(RNG_TWO, ret, s0, bits, glb, state);
    return r;
}

// RollChance(p, scale, state) -- the proc/crit gate.
//
// Recorded even though the draw it makes is ALSO recorded by the rand2 hook
// underneath it. That is the point: this frame is the only one that knows which
// passive rolled, because rand2's return address lands inside RollChance and
// RollChance's lands in the caller. The decoder can pair them by sequence.
//
// When p >= 1.0 it short-circuits and takes no draw at all. That case is still
// recorded (result = 1, s0 unchanged) because "a proc that was guaranteed"
// still has to match between peers -- a peer that thought it was 0.9 would take
// a draw here and desync the stream position, not just the outcome.
bool __fastcall h_rollchance(double p, double scale, uint64_t* state) {
    if (!watching()) return o_rollchance(p, scale, state);
    void*    ret = _ReturnAddress();
    bool     glb = (state == rng_global_stream());
    uint64_t s0  = state ? state[0] : 0;
    bool     r   = o_rollchance(p, scale, state);
    note(RNG_ROLL, ret, s0, (uint64_t)r, glb, state);
    return r;
}

void* rng_detour_randint()   { return (void*)&h_randint;   }
void* rng_detour_randfloat() { return (void*)&h_randfloat; }
void* rng_detour_rand2()     { return (void*)&h_rand2;     }
void* rng_detour_rollchance(){ return (void*)&h_rollchance;}

void** rng_original_randint()   { return (void**)&o_randint;   }
void** rng_original_randfloat() { return (void**)&o_randfloat; }
void** rng_original_rand2()     { return (void**)&o_rand2;     }
void** rng_original_rollchance(){ return (void**)&o_rollchance;}

void rng_counters(uint64_t* total, uint64_t* global) {
    if (total)  *total  = (uint64_t)InterlockedCompareExchange64(&g_total, 0, 0);
    if (global) *global = (uint64_t)InterlockedCompareExchange64(&g_global, 0, 0);
}

void rng_set_base(uintptr_t base) { g_base = base; }

bool rng_flight_arm() {
    const DWORD tid = GetCurrentThreadId();
    if (g_flight_tid == tid) return false;
    g_flight_tid = tid;
    return true;
}

void rng_flight_disarm() { g_flight_tid = 0; }

uint32_t rng_flight_mark() { return g_flight_seq; }

// Safe against the dump: a range [from, to) is only ever read with `to` at or
// below the current number (marks are taken at node entry and turn
// boundaries, never inside a rewound call), and after a rewind to `mark` every
// number in [mark, current) has been rewritten by a real draw; an entry a
// rewind left behind carries a number above the current one, and the
// per-entry check skips it. (The rewound draws also clobbered the slots of
// entries 4096 numbers older: those are skipped too, silently -- many battles
// back.)
void rng_flight_rewind(uint32_t mark) {
    if (mark <= g_flight_seq) g_flight_seq = mark;
}

// Armed. Whether the four hooks are actually in is the hook table's to say
// (hooks_is_live): MinHook fills the trampolines at create time, even when the
// enable that follows fails.
bool rng_flight_live() { return g_flight_tid != 0; }

namespace {
const char* fn_name(uint8_t fn) {
    switch (fn) {
        case RNG_INT:   return "randint";
        case RNG_FLOAT: return "randfloat";
        case RNG_TWO:   return "rand2";
        case RNG_ROLL:  return "RollChance";
        default:        return "?";
    }
}
} // namespace

void rng_flight_dump(uint32_t from, uint32_t to, const char* what) {
    if (!rng_flight_live()) {
        log_line("RNG", "%s: no draw record (the flight recorder is not armed)", what);
        return;
    }
    if (to < from) to = from;
    const uint32_t n      = to - from;
    const uint32_t oldest = g_flight_seq > kFlightSize ? g_flight_seq - kFlightSize : 0;
    const uint32_t start  = from < oldest ? oldest : from;
    log_line("RNG", "%s: %u call-based draw(s) on the simulation stream on this "
                    "peer%s -- diff this list against the other peer's; inlined "
                    "xoshiro steps are not in it",
             what, n, from < oldest ? " (the oldest ones were overwritten)" : "");
    // Long turns are cut in the middle: the first difference is what matters,
    // and the tail shows whether the two lists ever came back into step.
    constexpr uint32_t kHead = 160, kTail = 40;
    for (uint32_t s = start; s < to; ++s) {
        if (to - start > kHead + kTail && s == start + kHead) {
            log_line("RNG", "  ... %u draw(s) not shown ...", to - kTail - s);
            s = to - kTail;
        }
        const FlightDraw& d = g_flight[s & (kFlightSize - 1)];
        if (d.seq != s) continue;
        log_line("RNG", "  #%-4u %-10s Mewgenics.exe+%08X  s0=%016llx -> %016llx",
                 s - from, fn_name(d.fn), d.site,
                 (unsigned long long)d.s0, (unsigned long long)d.result);
    }
}

} // namespace mgmp
