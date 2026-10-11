// mm_npc_state -- Mewgenics Multi : l'etat des PNJ. Voir mm_npc_state.h.

#include "mm_npc_state.h"

#include <cstring>

namespace mgmp {
namespace {

// Versions de npc_progress lues ici : a partir de 0x20 tous les champs d'un PNJ
// sont la, et 0x26 est celle du jeu vise (1.1.21239). Une version plus recente
// pourrait avoir ajoute un champ : on ne la touche pas.
constexpr uint32_t kMinVersion = 0x20;
constexpr uint32_t kMaxVersion = 0x26;

constexpr uint32_t kMagic   = 0x43504E4D;   // « MNPC »
constexpr uint8_t  kFormat  = 1;
constexpr size_t   kMaxName = 256;

struct Reader {
    const uint8_t* p;
    size_t         n;
    size_t         pos = 0;
    bool           ok  = true;

    bool need(size_t k) {
        if (!ok || k > n || pos > n - k) ok = false;
        return ok;
    }
    template <typename T> T get() {
        T v{};
        if (need(sizeof(T))) { memcpy(&v, p + pos, sizeof(T)); pos += sizeof(T); }
        return v;
    }
    void raw(void* out, size_t k) {
        if (need(k)) { memcpy(out, p + pos, k); pos += k; }
    }
    bool skip(size_t k) {
        if (need(k)) pos += k;
        return ok;
    }
    std::string str64(size_t max) {        // u64 longueur + octets (la save)
        const uint64_t len = get<uint64_t>();
        std::string s;
        if (!ok || len > max || !need((size_t)len)) { ok = false; return s; }
        s.assign((const char*)p + pos, (size_t)len);
        pos += (size_t)len;
        return s;
    }
};

// Ou sont, dans le bloc, la faveur et « recompense prete » de chaque PNJ.
struct Where {
    size_t favor[kNpcCount] = {};
    size_t ready[kNpcCount] = {};
};

// Lit les 8 PNJ (et, si `at`, la position de leurs champs dans le bloc).
bool parse_impl(const std::vector<uint8_t>& blob, NpcState (&out)[kNpcCount],
                uint32_t* version, Where* at) {
    Reader r{blob.data(), blob.size()};
    const uint32_t ver = r.get<uint32_t>();
    if (version) *version = ver;
    if (!r.ok || ver < kMinVersion || ver > kMaxVersion) return false;
    for (int i = 0; i < kNpcCount; ++i) {
        NpcState s{};
        const uint64_t taken = r.get<uint64_t>();
        if (!r.ok || taken > 1000) return false;
        for (uint64_t k = 0; k < taken && r.ok; ++k) {
            r.str64(kMaxName);
            r.skip(4);
        }
        s.taken = taken;
        s.cur   = r.str64(kMaxName);
        s.a38   = r.get<uint32_t>();
        if (at) at->favor[i] = r.pos;
        s.favor = r.get<uint32_t>();
        s.c44   = r.get<uint32_t>();
        if (at) at->ready[i] = r.pos;
        s.ready = r.get<uint8_t>();
        s.e4c   = r.get<uint32_t>();
        s.f50   = r.get<uint8_t>();
        s.g51   = r.get<uint8_t>();
        r.raw(s.h58, sizeof(s.h58));
        s.i54   = r.get<uint32_t>();
        if (!r.ok) return false;
        out[i] = std::move(s);
    }
    return true;
}

template <typename T> void put(std::vector<uint8_t>& b, T v) {
    const uint8_t* p = (const uint8_t*)&v;
    b.insert(b.end(), p, p + sizeof(T));
}

} // namespace

const char* npc_name(int idx) {
    static const char* const kNames[kNpcCount] = {
        "Beanies", "Butch", "Tink", "Frank", "Jack", "Tracy", "l'orgue", "Steven",
    };
    return idx >= 0 && idx < kNpcCount ? kNames[idx] : "?";
}

bool npc_progress_parse(const std::vector<uint8_t>& blob, NpcState (&out)[kNpcCount],
                        uint32_t* version) {
    return parse_impl(blob, out, version, nullptr);
}

int npc_progress_for_guest(std::vector<uint8_t>& blob, const std::vector<uint8_t>& own) {
    NpcState host[kNpcCount];
    Where    at;
    if (!parse_impl(blob, host, nullptr, &at)) return -1;
    NpcState mine[kNpcCount];
    const bool have_own = parse_impl(own, mine, nullptr, nullptr);
    int lowered = 0;
    for (int i = 0; i < kNpcCount; ++i) {
        if (!host[i].ready) continue;
        const bool keep = have_own && npc_same_step(host[i], mine[i]) && !mine[i].ready &&
                          mine[i].favor < host[i].favor;
        const uint32_t favor = keep ? mine[i].favor : 0;
        blob[at.ready[i]] = 0;
        memcpy(blob.data() + at.favor[i], &favor, sizeof(favor));
        ++lowered;
    }
    return lowered;
}

std::vector<uint8_t> npc_state_encode(const NpcState (&npcs)[kNpcCount]) {
    std::vector<uint8_t> b;
    b.reserve(kNpcCount * 96);
    put<uint32_t>(b, kMagic);
    put<uint8_t>(b, kFormat);
    put<uint8_t>(b, (uint8_t)kNpcCount);
    for (const NpcState& s : npcs) {
        const size_t len = s.cur.size() < kMaxName ? s.cur.size() : kMaxName;
        put<uint64_t>(b, s.taken);
        put<uint16_t>(b, (uint16_t)len);
        b.insert(b.end(), s.cur.begin(), s.cur.begin() + (ptrdiff_t)len);
        put<uint32_t>(b, s.a38);
        put<uint32_t>(b, s.favor);
        put<uint32_t>(b, s.c44);
        put<uint8_t>(b, s.ready);
        put<uint32_t>(b, s.e4c);
        put<uint8_t>(b, s.f50);
        put<uint8_t>(b, s.g51);
        put<uint32_t>(b, s.i54);
        b.insert(b.end(), s.h58, s.h58 + sizeof(s.h58));
    }
    return b;
}

bool npc_state_decode(const uint8_t* p, size_t n, NpcState (&out)[kNpcCount]) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kMagic || r.get<uint8_t>() != kFormat ||
        r.get<uint8_t>() != kNpcCount || !r.ok)
        return false;
    NpcState tmp[kNpcCount];
    for (NpcState& s : tmp) {
        s.taken = r.get<uint64_t>();
        const uint16_t len = r.get<uint16_t>();
        if (!r.ok || len > kMaxName || !r.need(len)) return false;
        s.cur.assign((const char*)p + r.pos, len);
        r.pos += len;
        s.a38   = r.get<uint32_t>();
        s.favor = r.get<uint32_t>();
        s.c44   = r.get<uint32_t>();
        s.ready = r.get<uint8_t>();
        s.e4c   = r.get<uint32_t>();
        s.f50   = r.get<uint8_t>();
        s.g51   = r.get<uint8_t>();
        s.i54   = r.get<uint32_t>();
        r.raw(s.h58, sizeof(s.h58));
        if (!r.ok) return false;
    }
    if (r.pos != n) return false;
    for (int i = 0; i < kNpcCount; ++i) out[i] = std::move(tmp[i]);
    return true;
}

bool npc_same_step(const NpcState& host, const NpcState& mine) {
    return host.cur == mine.cur && host.taken == mine.taken;
}

NpcState npc_guest_target(const NpcState& host, const NpcState& mine) {
    NpcState t = mine;
    if (!npc_same_step(host, mine)) return t;
    t.a38 = host.a38;
    t.c44 = host.c44;
    t.e4c = host.e4c;
    t.f50 = host.f50;
    t.g51 = host.g51;
    t.i54 = host.i54;
    memcpy(t.h58, host.h58, sizeof(t.h58));
    if (!host.ready) t.favor = host.favor;
    return t;
}

bool npc_simple_fields_differ(const NpcState& host, const NpcState& mine) {
    return host.a38 != mine.a38 || host.favor != mine.favor || host.c44 != mine.c44 ||
           host.e4c != mine.e4c || host.f50 != mine.f50 || host.g51 != mine.g51 ||
           host.i54 != mine.i54 || memcmp(host.h58, mine.h58, sizeof(host.h58)) != 0;
}

} // namespace mgmp
