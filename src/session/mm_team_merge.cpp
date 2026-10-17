// mm_team_merge -- Mewgenics Multi : l'equipe mixte, partie portable. Voir
// mm_team_merge.h.

#include "mm_team_merge.h"

#include "mgmp_log.h"
#include "sqlite3.h"

#include <algorithm>
#include <cstring>
#include <set>

namespace mgmp {
namespace {

constexpr size_t   kMaxCatBytes = 1u << 16;    // un chat fait moins d'un kilo-octet
constexpr size_t   kMaxTeam     = 16;
constexpr size_t   kMaxStorageBytes = 512u << 10; // le rangement : quelques kilo-octets (kMaxInvBytes)

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
};

template <typename T> void put(std::vector<uint8_t>& b, T v) {
    const uint8_t* q = (const uint8_t*)&v;
    b.insert(b.end(), q, q + sizeof(T));
}

void utf16_to_utf8(const uint8_t* p, size_t chars, std::string& out) {
    out.clear();
    for (size_t i = 0; i < chars; ++i) {
        uint32_t c = (uint32_t)p[2 * i] | ((uint32_t)p[2 * i + 1] << 8);
        if (c >= 0xD800 && c < 0xDC00 && i + 1 < chars) {
            const uint32_t d = (uint32_t)p[2 * i + 2] | ((uint32_t)p[2 * i + 3] << 8);
            if (d >= 0xDC00 && d < 0xE000) { c = 0x10000 + ((c - 0xD800) << 10) + (d - 0xDC00); ++i; }
        }
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
        else if (c < 0x10000) {
            out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        } else {
            out += (char)(0xF0 | (c >> 18)); out += (char)(0x80 | ((c >> 12) & 0x3F));
            out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F));
        }
    }
}

// --- sqlite -----------------------------------------------------------------

bool exec(sqlite3* db, const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        log_line("TEAM", "!! sqlite : %s -- %s", sql, err ? err : sqlite3_errmsg(db));
        sqlite3_free(err);
        return false;
    }
    return true;
}

// 1 trouve, 0 absent, -1 erreur.
int get_row(sqlite3* db, const char* sql, const char* text_key, int64_t int_key,
            std::vector<uint8_t>& out) {
    out.clear();
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &st, nullptr) != SQLITE_OK) {
        log_line("TEAM", "!! sqlite : %s", sqlite3_errmsg(db));
        return -1;
    }
    if (text_key) sqlite3_bind_text(st, 1, text_key, -1, SQLITE_STATIC);
    else          sqlite3_bind_int64(st, 1, int_key);
    int found = 0;
    const int rc = sqlite3_step(st);
    if (rc == SQLITE_ROW) {
        const void* q = sqlite3_column_blob(st, 0);
        const int   n = sqlite3_column_bytes(st, 0);
        if (q && n > 0) out.assign((const uint8_t*)q, (const uint8_t*)q + n);
        found = 1;
    } else if (rc != SQLITE_DONE) {
        log_line("TEAM", "!! sqlite : %s", sqlite3_errmsg(db));
        found = -1;
    }
    sqlite3_finalize(st);
    return found;
}

bool put_blob(sqlite3* db, const char* sql, const char* text_key, int64_t int_key,
              const std::vector<uint8_t>& b) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &st, nullptr) != SQLITE_OK) {
        log_line("TEAM", "!! sqlite : %s", sqlite3_errmsg(db));
        return false;
    }
    if (text_key) sqlite3_bind_text(st, 1, text_key, -1, SQLITE_STATIC);
    else          sqlite3_bind_int64(st, 1, int_key);
    sqlite3_bind_blob(st, 2, b.empty() ? "" : (const void*)b.data(), (int)b.size(), SQLITE_TRANSIENT);
    const bool ok = sqlite3_step(st) == SQLITE_DONE;
    if (!ok) log_line("TEAM", "!! sqlite : %s", sqlite3_errmsg(db));
    sqlite3_finalize(st);
    return ok;
}

} // namespace

// --- le message ----------------------------------------------------------------

std::vector<uint8_t> team_encode(const TeamMsg& m) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, m.kind);
    put<uint8_t>(b, m.seq);
    put<int64_t>(b, m.day);
    put<uint8_t>(b, (uint8_t)std::min(m.cats.size(), kMaxTeam));
    for (size_t i = 0; i < m.cats.size() && i < kMaxTeam; ++i) {
        const TeamCat& c = m.cats[i];
        put<uint64_t>(b, c.own_key);
        put<uint64_t>(b, c.host_key);
        put<uint8_t>(b, c.fate);
        put<uint32_t>(b, (uint32_t)c.data.size());
        b.insert(b.end(), c.data.begin(), c.data.end());
    }
    return b;
}

bool team_decode(const uint8_t* p, size_t n, TeamMsg& m) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat) return false;
    TeamMsg out;
    out.kind = r.get<uint8_t>();
    out.seq  = r.get<uint8_t>();
    out.day  = r.get<int64_t>();
    const uint8_t count = r.get<uint8_t>();
    if (!r.ok || count > kMaxTeam || (out.kind != kTeamDepart && out.kind != kTeamResult)) return false;
    out.cats.resize(count);
    for (TeamCat& c : out.cats) {
        c.own_key  = r.get<uint64_t>();
        c.host_key = r.get<uint64_t>();
        c.fate     = r.get<uint8_t>();
        const uint32_t sz = r.get<uint32_t>();
        if (!r.ok || c.fate > kFateGone || sz == 0 || sz > kMaxCatBytes || !r.need(sz)) return false;
        c.data.assign(p + r.pos, p + r.pos + sz);
        r.pos += sz;
    }
    if (r.pos != n) return false;
    m = std::move(out);
    return true;
}

