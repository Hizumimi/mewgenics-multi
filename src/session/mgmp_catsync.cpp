#include "mgmp_catsync.h"

#include "mgmp_addresses.h"
#include "mgmp_resolve.h"
#include "mgmp_bytestream.h"
#include "mgmp_config.h"
#include "mgmp_tuning.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_lockstep.h"   // lockstep_in_battle
#include "mgmp_savefile.h"   // savefile_in_coop (Mewgenics Multi)
#include "mm_rungear.h"      // rungear_hold_pushes (Mewgenics Multi)
#include "mgmp_follow.h"     // follow_epoch, follow_apply_bound (Mewgenics Multi)

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace mgmp {
namespace {

// The ByteStream's layout now lives in mgmp_bytestream.h -- mgmp_runhist drives
// the same struct through a different serializer, and one copy of an offset
// table is the only number of copies worth having.

typedef void  (__fastcall* fn_serialize_cat)(void* cat, void* stream, bool flag);
typedef void  (__fastcall* fn_bs_dtor)(void* stream);
typedef void* (__fastcall* fn_ofstream_ctor)(void* self);
typedef void* (__fastcall* fn_cat_by_id)(void* registry, uint64_t id);

struct State {
    uintptr_t base = 0;
    bool      resolved = false;   // every call target verified

    fn_serialize_cat serialize = nullptr;
    fn_bs_dtor       bs_dtor   = nullptr;
    fn_ofstream_ctor of_ctor   = nullptr;
    fn_cat_by_id     by_id     = nullptr;
    const void**     director_slot = nullptr;

    bool on        = false;
    bool is_client = false;
    bool announced = false;
    // One line per battle, not one per cat: the host pushes the whole run on a
    // reconnect and this would otherwise be a dozen identical lines.
    bool declined_in_battle = false;
    bool held_in_battle     = false;   // same, for the deferring path

    // What we last sent per cat, so an unchanged cat costs nothing on the wire.
    // Flat and small: a run is a few dozen cats, and a linear scan of 64 u64s is
    // not worth a hash map on a path that runs once per map node.
    static constexpr uint32_t kMaxCats = 64;
    uint64_t sent_id[kMaxCats]   = {};
    uint64_t sent_hash[kMaxCats] = {};
    uint32_t sent_count = 0;

    // Cats that arrived at a moment this peer must not write them, held until
    // the map-follow tick. Coalesced by id -- a CATDATA is whole state, not a
    // delta, so a newer one for the same cat makes the older one worthless.
    // Mewgenics Multi : coalesced by id AND arrival rank (follow_epoch) -- the
    // state before a node the peer has not entered yet is not worthless just
    // because the state after it has arrived too. In arrival order.
    uint64_t pend_id[kMaxCats]   = {};
    uint64_t pend_hash[kMaxCats] = {};
    uint8_t* pend_data[kMaxCats] = {};
    uint32_t pend_size[kMaxCats] = {};
    uint32_t pend_epoch[kMaxCats] = {};
    // Mewgenics Multi : le numero d'arrivee (follow_note_arrival) de chacune, et
    // le plus grand de celles deja appliquees ou laissees parce qu'identiques
    // (mm_rungear : une operation d'objet arrivee avant est deja dedans).
    uint32_t pend_arrival[kMaxCats] = {};
    uint32_t last_arrival = 0;
    uint32_t pend_count = 0;

    uint32_t pushed = 0, applied = 0, skipped = 0, deferred = 0, coalesced = 0;

    // Mewgenics Multi (protocole 58) : tous les chats de la course a l'entree de la
    // case en cours (catsync_publish_entry), pour un joueur qui arrive en pleine
    // case. Remplace a chaque entree.
    struct EntryCat {
        uint64_t             id   = 0;
        uint64_t             hash = 0;
        std::vector<uint8_t> bytes;
    };
    std::vector<EntryCat> entry;
    bool     has_entry  = false;
    uint32_t entry_sent = 0;
};

State g;

// --- SEH shims --------------------------------------------------------------
//
// Every one of these calls into the game with a pointer whose meaning is
// recovered rather than declared. The project's rule is that a wrong guess
// produces a bad log line and never a crash in the game process, so each raw
// call lives in its own function with no C++ objects in scope and a __try around
// it. They return false instead of propagating.

bool safe_ofstream_ctor(void* p) {
    __try { g.of_ctor(p); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_bs_dtor(void* p) {
    __try { g.bs_dtor(p); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_serialize(void* cat, void* stream) {
    // `true` at every observed call site (GlobalProgressionData, the two save
    // paths, the winning-teams writer); none passes false.
    __try { g.serialize(cat, stream, true); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_by_id(void* registry, uint64_t id, void** out) {
    __try { *out = g.by_id(registry, id); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// --- stream lifecycle -------------------------------------------------------

bool bs_construct(uint8_t* bs) {
    memset(bs, 0, kByteStreamSize);
    if (!safe_ofstream_ctor(bs + kBS_Ofstream)) return false;
    // Both endian words stay 0, so read() compares 0 != 0 and never byte-swaps.
    // This is the element-size cap that guards the swap it will not do.
    *(uint32_t*)(bs + kBS_SwapElem) = 8;
    return true;
}

// --- Mewgenics Multi : les emplacements d'objet avant une relecture ---------
//
// LE BOGUE DE LA DUPLICATION. SerializeCatData lit les cinq emplacements d'objet
// du chat (CatData+0x9B0, pas de 0x60) par sub_14022D2A0 : un octet « present »,
// puis l'objet. En lecture, un objet ABSENT fait revenir la fonction sans toucher
// a l'emplacement (« if (!present) return » apres la lecture de l'octet). Relu
// par-dessus un chat vivant, un emplacement que le flux dit vide garde donc
// l'objet qu'il avait : un objet rendu au sac chez l'un restait porte chez
// l'autre, ET revenait dans son sac avec la poussee du sac. Le jeu ne lit un chat
// que dans un objet neuf, il ne le voit jamais ; nous relisons des chats vivants.
//
// D'ou : avant toute relecture, chaque emplacement est vide comme le jeu le teste
// (id -1, nom vide -- std::string::clear sur le nom et sur le texte, +0x08 et
// +0x28 : taille 0 et premier caractere nul, le tampon reste a sa place). Ceux que
// le flux dit presents sont reecrits en entier, les autres restent vides.
constexpr uintptr_t kCat_Slots     = 0x9B0;
constexpr uintptr_t kSlot_Stride   = 0x60;
constexpr uint32_t  kSlot_Count    = 5;
constexpr uintptr_t kEq_NameField  = 0x08;
constexpr uintptr_t kEq_TextField  = 0x28;
constexpr uintptr_t kStr_Size      = 0x10;
constexpr uintptr_t kStr_Cap       = 0x18;

bool clear_game_string(uint8_t* str) {
    uint64_t cap = 0;
    if (!mem_read(str + kStr_Cap, &cap, sizeof(cap))) return false;
    uint8_t* chars = str;
    if (cap > 15) {
        if (!mem_read(str, &chars, sizeof(chars)) || !chars) return false;
    }
    const uint64_t zero = 0;
    const char nul = 0;
    return mem_write(str + kStr_Size, &zero, sizeof(zero)) && mem_write(chars, &nul, 1);
}

bool clear_equipment(void* cat) {
    if (!cat) return false;
    bool ok = true;
    for (uint32_t k = 0; k < kSlot_Count; ++k) {
        uint8_t* slot = (uint8_t*)cat + kCat_Slots + k * kSlot_Stride;
        const int64_t none = -1;
        ok = mem_write(slot, &none, sizeof(none)) && ok;
        ok = clear_game_string(slot + kEq_NameField) && ok;
        ok = clear_game_string(slot + kEq_TextField) && ok;
    }
    return ok;
}

// --- the run's cats ---------------------------------------------------------
//
// director+1432 is the id -> CatData* registry, +1468/+1472 the run's id list
// (count, data). All three read off sub_1403B2060, which is what the inventory
// screen itself walks. We read the ids directly rather than calling that
// function because it allocates a CustomVector we would only free again.
struct RunCats {
    const void* registry = nullptr;
    const uint64_t* ids  = nullptr;
    uint32_t count       = 0;
};

bool run_cats(RunCats& out) {
    if (!g.director_slot) return false;
    const void* dir = nullptr;
    if (!mem_read(g.director_slot, &dir, sizeof(dir)) || !dir) return false;

    const uint8_t* d = (const uint8_t*)dir;
    if (!mem_read(d + kDir_CatRegistry, &out.registry, sizeof(out.registry))) return false;
    if (!mem_read(d + kDir_CatIdCount,  &out.count,    sizeof(out.count)))    return false;
    if (!mem_read(d + kDir_CatIdData,   &out.ids,      sizeof(out.ids)))      return false;

    // Refuse anything implausible rather than walking it. A run has a few dozen
    // cats; a five-digit count means the offsets moved and we are reading some
    // other object, which is exactly when NOT to start calling game functions
    // with the results.
    if (!out.registry || !out.ids) return false;
    if (out.count == 0 || out.count > 4096) return false;
    return true;
}

uint64_t fnv1a(const void* p, uint32_t n) {
    const uint8_t* b = (const uint8_t*)p;
    uint64_t h = 0xCBF29CE484222325ull;
    for (uint32_t i = 0; i < n; ++i) { h ^= b[i]; h *= 0x100000001B3ull; }
    return h;
}

// Serializes one cat into a caller-owned buffer. Returns 0 on any failure.
// `out` must be freed with free() by the caller.
uint32_t serialize_cat(void* cat, uint8_t** out) {
    *out = nullptr;
    uint8_t bs[kByteStreamSize];
    if (!bs_construct(bs)) return 0;

    *(uint32_t*)(bs + kBS_Mode) = 1;             // write, growing the buffer
    uint32_t n = 0;
    if (safe_serialize(cat, bs)) {
        const void* buf = *(const void**)(bs + kBS_WriteBuf);
        uint32_t    len = *(uint32_t*)(bs + kBS_WriteLen);
        if (buf && len && len <= kMaxCatBytes) {
            uint8_t* copy = (uint8_t*)malloc(len);
            // Copy out before the destructor runs: the buffer belongs to the
            // GAME's heap and the destructor frees it there.
            if (copy && mem_read(buf, copy, len)) { *out = copy; n = len; }
            else if (copy) free(copy);
        }
    }
    safe_bs_dtor(bs);
    return n;
}

uint64_t* remembered(uint64_t id) {
    for (uint32_t i = 0; i < g.sent_count; ++i)
        if (g.sent_id[i] == id) return &g.sent_hash[i];
    if (g.sent_count >= State::kMaxCats) return nullptr;
    g.sent_id[g.sent_count] = id;
    g.sent_hash[g.sent_count] = 0;
    return &g.sent_hash[g.sent_count++];
}

// Read from the CONFIG rather than the live session, for the same reason
// mgmp_savefile does: the host publishes at its first map node, which can be
// before a peer has finished connecting.
//
// NOT LATCHED -- see the note on mgmp_savefile's ensure_state. The panel's
// connect buttons can set the role after this has already run once, and a
// latched "off" is a module that never arms for the rest of the process.
void ensure_state() {
    const Config& c = config();
    bool host   = _stricmp(c.net_role, "host")   == 0;
    bool client = _stricmp(c.net_role, "client") == 0;
    g.is_client = client;
    g.on = tune::kCatSync && (host || client) && g.resolved;

    static bool said = false;
    if (!g.on && tune::kCatSync && (host || client) && !said) {
        said = true;
        log_line("CATSYNC", "!! disabled: the game functions it calls did not "
                            "verify against this build");
    }
}

} // namespace

// Defined with the rest of the hold machinery, below the publish path; declared
// here because catsync_shutdown drains the hold and is written above it.
static void free_pending();

// ---------------------------------------------------------------------------

void catsync_set_base(uintptr_t base) {
    g.base = base;
    g.resolved = false;

    // Only the four this module calls. It used to walk all of C_COUNT, which
    // meant drift in an address belonging to some OTHER module silently turned
    // CAT SYNC off -- the wrong feature, named in the wrong log line. Same
    // convention mgmp_invsync already states.
    static const int kNeed[] = { C_SerializeCatData, C_ByteStreamDtor,
                                 C_OfstreamCtor,     C_CatDataById };
    for (int idx : kNeed) {
        if (!addr_of_call((Call)idx)) {
            log_line("CATSYNC", "!! %s did not resolve by signature "
                                "-- cat sync is OFF", kCalls[idx].name);
            return;
        }
    }

    g.serialize     = (fn_serialize_cat)addr_of_call(C_SerializeCatData);
    g.bs_dtor       = (fn_bs_dtor)      addr_of_call(C_ByteStreamDtor);
    g.of_ctor       = (fn_ofstream_ctor)addr_of_call(C_OfstreamCtor);
    g.by_id         = (fn_cat_by_id)    addr_of_call(C_CatDataById);
    g.director_slot = (const void**)    addr_of_data(D_MewDirectorPtr);
    if (!g.director_slot) {
        log_line("CATSYNC", "!! the MewDirector* global did not resolve -- cat sync is OFF");
        return;
    }
    g.resolved = true;
}

// Mewgenics Multi (equipe mixte) : le registre du jeu et son serialiseur, pour
// les chats de la maison d'un invite et l'etat final des chats d'invites chez
// l'hote. Rien de la synchro des chats ci-dessous n'est touche.
void* catsync_cat_by_key(uint64_t key) {
    if (!g.resolved || !g.director_slot || !key) return nullptr;
    const void* dir = nullptr;
    const void* registry = nullptr;
    if (!mem_read(g.director_slot, &dir, sizeof(dir)) || !dir) return nullptr;
    if (!mem_read((const uint8_t*)dir + kDir_CatRegistry, &registry, sizeof(registry)) || !registry)
        return nullptr;
    void* cat = nullptr;
    if (!safe_by_id((void*)registry, key, &cat)) return nullptr;
    return cat;
}

bool catsync_serialize_cat(void* cat, std::vector<uint8_t>& out) {
    out.clear();
    if (!g.resolved || !cat) return false;
    uint8_t* bytes = nullptr;
    const uint32_t n = serialize_cat(cat, &bytes);
    if (!n) return false;
    out.assign(bytes, bytes + n);
    free(bytes);
    return true;
}

// Le chemin de lecture d'apply_now, sur un chat donne : le tampon est EMPRUNTE
// (kBS_ReadOwns a 0), le jeu ne le libere pas.
bool catsync_deserialize_cat(void* cat, const std::vector<uint8_t>& bytes) {
    if (!g.resolved || !cat || bytes.empty() || bytes.size() > kMaxCatBytes) return false;
    uint8_t bs[kByteStreamSize];
    if (!bs_construct(bs)) return false;
    *(uint32_t*)(bs + kBS_Mode)       = 0;           // read
    *(const void**)(bs + kBS_ReadBuf) = bytes.data();
    *(uint8_t*)(bs + kBS_ReadOwns)    = 0;
    *(uint32_t*)(bs + kBS_ReadLen)    = (uint32_t)bytes.size();
    *(uint32_t*)(bs + kBS_ReadPos)    = 0;
    if (!clear_equipment(cat))
        log_line("CATSYNC", "!! could not empty a cat's item slots before reading it -- an item it no "
                            "longer has may stay on it");
    const bool ok = safe_serialize(cat, bs);
    safe_bs_dtor(bs);
    return ok;
}

void catsync_init() {
    ensure_state();
    if (!g.on || g.announced) return;
    g.announced = true;
    log_line_lvl(LogLevel::Trace, "CATSYNC", "armed -- %s",
             g.is_client ? "cats will be overwritten with the host's"
                         : "this peer's cats will be published to the client");
}

void catsync_shutdown() {
    const uint32_t stranded = g.pend_count;
    free_pending();
    g.entry.clear();
    g.has_entry = false;
    if (!g.announced) return;
    // `stranded` is the drop this module is allowed to make, so it says so:
    // a cat held for a map tick that never came is a cat this peer never got.
    // It is also the ONLY thing in this line that means something went wrong,
    // which is why it and not the line decides the severity.
    log_line_lvl(stranded ? LogLevel::Warn : LogLevel::Trace, "CATSYNC",
             "done: %u pushed, %u applied, %u unchanged, "
             "%u held (%u superseded), %u still held at exit",
             g.pushed, g.applied, g.skipped, g.deferred, g.coalesced, stranded);
}

// Drop the "already sent this" cache, so the next publish sends every cat
// whatever its bytes are. For a peer that RECONNECTS: the dedupe is per-run,
// not per-peer, so without this the next node's publish tells a freshly
// arrived peer about only the cats that happened to change since the last
// node -- and it has none of the others.
void catsync_forget() {
    ensure_state();
    if (!g.on || g.is_client) return;
    g.sent_count = 0;
    memset(g.sent_id,   0, sizeof(g.sent_id));
    memset(g.sent_hash, 0, sizeof(g.sent_hash));
}

static uint32_t publish_impl(const char* why, bool entry);

uint32_t catsync_publish(const char* why)       { return publish_impl(why, false); }
uint32_t catsync_publish_entry(const char* why) { return publish_impl(why, true); }

bool catsync_send_entry_to(uint8_t peer) {
    ensure_state();
    if (!g.on || g.is_client || !net_active() || !g.has_entry) return false;
    uint32_t sent = 0, failed = 0;
    for (const State::EntryCat& c : g.entry) {
        CatDataMsg m{};
        m.id   = c.id;
        m.size = (uint32_t)c.bytes.size();
        m.hash = c.hash;
        m.data = const_cast<uint8_t*>(c.bytes.data());
        if (net_send_catdata_to(peer, m)) ++sent; else ++failed;
    }
    g.entry_sent += sent;
    log_line("CATSYNC", "-> %u cat(s) as they were when the host entered this node sent to peer %u%s",
             sent, (unsigned)peer, failed ? " -- SOME FAILED" : "");
    return failed == 0;
}

static uint32_t publish_impl(const char* why, bool entry) {
    ensure_state();
    if (!g.on || g.is_client) return 0;
    // L'entree d'une case : la copie d'avant ne vaut plus rien, meme si la
    // lecture echoue (un arrivant aurait alors l'etat actuel, comme avant). Prise
    // meme quand plus personne n'est connecte (la session reste ouverte) : c'est
    // justement celui qui se connecte en pleine case qui en a besoin. (Avant la
    // premiere session prete, mgmp_follow n'appelle pas ceci : un arrivant n'a
    // alors pas de case a suivre, il attend la suivante sur la carte.)
    if (entry) { g.entry.clear(); g.has_entry = false; }
    const bool online = net_active();
    if (!online && !entry) return 0;

    RunCats rc{};
    if (!run_cats(rc)) {
        // Mewgenics Multi : hors aventure (a la maison, au menu), il n'y a pas
        // de liste a lire -- un invite qui se connecte pendant que l'hote est
        // chez lui tombe ici a chaque fois, et ce n'est pas une panne.
        if (!savefile_adventure_is_loaded())
            log_line_lvl(LogLevel::Trace, "CATSYNC",
                         "no run loaded -- nothing to publish (%s)", why);
        else
            log_line("CATSYNC", "!! could not read the run's cat list -- nothing "
                                "published (%s)", why);
        return 0;
    }

    uint32_t sent = 0, same = 0, failed = 0, unread = 0;
    for (uint32_t i = 0; i < rc.count; ++i) {
        uint64_t id = 0;
        if (!mem_read(&rc.ids[i], &id, sizeof(id)) || !id) continue;

        void* cat = nullptr;
        if (!safe_by_id((void*)rc.registry, id, &cat) || !cat) { ++failed; ++unread; continue; }

        uint8_t* bytes = nullptr;
        uint32_t n = serialize_cat(cat, &bytes);
        if (!n) { ++failed; ++unread; continue; }

        CatDataMsg m{};
        m.id   = id;
        m.size = n;
        m.hash = fnv1a(bytes, n);
        m.data = bytes;

        if (entry) {
            State::EntryCat c;
            c.id   = id;
            c.hash = m.hash;
            c.bytes.assign(bytes, bytes + n);
            g.entry.push_back(std::move(c));
        }
        if (!online) { free(bytes); continue; }

        uint64_t* prev = remembered(id);
        if (prev && *prev == m.hash) { ++same; free(bytes); continue; }

        if (net_send_catdata(m)) {
            if (prev) *prev = m.hash;
            ++sent;
        } else {
            ++failed;
        }
        free(bytes);
    }

    g.pushed  += sent;
    g.skipped += same;
    // Une copie partielle ferait construire a l'arrivant un combat avec des chats
    // manquants : entiere ou rien (il aurait alors l'etat actuel, comme avant).
    if (entry) g.has_entry = !unread && !g.entry.empty();
    if (entry && unread) g.entry.clear();
    if (sent || failed)
        log_line("CATSYNC", "-> %u cat(s) changed and sent, %u unchanged%s (%s)",
                 sent, same,
                 failed ? " -- SOME FAILED, see above" : "", why);
    return sent;
}

// --- applying, and holding until it is safe to apply ------------------------

static void apply_now(const CatDataMsg& m, const char* when) {
    if (fnv1a(m.data, m.size) != m.hash) {
        log_line("CATSYNC", "!! cat %016llx failed its hash -- not applied",
                 (unsigned long long)m.id);
        return;
    }

    RunCats rc{};
    if (!run_cats(rc)) {
        // Before a save is loaded there is no run to write into. Not an error:
        // the host may publish while this peer is still on the menu.
        return;
    }

    void* cat = nullptr;
    if (!safe_by_id((void*)rc.registry, m.id, &cat) || !cat) {
        log_line("CATSYNC", "!! cat %016llx is not in this peer's run -- not "
                            "applied. The two runs are not the same run.",
                 (unsigned long long)m.id);
        return;
    }

    uint8_t bs[kByteStreamSize];
    if (!bs_construct(bs)) return;

    *(uint32_t*)(bs + kBS_Mode)    = 0;              // read
    *(const void**)(bs + kBS_ReadBuf) = m.data;      // BORROWED, see kBS_ReadOwns
    *(uint8_t*)(bs + kBS_ReadOwns) = 0;              // ...so the game must not free it
    *(uint32_t*)(bs + kBS_ReadLen) = m.size;
    *(uint32_t*)(bs + kBS_ReadPos) = 0;

    // Mewgenics Multi : les emplacements d'objet d'abord (voir clear_equipment).
    if (!clear_equipment(cat))
        log_line("CATSYNC", "!! could not empty cat %016llx's item slots before reading it -- an item "
                            "it no longer has may stay on it", (unsigned long long)m.id);

    bool ok = safe_serialize(cat, bs);
    safe_bs_dtor(bs);

    if (ok) {
        ++g.applied;
        log_line("CATSYNC", "<- applied cat %016llx (%u bytes, %s)",
                 (unsigned long long)m.id, m.size, when);
    } else {
        log_line("CATSYNC", "!! deserializing cat %016llx faulted -- this peer's "
                            "copy of that cat is now UNTRUSTWORTHY",
                 (unsigned long long)m.id);
    }
}

static void free_pending() {
    for (uint32_t i = 0; i < g.pend_count; ++i) {
        free(g.pend_data[i]);
        g.pend_data[i] = nullptr;
        g.pend_size[i] = 0;
    }
    g.pend_count = 0;
}

static bool stash_pending(const CatDataMsg& m) {
    uint8_t* copy = (uint8_t*)malloc(m.size);
    if (!copy) return false;
    memcpy(copy, m.data, m.size);

    const uint32_t epoch   = follow_epoch();
    const uint32_t arrival = follow_note_arrival();
    for (uint32_t i = 0; i < g.pend_count; ++i) {
        if (g.pend_id[i] != m.id || g.pend_epoch[i] != epoch) continue;
        free(g.pend_data[i]);
        g.pend_data[i]    = copy;
        g.pend_size[i]    = m.size;
        g.pend_hash[i]    = m.hash;
        g.pend_arrival[i] = arrival;
        ++g.coalesced;
        return true;
    }
    if (g.pend_count >= State::kMaxCats) { free(copy); return false; }

    g.pend_id[g.pend_count]      = m.id;
    g.pend_hash[g.pend_count]    = m.hash;
    g.pend_data[g.pend_count]    = copy;
    g.pend_size[g.pend_count]    = m.size;
    g.pend_epoch[g.pend_count]   = epoch;
    g.pend_arrival[g.pend_count] = arrival;
    ++g.pend_count;
    return true;
}

static void note_applied(uint32_t arrival) {
    if (arrival > g.last_arrival) g.last_arrival = arrival;
}

// Deferral is only correct while something is guaranteed to drain the queue,
// and that something is the client's map-follow tick -- the same rule and the
// same helper shape as mgmp_invsync.
static bool defer_applies() { return g.is_client && config().net_follow; }

static void apply_pending_where(uint32_t bound, uint32_t arrival_limit, const char* why) {
    ensure_state();
    if (!g.on || !g.pend_count) return;

    // In arrival order: the same cat held twice (before and after a node) is
    // applied oldest first. What arrived after the ENTERNODE of rank `bound`
    // stays, still in order. Mewgenics Multi : et ce qui est arrive apres le
    // numero `arrival_limit` (mm_rungear : un etat de l'hote plus recent).
    const uint32_t n = g.pend_count;
    uint32_t kept = 0;
    for (uint32_t i = 0; i < n; ++i) {
        if (g.pend_epoch[i] >= bound || g.pend_arrival[i] >= arrival_limit) {
            g.pend_id[kept]      = g.pend_id[i];
            g.pend_hash[kept]    = g.pend_hash[i];
            g.pend_data[kept]    = g.pend_data[i];
            g.pend_size[kept]    = g.pend_size[i];
            g.pend_epoch[kept]   = g.pend_epoch[i];
            g.pend_arrival[kept] = g.pend_arrival[i];
            ++kept;
            continue;
        }
        CatDataMsg m{};
        m.id   = g.pend_id[i];
        m.hash = g.pend_hash[i];
        m.size = g.pend_size[i];
        m.data = g.pend_data[i];
        apply_now(m, why);
        note_applied(g.pend_arrival[i]);
        free(g.pend_data[i]);
    }
    for (uint32_t i = kept; i < n; ++i) { g.pend_data[i] = nullptr; g.pend_size[i] = 0; }
    g.pend_count = kept;
    if (kept)
        log_line("CATSYNC", "%u cat push(es) from after the next node stay held (%s)", kept, why);
    else
        g.held_in_battle = false;   // re-arms the line for the next battle
}

void catsync_apply_pending_before(uint32_t bound, const char* why) {
    apply_pending_where(bound, UINT32_MAX, why);
}

void catsync_apply_pending_before_arrival(uint32_t arrival_limit, const char* why) {
    apply_pending_where(follow_apply_bound(), arrival_limit, why);
}

void catsync_apply_pending(const char* why) {
    catsync_apply_pending_before(follow_apply_bound(), why);
}

void catsync_on_message(const CatDataMsg& m) {
    ensure_state();
    if (!g.on) return;
    if (!g.is_client) {
        // The host is authoritative; a cat arriving here means both peers think
        // they own the run, which is a configuration error worth naming rather
        // than a state to merge.
        log_line("CATSYNC", "!! received a cat from the peer while hosting -- "
                            "ignored (both peers configured as host?)");
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
                log_line("CATSYNC", "not in the host's adventure copy -- this push "
                                  "is dropped, the player's own save is left alone");
            }
            return;
        }
        if (!stash_pending(m))
            log_line("CATSYNC", "!! could not hold cat %016llx while joining the "
                                "adventure -- it will come with the next push",
                     (unsigned long long)m.id);
        return;
    }

    // NOT WHILE A BATTLE THIS PEER IS ALREADY IN IS RUNNING -- but HELD, never
    // dropped. The difference cost a whole run on 2026-08-26 and is the reason
    // this branch is written twice over.
    //
    // Measured 2026-08-26: a client that dropped its socket mid-fight and
    // pressed `join` had four cats pushed at it by the host's reconnect
    // catch-up and applied them on the spot -- and a cat visibly gained a
    // shield with no action behind it. The catch-up is right to send them (a
    // peer whose process restarted has no run at all and needs every one), but
    // applying them into a live fight writes cat state that the battle's
    // Character objects derive from, outside the deterministic stream the
    // per-turn hash covers. It is the "is CatData written during a battle"
    // question this project wrote down as the first thing to check -- and the
    // answer turned out to be that WE write it.
    //
    // A peer that only lost its socket already has the right cats: it never
    // left the run. Declining is the same rule mgmp_savefile already applies
    // for the same reason -- see savefile_on_message refusing once `applied` is
    // set. A genuinely fresh joiner has no snapshot, so this does not fire for
    // the mid-fight join case that needs the data.
    //
    // What the original version got wrong is what happens NEXT. It returned,
    // and the host dedupes against the last hash it sent, so the push was gone
    // for the rest of the run. Worse, lockstep_in_battle() is "a roster has
    // been snapshotted and not yet replaced", which stays true on the map
    // screen BETWEEN battles -- so from the first fight onwards this peer
    // refused every ordinary per-node cat push. Measured 2026-08-26: 0 cats
    // applied over 22 turns and three map nodes, the two runs forked, and the
    // next battle opened with a turn-0 state-only mismatch.
    //
    // So: hold, and apply at the map-follow tick, exactly where the inventory
    // already lands. That is still before EnterNode, so it is in time for the
    // battle or shop the node opens, and it is off the battle screen, so it
    // never writes into a live fight.
    if (!lockstep_in_battle()) {
        g.declined_in_battle = false;   // re-arms the line for the next battle
    } else if (defer_applies()) {
        if (!stash_pending(m)) {
            log_line("CATSYNC", "!! could not hold cat %016llx for the map tick "
                                "(%u already held) -- applying it here instead, "
                                "into a live battle",
                     (unsigned long long)m.id, g.pend_count);
            apply_now(m, "on arrival, deferral failed");
            return;
        }
        ++g.deferred;
        if (!g.held_in_battle) {
            g.held_in_battle = true;
            log_line("CATSYNC", "holding cat pushes while a battle is in progress "
                                "-- they will be applied on the map, just before "
                                "this peer enters the next node");
        }
        return;
    } else {
        // net_follow off: nothing would ever drain the queue, so the old
        // behaviour is all that is available. This peer is driving its own map.
        if (!g.declined_in_battle) {
            g.declined_in_battle = true;
            log_line("CATSYNC", "declining cat pushes while a battle is in progress "
                                "-- net_follow is off, so nothing would ever apply "
                                "a held one. This peer's cats will DRIFT.");
        }
        return;
    }

    // Mewgenics Multi : pas pendant qu'un ecran des objets est ouvert chez ce
    // joueur -- ses boites tiennent les objets de ses chats, que la relecture
    // remplace. Gardee, appliquee des qu'il le ferme (mm_rungear).
    //
    // Ni pendant qu'une photo de ce joueur attend sa reponse (ses objets
    // pourraient encore revenir comme a l'ouverture, par-dessus). Ni hors de
    // la carte, ni dans la case qu'il joue (follow_node_live : la carte tourne
    // encore pendant la transition d'entree -- un evenement que l'hote a deja
    // fini, et dont il pousse l'etat d'apres, n'est pas encore rejoue ici), ni
    // quand l'hote est entre dans une case que ce joueur n'a pas encore
    // suivie : c'est alors l'etat d'apres cette case, applique avant elle,
    // elle le referait par-dessus. Gardee, appliquee sur la carte
    // (mm_rungear) ou apres la case (follow_map_update).
    if (defer_applies() && (rungear_hold_pushes() || follow_pending_count() || follow_node_live() ||
                            !follow_on_map())) {
        if (stash_pending(m)) { ++g.deferred; return; }
        log_line("CATSYNC", "!! could not hold cat %016llx (inventory screen open, inside "
                            "a node or a node still to follow) -- applying it now",
                 (unsigned long long)m.id);
    }

    // Mewgenics Multi : jamais une poussee plus recente avant une plus ancienne
    // encore gardee (celles recues pendant que le joueur rejoignait) -- sinon
    // l'ancienne, appliquee plus tard, ecraserait la nouvelle.
    if (g.pend_count) catsync_apply_pending("an older push was still held");
    const uint32_t arrival = follow_note_arrival();
    apply_now(m, "on arrival");
    note_applied(arrival);
}

uint32_t catsync_pending_count() {
    ensure_state();
    return g.pend_count;
}

void catsync_drop_pending() {
    ensure_state();
    if (g.pend_count)
        log_line("CATSYNC", "dropped %u held cat push(es) from before the host's save "
                            "that just arrived", g.pend_count);
    free_pending();
    note_applied(follow_note_arrival());   // Mewgenics Multi : la save les contient
}

uint32_t catsync_last_applied_arrival() {
    return g.last_arrival;
}

uint32_t catsync_drop_identical(const char* why) {
    ensure_state();
    if (!g.on || !g.is_client || !g.pend_count) return g.pend_count;
    RunCats rc{};
    if (!run_cats(rc)) return g.pend_count;

    // Seulement celles qui s'appliqueraient maintenant (d'avant la prochaine case
    // a suivre), dans l'ordre : une poussee d'un chat n'est comparee que si aucune
    // plus ancienne du meme chat n'attend encore (sinon la comparer a l'etat
    // actuel ne dirait rien).
    const uint32_t bound = follow_apply_bound();
    const uint32_t n = g.pend_count;
    uint32_t kept = 0, dropped = 0;
    for (uint32_t i = 0; i < n; ++i) {
        bool older_kept = false;
        for (uint32_t k = 0; k < kept; ++k)
            if (g.pend_id[k] == g.pend_id[i]) { older_kept = true; break; }
        bool same = false;
        if (g.pend_epoch[i] < bound && !older_kept) {
            void* cat = nullptr;
            uint8_t* bytes = nullptr;
            uint32_t len = 0;
            if (safe_by_id((void*)rc.registry, g.pend_id[i], &cat) && cat)
                len = serialize_cat(cat, &bytes);
            same = len && len == g.pend_size[i] && memcmp(bytes, g.pend_data[i], len) == 0;
            free(bytes);
        }
        if (same) {
            note_applied(g.pend_arrival[i]);
            free(g.pend_data[i]);
            ++dropped;
            continue;
        }
        g.pend_id[kept]      = g.pend_id[i];
        g.pend_hash[kept]    = g.pend_hash[i];
        g.pend_data[kept]    = g.pend_data[i];
        g.pend_size[kept]    = g.pend_size[i];
        g.pend_epoch[kept]   = g.pend_epoch[i];
        g.pend_arrival[kept] = g.pend_arrival[i];
        ++kept;
    }
    for (uint32_t i = kept; i < n; ++i) { g.pend_data[i] = nullptr; g.pend_size[i] = 0; }
    g.pend_count = kept;
    if (dropped)
        log_line("CATSYNC", "%u held cat push(es) identical to this peer's cats -- nothing to apply (%s)",
                 dropped, why);
    return kept;
}

} // namespace mgmp
