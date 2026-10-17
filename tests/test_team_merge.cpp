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

void test_embark() {
    std::printf("depart ensemble\n");
    const std::vector<uint64_t> keys = { 41, 43, 49, 50 };
    const Bytes w = mgmp::team_embark_encode(7, keys);
    CHECK(mgmp::team_kind(w.data(), w.size()) == mgmp::kTeamEmbark);
    std::vector<uint64_t> back;
    uint8_t seq = 0;
    CHECK(mgmp::team_embark_decode(w.data(), w.size(), seq, back) && seq == 7 && back == keys);
    Bytes more = w; more.push_back(0);
    CHECK(!mgmp::team_embark_decode(more.data(), more.size(), seq, back));
    CHECK(!mgmp::team_embark_decode(w.data(), w.size() - 1, seq, back));
    // Une equipe vide ou une cle nulle : refuse (le jeu ne part pas sans chat).
    const Bytes none = mgmp::team_embark_encode(7, {});
    CHECK(!mgmp::team_embark_decode(none.data(), none.size(), seq, back));
    const Bytes zero = mgmp::team_embark_encode(7, { 41, 0 });
    CHECK(!mgmp::team_embark_decode(zero.data(), zero.size(), seq, back));
    // Les decodeurs ne se melangent pas.
    std::vector<mgmp::TeamOwner> own;
    CHECK(!mgmp::team_owners_decode(w.data(), w.size(), seq, own));
    const Bytes ow = mgmp::team_owners_encode(7, {});
    CHECK(!mgmp::team_embark_decode(ow.data(), ow.size(), seq, back));
}

void test_gear() {
    std::printf("choix des objets : photos et reponse\n");
    mgmp::GearMsg m;
    m.vote = 9; m.snap = 3; m.final = true; m.has_data = true;
    m.storage = Bytes{ 2, 0, 0, 0, 0xAA, 0xBB, 0xCC };
    m.cats.resize(2);
    m.cats[0].key = 47; m.cats[0].data = cat(0xB1, "Garry", 0x3);
    m.cats[1].key = 48; m.cats[1].data = cat(0xB2, "Eden", 0x3);
    const Bytes w = mgmp::team_gear_encode(m);
    CHECK(mgmp::team_kind(w.data(), w.size()) == mgmp::kTeamGear);
    mgmp::GearMsg back;
    CHECK(mgmp::team_gear_decode(w.data(), w.size(), back));
    CHECK(back.vote == 9 && back.snap == 3 && back.final && back.has_data && back.storage == m.storage);
    CHECK(back.cats.size() == 2 && back.cats[1].key == 48 && back.cats[1].data == m.cats[1].data);
    Bytes more = w; more.push_back(0);
    CHECK(!mgmp::team_gear_decode(more.data(), more.size(), back));
    CHECK(!mgmp::team_gear_decode(w.data(), w.size() - 1, back));
    // Sans donnees : « fini », rien d'autre.
    mgmp::GearMsg fin;
    fin.vote = 9; fin.snap = 3; fin.final = true;
    const Bytes f = mgmp::team_gear_encode(fin);
    CHECK(mgmp::team_gear_decode(f.data(), f.size(), back) && back.final && !back.has_data &&
          back.cats.empty() && back.storage.empty() && back.snap == 3);
    // Un rangement trop court (moins que son nombre d'objets), une cle nulle :
    // refuses.
    mgmp::GearMsg bad = m;
    bad.storage = Bytes{ 0, 0 };
    const Bytes b1 = mgmp::team_gear_encode(bad);
    CHECK(!mgmp::team_gear_decode(b1.data(), b1.size(), back));
    bad = m;
    bad.cats[0].key = 0;
    const Bytes b2 = mgmp::team_gear_encode(bad);
    CHECK(!mgmp::team_gear_decode(b2.data(), b2.size(), back));
    // La reponse de l'hote, avec et sans rangement.
    mgmp::GearAck a;
    a.vote = 9; a.snap = 3; a.has_storage = true; a.storage = m.storage;
    const Bytes aw = mgmp::team_gear_ack_encode(a);
    CHECK(mgmp::team_kind(aw.data(), aw.size()) == mgmp::kTeamGearAck);
    mgmp::GearAck ab;
    CHECK(mgmp::team_gear_ack_decode(aw.data(), aw.size(), ab));
    CHECK(ab.vote == 9 && ab.snap == 3 && ab.has_storage && ab.storage == m.storage);
    Bytes amore = aw; amore.push_back(1);
    CHECK(!mgmp::team_gear_ack_decode(amore.data(), amore.size(), ab));
    mgmp::GearAck none;
    none.vote = 9;
    const Bytes nw = mgmp::team_gear_ack_encode(none);
    CHECK(mgmp::team_gear_ack_decode(nw.data(), nw.size(), ab) && ab.snap == 0 && !ab.has_storage);
    // Les decodeurs ne se melangent pas.
    CHECK(!mgmp::team_gear_ack_decode(w.data(), w.size(), ab));
    CHECK(!mgmp::team_gear_decode(aw.data(), aw.size(), back));
    uint8_t seq = 0;
    std::vector<uint64_t> keys;
    CHECK(!mgmp::team_embark_decode(w.data(), w.size(), seq, keys));
}

