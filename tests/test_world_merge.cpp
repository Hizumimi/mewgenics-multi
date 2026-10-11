// test_world_merge -- Mewgenics Multi : le monde de l'hote recopie dans la save
// d'un invite (src/session/mm_world_merge.cpp), sur deux vraies bases sqlite
// construites ici avec le schema et les formats des saves du jeu :
//
//   - npc_progress : celui de l'hote, sans ses recompenses pretes (faveur
//     rabaissee sous le palier) ;
//   - unlocks (u32 version + 7 listes) et tutorial_tokens (une liste) : les noms
//     de l'hote en plus de ceux de l'invite ;
//   - house_unlocks : agrandissements en plus, la plus grande maison ;
//   - proprietes du monde : la plus grande valeur ; proprietes de la maison :
//     intactes ;
//   - une save abimee : rien n'est change (une seule transaction).
//
// Et l'etat des PNJ (src/session/mm_npc_state.cpp) : lecture de npc_progress,
// recompenses pretes laissees a l'hote, message en direct et ce que l'invite en
// reprend.

#include "mm_world_merge.h"
#include "mm_npc_state.h"
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

// npc_progress au format du jeu (version 38) : 8 PNJ, puis l'histoire (`tail`).
struct TNpc {
    Names       taken;
    std::string cur;
    uint32_t    a38 = 0, favor = 0, c44 = 0;
    uint8_t     ready = 0;
    uint32_t    e4c = 0;
    uint8_t     f50 = 1, g51 = 1, h = 0;
    uint32_t    i54 = 0;
};

Bytes npc_blob(uint32_t ver, const std::vector<TNpc>& npcs, const Bytes& tail) {
    Bytes b;
    u32(b, ver);
    for (const auto& n : npcs) {
        u64(b, n.taken.size());
        for (const auto& s : n.taken) { str(b, s); u32(b, 1); }
        str(b, n.cur);
        u32(b, n.a38); u32(b, n.favor); u32(b, n.c44);
        b.push_back(n.ready);
        u32(b, n.e4c);
        b.push_back(n.f50); b.push_back(n.g51);
        for (int i = 0; i < 32; ++i) b.push_back((uint8_t)(n.h + i));
        u32(b, n.i54);
    }
    b.insert(b.end(), tail.begin(), tail.end());
    return b;
}

std::vector<TNpc> eight_npcs() {
    std::vector<TNpc> v(8);
    const char* cur[8] = { "beanies_quests_intro", "upgrade_storage_2", "tink_basestats",
                           "house_upgrade_mediumhouse", "jack_shopupgrade2", "tracy_blankcollar1",
                           "organ_unlock", "steven_milliontrashed" };
    for (int i = 0; i < 8; ++i) { v[i].cur = cur[i]; v[i].h = (uint8_t)(i * 16); }
    v[1].taken = { "upgrade_storage_1" };
    v[3].taken = { "house_upgrade_attic" };
    v[3].favor = 4;
    v[4].taken = { "jack_shopupgrade1" };
    v[4].e4c   = 1;
    v[5].c44   = 5;
    v[7].favor = 6;
    v[7].g51   = 1;
    return v;
}

