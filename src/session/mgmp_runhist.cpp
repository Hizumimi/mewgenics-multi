// mgmp_runhist -- see mgmp_runhist.h for why the run history has to be synced.
#include "mgmp_runhist.h"

#include "mgmp_addresses.h"
#include "mgmp_resolve.h"
#include "mgmp_bytestream.h"
#include "mgmp_config.h"
#include "mgmp_tuning.h"
#include "mgmp_lockstep.h"   // lockstep_in_battle
#include "mgmp_savefile.h"   // savefile_in_coop (Mewgenics Multi)
#include "mgmp_follow.h"     // follow_epoch, follow_apply_bound (Mewgenics Multi)
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"

#include <windows.h>
#include <cstdlib>
#include <cstring>

namespace mgmp {
namespace {

typedef void  (__fastcall* fn_serialize_hist)(void* hist, void* stream);
typedef void  (__fastcall* fn_bs_dtor)(void* stream);
typedef void* (__fastcall* fn_ofstream_ctor)(void* self);

struct State {
    uintptr_t base     = 0;
    bool      resolved = false;

    fn_serialize_hist serialize = nullptr;
    fn_bs_dtor        bs_dtor   = nullptr;
    fn_ofstream_ctor  of_ctor   = nullptr;
    const void**      director_slot = nullptr;

    bool on        = false;
    bool is_client = false;
    bool announced = false;
    bool declined_in_battle = false;
    bool held_in_battle     = false;

    uint64_t sent_hash = 0;      // what we last put on the wire

    // A history that arrived at a moment this peer must not write it. Whole
    // state, not a delta, so a newer one simply replaces the older.
    // Mewgenics Multi : of the same arrival rank (follow_epoch) only -- the
    // history before a node this peer has not entered yet is what that node
    // rolls its event from, and the one after it must not replace it.
    struct Pend {
        uint32_t epoch = 0;
        uint32_t arrival = 0;   // Mewgenics Multi (protocole 60) : follow_note_arrival a son arrivee
        uint8_t* data  = nullptr;
        uint32_t size  = 0;
        uint64_t hash  = 0;
    };
    // Mewgenics Multi (protocole 59) : une de plus que la file des cases de
    // mgmp_follow (8) -- l'etat d'avant chaque case en attente, et celui d'apres
    // la derniere. A 4, un invite en retard de 5 cases appliquait avant la case
    // N+4 l'etat d'avant N+3 (celui d'avant N+4 avait ete remplace).
    static constexpr uint32_t kMaxPend = 9;
    Pend     pend[kMaxPend];
    uint32_t pend_count = 0;

    uint32_t pushed = 0, applied = 0, skipped = 0, deferred = 0, coalesced = 0;