void test_rungear() {
    std::printf("objets en aventure : photo et reponse\n");
    mgmp::RunGearMsg m;
    m.seq = 4; m.base = 17;
    m.backpack = Bytes{ 1, 0, 0, 0, 0x10, 0x20 };
    m.trash    = Bytes{ 0, 0, 0, 0 };
    m.cats.resize(2);
    m.cats[0].key = 47; m.cats[0].data = cat(0xB1, "Garry", 0x3);
    m.cats[1].key = 48; m.cats[1].data = cat(0xB2, "Eden", 0x3);
    const Bytes w = mgmp::team_rungear_encode(m);
    CHECK(mgmp::team_kind(w.data(), w.size()) == mgmp::kTeamRunGear);
    mgmp::RunGearMsg back;
    CHECK(mgmp::team_rungear_decode(w.data(), w.size(), back));
    CHECK(back.seq == 4 && back.base == 17 && back.backpack == m.backpack && back.trash == m.trash);
    CHECK(back.cats.size() == 2 && back.cats[0].key == 47 && back.cats[1].data == m.cats[1].data);
    Bytes more = w; more.push_back(0);
    CHECK(!mgmp::team_rungear_decode(more.data(), more.size(), back));
    CHECK(!mgmp::team_rungear_decode(w.data(), w.size() - 1, back));
    // Un sac plus court que son nombre d'objets, une cle nulle : refuses.
    mgmp::RunGearMsg bad = m;
    bad.trash = Bytes{ 0, 0 };
    const Bytes b1 = mgmp::team_rungear_encode(bad);
    CHECK(!mgmp::team_rungear_decode(b1.data(), b1.size(), back));
    bad = m;
    bad.cats[1].key = 0;
    const Bytes b2 = mgmp::team_rungear_encode(bad);
    CHECK(!mgmp::team_rungear_decode(b2.data(), b2.size(), back));
    // Sans chat (il n'a touche qu'au sac) : permis.
    mgmp::RunGearMsg bag = m;
    bag.cats.clear();
    const Bytes b3 = mgmp::team_rungear_encode(bag);
    CHECK(mgmp::team_rungear_decode(b3.data(), b3.size(), back) && back.cats.empty());
    // La reponse.
    mgmp::RunGearAck a;
    a.seq = 4; a.result = mgmp::kRunGearStale; a.rev = 19;
    const Bytes aw = mgmp::team_rungear_ack_encode(a);
    CHECK(mgmp::team_kind(aw.data(), aw.size()) == mgmp::kTeamRunGearAck);
    mgmp::RunGearAck ab;
    CHECK(mgmp::team_rungear_ack_decode(aw.data(), aw.size(), ab));
    CHECK(ab.seq == 4 && ab.result == mgmp::kRunGearStale && ab.rev == 19);
    mgmp::RunGearAck z = a; z.result = 0;
    const Bytes zw = mgmp::team_rungear_ack_encode(z);
    CHECK(!mgmp::team_rungear_ack_decode(zw.data(), zw.size(), ab));
    // Refusee par une case : le dernier code connu ; au-dela, illisible.
    mgmp::RunGearAck nd = a; nd.result = mgmp::kRunGearNode;
    const Bytes ndw = mgmp::team_rungear_ack_encode(nd);
    CHECK(mgmp::team_rungear_ack_decode(ndw.data(), ndw.size(), ab) && ab.result == mgmp::kRunGearNode);
    mgmp::RunGearAck over = a; over.result = mgmp::kRunGearNode + 1;
    const Bytes ow = mgmp::team_rungear_ack_encode(over);
    CHECK(!mgmp::team_rungear_ack_decode(ow.data(), ow.size(), ab));
    Bytes amore = aw; amore.push_back(1);
    CHECK(!mgmp::team_rungear_ack_decode(amore.data(), amore.size(), ab));
    // Les decodeurs ne se melangent pas.
    CHECK(!mgmp::team_rungear_ack_decode(w.data(), w.size(), ab));
    CHECK(!mgmp::team_rungear_decode(aw.data(), aw.size(), back));
    mgmp::GearMsg g;
    CHECK(!mgmp::team_gear_decode(w.data(), w.size(), g));
}