std::vector<uint8_t> team_owners_encode(uint8_t seq, const std::vector<TeamOwner>& owners) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamOwners);
    put<uint8_t>(b, seq);
    put<uint8_t>(b, (uint8_t)std::min(owners.size(), kMaxTeam));
    for (size_t i = 0; i < owners.size() && i < kMaxTeam; ++i) {
        put<uint64_t>(b, owners[i].cat_id);
        put<uint64_t>(b, owners[i].host_key);
        put<uint8_t>(b, owners[i].peer);
    }
    return b;
}

bool team_owners_decode(const uint8_t* p, size_t n, uint8_t& seq, std::vector<TeamOwner>& owners) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat ||
        r.get<uint8_t>() != kTeamOwners)
        return false;
    const uint8_t sq    = r.get<uint8_t>();
    const uint8_t count = r.get<uint8_t>();
    if (!r.ok || count > kMaxTeam) return false;
    std::vector<TeamOwner> out(count);
    for (TeamOwner& o : out) {
        o.cat_id   = r.get<uint64_t>();
        o.host_key = r.get<uint64_t>();
        o.peer     = r.get<uint8_t>();
    }
    if (!r.ok || r.pos != n) return false;
    seq    = sq;
    owners = std::move(out);
    return true;
}

std::vector<uint8_t> team_embark_encode(uint8_t seq, const std::vector<uint64_t>& keys) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamEmbark);
    put<uint8_t>(b, seq);
    put<uint8_t>(b, (uint8_t)std::min(keys.size(), kMaxTeam));
    for (size_t i = 0; i < keys.size() && i < kMaxTeam; ++i) put<uint64_t>(b, keys[i]);
    return b;
}

bool team_embark_decode(const uint8_t* p, size_t n, uint8_t& seq, std::vector<uint64_t>& keys) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat ||
        r.get<uint8_t>() != kTeamEmbark)
        return false;
    const uint8_t sq    = r.get<uint8_t>();
    const uint8_t count = r.get<uint8_t>();
    if (!r.ok || count == 0 || count > kMaxTeam) return false;
    std::vector<uint64_t> out(count);
    for (uint64_t& k : out) {
        k = r.get<uint64_t>();
        if (r.ok && k == 0) return false;
    }
    if (!r.ok || r.pos != n) return false;
    seq  = sq;
    keys = std::move(out);
    return true;
}

std::vector<uint8_t> team_gear_encode(const GearMsg& m) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamGear);
    put<uint8_t>(b, m.vote);
    put<uint32_t>(b, m.snap);
    put<uint8_t>(b, (uint8_t)((m.final ? 1 : 0) | (m.has_data ? 2 : 0)));
    if (!m.has_data) return b;
    put<uint32_t>(b, (uint32_t)m.storage.size());
    b.insert(b.end(), m.storage.begin(), m.storage.end());
    put<uint8_t>(b, (uint8_t)std::min(m.cats.size(), kMaxTeam));
    for (size_t i = 0; i < m.cats.size() && i < kMaxTeam; ++i) {
        put<uint64_t>(b, m.cats[i].key);
        put<uint32_t>(b, (uint32_t)m.cats[i].data.size());
        b.insert(b.end(), m.cats[i].data.begin(), m.cats[i].data.end());
    }
    return b;
}

bool team_gear_decode(const uint8_t* p, size_t n, GearMsg& m) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat ||
        r.get<uint8_t>() != kTeamGear)
        return false;
    GearMsg out;
    out.vote = r.get<uint8_t>();
    out.snap = r.get<uint32_t>();
    const uint8_t flags = r.get<uint8_t>();
    if (!r.ok || (flags & ~3u)) return false;
    out.final    = (flags & 1) != 0;
    out.has_data = (flags & 2) != 0;
    if (out.has_data) {
        const uint32_t sn = r.get<uint32_t>();
        if (!r.ok || sn < 4 || sn > kMaxStorageBytes || !r.need(sn)) return false;
        out.storage.assign(p + r.pos, p + r.pos + sn);
        r.pos += sn;
        const uint8_t count = r.get<uint8_t>();
        if (!r.ok || count > kMaxTeam) return false;
        for (uint8_t i = 0; i < count; ++i) {
            GearCat c;
            c.key = r.get<uint64_t>();
            const uint32_t cn = r.get<uint32_t>();
            if (!r.ok || c.key == 0 || cn == 0 || cn > kMaxCatBytes || !r.need(cn)) return false;
            c.data.assign(p + r.pos, p + r.pos + cn);
            r.pos += cn;
            out.cats.push_back(std::move(c));
        }
    }
    if (!r.ok || r.pos != n) return false;
    m = std::move(out);
    return true;
}

std::vector<uint8_t> team_gear_ack_encode(const GearAck& a) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamGearAck);
    put<uint8_t>(b, a.vote);
    put<uint32_t>(b, a.snap);
    put<uint8_t>(b, a.has_storage ? 1 : 0);
    if (a.has_storage) {
        put<uint32_t>(b, (uint32_t)a.storage.size());
        b.insert(b.end(), a.storage.begin(), a.storage.end());
    }
    return b;
}

bool team_gear_ack_decode(const uint8_t* p, size_t n, GearAck& a) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat ||
        r.get<uint8_t>() != kTeamGearAck)
        return false;
    GearAck out;
    out.vote = r.get<uint8_t>();
    out.snap = r.get<uint32_t>();
    const uint8_t flags = r.get<uint8_t>();
    if (!r.ok || flags > 1) return false;
    out.has_storage = flags == 1;
    if (out.has_storage) {
        const uint32_t sn = r.get<uint32_t>();
        if (!r.ok || sn < 4 || sn > kMaxStorageBytes || !r.need(sn)) return false;
        out.storage.assign(p + r.pos, p + r.pos + sn);
        r.pos += sn;
    }
    if (!r.ok || r.pos != n) return false;
    a = std::move(out);
    return true;
}

