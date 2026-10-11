// test_world_merge -- Mewgenics Multi : le monde de l'hote recopie dans la save
// d'un invite (src/session/mm_world_merge.cpp), sur deux vraies bases sqlite
// construites ici avec le schema et les formats des saves du jeu :
//
//   - npc_progress : celui de l'hote, tel quel ;
//   - unlocks (u32 version + 7 listes) et tutorial_tokens (une liste) : les noms
//     de l'hote en plus de ceux de l'invite ;
//   - house_unlocks : agrandissements en plus, la plus grande maison ;
//   - proprietes du monde : la plus grande valeur ; proprietes de la maison :
//     intactes ;
//   - une save abimee : rien n'est change (une seule transaction).

#include "mm_world_merge.h"
#include "sqlite3.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {
void log_line(const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::printf("  [%s] ", tag);
    std::vprintf(fmt, ap);
    std::printf("\n");
    va_end(ap);
}
} // namespace mgmp

namespace {

int g_fail = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

using Bytes = std::vector<uint8_t>;
using Names = std::vector<std::string>;

void u32(Bytes& b, uint32_t v) { const uint8_t* p = (const uint8_t*)&v; b.insert(b.end(), p, p + 4); }
void u64(Bytes& b, uint64_t v) { const uint8_t* p = (const uint8_t*)&v; b.insert(b.end(), p, p + 8); }
void str(Bytes& b, const std::string& s) { u64(b, s.size()); b.insert(b.end(), s.begin(), s.end()); }

Bytes lists(bool ver, const std::vector<Names>& ls) {
    Bytes b;
    if (ver) u32(b, 3);
    for (const auto& l : ls) { u64(b, l.size()); for (const auto& s : l) str(b, s); }
    return b;
}

Bytes house(const std::string& h, const Names& ups) {
    Bytes b;
    u32(b, 1);
    str(b, h);
    u64(b, ups.size());
    for (const auto& s : ups) str(b, s);
    u64(b, 0);                       // la queue vue sur toutes les saves
    return b;
}

void exec(sqlite3* db, const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::printf("FAIL sql: %s -- %s\n", sql, err ? err : "?");
        ++g_fail;
    }
    sqlite3_free(err);
}

sqlite3* new_save() {
    sqlite3* db = nullptr;
    sqlite3_open(":memory:", &db);
    exec(db, "CREATE TABLE files (key TEXT PRIMARY KEY,data BLOB) STRICT");
    exec(db, "CREATE TABLE properties (key TEXT PRIMARY KEY,data ANY) STRICT");
    exec(db, "CREATE TABLE cats (key INTEGER PRIMARY KEY,data BLOB) STRICT");
    return db;
}

void put_file(sqlite3* db, const char* key, const Bytes& b) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO files VALUES(?1, ?2)", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_blob(st, 2, b.data(), (int)b.size(), SQLITE_TRANSIENT);
    CHECK(sqlite3_step(st) == SQLITE_DONE);
    sqlite3_finalize(st);
}

void put_int(sqlite3* db, const char* key, int64_t v) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO properties VALUES(?1, ?2)", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_int64(st, 2, v);
    CHECK(sqlite3_step(st) == SQLITE_DONE);
    sqlite3_finalize(st);
}

void put_text(sqlite3* db, const char* key, const char* v) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO properties VALUES(?1, ?2)", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_text(st, 2, v, -1, SQLITE_STATIC);
    CHECK(sqlite3_step(st) == SQLITE_DONE);
    sqlite3_finalize(st);
}

Bytes get_file(sqlite3* db, const char* key) {
    Bytes b;
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "SELECT data FROM files WHERE key = ?1", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    if (sqlite3_step(st) == SQLITE_ROW) {
        const uint8_t* p = (const uint8_t*)sqlite3_column_blob(st, 0);
        b.assign(p, p + sqlite3_column_bytes(st, 0));
    }
    sqlite3_finalize(st);
    return b;
}