void test_item_ops() {
    std::printf("objets en direct : mouvements, reponses, etat, empreintes\n");
    mgmp::ItemOp m;
    m.seq = 7; m.origin = 2; m.op = mgmp::kItemEquip; m.slot = 3;
    m.cat_key = 46; m.cat_id = 0x1122334455667788ull; m.item = 0xABCDEF0123456789ull; m.prev = 0;
    m.name = "tk_GlowingCoin";
    const Bytes w = mgmp::team_item_op_encode(m);
    CHECK(mgmp::team_kind(w.data(), w.size()) == mgmp::kTeamItemOp);
    mgmp::ItemOp back;
    CHECK(mgmp::team_item_op_decode(w.data(), w.size(), back));
    CHECK(back.seq == 7 && back.origin == 2 && back.op == mgmp::kItemEquip && back.slot == 3);
    CHECK(back.cat_key == 46 && back.cat_id == m.cat_id && back.item == m.item && back.prev == 0);
    CHECK(back.name == "tk_GlowingCoin" && back.cat.empty());
    CHECK(!mgmp::team_item_op_decode(w.data(), w.size() - 1, back));
    Bytes more = w; more.push_back(0);
    CHECK(!mgmp::team_item_op_decode(more.data(), more.size(), back));
    // Rendre au sac : l'emplacement et l'empreinte.
    mgmp::ItemOp u = m; u.op = mgmp::kItemUnequip; u.slot = 4;
    const Bytes uw = mgmp::team_item_op_encode(u);
    CHECK(mgmp::team_item_op_decode(uw.data(), uw.size(), back) && back.op == mgmp::kItemUnequip && back.slot == 4);
    // Un livre : le chat apres, l'empreinte du livre (0 : aucun).
    mgmp::ItemOp l = m; l.op = mgmp::kItemLearn; l.item = 0; l.cat = cat(0xB1, "Garry", 0x3);
    const Bytes lw = mgmp::team_item_op_encode(l);
    CHECK(mgmp::team_item_op_decode(lw.data(), lw.size(), back) && back.op == mgmp::kItemLearn && back.cat == l.cat &&
          back.item == 0);
    // Refuses : un livre sans chat, un mouvement d'objet sans empreinte ou avec un
    // chat, un emplacement hors des cinq, une cle nulle, un genre inconnu.
    mgmp::ItemOp bad = l; bad.cat.clear();
    Bytes bw = mgmp::team_item_op_encode(bad);
    CHECK(!mgmp::team_item_op_decode(bw.data(), bw.size(), back));
    bad = m; bad.item = 0; bw = mgmp::team_item_op_encode(bad);
    CHECK(!mgmp::team_item_op_decode(bw.data(), bw.size(), back));
    bad = m; bad.cat = l.cat; bw = mgmp::team_item_op_encode(bad);
    CHECK(!mgmp::team_item_op_decode(bw.data(), bw.size(), back));
    bad = m; bad.slot = 5; bw = mgmp::team_item_op_encode(bad);
    CHECK(!mgmp::team_item_op_decode(bw.data(), bw.size(), back));
    bad = m; bad.cat_key = 0; bw = mgmp::team_item_op_encode(bad);
    CHECK(!mgmp::team_item_op_decode(bw.data(), bw.size(), back));
    bad = m; bad.op = 4; bw = mgmp::team_item_op_encode(bad);
    CHECK(!mgmp::team_item_op_decode(bw.data(), bw.size(), back));
    // Un nom trop long est coupe a 63 octets, et se relit.
    mgmp::ItemOp longname = m; longname.name = std::string(100, 'x');
    const Bytes nw = mgmp::team_item_op_encode(longname);
    CHECK(mgmp::team_item_op_decode(nw.data(), nw.size(), back) && back.name.size() == 63);

    mgmp::ItemAck a; a.seq = 9; a.result = mgmp::kItemConflict;
    const Bytes aw = mgmp::team_item_ack_encode(a);
    CHECK(mgmp::team_kind(aw.data(), aw.size()) == mgmp::kTeamItemAck);
    mgmp::ItemAck ab;
    CHECK(mgmp::team_item_ack_decode(aw.data(), aw.size(), ab) && ab.seq == 9 && ab.result == mgmp::kItemConflict);
    a.result = 0;
    Bytes zw = mgmp::team_item_ack_encode(a);
    CHECK(!mgmp::team_item_ack_decode(zw.data(), zw.size(), ab));
    a.result = mgmp::kItemNode + 1;
    zw = mgmp::team_item_ack_encode(a);
    CHECK(!mgmp::team_item_ack_decode(zw.data(), zw.size(), ab));

    const Bytes rw = mgmp::team_item_sync_req_encode(12);
    CHECK(mgmp::team_kind(rw.data(), rw.size()) == mgmp::kTeamItemSyncReq);
    uint32_t seq = 0;
    CHECK(mgmp::team_item_sync_req_decode(rw.data(), rw.size(), seq) && seq == 12);
    CHECK(!mgmp::team_item_sync_req_decode(rw.data(), rw.size() - 1, seq));

    mgmp::RunGearMsg st;
    st.seq = 12; st.base = 40;
    st.backpack = {2, 0, 0, 0, 9, 9}; st.trash = {0, 0, 0, 0};
    mgmp::GearCat c; c.key = 46; c.data = cat(0xB1, "Garry", 0x3); st.cats.push_back(c);
    const Bytes sw = mgmp::team_item_sync_encode(st);
    CHECK(mgmp::team_kind(sw.data(), sw.size()) == mgmp::kTeamItemSync);
    mgmp::RunGearMsg sb;
    CHECK(mgmp::team_item_sync_decode(sw.data(), sw.size(), sb));
    CHECK(sb.seq == 12 && sb.base == 40 && sb.backpack == st.backpack && sb.trash == st.trash &&
          sb.cats.size() == 1 && sb.cats[0].key == 46 && sb.cats[0].data == c.data);
    // Les decodeurs ne se melangent pas.
    CHECK(!mgmp::team_rungear_decode(sw.data(), sw.size(), sb));
    CHECK(!mgmp::team_item_sync_decode(w.data(), w.size(), sb));
    CHECK(!mgmp::team_item_op_decode(aw.data(), aw.size(), back));
    CHECK(!mgmp::team_item_ack_decode(w.data(), w.size(), ab));

    // L'empreinte : les memes champs, la meme ; un champ de l'objet change, une
    // autre ; ce que ce joueur en a vu (sa place d, les bits « nouvel objet » de
    // c) ne la change pas, ni ce que la save ne garde pas de f ; jamais 0 ; le
    // nom et le texte ne se confondent pas.
    mgmp::ItemFields f1; f1.name = "tk_GlowingCoin"; f1.text = ""; f1.a = 1; f1.b = 2; f1.c = 0x23; f1.d = 4; f1.e = 5; f1.f = 6;
    mgmp::ItemFields f2 = f1;
    CHECK(mgmp::item_digest(f1) == mgmp::item_digest(f2) && mgmp::item_digest(f1) != 0);
    f2.d = 1002; CHECK(mgmp::item_digest(f1) == mgmp::item_digest(f2));
    f2 = f1; f2.d = 4000001; f2.c &= ~0x2u; CHECK(mgmp::item_digest(f1) == mgmp::item_digest(f2));
    f2 = f1; f2.c &= ~0x6u; CHECK(mgmp::item_digest(f1) == mgmp::item_digest(f2));
    f2 = f1; f2.c |= 0x4u; CHECK(mgmp::item_digest(f1) == mgmp::item_digest(f2));
    f2 = f1; f2.f = 6 + 0x100; CHECK(mgmp::item_digest(f1) == mgmp::item_digest(f2));
    f2 = f1; f2.c &= ~0x1u; CHECK(mgmp::item_digest(f1) != mgmp::item_digest(f2));
    f2 = f1; f2.c ^= 0x20u; CHECK(mgmp::item_digest(f1) != mgmp::item_digest(f2));
    f2 = f1; f2.a = 0xFFFFFFFFu; CHECK(mgmp::item_digest(f1) != mgmp::item_digest(f2));
    f2 = f1; f2.b = 3; CHECK(mgmp::item_digest(f1) != mgmp::item_digest(f2));
    f2 = f1; f2.e = 0; CHECK(mgmp::item_digest(f1) != mgmp::item_digest(f2));
    f2 = f1; f2.f = 0; CHECK(mgmp::item_digest(f1) != mgmp::item_digest(f2));
    f2 = f1; f2.text = "x"; CHECK(mgmp::item_digest(f1) != mgmp::item_digest(f2));
    mgmp::ItemFields n1; n1.name = "ab"; n1.text = "c";
    mgmp::ItemFields n2; n2.name = "a";  n2.text = "bc";
    CHECK(mgmp::item_digest(n1) != mgmp::item_digest(n2));
    mgmp::ItemFields e1; CHECK(mgmp::item_digest(e1) != 0);
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

void test_storage() {
    std::printf("retour chez l'invite : son rangement\n");
    const Bytes disk = { 2, 0, 0, 0, 0x10, 0x11 };      // dans le fichier a « J'arrive »
    const Bytes mem  = { 3, 0, 0, 0, 0x10, 0x11, 0x12 }; // en memoire (pas encore ecrit)
    const Bytes post = { 1, 0, 0, 0, 0x10 };             // apres le choix des objets
    mgmp::TeamStorageChange sc;
    sc.cats = { 3 };
    sc.pre_disk = disk; sc.pre_disk_found = true; sc.pre_mem = mem; sc.post = post;
    std::vector<mgmp::TeamCat> res(1);
    res[0].own_key = 3; res[0].data = cat(0xB1, "Garry", 0x3 | mgmp::kCatGuest, 0x77);
    mgmp::TeamApplySummary sum;

    // La save a encore le rangement du fichier : il devient `post`, avec le chat.
    sqlite3* o = new_save();
    put_cat(o, 3, cat(0xB1, "Garry", 0x3, 0x10));
    put_file(o, "inventory_storage", disk);
    CHECK(mgmp::team_apply_results(o, res, nullptr, sum, &sc));
    CHECK(sum.storage == mgmp::kStorageWritten && sum.back.size() == 1);
    CHECK(get_file(o, "inventory_storage") == post);
    CHECK(get_cat(o, 3) == cat(0xB1, "Garry", 0x3, 0x77));
    // Une seconde fois : deja fait.
    CHECK(mgmp::team_apply_results(o, res, nullptr, sum, &sc) && sum.storage == mgmp::kStorageSame);
    // Le jeu avait ecrit celui de la memoire depuis : aussi « inchange ».
    put_file(o, "inventory_storage", mem);
    CHECK(mgmp::team_apply_results(o, res, nullptr, sum, &sc) && sum.storage == mgmp::kStorageWritten);
    CHECK(get_file(o, "inventory_storage") == post);
    // Joue depuis : laisse tel quel, le chat est quand meme recopie.
    const Bytes other = { 4, 0, 0, 0, 1, 2, 3, 4 };
    put_file(o, "inventory_storage", other);
    put_cat(o, 3, cat(0xB1, "Garry", 0x3, 0x10));
    CHECK(mgmp::team_apply_results(o, res, nullptr, sum, &sc) && sum.storage == mgmp::kStorageChanged);
    CHECK(get_file(o, "inventory_storage") == other);
    CHECK(get_cat(o, 3) == cat(0xB1, "Garry", 0x3, 0x77));
    sqlite3_close(o);

    // Un chat pas recopie (il a change depuis le depart) : le rangement non plus.
    sqlite3* l = new_save();
    put_cat(l, 3, cat(0xB1, "Garry", 0x3, 0x21));
    put_file(l, "inventory_storage", disk);
    std::vector<mgmp::TeamCat> dep(1);
    dep[0].own_key = 3; dep[0].data = cat(0xB1, "Garry", 0x3, 0x20);
    CHECK(mgmp::team_apply_results(l, res, &dep, sum, &sc));
    CHECK(sum.storage == mgmp::kStorageCatsKept && sum.changed.size() == 1);
    CHECK(get_file(l, "inventory_storage") == disk);
    // Deux chats, l'un recopie, l'autre pas (illisible ici) : rien pour le rangement.
    put_cat(l, 3, cat(0xB1, "Garry", 0x3, 0x10));
    std::vector<mgmp::TeamCat> two(2);
    two[0] = res[0];
    two[1].own_key = 9; two[1].data = cat(0xB9, "Absent", 0x3);
    CHECK(mgmp::team_apply_results(l, two, nullptr, sum, &sc));
    CHECK(sum.storage == mgmp::kStorageCatsKept && sum.skipped == 1 && sum.back.size() == 1);
    CHECK(get_file(l, "inventory_storage") == disk);
    // Deux chats partis, un seul dans les resultats (l'hote ne l'a pas
    // retrouve) : le rangement ne change pas -- l'objet du second serait perdu.
    put_cat(l, 3, cat(0xB1, "Garry", 0x3, 0x10));
    put_cat(l, 4, cat(0xB2, "Eden", 0x3, 0x20));
    mgmp::TeamStorageChange both = sc;
    both.cats = { 3, 4 };
    CHECK(mgmp::team_apply_results(l, res, nullptr, sum, &both));
    CHECK(sum.storage == mgmp::kStorageCatsKept && sum.back.size() == 1 && sum.skipped == 0);
    CHECK(get_file(l, "inventory_storage") == disk);
    CHECK(get_cat(l, 3) == cat(0xB1, "Garry", 0x3, 0x77));
    CHECK(get_cat(l, 4) == cat(0xB2, "Eden", 0x3, 0x20));
    sqlite3_close(l);

    // Pas de rangement dans la save au depart ni maintenant : il est cree.
    sqlite3* n = new_save();
    put_cat(n, 3, cat(0xB1, "Garry", 0x3, 0x10));
    mgmp::TeamStorageChange nr = sc;
    nr.pre_disk.clear(); nr.pre_disk_found = false;
    CHECK(mgmp::team_apply_results(n, res, nullptr, sum, &nr) && sum.storage == mgmp::kStorageWritten);
    CHECK(get_file(n, "inventory_storage") == post);
    // ... mais il en a un maintenant, autre : laisse tel quel.
    put_file(n, "inventory_storage", other);
    put_cat(n, 3, cat(0xB1, "Garry", 0x3, 0x10));
    CHECK(mgmp::team_apply_results(n, res, nullptr, sum, &nr) && sum.storage == mgmp::kStorageChanged);
    CHECK(get_file(n, "inventory_storage") == other);
    // Sans `storage`, ou un `post` qui n'en est pas un : rien a faire.
    CHECK(mgmp::team_apply_results(n, res, nullptr, sum) && sum.storage == mgmp::kStorageNone);
    mgmp::TeamStorageChange shortp = sc;
    shortp.post = Bytes{ 1, 0 };
    CHECK(mgmp::team_apply_results(n, res, nullptr, sum, &shortp) && sum.storage == mgmp::kStorageNone);
    CHECK(get_file(n, "inventory_storage") == other);
    // team_read_storage
    Bytes rd;
    CHECK(mgmp::team_read_storage(n, rd) == 1 && rd == other);
    sqlite3* e = new_save();
    CHECK(mgmp::team_read_storage(e, rd) == 0 && rd.empty());
    CHECK(mgmp::team_read_storage(nullptr, rd) == -1);
    sqlite3_close(e);
    sqlite3_close(n);
}

// La bourse d'un invite ajoutee a sa maison (mm_purse) : properties, entiers.
int64_t get_prop(sqlite3* db, const char* key, bool& found) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db, "SELECT data FROM properties WHERE key = ?1", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, key, -1, SQLITE_STATIC);
    found = sqlite3_step(st) == SQLITE_ROW && sqlite3_column_type(st, 0) == SQLITE_INTEGER;
    const int64_t v = found ? sqlite3_column_int64(st, 0) : 0;
    sqlite3_finalize(st);
    return v;
}