namespace {

void put_blob(std::vector<uint8_t>& b, const std::vector<uint8_t>& blob) {
    put<uint32_t>(b, (uint32_t)blob.size());
    b.insert(b.end(), blob.begin(), blob.end());
}

// Un bloc d'inventaire : au moins le nombre d'objets (u32), un rangement vide
// fait 4 octets.
bool get_blob(Reader& r, const uint8_t* p, std::vector<uint8_t>& out) {
    const uint32_t n = r.get<uint32_t>();
    if (!r.ok || n < 4 || n > kMaxStorageBytes || !r.need(n)) return false;
    out.assign(p + r.pos, p + r.pos + n);
    r.pos += n;
    return true;
}

} // namespace

std::vector<uint8_t> team_rungear_encode(const RunGearMsg& m) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamRunGear);
    put<uint32_t>(b, m.seq);
    put<uint32_t>(b, m.base);
    put_blob(b, m.backpack);
    put_blob(b, m.trash);
    put<uint8_t>(b, (uint8_t)std::min(m.cats.size(), kMaxTeam));
    for (size_t i = 0; i < m.cats.size() && i < kMaxTeam; ++i) {
        put<uint64_t>(b, m.cats[i].key);
        put_blob(b, m.cats[i].data);
    }
    return b;
}

bool team_rungear_decode(const uint8_t* p, size_t n, RunGearMsg& m) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat ||
        r.get<uint8_t>() != kTeamRunGear)
        return false;
    RunGearMsg out;
    out.seq  = r.get<uint32_t>();
    out.base = r.get<uint32_t>();
    if (!r.ok || !get_blob(r, p, out.backpack) || !get_blob(r, p, out.trash)) return false;
    const uint8_t count = r.get<uint8_t>();
    if (!r.ok || count > kMaxTeam) return false;
    for (uint8_t i = 0; i < count; ++i) {
        GearCat c;
        c.key = r.get<uint64_t>();
        const uint32_t cn = r.get<uint32_t>();
        if (!r.ok || c.key == 0 || cn == 0 || cn > kMaxCatBytes || !r.need(cn)) return false;
        c.data.assign(p + r.pos, p + r.pos + cn);
        r.pos += cn;
        out.cats.push_back(std::move(c));
    }
    if (!r.ok || r.pos != n) return false;
    m = std::move(out);
    return true;
}

std::vector<uint8_t> team_rungear_ack_encode(const RunGearAck& a) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamRunGearAck);
    put<uint32_t>(b, a.seq);
    put<uint8_t>(b, a.result);
    put<uint32_t>(b, a.rev);
    return b;
}

bool team_rungear_ack_decode(const uint8_t* p, size_t n, RunGearAck& a) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat ||
        r.get<uint8_t>() != kTeamRunGearAck)
        return false;
    RunGearAck out;
    out.seq    = r.get<uint32_t>();
    out.result = r.get<uint8_t>();
    out.rev    = r.get<uint32_t>();
    if (!r.ok || r.pos != n || out.result < kRunGearApplied || out.result > kRunGearNode) return false;
    a = out;
    return true;
}

// --- les objets en direct (protocole 44) ---------------------------------------

namespace {

void put_head(std::vector<uint8_t>& b, uint8_t kind) {
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kind);
}

bool get_head(Reader& r, uint8_t kind) {
    return r.get<uint32_t>() == kTeamMagic && r.get<uint8_t>() == kTeamFormat && r.get<uint8_t>() == kind && r.ok;
}

constexpr size_t kMaxItemName = 63;

void fnv_bytes(uint64_t& h, const void* p, size_t n) {
    const uint8_t* b = (const uint8_t*)p;
    for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 0x100000001B3ull; }
}

} // namespace

std::vector<uint8_t> team_item_op_encode(const ItemOp& m) {
    std::vector<uint8_t> b;
    put_head(b, kTeamItemOp);
    put<uint32_t>(b, m.seq);
    put<uint8_t>(b, m.origin);
    put<uint8_t>(b, m.op);
    put<uint8_t>(b, m.slot);
    put<uint64_t>(b, m.cat_key);
    put<uint64_t>(b, m.cat_id);
    put<uint64_t>(b, m.item);
    put<uint64_t>(b, m.prev);
    const size_t nl = std::min(m.name.size(), kMaxItemName);
    put<uint8_t>(b, (uint8_t)nl);
    b.insert(b.end(), m.name.begin(), m.name.begin() + nl);
    put_blob(b, m.cat);
    return b;
}

bool team_item_op_decode(const uint8_t* p, size_t n, ItemOp& m) {
    if (!p) return false;
    Reader r{p, n};
    if (!get_head(r, kTeamItemOp)) return false;
    ItemOp out;
    out.seq     = r.get<uint32_t>();
    out.origin  = r.get<uint8_t>();
    out.op      = r.get<uint8_t>();
    out.slot    = r.get<uint8_t>();
    out.cat_key = r.get<uint64_t>();
    out.cat_id  = r.get<uint64_t>();
    out.item    = r.get<uint64_t>();
    out.prev    = r.get<uint64_t>();
    const uint8_t nl = r.get<uint8_t>();
    if (!r.ok || nl > kMaxItemName || !r.need(nl)) return false;
    out.name.assign((const char*)p + r.pos, nl);
    r.pos += nl;
    const uint32_t cn = r.get<uint32_t>();
    if (!r.ok || cn > kMaxCatBytes || !r.need(cn)) return false;
    out.cat.assign(p + r.pos, p + r.pos + cn);
    r.pos += cn;
    if (!r.ok || r.pos != n) return false;
    if (out.op < kItemEquip || out.op > kItemLearn || out.slot >= kItemSlots || !out.cat_key) return false;
    if (out.op != kItemLearn && (!out.item || !out.cat.empty())) return false;
    if (out.op == kItemLearn && out.cat.empty()) return false;
    m = std::move(out);
    return true;
}