void test_merge() {
    std::printf("merge\n");
    sqlite3* h = new_save();
    std::vector<TNpc> hn = eight_npcs();
    hn[3].favor = 6;
    hn[3].ready = 1;                 // une recompense prete chez l'hote (Frank)
    const Bytes host_npc = npc_blob(38, hn, text("HISTOIRE"));
    put_file(h, "npc_progress", host_npc);
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
    std::vector<TNpc> on = eight_npcs();
    on[3].favor = 5;                 // sa copie d'avant le dernier don de l'hote
    put_file(o, "npc_progress", npc_blob(38, on, text("SON HISTOIRE")));
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
    CHECK(sum.npc_ready_cleared == 1);
    CHECK(sum.unlocks == 3);         // Fighter, Medic, ratking
    CHECK(sum.tutorials == 1);       // a
    CHECK(sum.house_ups == 1);       // MediumHouse
    CHECK(sum.house_from == "House1" && sum.house_to == "House2");

    {
        // Celui de l'hote, sauf Frank : sa recompense prete reste a l'hote, et
        // l'invite garde sa faveur d'avant (5), sous le palier.
        hn[3].ready = 0;
        hn[3].favor = 5;
        CHECK(get_file(o, "npc_progress") == npc_blob(38, hn, text("HISTOIRE")));
        mgmp::NpcState st[mgmp::kNpcCount];
        CHECK(mgmp::npc_progress_parse(get_file(o, "npc_progress"), st));
        CHECK(st[3].favor == 5 && st[3].ready == 0);
    }
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

void test_unknown_npc_format_is_copied() {
    std::printf("npc_progress illisible\n");
    sqlite3* h = new_save();
    put_file(h, "npc_progress", text("HOSTNPC"));
    const Bytes hb = serialize(h);
    sqlite3_close(h);
    sqlite3* host = mgmp::world_open_bytes(hb.data(), hb.size());
    sqlite3* o = new_save();
    put_file(o, "npc_progress", text("OWNNPC"));
    mgmp::WorldSummary sum;
    CHECK(mgmp::world_apply(host, o, sum));
    CHECK(sum.npc && sum.npc_ready_cleared == -1);
    CHECK(get_file(o, "npc_progress") == text("HOSTNPC"));   // tel quel, comme la build 11
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


void test_npc_progress() {
    std::printf("npc_progress\n");
    std::vector<TNpc> n = eight_npcs();
    n[4].favor = 3;
    n[4].ready = 1;
    n[6].ready = 1;
    const Bytes tail = text("le reste de l'histoire");
    Bytes b = npc_blob(38, n, tail);

    mgmp::NpcState st[mgmp::kNpcCount];
    uint32_t ver = 0;
    CHECK(mgmp::npc_progress_parse(b, st, &ver));
    CHECK(ver == 38);
    CHECK(st[0].cur == "beanies_quests_intro" && st[0].taken == 0);
    CHECK(st[1].cur == "upgrade_storage_2" && st[1].taken == 1);
    CHECK(st[3].favor == 4 && st[3].taken == 1);
    CHECK(st[4].favor == 3 && st[4].ready == 1 && st[4].e4c == 1);
    CHECK(st[5].c44 == 5);
    CHECK(st[7].favor == 6 && st[7].g51 == 1);
    CHECK(st[2].h58[0] == 32 && st[2].h58[31] == 63);

    // Pour l'invite : les recompenses pretes restent a l'hote. Sans save a lui
    // (ou illisible), la faveur de ces PNJ retombe a 0 ; rien d'autre ne bouge.
    Bytes c = b;
    CHECK(mgmp::npc_progress_for_guest(c, Bytes{}) == 2);
    CHECK(c.size() == b.size());
    std::vector<TNpc> want = n;
    want[4].ready = 0; want[4].favor = 0;
    want[6].ready = 0; want[6].favor = 0;
    CHECK(c == npc_blob(38, want, tail));
    CHECK(mgmp::npc_progress_for_guest(c, Bytes{}) == 0);   // plus rien de pret
    Bytes c2 = b;
    CHECK(mgmp::npc_progress_for_guest(c2, text("illisible")) == 2 && c2 == c);

    // Avec sa save : il garde sa faveur s'il en est a la meme recompense, sans
    // drapeau, et avec moins que l'hote.
    {
        std::vector<TNpc> own = eight_npcs();
        own[4].favor = 2;            // Jack : meme etape, 2 < 3 -> garde 2
        own[6].favor = 0;            // l'orgue : meme etape, 0 < 0 faux -> 0
        Bytes x = b;
        CHECK(mgmp::npc_progress_for_guest(x, npc_blob(38, own, tail)) == 2);
        mgmp::NpcState r[mgmp::kNpcCount];
        CHECK(mgmp::npc_progress_parse(x, r));
        CHECK(r[4].favor == 2 && r[4].ready == 0 && r[6].favor == 0 && r[6].ready == 0);

        own[4].ready = 1;            // sa save a deja le drapeau : 0
        x = b;
        CHECK(mgmp::npc_progress_for_guest(x, npc_blob(38, own, tail)) == 2);
        CHECK(mgmp::npc_progress_parse(x, r) && r[4].favor == 0);

        own[4].ready = 0;
        own[4].cur = "jack_shopupgrade3";   // pas la meme etape : 0
        x = b;
        CHECK(mgmp::npc_progress_for_guest(x, npc_blob(38, own, tail)) == 2);
        CHECK(mgmp::npc_progress_parse(x, r) && r[4].favor == 0);

        own[4].cur = "jack_shopupgrade2";
        own[4].taken = { "jack_shopupgrade1", "autre" };   // liste differente : 0
        x = b;
        CHECK(mgmp::npc_progress_for_guest(x, npc_blob(38, own, tail)) == 2);
        CHECK(mgmp::npc_progress_parse(x, r) && r[4].favor == 0);

        own[4].taken = { "jack_shopupgrade1" };
        own[4].favor = 7;            // plus que l'hote : 0
        x = b;
        CHECK(mgmp::npc_progress_for_guest(x, npc_blob(38, own, tail)) == 2);
        CHECK(mgmp::npc_progress_parse(x, r) && r[4].favor == 0);
    }

    // Les versions lues : 0x20 a 0x26. Une autre, un bloc tronque ou trop court : intact.
    Bytes v20 = npc_blob(0x20, n, tail);
    CHECK(mgmp::npc_progress_parse(v20, st, &ver) && ver == 0x20);
    Bytes newer = npc_blob(0x27, n, tail);
    const Bytes newer0 = newer;
    CHECK(mgmp::npc_progress_for_guest(newer, Bytes{}) == -1 && newer == newer0);
    Bytes older = npc_blob(0x1f, n, tail);
    CHECK(mgmp::npc_progress_for_guest(older, Bytes{}) == -1);
    Bytes cut(b.begin(), b.begin() + 300);
    const Bytes cut0 = cut;
    CHECK(mgmp::npc_progress_for_guest(cut, Bytes{}) == -1 && cut == cut0);
    Bytes tiny = text("HOSTNPC");
    CHECK(mgmp::npc_progress_for_guest(tiny, Bytes{}) == -1 && tiny == text("HOSTNPC"));
    CHECK(!mgmp::npc_progress_parse(Bytes{}, st));
    CHECK(std::string(mgmp::npc_name(4)) == "Jack" && std::string(mgmp::npc_name(9)) == "?");
}

void test_npc_wire() {
    std::printf("npc en direct\n");
    mgmp::NpcState a[mgmp::kNpcCount];
    CHECK(mgmp::npc_progress_parse(npc_blob(38, eight_npcs(), text("x")), a));
    a[4].favor = 1;
    a[4].taken = 40;                 // en memoire : la taille du tableau en octets
    const Bytes w = mgmp::npc_state_encode(a);

    mgmp::NpcState b[mgmp::kNpcCount];
    CHECK(mgmp::npc_state_decode(w.data(), w.size(), b));
    for (int i = 0; i < mgmp::kNpcCount; ++i) {
        CHECK(b[i].cur == a[i].cur && b[i].taken == a[i].taken);
        CHECK(!mgmp::npc_simple_fields_differ(a[i], b[i]));
        CHECK(b[i].ready == a[i].ready);
        CHECK(mgmp::npc_same_step(a[i], b[i]));
    }

    // Rien de faux n'est accepte : un octet de trop, de moins, une autre marque.
    Bytes more = w; more.push_back(0);
    CHECK(!mgmp::npc_state_decode(more.data(), more.size(), b));
    CHECK(!mgmp::npc_state_decode(w.data(), w.size() - 1, b));
    Bytes bad = w; bad[0] ^= 1;
    CHECK(!mgmp::npc_state_decode(bad.data(), bad.size(), b));
    CHECK(!mgmp::npc_state_decode(nullptr, 0, b));
    // Une recompense visee de plus de 256 octets, ou coupee avant sa fin.
    Bytes huge = w;
    const uint16_t big = 300;
    memcpy(huge.data() + 6 + 8, &big, 2);          // longueur du premier nom
    CHECK(!mgmp::npc_state_decode(huge.data(), huge.size(), b));
    CHECK(!mgmp::npc_state_decode(w.data(), 6 + 8 + 2 + 5, b));

    // Ce que l'invite reprend : la faveur change, l'etape est la meme.
    mgmp::NpcState host = a[4], mine = a[4];
    mine.favor = 0;
    CHECK(mgmp::npc_same_step(host, mine) && mgmp::npc_simple_fields_differ(host, mine));
    mine.favor = 1;
    mine.ready = 1;                  // « prete » ne compte pas dans les champs simples
    CHECK(!mgmp::npc_simple_fields_differ(host, mine));
    mine.cur = "jack_shopupgrade3";  // l'hote a pris la recompense : pas la meme etape
    CHECK(!mgmp::npc_same_step(host, mine));
    mine = a[4];
    mine.taken = 80;
    CHECK(!mgmp::npc_same_step(host, mine));

    // npc_guest_target : ce que l'invite prend en direct.
    host = a[4];
    host.favor = 2; host.c44 = 9; host.h58[0] = 0xAB;
    mine = a[4];
    mine.favor = 1; mine.ready = 0;
    mgmp::NpcState t = mgmp::npc_guest_target(host, mine);
    CHECK(t.favor == 2 && t.c44 == 9 && t.h58[0] == 0xAB && t.ready == 0);
    CHECK(t.cur == mine.cur && t.taken == mine.taken);
    host.ready = 1;                  // prete chez l'hote : la faveur reste la sienne
    host.favor = 3;
    t = mgmp::npc_guest_target(host, mine);
    CHECK(t.favor == 1 && t.c44 == 9 && t.ready == 0);
    mine.ready = 1;                  // jamais « prete » ecrit : la sienne reste
    host.ready = 0;
    t = mgmp::npc_guest_target(host, mine);
    CHECK(t.ready == 1 && t.favor == 3);
    mine.ready = 0;
    mine.cur = "jack_shopupgrade3";  // pas la meme etape : rien ne change
    t = mgmp::npc_guest_target(host, mine);
    CHECK(t.favor == mine.favor && t.c44 == mine.c44 && t.h58[0] == mine.h58[0] && t.cur == mine.cur);
}

} // namespace

int main() {
    test_rules();
    test_npc_progress();
    test_npc_wire();
    test_owner();
    test_digest();
    test_merge();
    test_unknown_npc_format_is_copied();
    test_broken_own_save_is_left_alone();
    if (g_fail) { std::printf("%d echec(s)\n", g_fail); return 1; }
    std::printf("ok\n");
    return 0;
}