void test_purse_apply() {
    sqlite3* db = new_save();
    exec(db, "INSERT INTO properties VALUES('house_gold', 17)");
    exec(db, "INSERT INTO properties VALUES('adventure_coins', 40)");
    mgmp::TeamPurseApply r;
    CHECK(mgmp::team_apply_purse(db, 27, 12, &r));
    bool fg = false, ff = false, fa = false;
    CHECK(get_prop(db, "house_gold", fg) == 44 && fg);
    CHECK(get_prop(db, "house_food", ff) == 37 && ff);                 // absente : 25 (le jeu), plus 12
    CHECK(get_prop(db, "adventure_coins", fa) == 40 && fa);            // le reste n'est pas touche
    CHECK(r.gold_before == 17 && r.gold_after == 44 && r.food_before == 25 && r.food_after == 37);

    // Rien que des pieces : la nourriture n'est pas reecrite.
    CHECK(mgmp::team_apply_purse(db, 3, 0, &r));
    CHECK(get_prop(db, "house_gold", fg) == 47 && get_prop(db, "house_food", ff) == 37);
    // Negatif : zero.
    CHECK(mgmp::team_apply_purse(db, -5, -5, &r));
    CHECK(get_prop(db, "house_gold", fg) == 47 && get_prop(db, "house_food", ff) == 37);
    // Plafond int32.
    exec(db, "UPDATE properties SET data = 2147483600 WHERE key = 'house_gold'");
    CHECK(mgmp::team_apply_purse(db, 1000, 0, &r));
    CHECK(get_prop(db, "house_gold", fg) == 2147483647);

    // Pas un entier : la valeur que le jeu en lit (25), remise en entier.
    exec(db, "UPDATE properties SET data = 'beaucoup' WHERE key = 'house_food'");
    CHECK(mgmp::team_apply_purse(db, 0, 10, &r));
    CHECK(get_prop(db, "house_food", ff) == 35 && ff && r.food_before == 25);
    // L'or absent : 0.
    exec(db, "DELETE FROM properties WHERE key = 'house_gold'");
    CHECK(mgmp::team_apply_purse(db, 4, 0, &r) && get_prop(db, "house_gold", fg) == 4 && r.gold_before == 0);
    // Et la transaction n'est pas restee ouverte : une autre passe.
    exec(db, "UPDATE properties SET data = 5 WHERE key = 'house_food'");
    CHECK(mgmp::team_apply_purse(db, 0, 1, &r) && get_prop(db, "house_food", ff) == 6);
    sqlite3_close(db);

    // Sans table properties : refuse.
    sqlite3* bare = nullptr;
    sqlite3_open(":memory:", &bare);
    CHECK(!mgmp::team_apply_purse(bare, 1, 1, &r));
    sqlite3_close(bare);
}