std::vector<uint8_t> team_item_ack_encode(const ItemAck& a) {
    std::vector<uint8_t> b;
    put_head(b, kTeamItemAck);
    put<uint32_t>(b, a.seq);
    put<uint8_t>(b, a.result);
    return b;
}

bool team_item_ack_decode(const uint8_t* p, size_t n, ItemAck& a) {
    if (!p) return false;
    Reader r{p, n};
    if (!get_head(r, kTeamItemAck)) return false;
    ItemAck out;
    out.seq    = r.get<uint32_t>();
    out.result = r.get<uint8_t>();
    if (!r.ok || r.pos != n || out.result < kItemOk || out.result > kItemNode) return false;
    a = out;
    return true;
}

std::vector<uint8_t> team_item_sync_req_encode(uint32_t seq) {
    std::vector<uint8_t> b;
    put_head(b, kTeamItemSyncReq);
    put<uint32_t>(b, seq);
    return b;
}

bool team_item_sync_req_decode(const uint8_t* p, size_t n, uint32_t& seq) {
    if (!p) return false;
    Reader r{p, n};
    if (!get_head(r, kTeamItemSyncReq)) return false;
    const uint32_t s = r.get<uint32_t>();
    if (!r.ok || r.pos != n) return false;
    seq = s;
    return true;
}

std::vector<uint8_t> team_item_sync_encode(const RunGearMsg& m) {
    std::vector<uint8_t> b = team_rungear_encode(m);
    if (b.size() > 5) b[5] = kTeamItemSync;     // le meme corps, sous un autre genre
    return b;
}

bool team_item_sync_decode(const uint8_t* p, size_t n, RunGearMsg& m) {
    if (!p || n < 6 || p[5] != kTeamItemSync) return false;
    std::vector<uint8_t> b(p, p + n);
    b[5] = kTeamRunGear;
    return team_rungear_decode(b.data(), b.size(), m);
}

uint64_t item_digest(const ItemFields& it) {
    uint64_t h = 0xCBF29CE484222325ull;
    const uint8_t zero = 0;
    fnv_bytes(h, it.name.data(), it.name.size());
    fnv_bytes(h, &zero, 1);
    fnv_bytes(h, it.text.data(), it.text.size());
    fnv_bytes(h, &zero, 1);
    // Ni d (sa place a l'ecran) ni les bits « nouvel objet » de c : ce que ce
    // joueur en a vu (mm_team_merge.h). f : l'octet que la save en garde.
    const uint32_t ints[3] = { it.a, it.b, it.c & ~kItemSeenBits };
    for (uint32_t v : ints) {
        const uint8_t le[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
        fnv_bytes(h, le, 4);
    }
    const uint8_t tail[2] = { it.e, (uint8_t)it.f };
    fnv_bytes(h, tail, 2);
    return h ? h : 1;
}

uint8_t team_kind(const uint8_t* p, size_t n) {
    if (!p) return 0;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat) return 0;
    const uint8_t kind = r.get<uint8_t>();
    if (!r.ok || kind < kTeamDepart || kind > kTeamLast) return 0;
    return kind;
}

int team_read_storage(sqlite3* db, std::vector<uint8_t>& out) {
    out.clear();
    if (!db) return -1;
    return get_row(db, "SELECT data FROM files WHERE key = ?1", "inventory_storage", 0, out);
}

// --- un chat serialise ---------------------------------------------------------

bool cat_info(const std::vector<uint8_t>& raw, CatInfo& out) {
    Reader r{raw.data(), raw.size()};
    if (r.get<uint32_t>() != kCatVersion) return false;
    CatInfo c;
    c.id = r.get<uint64_t>();
    const uint64_t chars = r.get<uint64_t>();
    if (!r.ok || chars > 256 || !r.need((size_t)chars * 2)) return false;
    utf16_to_utf8(raw.data() + r.pos, (size_t)chars, c.name);
    r.pos += (size_t)chars * 2;
    const uint64_t n2 = r.get<uint64_t>();
    if (!r.ok || n2 > 4096 || !r.need((size_t)n2)) return false;
    r.pos += (size_t)n2;
    r.get<uint32_t>();                 // +0x58
    r.get<uint32_t>();                 // +0x5C
    c.flags_at = r.pos;
    c.flags = r.get<uint64_t>();
    if (!r.ok) return false;
    out = std::move(c);
    return true;
}

bool cat_set_flags(std::vector<uint8_t>& raw, uint64_t flags) {
    CatInfo c;
    if (!cat_info(raw, c)) return false;
    memcpy(raw.data() + c.flags_at, &flags, sizeof(flags));
    return true;
}

// --- le bloc de la table cats --------------------------------------------------

std::vector<uint8_t> cat_blob_pack(const std::vector<uint8_t>& raw) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, (uint32_t)raw.size());
    // Une seule sequence, sans correspondance : jeton (litteraux sur 4 bits,
    // 15 = longueur continuee par des octets 255... puis le reste), puis les
    // litteraux. C'est la forme que la derniere sequence de tout bloc LZ4 a.
    const size_t n = raw.size();
    if (n < 15) {
        b.push_back((uint8_t)(n << 4));
    } else {
        b.push_back(0xF0);
        size_t rest = n - 15;
        while (rest >= 255) { b.push_back(255); rest -= 255; }
        b.push_back((uint8_t)rest);
    }
    b.insert(b.end(), raw.begin(), raw.end());
    return b;
}

