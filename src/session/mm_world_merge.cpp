// mm_world_merge -- Mewgenics Multi : recopier le monde d'une save dans une
// autre. Voir mm_world_merge.h et mm_world.h.

#include "mm_world_merge.h"
#include "mm_npc_state.h"

#include "mgmp_log.h"
#include "sqlite3.h"

#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- ce qui fait partie du monde --------------------------------------------
//
// Les blocs de la table « files » (voir mm_world.h pour la regle de chacun) et
// les proprietes de la table « properties » qui appartiennent a l'histoire.
// Releve sur trois saves du joueur (jours 5, 25 et 52) et dans le code qui les
// lit (sub_1401CC610 pour mapflag_/sfunlockcounter_, le serialiseur des PNJ
// sub_1401D3B90, la maison sub_1401E8B40).
constexpr const char* kWorldPrefixes[] = {
    "NPCRSTRACKER_",              // ce que les PNJ ont deja dit ou vendu
    "map_unlock_",                // zones de la carte, par chapitre
    "mapflag_",                   // zones debloquees (Sewers, Junkyard...)
    "sfunlockcounter_",           // compteurs vers un deblocage (musiques, boss)
    "unlock_beanies_",            // bonnets debloques par chapitre
    "catinfo_unlock_",            // infos sur les chats offertes par Tink
    "WorldEventLegacyCounter_",   // evenements du monde deja vus
    "PlotFlag_",                  // drapeaux de l'histoire
};
constexpr const char* kWorldKeys[] = {
    "butch_box_unlocked", "house_pipe_unlocked", "departed_first_real_adventure",
    "game_began", "tutorial_skipped", "organname_set",
    "house_storage_upgrades",     // le rangement agrandi par Butch : « déblocages à tous »
};

bool is_world_prop_impl(const char* key) {
    if (!key) return false;
    for (const char* p : kWorldPrefixes)
        if (strncmp(key, p, strlen(p)) == 0) return true;
    for (const char* k : kWorldKeys)
        if (strcmp(key, k) == 0) return true;
    return false;
}

// --- sqlite -----------------------------------------------------------------