// Un seau d'objets tel que le jeu l'ecrit (u32 nombre, puis chaque objet en
// version 5) : faux s'il n'est pas lu jusqu'au dernier octet.
bool read_bucket(const Bytes& b, std::vector<mgmp::ItemFields>& out) {
    out.clear();
    if (b.size() < 4) return false;
    uint32_t n = 0;
    memcpy(&n, b.data(), 4);
    size_t p = 4;
    auto need = [&](uint64_t k) { return p <= b.size() && k <= (uint64_t)(b.size() - p); };
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t v = 0;
        uint64_t len = 0;
        if (!need(5)) return false;
        memcpy(&v, &b[p], 4);
        const uint8_t present = b[p + 4];
        p += 5;
        if (v != 5 || present != 1 || !need(8)) return false;
        mgmp::ItemFields f;
        memcpy(&len, &b[p], 8); p += 8;
        if (!need(len)) return false;
        f.name.assign((const char*)b.data() + p, (size_t)len); p += (size_t)len;
        if (!need(8)) return false;
        memcpy(&len, &b[p], 8); p += 8;
        if (!need(len)) return false;
        f.text.assign((const char*)b.data() + p, (size_t)len); p += (size_t)len;
        if (!need(18)) return false;
        memcpy(&f.a, &b[p], 4); memcpy(&f.b, &b[p + 4], 4); memcpy(&f.c, &b[p + 8], 4); memcpy(&f.d, &b[p + 12], 4);
        f.e = b[p + 16];
        f.f = b[p + 17];
        p += 18;
        out.push_back(f);
    }
    return p == b.size();
}