bool cat_blob_unpack(const uint8_t* p, size_t n, std::vector<uint8_t>& raw) {
    raw.clear();
    if (!p || n < 5) return false;
    uint32_t size = 0;
    memcpy(&size, p, 4);
    if (size == 0 || size > kMaxCatBytes) return false;
    raw.reserve(size);
    size_t i = 4;
    while (i < n) {
        const uint8_t token = p[i++];
        size_t lit = token >> 4;
        if (lit == 15) {
            uint8_t more = 255;
            while (more == 255) {
                if (i >= n) return false;
                more = p[i++];
                lit += more;
            }
        }
        if (lit > n - i || raw.size() + lit > size) return false;
        raw.insert(raw.end(), p + i, p + i + lit);
        i += lit;
        if (i >= n) break;                       // derniere sequence : litteraux seuls
        if (n - i < 2) return false;
        const size_t off = (size_t)p[i] | ((size_t)p[i + 1] << 8);
        i += 2;
        size_t len = (size_t)(token & 15);
        if (len == 15) {
            uint8_t more = 255;
            while (more == 255) {
                if (i >= n) return false;
                more = p[i++];
                len += more;
            }
        }
        len += 4;
        if (off == 0 || off > raw.size() || raw.size() + len > size) return false;
        const size_t from = raw.size() - off;
        for (size_t k = 0; k < len; ++k) raw.push_back(raw[from + k]);
    }
    return raw.size() == size;
}

// --- house_state ----------------------------------------------------------------

namespace {
struct HouseEntry { size_t at, len; uint64_t key; };

bool house_entries(const std::vector<uint8_t>& hs, std::vector<HouseEntry>& out) {
    out.clear();
    Reader r{hs.data(), hs.size()};
    r.get<uint32_t>();                                  // version
    const uint32_t count = r.get<uint32_t>();
    if (!r.ok || count > 4096) return false;
    for (uint32_t i = 0; i < count; ++i) {
        HouseEntry e{r.pos, 0, 0};
        e.key = r.get<uint64_t>();
        const uint64_t n = r.get<uint64_t>();
        if (!r.ok || n > 256 || !r.need((size_t)n + 24)) return false;
        r.pos += (size_t)n + 24;
        e.len = r.pos - e.at;
        out.push_back(e);
    }
    return r.ok;
}
} // namespace

bool house_state_keys(const std::vector<uint8_t>& hs, std::vector<uint64_t>& keys) {
    std::vector<HouseEntry> es;
    if (!house_entries(hs, es)) return false;
    keys.clear();
    for (const HouseEntry& e : es) keys.push_back(e.key);
    return true;
}

int house_state_remove(std::vector<uint8_t>& hs, const std::vector<uint64_t>& keys) {
    std::vector<HouseEntry> es;
    if (!house_entries(hs, es)) return -1;
    const std::set<uint64_t> drop(keys.begin(), keys.end());
    std::vector<uint8_t> out(hs.begin(), hs.begin() + 8);
    const size_t tail = es.empty() ? 8 : es.back().at + es.back().len;
    uint32_t kept = 0;
    int removed = 0;
    for (const HouseEntry& e : es) {
        if (drop.count(e.key)) { ++removed; continue; }
        out.insert(out.end(), hs.begin() + (ptrdiff_t)e.at, hs.begin() + (ptrdiff_t)(e.at + e.len));
        ++kept;
    }
    out.insert(out.end(), hs.begin() + (ptrdiff_t)tail, hs.end());
    memcpy(out.data() + 4, &kept, 4);
    hs = std::move(out);
    return removed;
}

// --- hote : les cles des chats d'invites -----------------------------------------------

bool team_plan_keys(sqlite3* host, const std::vector<TeamCat>& cats, uint64_t min_key,
                    std::vector<KeyPlan>& plan, std::string& why) {
    plan.clear();
    why.clear();
    if (!host || cats.empty()) { why = "rien a planifier"; return false; }
    std::vector<KeyPlan>  out(cats.size());
    std::vector<uint64_t> ids(cats.size(), 0);
    std::vector<bool>     plain(cats.size(), false);   // une ligne du meme id, sans la marque
    for (size_t i = 0; i < cats.size(); ++i) {
        CatInfo info;
        if (!cat_info(cats[i].data, info)) {
            out[i].refused = true;
            out[i].why     = "il n'a pas le format attendu";
            continue;
        }
        if (std::find(ids.begin(), ids.end(), info.id) != ids.end()) {
            out[i].refused = true;
            out[i].why     = info.name + " est deux fois dans la liste";
            continue;
        }
        ids[i] = info.id;
    }
    // Toute la table : la plus grande cle, et les ids deja la (morts compris).
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(host, "SELECT key, data FROM cats", -1, &st, nullptr) != SQLITE_OK) {
        why = std::string("sqlite : ") + sqlite3_errmsg(host);
        return false;
    }
    int64_t top = 0;
    int rc;
    std::vector<uint8_t> raw;
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        if (sqlite3_column_type(st, 0) != SQLITE_INTEGER) continue;
        const int64_t key = (int64_t)sqlite3_column_int64(st, 0);
        top = std::max<int64_t>(top, key);
        const uint8_t* p = (const uint8_t*)sqlite3_column_blob(st, 1);
        const int      n = sqlite3_column_bytes(st, 1);
        CatInfo have;
        if (key <= 0 || !p || n <= 0 || !cat_blob_unpack(p, (size_t)n, raw) || !cat_info(raw, have) ||
            !have.id)
            continue;
        for (size_t i = 0; i < cats.size(); ++i) {
            if (out[i].refused || ids[i] != have.id) continue;
            if (!(have.flags & kCatGuest)) { plain[i] = true; continue; }
            if (!out[i].reuse || (uint64_t)key > out[i].key) {   // la plus haute ligne marquee
                out[i].key   = (uint64_t)key;
                out[i].reuse = true;
            }
        }
    }
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) { why = std::string("sqlite : ") + sqlite3_errmsg(host); return false; }
    uint64_t next = std::max<uint64_t>((uint64_t)std::max<int64_t>(top, 0) + 1, std::max<uint64_t>(min_key, 1));
    for (size_t i = 0; i < cats.size(); ++i) {
        KeyPlan& kp = out[i];
        if (kp.refused || kp.reuse) continue;
        if (plain[i]) {
            CatInfo info;
            cat_info(cats[i].data, info);
            kp.refused = true;
            kp.why     = info.name + " est deja dans la save de l'hote (une save copiee de l'autre ?)";
            continue;
        }
        kp.key = next++;
    }
    plan = std::move(out);
    return true;
}