bool exec(sqlite3* db, const char* sql) {
    char* err = nullptr;
    const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        log_line("WORLD", "!! sqlite : %s -- %s", sql, err ? err : sqlite3_errmsg(db));
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool looks_like_save_impl(sqlite3* db) {
    sqlite3_stmt* st = nullptr;
    bool ok = sqlite3_prepare_v2(db, "PRAGMA quick_check", -1, &st, nullptr) == SQLITE_OK &&
              sqlite3_step(st) == SQLITE_ROW &&
              sqlite3_column_text(st, 0) &&
              strcmp((const char*)sqlite3_column_text(st, 0), "ok") == 0;
    sqlite3_finalize(st);
    if (!ok) return false;
    // Deux vraies tables (pas des vues) : files et properties.
    st = nullptr;
    ok = sqlite3_prepare_v2(db, "SELECT count(*) FROM sqlite_master WHERE type = 'table' "
                                "AND name IN ('files', 'properties')", -1, &st, nullptr) == SQLITE_OK &&
         sqlite3_step(st) == SQLITE_ROW && sqlite3_column_int(st, 0) == 2;
    sqlite3_finalize(st);
    return ok;
}

// 1 = trouve, 0 = absent, -1 = erreur de lecture (la transaction echoue alors :
// une erreur ne doit jamais passer pour « absent », ce qui ferait ecraser).
int get_blob(sqlite3* db, const char* key, std::vector<uint8_t>& out) {
    out.clear();
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT data FROM files WHERE key = ?1", -1, &st, nullptr) != SQLITE_OK) {
        log_line("WORLD", "!! sqlite : lecture de %s -- %s", key, sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    int found = 0;
    const int rc = sqlite3_step(st);
    if (rc == SQLITE_ROW) {
        const void* p = sqlite3_column_blob(st, 0);
        const int   n = sqlite3_column_bytes(st, 0);
        if (p && n > 0) out.assign((const uint8_t*)p, (const uint8_t*)p + n);
        found = 1;
    } else if (rc != SQLITE_DONE) {
        log_line("WORLD", "!! sqlite : lecture de %s -- %s", key, sqlite3_errmsg(db));
        found = -1;
    }
    sqlite3_finalize(st);
    return found;
}

bool put_blob(sqlite3* db, const char* key, const std::vector<uint8_t>& b) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO files(key, data) VALUES(?1, ?2)", -1,
                           &st, nullptr) != SQLITE_OK) {
        log_line("WORLD", "!! sqlite : %s", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_blob(st, 2, b.empty() ? "" : (const void*)b.data(), (int)b.size(), SQLITE_TRANSIENT);
    const bool ok = sqlite3_step(st) == SQLITE_DONE;
    if (!ok) log_line("WORLD", "!! sqlite : ecriture de %s -- %s", key, sqlite3_errmsg(db));
    sqlite3_finalize(st);
    return ok;
}

// --- les blocs faits de listes de noms ----------------------------------------
//
// unlocks : u32 version (3), puis sept listes ; tutorial_tokens : une liste.
// Une liste = u64 nombre, puis pour chaque nom u64 longueur + octets. Lu sur
// les saves du joueur (« Colorless, Fighter... », « ratking, queenhippo »).
struct Lists {
    bool     has_ver = false;
    uint32_t ver     = 0;
    std::vector<std::vector<std::string>> lists;
};

bool rd_u64(const std::vector<uint8_t>& b, size_t& pos, uint64_t& v) {
    if (pos + 8 > b.size()) return false;
    memcpy(&v, b.data() + pos, 8);
    pos += 8;
    return true;
}

bool rd_str(const std::vector<uint8_t>& b, size_t& pos, std::string& s) {
    uint64_t n = 0;
    if (!rd_u64(b, pos, n) || n > 4096 || pos + n > b.size()) return false;
    s.assign((const char*)b.data() + pos, (size_t)n);
    pos += (size_t)n;
    return true;
}

void wr_u64(std::vector<uint8_t>& b, uint64_t v) {
    const uint8_t* p = (const uint8_t*)&v;
    b.insert(b.end(), p, p + 8);
}

void wr_str(std::vector<uint8_t>& b, const std::string& s) {
    wr_u64(b, s.size());
    b.insert(b.end(), s.begin(), s.end());
}

bool parse_lists(const std::vector<uint8_t>& b, bool has_ver, Lists& out) {
    out = Lists{};
    out.has_ver = has_ver;
    size_t pos = 0;
    if (has_ver) {
        if (b.size() < 4) return false;
        memcpy(&out.ver, b.data(), 4);
        pos = 4;
    }
    while (pos < b.size()) {
        uint64_t n = 0;
        if (!rd_u64(b, pos, n) || n > 100000) return false;
        std::vector<std::string> items;
        for (uint64_t i = 0; i < n; ++i) {
            std::string s;
            if (!rd_str(b, pos, s)) return false;
            items.push_back(std::move(s));
        }
        out.lists.push_back(std::move(items));
        if (out.lists.size() > 64) return false;
    }
    return pos == b.size() && !out.lists.empty();
}

std::vector<uint8_t> write_lists(const Lists& l) {
    std::vector<uint8_t> b;
    if (l.has_ver) {
        const uint8_t* p = (const uint8_t*)&l.ver;
        b.insert(b.end(), p, p + 4);
    }
    for (const auto& items : l.lists) {
        wr_u64(b, items.size());
        for (const auto& s : items) wr_str(b, s);
    }
    return b;
}

// Les noms de l'hote en plus des siens, liste par liste. Faux si les deux
// formats ne s'accordent pas (alors on prend le bloc de l'hote tel quel).
bool union_lists(const std::vector<uint8_t>& own_b, const std::vector<uint8_t>& host_b,
                 bool has_ver, std::vector<uint8_t>& out, int& added) {
    added = 0;
    Lists own, host;
    if (!parse_lists(own_b, has_ver, own) || !parse_lists(host_b, has_ver, host)) return false;
    if (own.ver != host.ver || own.lists.size() != host.lists.size()) return false;
    for (size_t i = 0; i < own.lists.size(); ++i) {
        std::set<std::string> have(own.lists[i].begin(), own.lists[i].end());
        for (const auto& s : host.lists[i])
            if (have.insert(s).second) { own.lists[i].push_back(s); ++added; }
    }
    out = write_lists(own);
    return true;
}

// --- house_unlocks ----------------------------------------------------------
//
// u32 version (1), la maison actuelle (« House1 »), puis la liste des
// agrandissements (« Default », « SmallHouse_Attic », « MediumHouse »...), puis
// une queue (huit octets nuls sur toutes les saves vues), gardee telle quelle.
// House1/2/3 vont avec les agrandissements MediumHouse et LargeHouse
// (data/house.gon) : la plus grande des deux maisons gagne.
struct HouseUnlocks {
    uint32_t                 ver = 0;
    std::string              house;
    std::vector<std::string> ups;
    std::vector<uint8_t>     tail;
};

bool parse_house(const std::vector<uint8_t>& b, HouseUnlocks& h) {
    h = HouseUnlocks{};
    if (b.size() < 4) return false;
    memcpy(&h.ver, b.data(), 4);
    size_t pos = 4;
    if (!rd_str(b, pos, h.house) || h.house.size() > 64) return false;
    uint64_t n = 0;
    if (!rd_u64(b, pos, n) || n > 1000) return false;
    for (uint64_t i = 0; i < n; ++i) {
        std::string s;
        if (!rd_str(b, pos, s)) return false;
        h.ups.push_back(std::move(s));
    }
    h.tail.assign(b.begin() + (ptrdiff_t)pos, b.end());
    return true;
}

std::vector<uint8_t> write_house(const HouseUnlocks& h) {
    std::vector<uint8_t> b;
    const uint8_t* p = (const uint8_t*)&h.ver;
    b.insert(b.end(), p, p + 4);
    wr_str(b, h.house);
    wr_u64(b, h.ups.size());
    for (const auto& s : h.ups) wr_str(b, s);
    b.insert(b.end(), h.tail.begin(), h.tail.end());
    return b;
}

int house_rank(const std::string& s) {
    if (s.size() < 6 || s.compare(0, 5, "House") != 0) return 0;
    return atoi(s.c_str() + 5);
}

// --- appliquer le monde de l'hote ------------------------------------------------

bool merge_names(sqlite3* host, sqlite3* own, const char* key, bool has_ver, int& added) {
    added = 0;
    std::vector<uint8_t> hb, ob, merged;
    const int hf = get_blob(host, key, hb);
    if (hf < 0) return false;
    if (hf == 0 || hb.empty()) return true;                          // rien chez l'hote
    const int of = get_blob(own, key, ob);
    if (of < 0) return false;
    if (of == 0 || ob.empty()) return put_blob(own, key, hb);       // rien chez ce joueur
    if (union_lists(ob, hb, has_ver, merged, added)) {
        if (!added) return true;
        return put_blob(own, key, merged);
    }
    // Jamais rien d'enleve : un format qu'on ne comprend pas laisse le sien.
    log_line("WORLD", "!! %s : format inattendu, celui de ce joueur est laisse tel quel", key);
    return true;
}

bool merge_house(sqlite3* host, sqlite3* own, WorldSummary& sum) {
    std::vector<uint8_t> hb, ob;
    const int hf = get_blob(host, "house_unlocks", hb);
    const int of = get_blob(own, "house_unlocks", ob);
    if (hf < 0 || of < 0) return false;
    if (hf == 0 || of == 0) return true;
    HouseUnlocks h, o;
    if (!parse_house(hb, h) || !parse_house(ob, o)) {
        log_line("WORLD", "!! house_unlocks : format inattendu, la maison de ce joueur est laissee telle quelle");
        return true;
    }
    std::set<std::string> have(o.ups.begin(), o.ups.end());
    for (const auto& s : h.ups)
        if (have.insert(s).second) { o.ups.push_back(s); ++sum.house_ups; }
    const std::string before = o.house;
    if (house_rank(h.house) > house_rank(o.house)) o.house = h.house;
    if (!sum.house_ups && o.house == before) return true;
    sum.house_from = before;
    sum.house_to   = o.house;
    return put_blob(own, "house_unlocks", write_house(o));
}

// Pour chaque propriete du monde chez l'hote : la plus grande des deux valeurs
// (des drapeaux et des compteurs), ou la sienne si ce joueur ne l'a pas.
// Jamais rien d'enleve : ce que ce joueur avait debloque lui reste.
bool merge_props(sqlite3* host, sqlite3* own, WorldSummary& sum) {
    sqlite3_stmt* hs = nullptr;
    sqlite3_stmt* og = nullptr;
    sqlite3_stmt* op = nullptr;
    bool ok = sqlite3_prepare_v2(host, "SELECT key, data FROM properties", -1, &hs, nullptr) == SQLITE_OK &&
              sqlite3_prepare_v2(own, "SELECT data FROM properties WHERE key = ?1", -1, &og, nullptr) == SQLITE_OK &&
              sqlite3_prepare_v2(own, "INSERT OR REPLACE INTO properties(key, data) VALUES(?1, ?2)", -1, &op, nullptr) == SQLITE_OK;
    if (!ok) log_line("WORLD", "!! sqlite : %s", sqlite3_errmsg(own));
    while (ok) {
        const int rc = sqlite3_step(hs);
        if (rc == SQLITE_DONE) break;
        if (rc != SQLITE_ROW) { ok = false; break; }
        const char* key = (const char*)sqlite3_column_text(hs, 0);
        if (!is_world_prop_impl(key)) continue;
        const int htype = sqlite3_column_type(hs, 1);

        sqlite3_reset(og);
        sqlite3_bind_text(og, 1, key, -1, SQLITE_TRANSIENT);
        const int orc = sqlite3_step(og);
        bool write = false;
        if (orc == SQLITE_ROW) {
            if (htype == SQLITE_INTEGER && sqlite3_column_type(og, 0) == SQLITE_INTEGER &&
                sqlite3_column_int64(hs, 1) > sqlite3_column_int64(og, 0))
                write = true;
        } else if (orc == SQLITE_DONE) {
            write = true;
        } else {
            ok = false;
            break;
        }
        if (!write) continue;
        sqlite3_reset(op);
        sqlite3_bind_text(op, 1, key, -1, SQLITE_TRANSIENT);
        sqlite3_bind_value(op, 2, sqlite3_column_value(hs, 1));
        if (sqlite3_step(op) != SQLITE_DONE) {
            log_line("WORLD", "!! sqlite : propriete %s -- %s", key, sqlite3_errmsg(own));
            ok = false;
            break;
        }
        ++sum.props;
    }
    sqlite3_finalize(hs);
    sqlite3_finalize(og);
    sqlite3_finalize(op);
    return ok;
}

bool apply_world_impl(sqlite3* host, sqlite3* own, WorldSummary& sum) {
    // BEGIN differe, et aucune ecriture quand rien ne change : une save deja au
    // monde de l'hote ne prend jamais de verrou d'ecriture.
    if (!exec(own, "BEGIN")) return false;
    bool ok = true;

    // Les PNJ : ceux de l'hote -- un seul monde, sa faveur, ses quetes. Sauf
    // une recompense prete : c'est l'hote, qui a fait les dons, qui la prend ;
    // ce qu'elle debloque arrive ici par les autres blocs (npc_progress_for_guest).
    std::vector<uint8_t> npc, own_npc;
    const int hf = get_blob(host, "npc_progress", npc);
    const int of = hf > 0 ? get_blob(own, "npc_progress", own_npc) : 0;
    if (hf < 0 || of < 0) ok = false;
    else if (hf > 0 && !npc.empty()) {
        sum.npc_ready_cleared = npc_progress_for_guest(npc, own_npc);
        if (sum.npc_ready_cleared < 0)
            log_line("WORLD", "!! npc_progress : format inattendu, recopie tel quel");
        if (npc != own_npc) {
            ok = put_blob(own, "npc_progress", npc);
            sum.npc = ok;
        }
    }
    if (ok) ok = merge_names(host, own, "unlocks", true, sum.unlocks);
    if (ok) ok = merge_names(host, own, "tutorial_tokens", false, sum.tutorials);
    if (ok) ok = merge_house(host, own, sum);
    if (ok) ok = merge_props(host, own, sum);

    if (ok && exec(own, "COMMIT")) return true;
    exec(own, "ROLLBACK");
    return false;
}

} // namespace