mgmp::ItemFields loot_item(const char* name, const char* text, uint32_t a, uint32_t f) {
    mgmp::ItemFields it;
    it.name = name; it.text = text; it.a = a; it.b = 2; it.c = 0x26; it.d = 4000003; it.e = 0xFF; it.f = f;
    return it;
}

bool same_item(const mgmp::ItemFields& x, const mgmp::ItemFields& y) {
    return x.name == y.name && x.text == y.text && x.a == y.a && x.b == y.b && x.c == y.c && x.d == y.d &&
           x.e == y.e && (uint8_t)x.f == (uint8_t)y.f;
}

// Le butin gagne par un invite, ajoute au rangement de sa save (mm_loot).
void test_loot_apply() {
    const mgmp::ItemFields pipe = loot_item("Pipe", "", 0xFFFFFFFFu, 0);
    const mgmp::ItemFields book = loot_item("tk_Book", "Fireball", 3, 5);
    const mgmp::ItemFields horn = loot_item("ShoeHorn", "", 1, 1);

    // Le format : celui du serialiseur (version 5, u8 present, chaines u64).
    const Bytes rec = mgmp::team_item_record(book);
    CHECK(rec.size() == 4 + 1 + 8 + 7 + 8 + 8 + 16 + 2);
    Bytes one;
    u32(one, 1);
    one.insert(one.end(), rec.begin(), rec.end());
    std::vector<mgmp::ItemFields> got;
    CHECK(read_bucket(one, got) && got.size() == 1 && same_item(got[0], book));

    // Pas de rangement dans la save : il est cree avec les objets.
    sqlite3* db = new_save();
    uint32_t before = 99, after = 99;
    CHECK(mgmp::team_apply_loot(db, { pipe, book }, &before, &after) == 1);
    CHECK(before == 0 && after == 2);
    CHECK(read_bucket(get_file(db, "inventory_storage"), got) && got.size() == 2);
    CHECK(same_item(got[0], pipe) && same_item(got[1], book));
    // Un rangement existant : les objets s'ajoutent a la fin, le reste est intact.
    const Bytes prev = get_file(db, "inventory_storage");
    CHECK(mgmp::team_apply_loot(db, { horn }, &before, &after) == 1 && before == 2 && after == 3);
    const Bytes now = get_file(db, "inventory_storage");
    CHECK(read_bucket(now, got) && got.size() == 3 && same_item(got[2], horn));
    CHECK(now.size() > prev.size() && memcmp(now.data() + 4, prev.data() + 4, prev.size() - 4) == 0);
    // Rien a ajouter : rien n'est ecrit.
    CHECK(mgmp::team_apply_loot(db, {}, &before, &after) == 1 && get_file(db, "inventory_storage") == now);
    // Un rangement vide comme l'ecrit le jeu (4 octets a zero).
    put_file(db, "inventory_storage", Bytes{ 0, 0, 0, 0 });
    CHECK(mgmp::team_apply_loot(db, { pipe }, &before, &after) == 1 && before == 0 && after == 1);
    CHECK(read_bucket(get_file(db, "inventory_storage"), got) && got.size() == 1 && same_item(got[0], pipe));
    // Un nombre d'objets insense : rien n'est ecrit.
    const Bytes insane = { 0xFF, 0xFF, 0xFF, 0xFF, 1, 2, 3 };
    put_file(db, "inventory_storage", insane);
    CHECK(mgmp::team_apply_loot(db, { pipe }) == 0 && get_file(db, "inventory_storage") == insane);
    // La transaction n'est pas restee ouverte.
    put_file(db, "inventory_storage", one);
    CHECK(mgmp::team_apply_loot(db, { horn }, &before, &after) == 1 && before == 1 && after == 2);
    CHECK(read_bucket(get_file(db, "inventory_storage"), got) && got.size() == 2 && same_item(got[0], book));
    sqlite3_close(db);

    // Sans table files, ou sans base : erreur, et la base reste utilisable.
    sqlite3* bare = nullptr;
    sqlite3_open(":memory:", &bare);
    CHECK(mgmp::team_apply_loot(bare, { pipe }) == -1);
    exec(bare, "BEGIN IMMEDIATE");
    exec(bare, "COMMIT");
    sqlite3_close(bare);
    CHECK(mgmp::team_apply_loot(nullptr, { pipe }) == -1);

    // team_item_parse : l'inverse de team_item_record.
    size_t pos = 4;
    mgmp::ItemFields back;
    CHECK(mgmp::team_item_parse(one.data(), one.size(), pos, back) && pos == one.size() && same_item(back, book));
    pos = 4;
    CHECK(!mgmp::team_item_parse(one.data(), one.size() - 1, pos, back) && pos == 4);
    Bytes v4 = one;
    v4[4] = 4;                                              // une autre version
    pos = 4;
    CHECK(!mgmp::team_item_parse(v4.data(), v4.size(), pos, back));

    // Ce qui attend d'etre ajoute a une save : le fichier « .mgmp-attente ».
    std::vector<mgmp::PendingLoot> pl(2), pl2;
    pl[0].seq = 3; pl[0].key = (77ull << 8) | 1; pl[0].item = book; pl[0].name = "Livre de Boule de feu";
    pl[1].seq = 3; pl[1].key = (77ull << 8) | 4; pl[1].item = pipe; pl[1].name = "Pipe";
    std::vector<mgmp::PendingPayout> pp(1), pp2;
    pp[0].seq = 3; pp[0].coins = 27; pp[0].food = -2;
    const Bytes file = mgmp::team_pending_encode(pl, pp);
    CHECK(mgmp::team_pending_decode(file.data(), file.size(), pl2, pp2));
    CHECK(pl2.size() == 2 && pp2.size() == 1);
    CHECK(pl2[0].seq == 3 && pl2[0].key == pl[0].key && pl2[0].name == pl[0].name && same_item(pl2[0].item, book));
    CHECK(same_item(pl2[1].item, pipe) && pp2[0].coins == 27 && pp2[0].food == -2);
    // Coupe, un octet de trop, une autre tete : refuse (et rien n'est rendu).
    for (size_t cut = 0; cut < file.size(); cut += 7) {
        CHECK(!mgmp::team_pending_decode(file.data(), cut, pl2, pp2));
        CHECK(pl2.empty() && pp2.empty());
    }
    Bytes more = file;
    more.push_back(0);
    CHECK(!mgmp::team_pending_decode(more.data(), more.size(), pl2, pp2));
    Bytes bad = file;
    bad[0] ^= 1;
    CHECK(!mgmp::team_pending_decode(bad.data(), bad.size(), pl2, pp2));
    // Vide : un fichier valide.
    const Bytes none = mgmp::team_pending_encode({}, {});
    CHECK(mgmp::team_pending_decode(none.data(), none.size(), pl2, pp2) && pl2.empty() && pp2.empty());
}

} // namespace

int main() {
    test_loot_apply();
    test_purse_apply();
    test_message();
    test_cat();
    test_blob();
    test_house();
    test_owners();
    test_embark();
    test_gear();
    test_rungear();
    test_item_ops();
    test_plan_keys();
    test_guest_apply();
    test_storage();
    if (g_fail) { std::printf("%d echec(s)\n", g_fail); return 1; }
    std::printf("ok\n");
    return 0;
}