// --- invite : l'etat final de ses chats dans sa save ---------------------------------

bool team_apply_results(sqlite3* own, const std::vector<TeamCat>& results,
                        const std::vector<TeamCat>* departed, TeamApplySummary& sum,
                        const TeamStorageChange* storage) {
    sum = TeamApplySummary{};
    if (!own || results.empty()) return false;
    if (storage && storage->post.size() < 4) storage = nullptr;   // pas un rangement

    // D'abord tout preparer, sans rien ecrire.
    struct Plan { uint64_t key; std::vector<uint8_t> blob; std::string name; bool leaves; bool dead; };
    std::vector<Plan> plans;
    for (const TeamCat& r : results) {
        CatInfo fin;
        std::vector<uint8_t> raw = r.data;
        if (!cat_info(raw, fin)) {
            log_line("TEAM", "!! chat %llu : etat final illisible -- laisse tel quel",
                     (unsigned long long)r.own_key);
            ++sum.skipped;
            continue;
        }
        std::vector<uint8_t> have_blob, have;
        CatInfo cur;
        const int f = get_row(own, "SELECT data FROM cats WHERE key = ?1", nullptr,
                              (int64_t)r.own_key, have_blob);
        if (f < 0) return false;
        if (f == 0 || !cat_blob_unpack(have_blob.data(), have_blob.size(), have) ||
            !cat_info(have, cur) || cur.id != fin.id) {
            log_line("TEAM", "!! %s : cette save n'a pas ce chat a la cle %llu -- laisse tel quel",
                     fin.name.c_str(), (unsigned long long)r.own_key);
            ++sum.skipped;
            continue;
        }
        // Deja mort dans cette save : un etat final ne le ramene pas. Il vient
        // d'une aventure partie d'un etat plus ancien que cette mort (le meme
        // chat dans deux departs, le premier rendu en retard) ; vivant hors de
        // house_state, il disparaitrait.
        if (cur.flags & kCatDead) {
            log_line("TEAM", "%s est mort dans cette save depuis le depart -- son etat final ne le change pas",
                     fin.name.c_str());
            sum.changed.push_back(fin.name);
            continue;
        }
        if (departed) {
            const TeamCat* d = nullptr;
            for (const TeamCat& x : *departed)
                if (x.own_key == r.own_key) d = &x;
            if (!d || d->data != have) {
                log_line("TEAM", "%s a change depuis le depart -- son etat final arrive trop tard, "
                                 "il reste tel quel", fin.name.c_str());
                sum.changed.push_back(fin.name);
                continue;
            }
        }
        const bool dead   = (fin.flags & kCatDead) != 0;
        const bool leaves = dead || r.fate == kFateGone;
        cat_set_flags(raw, fin.flags & ~(kCatAway | kCatGuest));   // l'aventure est finie
        plans.push_back(Plan{r.own_key, cat_blob_pack(raw), fin.name, leaves, dead});
    }
    if (plans.empty()) {
        if (storage) sum.storage = kStorageCatsKept;
        return true;
    }

    // Les morts et les perdus quittent la maison : sans house_state lisible, on
    // ne les ecrit pas du tout.
    std::vector<uint64_t> leave_keys;
    for (const Plan& p : plans) if (p.leaves) leave_keys.push_back(p.key);
    std::vector<uint8_t> hs;
    bool hs_dirty = false;
    if (!leave_keys.empty()) {
        const int hf = get_row(own, "SELECT data FROM files WHERE key = ?1", "house_state", 0, hs);
        if (hf < 0) return false;
        if (hf == 0 || house_state_remove(hs, leave_keys) < 0) {
            log_line("TEAM", "!! house_state illisible : les chats morts ou perdus restent tels "
                             "qu'avant le depart");
            plans.erase(std::remove_if(plans.begin(), plans.end(),
                                       [&](const Plan& p) {
                                           if (p.leaves) ++sum.skipped;
                                           return p.leaves;
                                       }),
                        plans.end());
        } else {
            hs_dirty = true;
        }
    }
    if (plans.empty()) {
        if (storage) sum.storage = kStorageCatsKept;
        return true;
    }

    // Le rangement : seulement avec tous les chats partis (un chat laisse tel
    // quel, ou absent des resultats, n'a pas les objets qui ont quitte le
    // rangement), et si la save a encore celui du depart.
    bool write_storage = false;
    if (storage) {
        bool all = true;
        for (uint64_t k : storage->cats) {
            bool planned = false;
            for (const Plan& p : plans) planned |= p.key == k;
            all &= planned;
        }
        if (!sum.changed.empty() || sum.skipped || !all) {
            sum.storage = kStorageCatsKept;
        } else {
            std::vector<uint8_t> now;
            const int f = get_row(own, "SELECT data FROM files WHERE key = ?1", "inventory_storage", 0, now);
            if (f < 0) return false;
            if (f == 1 && now == storage->post)
                sum.storage = kStorageSame;
            else if (f == 0 ? !storage->pre_disk_found
                            : ((storage->pre_disk_found && now == storage->pre_disk) || now == storage->pre_mem))
                write_storage = true;
            else
                sum.storage = kStorageChanged;
        }
    }

    if (!exec(own, "BEGIN")) return false;
    bool ok = true;
    for (const Plan& p : plans) {
        if (!ok) break;
        ok = put_blob(own, "UPDATE cats SET data = ?2 WHERE key = ?1", nullptr, (int64_t)p.key, p.blob);
    }
    if (ok && hs_dirty)
        ok = put_blob(own, "INSERT OR REPLACE INTO files(key, data) VALUES(?1, ?2)", "house_state", 0, hs);
    if (ok && write_storage)
        ok = put_blob(own, "INSERT OR REPLACE INTO files(key, data) VALUES(?1, ?2)", "inventory_storage", 0,
                      storage->post);
    if (ok && exec(own, "COMMIT")) {
        for (const Plan& p : plans)
            (p.dead ? sum.lost : p.leaves ? sum.gone : sum.back).push_back(p.name);
        if (write_storage) sum.storage = kStorageWritten;
        return true;
    }
    exec(own, "ROLLBACK");
    if (storage) sum.storage = kStorageNone;
    return false;
}

