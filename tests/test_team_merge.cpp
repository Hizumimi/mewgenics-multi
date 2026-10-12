// test_team_merge -- Mewgenics Multi : l'equipe mixte, partie portable
// (src/session/mm_team_merge.cpp), sur de vraies bases sqlite construites ici
// avec le schema et les formats des saves du jeu :
//
//   - le message MSG_TEAM (vote, jour, sort de chaque chat) ;
//   - un chat serialise (version 19) : id, nom, drapeaux ;
//   - le bloc de la table cats (u32 taille + LZ4) : ecriture, lecture, et un
//     bloc avec correspondances comme ceux du jeu ;
//   - house_state : lire, retirer ;
//   - les proprietaires des chats de l'equipe (kTeamOwners) ;
//   - l'hote choisit les cles des chats d'un invite dans sa save : un chat
//     deja venu reprend sa ligne, et un chat que l'hote a deja pour lui (save
//     copiee) est refuse ;
//   - l'invite recopie l'etat final de ses chats (rentres, morts, perdus en
//     route ; jamais dans un autre chat ; en retard, seulement s'ils n'ont pas
//     change depuis le depart).

#include "mm_team_merge.h"
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

void u16(Bytes& b, uint16_t v) { const uint8_t* p = (const uint8_t*)&v; b.insert(b.end(), p, p + 2); }
void u32(Bytes& b, uint32_t v) { const uint8_t* p = (const uint8_t*)&v; b.insert(b.end(), p, p + 4); }
void u64(Bytes& b, uint64_t v) { const uint8_t* p = (const uint8_t*)&v; b.insert(b.end(), p, p + 8); }
void f64(Bytes& b, double v)   { const uint8_t* p = (const uint8_t*)&v; b.insert(b.end(), p, p + 8); }

// Un chat serialise, version 19, suivi d'une « suite » quelconque.
Bytes cat(uint64_t id, const char* name, uint64_t flags, uint8_t fill = 0x33) {
    Bytes b;
    u32(b, 19);
    u64(b, id);
    u64(b, strlen(name));
    for (const char* c = name; *c; ++c) u16(b, (uint16_t)(unsigned char)*c);
    u64(b, 0);                      // la chaine +0x38, vide
    u32(b, 1); u32(b, 1);           // +0x58, +0x5C
    u64(b, flags);
    for (int i = 0; i < 300; ++i) b.push_back((uint8_t)(fill + i));
    return b;
}

Bytes house(const std::vector<uint64_t>& keys) {
    Bytes b;
    u32(b, 0);
    u32(b, (uint32_t)keys.size());
    for (uint64_t k : keys) {
        u64(b, k);
        const char* room = k % 2 ? "Attic" : "Floor1_Large";
        u64(b, strlen(room));
        b.insert(b.end(), room, room + strlen(room));
        f64(b, 1.5 * (double)k); f64(b, 1.0); f64(b, (double)k);
    }
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
    exec(db, "CREATE TABLE cats (key INTEGER PRIMARY KEY,data BLOB) STRICT");
    exec(db, "CREATE TABLE files (key TEXT PRIMARY KEY,data BLOB) STRICT");
    exec(db, "CREATE TABLE properties (key TEXT PRIMARY KEY,data ANY) STRICT");
    return db;
}

void put_cat(sqlite3* db, int64_t key, const Bytes& raw) {
    const Bytes blob = mgmp::cat_blob_pack(raw);
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO cats VALUES(?1, ?2)", -1, &st, nullptr);
    sqlite3_bind_int64(st, 1, key);
    sqlite3_bind_blob(st, 2, blob.data(), (int)blob.size(), SQLITE_TRANSIENT);
    CHECK(sqlite3_step(st) == SQLITE_DONE);
    sqlite3_finalize(st);
}

void put_file(sqlite3* db, const char* key, const Bytes& b) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO files VALUES(?1, ?2)", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_blob(st, 2, b.data(), (int)b.size(), SQLITE_TRANSIENT);
    CHECK(sqlite3_step(st) == SQLITE_DONE);
    sqlite3_finalize(st);
}