// -1 = absente.
int64_t get_int(sqlite3* db, const char* key) {
    int64_t v = -1;
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "SELECT data FROM properties WHERE key = ?1", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    if (sqlite3_step(st) == SQLITE_ROW) v = sqlite3_column_int64(st, 0);
    sqlite3_finalize(st);
    return v;
}

Bytes serialize(sqlite3* db) {
    sqlite3_int64 n = 0;
    unsigned char* p = sqlite3_serialize(db, "main", &n, 0);
    Bytes b(p, p + n);
    sqlite3_free(p);
    return b;
}

Bytes text(const char* s) { return Bytes(s, s + strlen(s)); }

void test_merge() {
    std::printf("merge\n");
    sqlite3* h = new_save();
    put_file(h, "npc_progress", text("HOSTNPC"));
    put_file(h, "unlocks", lists(true, { { "Colorless", "Fighter", "Medic" }, {}, {}, {}, { "ratking" }, {}, {} }));
    put_file(h, "tutorial_tokens", lists(false, { { "a", "b" } }));
    put_file(h, "house_unlocks", house("House2", { "Default", "MediumHouse" }));
    put_file(h, "house_state", text("HOSTHOUSE"));
    put_int(h, "mapflag_SewersUnlocked", 1);
    put_int(h, "NPCRSTRACKER_purchase_item_a", 5);
    put_int(h, "catinfo_unlock_basestats", 1);
    put_int(h, "house_storage_upgrades", 2);
    put_int(h, "current_day", 5);
    put_int(h, "house_gold", 100);
    put_text(h, "next_house_boss", "guillotina_1");

    sqlite3* o = new_save();
    put_file(o, "npc_progress", text("OWNNPC"));
    put_file(o, "unlocks", lists(true, { { "Colorless", "Thief" }, {}, {}, {}, {}, {}, {} }));
    put_file(o, "tutorial_tokens", lists(false, { { "b", "c" } }));
    put_file(o, "house_unlocks", house("House1", { "Default", "SmallHouse_Attic" }));
    put_file(o, "house_state", text("OWNHOUSE"));
    put_int(o, "NPCRSTRACKER_purchase_item_a", 7);
    put_int(o, "house_storage_upgrades", 1);
    put_int(o, "current_day", 25);
    put_int(o, "house_gold", 3);
    put_int(o, "mapflag_CavesUnlocked", 1);

    const Bytes hb = serialize(h);
    sqlite3_close(h);
    CHECK(mgmp::world_is_sqlite(hb.data(), hb.size()));
    sqlite3* host = mgmp::world_open_bytes(hb.data(), hb.size());
    CHECK(host != nullptr);
    CHECK(mgmp::world_looks_like_save(host));

    mgmp::WorldSummary sum;
    CHECK(mgmp::world_apply(host, o, sum));
    CHECK(sum.npc);
    CHECK(sum.unlocks == 3);         // Fighter, Medic, ratking
    CHECK(sum.tutorials == 1);       // a
    CHECK(sum.house_ups == 1);       // MediumHouse
    CHECK(sum.house_from == "House1" && sum.house_to == "House2");

    CHECK(get_file(o, "npc_progress") == text("HOSTNPC"));
    CHECK(get_file(o, "unlocks") ==
          lists(true, { { "Colorless", "Thief", "Fighter", "Medic" }, {}, {}, {}, { "ratking" }, {}, {} }));
    CHECK(get_file(o, "tutorial_tokens") == lists(false, { { "b", "c", "a" } }));
    CHECK(get_file(o, "house_unlocks") == house("House2", { "Default", "SmallHouse_Attic", "MediumHouse" }));
    CHECK(get_file(o, "house_state") == text("OWNHOUSE"));          // sa maison
    CHECK(get_int(o, "mapflag_SewersUnlocked") == 1);              // celui de l'hote
    CHECK(get_int(o, "mapflag_CavesUnlocked") == 1);               // le sien reste
    CHECK(get_int(o, "NPCRSTRACKER_purchase_item_a") == 7);        // le plus grand
    CHECK(get_int(o, "catinfo_unlock_basestats") == 1);
    CHECK(get_int(o, "house_storage_upgrades") == 2);
    CHECK(get_int(o, "current_day") == 25);                        // son jour
    CHECK(get_int(o, "house_gold") == 3);                          // son or
    CHECK(get_int(o, "next_house_boss") == -1);                    // pas copie
    const int changes_after_first = sqlite3_total_changes(o);

    // Une seconde fois : plus rien a ajouter, et rien n'est ecrit.
    CHECK(mgmp::world_apply(host, o, sum));
    CHECK(!sum.npc && sum.unlocks == 0 && sum.tutorials == 0 && sum.house_ups == 0 && sum.props == 0);
    CHECK(sqlite3_total_changes(o) == changes_after_first);

    sqlite3_close(host);
    sqlite3_close(o);
}