// --- invite : sa bourse dans sa maison ---------------------------------------------

namespace {

// 1 trouvee (un entier), 0 absente ou pas un entier (`out` = `def`, ce que le
// jeu en lit alors), -1 erreur.
int get_property_int(sqlite3* db, const char* key, int64_t def, int64_t& out) {
    out = def;
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT data FROM properties WHERE key = ?1", -1, &st, nullptr) != SQLITE_OK)
        return -1;
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    const int rc = sqlite3_step(st);
    int res = -1;
    if (rc == SQLITE_ROW) {
        res = 0;
        if (sqlite3_column_type(st, 0) == SQLITE_INTEGER) {
            out = (int64_t)sqlite3_column_int64(st, 0);
            res = 1;
        }
    } else if (rc == SQLITE_DONE) {
        res = 0;
    }
    sqlite3_finalize(st);
    return res;
}

bool put_property_int(sqlite3* db, const char* key, int64_t v) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO properties(key, data) VALUES(?1, ?2)", -1, &st,
                           nullptr) != SQLITE_OK)
        return false;
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_int64(st, 2, (sqlite3_int64)v);
    const bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

int64_t add_clamped(int64_t have, int64_t more) {
    int64_t v = have + more;
    if (v < 0) v = 0;
    if (v > INT32_MAX) v = INT32_MAX;
    return v;
}

} // namespace

bool team_apply_purse(sqlite3* own, int64_t coins, int64_t food, TeamPurseApply* out) {
    if (!own) return false;
    if (coins < 0) coins = 0;
    if (food < 0) food = 0;
    if (coins > INT32_MAX) coins = INT32_MAX;
    if (food > INT32_MAX) food = INT32_MAX;
    TeamPurseApply r;
    // La lecture et l'ecriture dans la meme transaction.
    if (!exec(own, "BEGIN IMMEDIATE")) return false;
    // Les valeurs que le jeu prend quand la ligne manque (MewDirector, 0x1402067FC
    // et 0x14020684E : get_int("house_food", 25), get_int("house_gold", 0)).
    const int g = get_property_int(own, "house_gold", 0, r.gold_before);
    const int f = get_property_int(own, "house_food", 25, r.food_before);
    if (g < 0 || f < 0) {
        log_line("TEAM", "!! house_gold ou house_food illisible dans cette save -- la bourse n'y est pas ajoutee");
        exec(own, "ROLLBACK");
        return false;
    }
    r.gold_after = coins ? add_clamped(r.gold_before, coins) : r.gold_before;
    r.food_after = food ? add_clamped(r.food_before, food) : r.food_before;
    bool ok = true;
    if (coins) ok = put_property_int(own, "house_gold", r.gold_after);
    if (ok && food) ok = put_property_int(own, "house_food", r.food_after);
    if (ok && exec(own, "COMMIT")) {
        if (out) *out = r;
        return true;
    }
    exec(own, "ROLLBACK");
    return false;
}

// --- le butin (mm_loot) ----------------------------------------------------------------------

std::vector<uint8_t> team_item_record(const ItemFields& f) {
    // Ce qu'ecrit le serialiseur d'un objet (0x14022D2A0, version 5), sans table
    // de chaines : u32 version, u8 present, le nom et le texte (u64 n + octets),
    // a, b, c, d (u32), e (u8), f (u8).
    std::vector<uint8_t> b;
    put<uint32_t>(b, 5);
    put<uint8_t>(b, 1);
    put<uint64_t>(b, (uint64_t)f.name.size());
    b.insert(b.end(), f.name.begin(), f.name.end());
    put<uint64_t>(b, (uint64_t)f.text.size());
    b.insert(b.end(), f.text.begin(), f.text.end());
    put<uint32_t>(b, f.a);
    put<uint32_t>(b, f.b);
    put<uint32_t>(b, f.c);
    put<uint32_t>(b, f.d);
    put<uint8_t>(b, f.e);
    put<uint8_t>(b, (uint8_t)f.f);
    return b;
}