    // Mewgenics Multi (protocole 58) : l'historique a l'entree de la case en cours
    // (runhist_publish_entry), pour un joueur qui arrive en pleine case.
    uint8_t* entry_data = nullptr;
    uint32_t entry_size = 0;
    uint64_t entry_hash = 0;
};

State g;

// --- SEH shims --------------------------------------------------------------
//
// Same rule as mgmp_catsync: every call into the game goes through a pointer
// whose meaning is recovered rather than declared, so each raw call sits in its
// own function with no C++ objects in scope and a __try around it. A wrong
// guess must produce a bad log line, never a crash in the game process.

bool safe_ofstream_ctor(void* p) {
    __try { g.of_ctor(p); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_bs_dtor(void* p) {
    __try { g.bs_dtor(p); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_serialize(void* hist, void* stream) {
    __try { g.serialize(hist, stream); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool bs_construct(uint8_t* bs) {
    memset(bs, 0, kByteStreamSize);
    if (!safe_ofstream_ctor(bs + kBS_Ofstream)) return false;
    // Both endian words stay 0, so read() compares 0 != 0 and never byte-swaps.
    *(uint32_t*)(bs + kBS_SwapElem) = 8;
    return true;
}

uint64_t fnv1a(const void* p, uint32_t n) {
    const uint8_t* b = (const uint8_t*)p;
    uint64_t h = 0xCBF29CE484222325ull;
    for (uint32_t i = 0; i < n; ++i) { h ^= b[i]; h *= 0x100000001B3ull; }
    return h;
}

// The object is the POINTER STORED at MewDirector+1424, dereferenced -- both of
// the game's own call sites do `mov rcx, [rsi+590h]`. Handing the serializer
// the address of the field instead would give it a pointer to a pointer.
void* run_history() {
    if (!g.director_slot) return nullptr;
    const void* dir = nullptr;
    if (!mem_read(g.director_slot, &dir, sizeof(dir)) || !dir) return nullptr;
    void* hist = nullptr;
    if (!mem_read((const uint8_t*)dir + kDir_RunHistory, &hist, sizeof(hist))) return nullptr;
    return hist;
}

// Serializes the run history into a caller-owned buffer. Returns 0 on any
// failure; `out` must be freed with free().
uint32_t serialize_hist(uint8_t** out) {
    *out = nullptr;
    void* hist = run_history();
    if (!hist) return 0;

    uint8_t bs[kByteStreamSize];
    if (!bs_construct(bs)) return 0;

    *(uint32_t*)(bs + kBS_Mode) = 1;             // write, growing the buffer
    uint32_t n = 0;
    if (safe_serialize(hist, bs)) {
        const void* buf = *(const void**)(bs + kBS_WriteBuf);
        uint32_t    len = *(uint32_t*)(bs + kBS_WriteLen);
        if (buf && len && len <= kMaxRunHistBytes) {
            uint8_t* copy = (uint8_t*)malloc(len);
            // Copy out BEFORE the destructor runs: that buffer belongs to the
            // game's heap and the destructor frees it there.
            if (copy && mem_read(buf, copy, len)) { *out = copy; n = len; }
            else if (copy) free(copy);
        }
    }
    safe_bs_dtor(bs);
    return n;
}

// From the CONFIG rather than the live session, for the reason mgmp_catsync
// gives: the host publishes at its first map node, which can be before a peer
// has finished connecting.
//
// NOT LATCHED -- see the note on mgmp_savefile's ensure_state.
void ensure_state() {
    const Config& c = config();
    bool host   = _stricmp(c.net_role, "host")   == 0;
    bool client = _stricmp(c.net_role, "client") == 0;
    g.is_client = client;
    g.on = tune::kRunHist && (host || client) && g.resolved;

    static bool said = false;
    if (!g.on && tune::kRunHist && (host || client) && !said) {
        said = true;
        log_line("RUNHIST", "!! disabled: the game function it calls did not verify "
                            "against this build");
    }
}

} // namespace

// Defined with the rest of the hold machinery, below the publish path; declared
// here because runhist_shutdown drains the hold and is written above it.
static void free_pending();

// ---------------------------------------------------------------------------

void runhist_set_base(uintptr_t base) {
    g.base = base;
    g.resolved = false;

    // Scoped to the three calls THIS module makes, the way catsync is: drift in
    // an address belonging to another module must not report itself as run
    // history being off.
    static const int kNeed[] = { C_RunHistSerialize, C_ByteStreamDtor, C_OfstreamCtor };
    for (int idx : kNeed) {
        if (!addr_of_call((Call)idx)) {
            log_line("RUNHIST", "!! %s did not resolve by signature "
                                "-- run-history sync is OFF", kCalls[idx].name);
            return;
        }
    }

    g.serialize     = (fn_serialize_hist)addr_of_call(C_RunHistSerialize);
    g.bs_dtor       = (fn_bs_dtor)       addr_of_call(C_ByteStreamDtor);
    g.of_ctor       = (fn_ofstream_ctor) addr_of_call(C_OfstreamCtor);
    g.director_slot = (const void**)addr_of_data(D_MewDirectorPtr);
    g.resolved = true;
}

void runhist_init() {
    ensure_state();
    if (!g.on || g.announced) return;
    g.announced = true;
    log_line_lvl(LogLevel::Trace, "RUNHIST", "armed -- %s",
             g.is_client ? "this peer's run history will be overwritten with the host's"
                         : "this peer's run history will be published to the client");
}

static void free_entry() {
    free(g.entry_data);
    g.entry_data = nullptr;
    g.entry_size = 0;
    g.entry_hash = 0;
}

void runhist_shutdown() {
    const bool stranded = g.pend_count != 0;
    free_pending();
    free_entry();
    if (!g.announced) return;
    log_line_lvl(stranded ? LogLevel::Warn : LogLevel::Trace, "RUNHIST",
             "done: %u pushed, %u applied, %u unchanged, "
             "%u held (%u superseded)%s",
             g.pushed, g.applied, g.skipped, g.deferred, g.coalesced,
             stranded ? ", ONE STILL HELD at exit" : "");
}

void runhist_forget() {
    ensure_state();
    if (!g.on || g.is_client) return;
    g.sent_hash = 0;
}

uint64_t runhist_hash() {
    ensure_state();
    // Gated on the CALL TARGET being resolved, deliberately NOT on g.on.
    //
    // g.on is the sync switch. If turning the push off also silenced the hash,
    // then net_runhist = 0 would give both peers a hist_hash of 0, they would
    // compare equal, and mgmp_nodehash would print AGREES over histories that
    // had genuinely drifted apart -- a false pass produced by the very switch
    // somebody would flip while investigating this. Detection must survive its
    // subject being disabled.
    if (!g.resolved) return 0;
    uint8_t* bytes = nullptr;
    uint32_t n = serialize_hist(&bytes);
    if (!n) return 0;
    uint64_t h = fnv1a(bytes, n);
    free(bytes);
    return h;
}

static void publish_impl(const char* why, bool entry);

void runhist_publish(const char* why)       { publish_impl(why, false); }
void runhist_publish_entry(const char* why) { publish_impl(why, true); }

bool runhist_send_entry_to(uint8_t peer) {
    ensure_state();
    if (!g.on || g.is_client || !net_active() || !g.entry_data) return false;
    RunHistMsg m{};
    m.size = g.entry_size;
    m.hash = g.entry_hash;
    m.data = g.entry_data;
    const bool ok = net_send_runhist_to(peer, m);
    log_line("RUNHIST", "-> run history as it was when the host entered this node (%u bytes) sent to peer %u%s",
             m.size, (unsigned)peer, ok ? "" : " -- FAILED");
    return ok;
}

static void publish_impl(const char* why, bool entry) {
    ensure_state();
    if (!g.on || g.is_client) return;
    // Prise meme sans personne de connecte (voir catsync_publish_entry).
    if (entry) free_entry();
    const bool online = net_active();
    if (!online && !entry) return;

    uint8_t* bytes = nullptr;
    uint32_t n = serialize_hist(&bytes);
    if (!n) {
        // TWO CAUSES, AND THEY MUST NOT LOOK THE SAME.
        //
        // No history object means no adventure is loaded -- ordinary on the
        // save-selection screen, silent on purpose, the same silence
        // catsync_publish keeps when the run has no cat list yet.
        //
        // A history object that will not serialize is a real failure, and if it
        // stayed silent this module would do nothing at all for a whole session
        // while every log line said it was armed.
        if (run_history()) {
            static bool warned = false;
            if (!warned) {
                warned = true;
                log_line("RUNHIST", "!! the run history exists but would not "
                                    "serialize -- NOTHING is being published, and "
                                    "the peers' used-event lists will drift apart "
                                    "exactly as they did before this existed (%s)",
                         why);
            }
        }
        return;
    }

    RunHistMsg m{};
    m.size = n;
    m.hash = fnv1a(bytes, n);
    m.data = bytes;

    if (entry) {
        g.entry_data = (uint8_t*)malloc(n);
        if (g.entry_data) {
            memcpy(g.entry_data, bytes, n);
            g.entry_size = n;
            g.entry_hash = m.hash;
        }
    }
    if (!online) { free(bytes); return; }

    if (m.hash == g.sent_hash) { ++g.skipped; free(bytes); return; }

    if (net_send_runhist(m)) {
        g.sent_hash = m.hash;
        ++g.pushed;
        log_line("RUNHIST", "-> run history changed, %u bytes sent (%s)", n, why);
    }
    free(bytes);
}

// --- applying, and holding until it is safe to apply ------------------------

static void apply_now(const RunHistMsg& m, const char* when) {
    if (fnv1a(m.data, m.size) != m.hash) {
        log_line("RUNHIST", "!! the run history failed its hash -- not applied");
        return;
    }

    void* hist = run_history();
    if (!hist) return;                    // no adventure loaded yet

    uint8_t bs[kByteStreamSize];
    if (!bs_construct(bs)) return;

    *(uint32_t*)(bs + kBS_Mode)       = 0;          // read
    *(const void**)(bs + kBS_ReadBuf) = m.data;     // BORROWED...
    *(uint8_t*)(bs + kBS_ReadOwns)    = 0;          // ...so the game must not free it
    *(uint32_t*)(bs + kBS_ReadLen)    = m.size;
    *(uint32_t*)(bs + kBS_ReadPos)    = 0;

    bool ok = safe_serialize(hist, bs);
    safe_bs_dtor(bs);

    if (ok) {
        ++g.applied;
        log_line("RUNHIST", "<- applied the host's run history (%u bytes, %s)",
                 m.size, when);
    } else {
        log_line("RUNHIST", "!! deserializing the run history faulted -- this peer's "
                            "used-event list is now UNTRUSTWORTHY and the next event "
                            "it rolls may differ from the host's");
    }
}

static void free_pending() {
    for (uint32_t k = 0; k < g.pend_count; ++k) {
        free(g.pend[k].data);
        g.pend[k] = State::Pend{};
    }
    g.pend_count = 0;
}

static bool stash_pending(const RunHistMsg& m) {
    uint8_t* copy = (uint8_t*)malloc(m.size);
    if (!copy) return false;
    memcpy(copy, m.data, m.size);
    State::Pend p;
    p.epoch   = follow_epoch();
    p.arrival = follow_note_arrival();
    p.data  = copy;
    p.size  = m.size;
    p.hash  = m.hash;
    if (g.pend_count && g.pend[g.pend_count - 1].epoch == p.epoch) {
        ++g.coalesced;
        free(g.pend[g.pend_count - 1].data);
        g.pend[g.pend_count - 1] = p;
    } else if (g.pend_count >= State::kMaxPend) {
        ++g.coalesced;
        log_line("RUNHIST", "!! %u histories already held for nodes this peer has not "
                            "entered -- the newest replaces the last one", g.pend_count);
        free(g.pend[g.pend_count - 1].data);
        g.pend[g.pend_count - 1] = p;
    } else {
        g.pend[g.pend_count++] = p;
    }
    return true;
}

// Same rule and same helper shape as mgmp_invsync: deferral is only correct
// while the client's map-follow tick is guaranteed to drain the queue.
static bool defer_applies() { return g.is_client && config().net_follow; }

bool runhist_has_pending() { return g.pend_count != 0; }

bool runhist_has_pending_before_arrival(uint32_t limit) {
    for (uint32_t k = 0; k < g.pend_count; ++k)
        if (g.pend[k].arrival < limit) return true;
    return false;
}

void runhist_drop_pending_before_arrival(uint32_t limit) {
    ensure_state();
    uint32_t n = 0;
    while (n < g.pend_count && g.pend[n].arrival < limit) ++n;
    for (uint32_t k = 0; k < n; ++k) free(g.pend[k].data);
    for (uint32_t k = n; k < g.pend_count; ++k) g.pend[k - n] = g.pend[k];
    for (uint32_t k = g.pend_count - n; k < g.pend_count; ++k) g.pend[k] = State::Pend{};
    g.pend_count -= n;
    if (n)
        log_line("RUNHIST", "dropped %u held run histor%s from before the host's save (%u from after it stay%s)",
                 n, n > 1 ? "ies" : "y", g.pend_count, g.pend_count == 1 ? "s" : "");
}

static void apply_pending_where(uint32_t bound, uint32_t arrival_limit, const char* why);

void runhist_apply_pending_before_arrival(uint32_t limit, const char* why) {
    apply_pending_where(follow_apply_bound(), limit, why);
}

void runhist_apply_pending_before(uint32_t bound, const char* why) {
    apply_pending_where(bound, UINT32_MAX, why);
}

static void apply_pending_where(uint32_t bound, uint32_t arrival_limit, const char* why) {
    ensure_state();
    if (!g.on || !g.pend_count) return;

    // Of everything that arrived before the ENTERNODE of rank `bound`, the
    // newest; the rest of it is superseded, what came after stays held.
    // Mewgenics Multi : et avant le numero d'arrivee `arrival_limit`.
    uint32_t n = 0;
    while (n < g.pend_count && g.pend[n].epoch < bound && g.pend[n].arrival < arrival_limit) ++n;
    if (!n) return;
    RunHistMsg m{};
    m.size = g.pend[n - 1].size;
    m.hash = g.pend[n - 1].hash;
    m.data = g.pend[n - 1].data;
    apply_now(m, why);
    if (n > 1) g.coalesced += n - 1;
    for (uint32_t k = 0; k < n; ++k) free(g.pend[k].data);
    for (uint32_t k = n; k < g.pend_count; ++k) g.pend[k - n] = g.pend[k];
    for (uint32_t k = g.pend_count - n; k < g.pend_count; ++k) g.pend[k] = State::Pend{};
    g.pend_count -= n;
    if (g.pend_count)
        log_line("RUNHIST", "%u run histor%s from after the next node stay%s held (%s)", g.pend_count,
                 g.pend_count > 1 ? "ies" : "y", g.pend_count > 1 ? "" : "s", why);
    else
        g.held_in_battle = false;   // re-arms the line for the next battle
}

void runhist_apply_pending(const char* why) {
    runhist_apply_pending_before(follow_apply_bound(), why);
}

void runhist_on_message(const RunHistMsg& m) {
    ensure_state();
    if (!g.on) return;
    if (!g.is_client) {
        log_line("RUNHIST", "!! received a run history from the peer while hosting "
                            "-- ignored (both peers configured as host?)");
        return;
    }
    if (!m.data || !m.size) return;

    // MEWGENICS MULTI : PAS DANS LA SAVE DU JOUEUR. Entre deux aventures,
    // l'invite joue dans sa propre save ; ce que l'hote pousse de SA course n'a
    // rien a y faire. S'il est en train de rejoindre l'aventure (ou que la copie
    // se charge encore), la poussee est GARDEE : appliquee une fois la copie
    // chargee, avant la premiere case suivie (ou avant une poussee plus recente).
    if (!savefile_in_coop() || !savefile_adventure_is_loaded()) {
        const bool joining = savefile_in_coop() || savefile_join_armed();
        if (!joining) {
            static bool said = false;
            if (!said) {
                said = true;
                log_line("RUNHIST", "not in the host's adventure copy -- this push "
                                  "is dropped, the player's own save is left alone");
            }
            return;
        }
        stash_pending(m);
        return;
    }

    // NOT WHILE THIS PEER IS IN A BATTLE, the same rule and the same reason as
    // catsync_on_message. The host's reconnect catch-up republishes the whole
    // run, which is right for a peer whose process restarted and wrong for one
    // standing in a fight -- and this object feeds the event roller, so writing
    // it mid-battle changes what the NEXT node rolls, out of step with the
    // stream the per-turn hash covers. A genuinely fresh joiner is not in a
    // battle, so it still gets everything.
    //
    // HELD, not dropped -- see the long note in catsync_on_message. Dropping it
    // is permanent (the host dedupes against its last sent hash) and this is the
    // object the event roller reads, so a lost push is a run that rolls
    // different events from then on. Measured 2026-08-26: exactly that, with
    // NODEHASH naming the history as the first component to diverge.
    if (!lockstep_in_battle()) {
        g.declined_in_battle = false;
    } else if (defer_applies()) {
        if (!stash_pending(m)) {
            log_line("RUNHIST", "!! could not hold the run history for the map tick "
                                "-- applying it here instead, into a live battle");
            apply_now(m, "on arrival, deferral failed");
            return;
        }
        ++g.deferred;
        if (!g.held_in_battle) {
            g.held_in_battle = true;
            log_line("RUNHIST", "holding run-history pushes while a battle is in "
                                "progress -- the newest will be applied on the map, "
                                "just before this peer enters the next node");
        }
        return;
    } else {
        if (!g.declined_in_battle) {
            g.declined_in_battle = true;
            log_line("RUNHIST", "declining run-history pushes while a battle is in "
                                "progress -- net_follow is off, so nothing would "
                                "ever apply a held one. The used-event list will "
                                "DRIFT and the peers will roll different events.");
        }
        return;
    }

    // Mewgenics Multi : ni hors de la carte (dans une case), ni quand l'hote est
    // entre dans une case que ce joueur n'a pas encore suivie -- l'historique
    // d'apres cette case remplacerait celui dont elle tire son evenement. Gardee
    // (voir catsync_on_message), appliquee avant la case suivante.
    if (defer_applies() && savefile_in_coop() &&
        (follow_pending_count() || follow_node_live() || !follow_on_map() || savefile_keep_waiting())) {
        if (stash_pending(m)) { ++g.deferred; return; }
        log_line("RUNHIST", "!! could not hold the run history (inside a node or a node "
                            "still to follow) -- applying it now");
    }

    // Mewgenics Multi : une poussee gardee pendant que le joueur rejoignait
    // est plus ancienne que celle-ci, qui la remplace entierement.
    if (g.pend_count) free_pending();
    apply_now(m, "on arrival");
}

void runhist_drop_pending() {
    ensure_state();
    if (g.pend_count)
        log_line("RUNHIST", "dropped a held run history from before the host's save "
                            "that just arrived");
    free_pending();
}

} // namespace mgmp