Bytes get_cat(sqlite3* db, int64_t key) {
    Bytes blob, raw;
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "SELECT data FROM cats WHERE key = ?1", -1, &st, nullptr);
    sqlite3_bind_int64(st, 1, key);
    if (sqlite3_step(st) == SQLITE_ROW) {
        const uint8_t* p = (const uint8_t*)sqlite3_column_blob(st, 0);
        blob.assign(p, p + sqlite3_column_bytes(st, 0));
    }
    sqlite3_finalize(st);
    if (!blob.empty()) mgmp::cat_blob_unpack(blob.data(), blob.size(), raw);
    return raw;
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

int64_t count_cats(sqlite3* db) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "SELECT count(*) FROM cats", -1, &st, nullptr);
    int64_t n = sqlite3_step(st) == SQLITE_ROW ? sqlite3_column_int64(st, 0) : -1;
    sqlite3_finalize(st);
    return n;
}

void test_message() {
    std::printf("message\n");
    mgmp::TeamMsg m;
    m.kind = mgmp::kTeamResult;
    m.seq  = 7;
    m.day  = 52;
    m.cats.resize(2);
    m.cats[0].own_key = 3;  m.cats[0].data = cat(0x1111, "Vulcho", 0x3);
    m.cats[1].own_key = 17; m.cats[1].host_key = 48; m.cats[1].fate = mgmp::kFateGone;
    m.cats[1].data = cat(0x2222, "Samone", 0x40000);
    const Bytes w = mgmp::team_encode(m);
    mgmp::TeamMsg back;
    CHECK(mgmp::team_decode(w.data(), w.size(), back));
    CHECK(back.kind == mgmp::kTeamResult && back.seq == 7 && back.day == 52 && back.cats.size() == 2);
    CHECK(back.cats[1].own_key == 17 && back.cats[1].host_key == 48 &&
          back.cats[1].fate == mgmp::kFateGone && back.cats[1].data == m.cats[1].data);
    CHECK(back.cats[0].fate == mgmp::kFateHome);
    Bytes more = w; more.push_back(1);
    CHECK(!mgmp::team_decode(more.data(), more.size(), back));
    CHECK(!mgmp::team_decode(w.data(), w.size() - 1, back));
    Bytes bad = w; bad[5] = 9;                        // un autre genre de message
    CHECK(!mgmp::team_decode(bad.data(), bad.size(), back));
    Bytes fate = w; fate[4 + 1 + 1 + 1 + 8 + 1 + 16] = 5;   // un sort inconnu
    CHECK(!mgmp::team_decode(fate.data(), fate.size(), back));
    Bytes old = w; old[4] = 1;                        // l'ancien format
    CHECK(!mgmp::team_decode(old.data(), old.size(), back));
    CHECK(!mgmp::team_decode(nullptr, 0, back));
    // Un invite sans chat : un message vide, valide.
    mgmp::TeamMsg none;
    none.kind = mgmp::kTeamDepart;
    const Bytes e = mgmp::team_encode(none);
    CHECK(mgmp::team_decode(e.data(), e.size(), back) && back.kind == mgmp::kTeamDepart && back.cats.empty());
}

void test_cat() {
    std::printf("chat serialise\n");
    Bytes c = cat(0x1fd80af348a1ac67ULL, "Vulcho", 0x10003);
    mgmp::CatInfo info;
    CHECK(mgmp::cat_info(c, info));
    CHECK(info.id == 0x1fd80af348a1ac67ULL && info.name == "Vulcho" && info.flags == 0x10003);
    CHECK(info.flags_at == 4 + 8 + 8 + 12 + 8 + 4 + 4);
    CHECK(mgmp::cat_set_flags(c, 0x40021));
    CHECK(mgmp::cat_info(c, info) && info.flags == 0x40021);
    Bytes v18 = c; v18[0] = 18;                       // une autre version : refusee
    CHECK(!mgmp::cat_info(v18, info));
    Bytes cut(c.begin(), c.begin() + 30);
    CHECK(!mgmp::cat_info(cut, info));
}