bool world_is_sqlite(const uint8_t* p, size_t n) {
    return p && n >= 100 && memcmp(p, "SQLite format 3", 16) == 0;
}

sqlite3* world_open_bytes(const uint8_t* p, size_t n) {
    if (!world_is_sqlite(p, n)) return nullptr;
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) { sqlite3_close(db); return nullptr; }
    unsigned char* buf = (unsigned char*)sqlite3_malloc64(n);
    if (!buf) { sqlite3_close(db); return nullptr; }
    memcpy(buf, p, n);
    // FREEONCLOSE : sqlite libere le tampon, meme si l'ouverture echoue.
    const int rc = sqlite3_deserialize(db, "main", buf, (sqlite3_int64)n, (sqlite3_int64)n,
                                       SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY);
    if (rc != SQLITE_OK) { sqlite3_close(db); return nullptr; }
    // Des octets venus du reseau : pas de schema de confiance (vues, triggers,
    // fonctions), mode defensif.
    sqlite3_db_config(db, SQLITE_DBCONFIG_DEFENSIVE, 1, nullptr);
    sqlite3_db_config(db, SQLITE_DBCONFIG_TRUSTED_SCHEMA, 0, nullptr);
    return db;
}

bool world_looks_like_save(sqlite3* db) { return db && looks_like_save_impl(db); }