bool team_item_parse(const uint8_t* p, size_t n, size_t& pos, ItemFields& out) {
    if (!p || pos > n) return false;
    Reader r{p, n};
    r.pos = pos;
    if (r.get<uint32_t>() != 5 || r.get<uint8_t>() != 1) return false;
    ItemFields f;
    for (int k = 0; k < 2; ++k) {
        const uint64_t len = r.get<uint64_t>();
        if (!r.ok || len > (k ? 4096u : 512u) || !r.need((size_t)len)) return false;
        (k ? f.text : f.name).assign((const char*)p + r.pos, (size_t)len);
        r.pos += (size_t)len;
    }
    f.a = r.get<uint32_t>();
    f.b = r.get<uint32_t>();
    f.c = r.get<uint32_t>();
    f.d = r.get<uint32_t>();
    f.e = r.get<uint8_t>();
    f.f = r.get<uint8_t>();
    if (!r.ok || f.name.empty()) return false;
    pos = r.pos;
    out = std::move(f);
    return true;
}

namespace {
constexpr uint32_t kPendingMagic   = 0x5454414D;   // « MATT »
constexpr uint8_t  kPendingVersion = 1;
constexpr uint32_t kMaxPending     = 4096;
} // namespace

std::vector<uint8_t> team_pending_encode(const std::vector<PendingLoot>& loot, const std::vector<PendingPayout>& pay) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kPendingMagic);
    put<uint8_t>(b, kPendingVersion);
    put<uint32_t>(b, (uint32_t)loot.size());
    for (const PendingLoot& l : loot) {
        put<uint8_t>(b, l.seq);
        put<uint64_t>(b, l.key);
        const size_t nn = std::min(l.name.size(), (size_t)1024);
        put<uint16_t>(b, (uint16_t)nn);
        b.insert(b.end(), l.name.begin(), l.name.begin() + (ptrdiff_t)nn);
        const std::vector<uint8_t> rec = team_item_record(l.item);
        put<uint32_t>(b, (uint32_t)rec.size());
        b.insert(b.end(), rec.begin(), rec.end());
    }
    put<uint32_t>(b, (uint32_t)pay.size());
    for (const PendingPayout& p : pay) {
        put<uint8_t>(b, p.seq);
        put<int32_t>(b, p.coins);
        put<int32_t>(b, p.food);
    }
    return b;
}

bool team_pending_decode(const uint8_t* p, size_t n, std::vector<PendingLoot>& loot_out,
                         std::vector<PendingPayout>& pay_out) {
    loot_out.clear();
    pay_out.clear();
    if (!p) return false;
    std::vector<PendingLoot>   loot;
    std::vector<PendingPayout> pay;
    Reader r{p, n};
    if (r.get<uint32_t>() != kPendingMagic || r.get<uint8_t>() != kPendingVersion) return false;
    const uint32_t nl = r.get<uint32_t>();
    if (!r.ok || nl > kMaxPending) return false;
    for (uint32_t i = 0; i < nl; ++i) {
        PendingLoot l;
        l.seq = r.get<uint8_t>();
        l.key = r.get<uint64_t>();
        const uint16_t nn = r.get<uint16_t>();
        if (!r.ok || nn > 1024 || !r.need(nn)) return false;
        l.name.assign((const char*)p + r.pos, nn);
        r.pos += nn;
        const uint32_t rl = r.get<uint32_t>();
        if (!r.ok || !r.need(rl)) return false;
        size_t pos = r.pos;
        if (!team_item_parse(p, r.pos + rl, pos, l.item) || pos != r.pos + rl) return false;
        r.pos = pos;
        loot.push_back(std::move(l));
    }
    const uint32_t np = r.get<uint32_t>();
    if (!r.ok || np > kMaxPending) return false;
    for (uint32_t i = 0; i < np; ++i) {
        PendingPayout q;
        q.seq   = r.get<uint8_t>();
        q.coins = r.get<int32_t>();
        q.food  = r.get<int32_t>();
        pay.push_back(q);
    }
    if (!r.ok || r.pos != n) return false;
    loot_out = std::move(loot);
    pay_out  = std::move(pay);
    return true;
}

int team_apply_loot(sqlite3* own, const std::vector<ItemFields>& items, uint32_t* count_before,
                    uint32_t* count_after) {
    if (!own) return -1;
    if (items.empty()) return 1;
    if (!exec(own, "BEGIN IMMEDIATE")) return -1;
    std::vector<uint8_t> blob;
    const int f = get_row(own, "SELECT data FROM files WHERE key = ?1", "inventory_storage", 0, blob);
    if (f < 0) { exec(own, "ROLLBACK"); return -1; }
    // Un rangement absent ou vide : u32 nombre d'objets = 0 (ce qu'ecrit le jeu
    // pour un seau vide, 4 octets).
    if (blob.size() < 4) blob.assign(4, 0);
    uint32_t count = 0;
    memcpy(&count, blob.data(), 4);
    if (count > 100000 || blob.size() > kMaxStorageBytes) {
        log_line("TEAM", "!! rangement de cette save illisible (%u objet(s), %zu octets) -- le butin n'y est pas ajoute",
                 count, blob.size());
        exec(own, "ROLLBACK");
        return 0;
    }
    if (count_before) *count_before = count;
    for (const ItemFields& it : items) {
        const std::vector<uint8_t> rec = team_item_record(it);
        blob.insert(blob.end(), rec.begin(), rec.end());
        ++count;
    }
    memcpy(blob.data(), &count, 4);
    if (count_after) *count_after = count;
    if (put_blob(own, "INSERT OR REPLACE INTO files(key, data) VALUES(?1, ?2)", "inventory_storage", 0, blob) &&
        exec(own, "COMMIT"))
        return 1;
    exec(own, "ROLLBACK");
    return -1;
}

} // namespace mgmp