void test_blob() {
    std::printf("bloc LZ4\n");
    for (size_t n : { (size_t)1, (size_t)14, (size_t)15, (size_t)16, (size_t)269, (size_t)270,
                      (size_t)271, (size_t)1000 }) {
        Bytes raw;
        for (size_t i = 0; i < n; ++i) raw.push_back((uint8_t)(i * 7));
        const Bytes b = mgmp::cat_blob_pack(raw);
        Bytes back;
        CHECK(mgmp::cat_blob_unpack(b.data(), b.size(), back) && back == raw);
    }
    // Un bloc comme ceux du jeu, avec une correspondance : 8 litteraux, puis 24
    // octets recopies 8 en arriere, puis 5 litteraux pour finir.
    Bytes blk;
    u32(blk, 37);
    blk.push_back(0x8F);
    for (char ch : std::string("abcdefgh")) blk.push_back((uint8_t)ch);
    blk.push_back(8); blk.push_back(0);               // decalage 8
    blk.push_back(5);                                 // 15 + 5 + 4 = 24
    blk.push_back(0x50);
    for (char ch : std::string("XYZXY")) blk.push_back((uint8_t)ch);
    Bytes back;
    CHECK(mgmp::cat_blob_unpack(blk.data(), blk.size(), back));
    CHECK(std::string(back.begin(), back.end()) == "abcdefghabcdefghabcdefghabcdefghXYZXY");
    // Abimes : taille fausse, decalage hors du deja-ecrit, tronque.
    Bytes b1 = blk; b1[0] = 36;
    CHECK(!mgmp::cat_blob_unpack(b1.data(), b1.size(), back));
    Bytes b2 = blk; b2[4 + 1 + 8] = 9;
    CHECK(!mgmp::cat_blob_unpack(b2.data(), b2.size(), back));
    CHECK(!mgmp::cat_blob_unpack(blk.data(), 10, back));
}

void test_house() {
    std::printf("house_state\n");
    Bytes hs = house({ 5, 11, 9, 28 });
    std::vector<uint64_t> keys;
    CHECK(mgmp::house_state_keys(hs, keys) && keys == std::vector<uint64_t>({ 5, 11, 9, 28 }));
    CHECK(mgmp::house_state_remove(hs, { 11, 28, 99 }) == 2);
    CHECK(hs == house({ 5, 9 }));
    CHECK(mgmp::house_state_remove(hs, { 99 }) == 0 && hs == house({ 5, 9 }));
    Bytes bad = house({ 5 });
    bad.resize(bad.size() - 3);
    const Bytes bad0 = bad;
    CHECK(mgmp::house_state_remove(bad, { 5 }) == -1 && bad == bad0);
}

void test_owners() {
    std::printf("proprietaires\n");
    std::vector<mgmp::TeamOwner> own(3);
    own[0].cat_id = 0x1fd80af348a1ac67ULL; own[0].host_key = 5;  own[0].peer = 0;
    own[1].cat_id = 0xB1;                  own[1].host_key = 47; own[1].peer = 1;
    own[2].cat_id = 0xB2;                  own[2].host_key = 48; own[2].peer = 1;
    const Bytes w = mgmp::team_owners_encode(9, own);
    CHECK(mgmp::team_kind(w.data(), w.size()) == mgmp::kTeamOwners);
    std::vector<mgmp::TeamOwner> back;
    uint8_t seq = 0;
    CHECK(mgmp::team_owners_decode(w.data(), w.size(), seq, back));
    CHECK(seq == 9 && back.size() == 3 && back[0].cat_id == own[0].cat_id && back[2].peer == 1 &&
          back[1].host_key == 47);
    Bytes more = w; more.push_back(0);
    CHECK(!mgmp::team_owners_decode(more.data(), more.size(), seq, back));
    CHECK(!mgmp::team_owners_decode(w.data(), w.size() - 1, seq, back));
    // Vide : pas d'equipe mixte, mais un message valide.
    const Bytes none = mgmp::team_owners_encode(4, {});
    CHECK(mgmp::team_owners_decode(none.data(), none.size(), seq, back) && back.empty() && seq == 4);
    // Les deux decodeurs ne se melangent pas.
    mgmp::TeamMsg tm;
    CHECK(!mgmp::team_decode(w.data(), w.size(), tm));
    mgmp::TeamMsg one;
    one.kind = mgmp::kTeamDepart;
    one.cats.resize(1);
    one.cats[0].own_key = 3; one.cats[0].data = cat(0xB1, "Garry", 0x3);
    const Bytes dep = mgmp::team_encode(one);
    CHECK(mgmp::team_kind(dep.data(), dep.size()) == mgmp::kTeamDepart);
    CHECK(!mgmp::team_owners_decode(dep.data(), dep.size(), seq, back));
    CHECK(mgmp::team_kind(dep.data(), 5) == 0);
    Bytes junk = dep; junk[0] ^= 1;
    CHECK(mgmp::team_kind(junk.data(), junk.size()) == 0);
    CHECK(mgmp::team_kind(nullptr, 0) == 0);
}

