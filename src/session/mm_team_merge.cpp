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

constexpr uint32_t kTeamMagic  = 0x4D41454D;   // « MEAM » (Mewgenics multi, equipe)
constexpr uint8_t  kTeamFormat = 2;            // 2 : vote, jour de l'invite, sort de chaque chat, cle hote des proprietaires
constexpr size_t   kMaxCatBytes = 1u << 16;    // un chat fait moins d'un kilo-octet
constexpr size_t   kMaxTeam     = 16;

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

uint8_t team_kind(const uint8_t* p, size_t n) {
    if (!p) return 0;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat) return 0;
    const uint8_t kind = r.get<uint8_t>();
    if (!r.ok || kind < kTeamDepart || kind > kTeamOwners) return 0;
    return kind;
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
                        const std::vector<TeamCat>* departed, TeamApplySummary& sum) {
    sum = TeamApplySummary{};
    if (!own || results.empty()) return false;

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
    if (plans.empty()) return true;

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
    if (plans.empty()) return true;

    if (!exec(own, "BEGIN")) return false;
    bool ok = true;
    for (const Plan& p : plans) {
        if (!ok) break;
        ok = put_blob(own, "UPDATE cats SET data = ?2 WHERE key = ?1", nullptr, (int64_t)p.key, p.blob);
    }
    if (ok && hs_dirty)
        ok = put_blob(own, "INSERT OR REPLACE INTO files(key, data) VALUES(?1, ?2)", "house_state", 0, hs);
    if (ok && exec(own, "COMMIT")) {
        for (const Plan& p : plans)
            (p.dead ? sum.lost : p.leaves ? sum.gone : sum.back).push_back(p.name);
        return true;
    }
    exec(own, "ROLLBACK");
    return false;
}

} // namespace mgmp