bool world_is_world_prop(const char* key) { return is_world_prop_impl(key); }

std::string world_owner(sqlite3* db) {
    std::string out;
    sqlite3_stmt* st = nullptr;
    if (!db || sqlite3_prepare_v2(db, "SELECT data FROM properties WHERE key = 'owner_steamid'",
                                  -1, &st, nullptr) != SQLITE_OK)
        return out;
    if (sqlite3_step(st) == SQLITE_ROW && sqlite3_column_text(st, 0))
        out = (const char*)sqlite3_column_text(st, 0);
    sqlite3_finalize(st);
    return out;
}

bool world_apply(sqlite3* host, sqlite3* own, WorldSummary& sum) {
    sum = WorldSummary{};
    if (!host || !own) return false;
    return apply_world_impl(host, own, sum);
}

uint64_t world_digest(sqlite3* db) {
    uint64_t h = 1469598103934665603ULL;
    auto mix = [&h](const void* p, size_t n) {
        const uint8_t* b = (const uint8_t*)p;
        for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ULL; }
    };
    if (!db) return 0;
    for (const char* key : { "npc_progress", "unlocks", "tutorial_tokens", "house_unlocks" }) {
        std::vector<uint8_t> b;
        const int found = get_blob(db, key, b);
        mix(key, strlen(key) + 1);
        mix(&found, sizeof(found));
        if (!b.empty()) mix(b.data(), b.size());
    }
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT key, data FROM properties ORDER BY key", -1, &st,
                           nullptr) == SQLITE_OK) {
        while (sqlite3_step(st) == SQLITE_ROW) {
            const char* key = (const char*)sqlite3_column_text(st, 0);
            if (!is_world_prop_impl(key)) continue;
            mix(key, strlen(key) + 1);
            const int type = sqlite3_column_type(st, 1);
            mix(&type, sizeof(type));
            if (type == SQLITE_INTEGER) {
                const sqlite3_int64 v = sqlite3_column_int64(st, 1);
                mix(&v, sizeof(v));
            } else if (type != SQLITE_NULL) {
                const void* p = sqlite3_column_blob(st, 1);
                const int   n = sqlite3_column_bytes(st, 1);
                if (p && n > 0) mix(p, (size_t)n);
            }
        }
    }
    sqlite3_finalize(st);
    return h;
}

} // namespace mgmp