void test_broken_own_save_is_left_alone() {
    std::printf("broken own save\n");
    sqlite3* h = new_save();
    put_file(h, "npc_progress", text("HOSTNPC"));
    put_int(h, "mapflag_SewersUnlocked", 1);
    const Bytes hb = serialize(h);
    sqlite3_close(h);
    sqlite3* host = mgmp::world_open_bytes(hb.data(), hb.size());

    // Pas de table « properties » : la recopie echoue apres npc_progress, et la
    // transaction doit tout annuler.
    sqlite3* o = nullptr;
    sqlite3_open(":memory:", &o);
    exec(o, "CREATE TABLE files (key TEXT PRIMARY KEY,data BLOB) STRICT");
    put_file(o, "npc_progress", text("OWNNPC"));

    mgmp::WorldSummary sum;
    CHECK(!mgmp::world_apply(host, o, sum));
    CHECK(get_file(o, "npc_progress") == text("OWNNPC"));
    sqlite3_close(host);
    sqlite3_close(o);
}

void test_digest() {
    std::printf("digest\n");
    sqlite3* db = new_save();
    put_file(db, "npc_progress", text("NPC"));
    put_int(db, "mapflag_SewersUnlocked", 1);
    put_int(db, "current_day", 5);
    const uint64_t d0 = mgmp::world_digest(db);
    put_int(db, "current_day", 6);                 // la maison : le monde ne change pas
    put_int(db, "house_gold", 40);
    CHECK(mgmp::world_digest(db) == d0);
    put_int(db, "mapflag_CavesUnlocked", 1);       // le monde change
    const uint64_t d1 = mgmp::world_digest(db);
    CHECK(d1 != d0);
    put_file(db, "npc_progress", text("NPC2"));
    CHECK(mgmp::world_digest(db) != d1);
    sqlite3_close(db);
}

void test_owner() {
    std::printf("owner\n");
    sqlite3* db = new_save();
    CHECK(mgmp::world_owner(db).empty());
    put_int(db, "owner_steamid", 76561198172840805LL);
    CHECK(mgmp::world_owner(db) == "76561198172840805");
    sqlite3_close(db);
}

void test_rules() {
    std::printf("rules\n");
    CHECK(mgmp::world_is_world_prop("mapflag_SewersUnlocked"));
    CHECK(mgmp::world_is_world_prop("NPCRSTRACKER_tink_tips_comfort"));
    CHECK(mgmp::world_is_world_prop("house_storage_upgrades"));
    CHECK(!mgmp::world_is_world_prop("current_day"));
    CHECK(!mgmp::world_is_world_prop("house_gold"));
    CHECK(!mgmp::world_is_world_prop("random_seed"));
    CHECK(!mgmp::world_is_world_prop("on_adventure"));
    CHECK(!mgmp::world_is_world_prop(nullptr));
    const uint8_t junk[200] = {};
    CHECK(!mgmp::world_is_sqlite(junk, sizeof(junk)));
    CHECK(mgmp::world_open_bytes(junk, sizeof(junk)) == nullptr);
}

} // namespace

int main() {
    test_rules();
    test_owner();
    test_digest();
    test_merge();
    test_broken_own_save_is_left_alone();
    if (g_fail) { std::printf("%d echec(s)\n", g_fail); return 1; }
    std::printf("ok\n");
    return 0;
}