void test_plan_keys() {
    std::printf("cles des chats de l'invite chez l'hote\n");
    sqlite3* h = new_save();
    put_cat(h, 1, cat(0xA1, "Rodders", 0x1));
    put_cat(h, 46, cat(0xA2, "Milhouse", 0x40020));          // mort : compte aussi
    std::vector<mgmp::TeamCat> team(2);
    team[0].own_key = 3; team[0].data = cat(0xB1, "Garry", 0x3);
    team[1].own_key = 4; team[1].data = cat(0xB2, "Eden", 0x3);
    std::vector<mgmp::KeyPlan> plan;
    std::string why;
    // Le compteur du jeu en est a 46 : 47 et 48.
    CHECK(mgmp::team_plan_keys(h, team, 47, plan, why));
    CHECK(plan.size() == 2 && plan[0].key == 47 && plan[1].key == 48 && !plan[0].reuse && !plan[1].reuse);
    CHECK(count_cats(h) == 2);                               // rien d'ecrit
    // Le jeu a deja cree d'autres chats (compteur 60, pas encore dans la table).
    CHECK(mgmp::team_plan_keys(h, team, 61, plan, why));
    CHECK(plan[0].key == 61 && plan[1].key == 62);

    // Eden est deja venu : sa ligne (marquee) est restee a la cle 70. Il la
    // reprend ; Garry prend une cle neuve, apres elle.
    put_cat(h, 70, cat(0xB2, "Eden", 0x3 | mgmp::kCatGuest));
    CHECK(mgmp::team_plan_keys(h, team, 61, plan, why));
    CHECK(plan[0].key == 71 && !plan[0].reuse && plan[1].key == 70 && plan[1].reuse);

    // Un bloc illisible dans la table ne bloque rien.
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(h, "INSERT INTO cats VALUES(50, x'0102')", -1, &st, nullptr);
    CHECK(sqlite3_step(st) == SQLITE_DONE);
    sqlite3_finalize(st);
    CHECK(mgmp::team_plan_keys(h, team, 0, plan, why) && plan[0].key == 71);

    // Rodders est deja dans la save de l'hote pour elle (meme id, sans marque) :
    // lui seul est refuse, Garry part.
    std::vector<mgmp::TeamCat> copy(2);
    copy[0].own_key = 1; copy[0].data = cat(0xA1, "Rodders", 0x1);
    copy[1].own_key = 3; copy[1].data = cat(0xB1, "Garry", 0x3);
    CHECK(mgmp::team_plan_keys(h, copy, 0, plan, why));
    CHECK(plan.size() == 2 && plan[0].refused && plan[0].why.find("Rodders") != std::string::npos);
    CHECK(!plan[1].refused && plan[1].key == 71);
    // Un clone d'Eden reste chez l'hote (meme id, sans marque) : Eden reprend
    // quand meme sa ligne marquee.
    put_cat(h, 75, cat(0xB2, "Eden", 0x3));
    CHECK(mgmp::team_plan_keys(h, team, 0, plan, why));
    CHECK(!plan[1].refused && plan[1].reuse && plan[1].key == 70);
    CHECK(plan[0].key == 76);                                 // apres le clone
    // Deux lignes marquees pour le meme chat : la plus haute.
    put_cat(h, 80, cat(0xB2, "Eden", 0x3 | mgmp::kCatGuest));
    CHECK(mgmp::team_plan_keys(h, team, 0, plan, why));
    CHECK(plan[1].reuse && plan[1].key == 80 && plan[0].key == 81);
    // Deux fois le meme chat dans la liste : le second est refuse.
    std::vector<mgmp::TeamCat> twice(2);
    twice[0].data = cat(0xB1, "Garry", 0x3);
    twice[1].data = cat(0xB1, "Garry", 0x3);
    CHECK(mgmp::team_plan_keys(h, twice, 0, plan, why) && !plan[0].refused && plan[1].refused);
    // Un chat illisible : refuse, l'autre part.
    std::vector<mgmp::TeamCat> broken(2);
    broken[0].data = cat(0xC2, "Ok", 0);
    broken[1].data = Bytes(10, 0);
    CHECK(mgmp::team_plan_keys(h, broken, 0, plan, why) && !plan[0].refused && plan[1].refused);
    CHECK(!mgmp::team_plan_keys(h, {}, 0, plan, why));
    CHECK(!mgmp::team_plan_keys(nullptr, team, 0, plan, why));
    sqlite3_close(h);

    // Une save neuve, sans chat : a partir de 1 (ou du compteur).
    sqlite3* e = new_save();
    CHECK(mgmp::team_plan_keys(e, team, 0, plan, why) && plan[0].key == 1 && plan[1].key == 2);
    sqlite3_close(e);
}

void test_guest_apply() {
    std::printf("retour chez l'invite\n");
    sqlite3* o = new_save();
    put_cat(o, 3, cat(0xB1, "Garry", 0x3, 0x10));
    put_cat(o, 4, cat(0xB2, "Eden", 0x3, 0x20));
    put_cat(o, 5, cat(0xB3, "Puddle", 0x0, 0x30));
    put_cat(o, 6, cat(0xB4, "Tom", 0x1, 0x40));
    put_file(o, "house_state", house({ 3, 4, 5, 6 }));

    std::vector<mgmp::TeamCat> res(4);
    res[0].own_key = 3; res[0].host_key = 47;
    res[0].data = cat(0xB1, "Garry", 0x3 | mgmp::kCatAway | mgmp::kCatGuest, 0x77);   // rentre, change
    res[1].own_key = 4; res[1].host_key = 48;
    res[1].data = cat(0xB2, "Eden", 0x40021 | mgmp::kCatGuest, 0x88);                 // mort
    res[2].own_key = 5; res[2].host_key = 49;
    res[2].data = cat(0xFFFF, "Intrus", 0x1, 0x99);                                    // pas ce chat-la
    res[3].own_key = 6; res[3].host_key = 50; res[3].fate = mgmp::kFateGone;
    res[3].data = cat(0xB4, "Tom", 0x1 | mgmp::kCatGuest, 0x55);                       // perdu en route

    mgmp::TeamApplySummary sum;
    CHECK(mgmp::team_apply_results(o, res, nullptr, sum));
    CHECK(sum.back.size() == 1 && sum.back[0] == "Garry");
    CHECK(sum.lost.size() == 1 && sum.lost[0] == "Eden");
    CHECK(sum.gone.size() == 1 && sum.gone[0] == "Tom");
    CHECK(sum.skipped == 1 && sum.changed.empty());
    CHECK(get_cat(o, 3) == cat(0xB1, "Garry", 0x3, 0x77));              // ni « en aventure » ni la marque
    CHECK(get_cat(o, 4) == cat(0xB2, "Eden", 0x40021, 0x88));
    CHECK(get_cat(o, 5) == cat(0xB3, "Puddle", 0x0, 0x30));             // intact
    CHECK(get_cat(o, 6) == cat(0xB4, "Tom", 0x1, 0x55));                // vivant, mais parti
    CHECK(get_file(o, "house_state") == house({ 3, 5 }));

    // house_state illisible : les morts restent tels qu'avant le depart.
    put_cat(o, 4, cat(0xB2, "Eden", 0x3, 0x20));
    put_file(o, "house_state", Bytes{ 1, 2, 3 });
    std::vector<mgmp::TeamCat> res2(2);
    res2[0] = res[0];
    res2[1] = res[1];
    CHECK(mgmp::team_apply_results(o, res2, nullptr, sum));
    CHECK(sum.back.size() == 1 && sum.lost.empty() && sum.skipped == 1);
    CHECK(get_cat(o, 4) == cat(0xB2, "Eden", 0x3, 0x20));
    sqlite3_close(o);

    // En retard : seuls les chats qui n'ont pas change depuis le depart.
    sqlite3* l = new_save();
    put_cat(l, 3, cat(0xB1, "Garry", 0x3, 0x10));
    put_cat(l, 4, cat(0xB2, "Eden", 0x3, 0x21));                        // joue depuis le depart
    put_file(l, "house_state", house({ 3, 4 }));
    std::vector<mgmp::TeamCat> dep(2);
    dep[0].own_key = 3; dep[0].data = cat(0xB1, "Garry", 0x3, 0x10);
    dep[1].own_key = 4; dep[1].data = cat(0xB2, "Eden", 0x3, 0x20);
    std::vector<mgmp::TeamCat> late(2);
    late[0].own_key = 3; late[0].data = cat(0xB1, "Garry", 0x3 | mgmp::kCatGuest, 0x77);
    late[1].own_key = 4; late[1].data = cat(0xB2, "Eden", 0x40021 | mgmp::kCatGuest, 0x88);
    CHECK(mgmp::team_apply_results(l, late, &dep, sum));
    CHECK(sum.back.size() == 1 && sum.changed.size() == 1 && sum.changed[0] == "Eden" && sum.lost.empty());
    CHECK(get_cat(l, 3) == cat(0xB1, "Garry", 0x3, 0x77));
    CHECK(get_cat(l, 4) == cat(0xB2, "Eden", 0x3, 0x21));
    CHECK(get_file(l, "house_state") == house({ 3, 4 }));
    sqlite3_close(l);

    // Deja mort dans cette save (le meme chat dans deux departs, le premier
    // rendu en retard) : un etat final vivant ne le ramene pas.
    sqlite3* m = new_save();
    put_cat(m, 3, cat(0xB1, "Garry", 0x40003, 0x10));
    put_cat(m, 4, cat(0xB2, "Eden", 0x3, 0x20));
    put_file(m, "house_state", house({ 4 }));
    std::vector<mgmp::TeamCat> alive(2);
    alive[0].own_key = 3; alive[0].data = cat(0xB1, "Garry", 0x3 | mgmp::kCatGuest, 0x77);
    alive[1].own_key = 4; alive[1].data = cat(0xB2, "Eden", 0x3 | mgmp::kCatGuest, 0x88);
    CHECK(mgmp::team_apply_results(m, alive, nullptr, sum));
    CHECK(sum.changed.size() == 1 && sum.changed[0] == "Garry");
    CHECK(sum.back.size() == 1 && sum.back[0] == "Eden" && sum.lost.empty() && sum.skipped == 0);
    CHECK(get_cat(m, 3) == cat(0xB1, "Garry", 0x40003, 0x10));
    CHECK(get_cat(m, 4) == cat(0xB2, "Eden", 0x3, 0x88));
    CHECK(get_file(m, "house_state") == house({ 4 }));
    sqlite3_close(m);
}

} // namespace

int main() {
    test_message();
    test_cat();
    test_blob();
    test_house();
    test_owners();
    test_plan_keys();
    test_guest_apply();
    if (g_fail) { std::printf("%d echec(s)\n", g_fail); return 1; }
    std::printf("ok\n");
    return 0;
}
