// tests/test_npcgift.cpp -- Mewgenics Multi : les dons de l'invite aux PNJ, la
// partie portable (mm_npcgift_core).
#include "mm_npcgift_core.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// mm_team_merge.cpp (team_kind, item_digest) ecrit au journal du mod.
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

using namespace mgmp;

static int g_fail = 0, g_ok = 0;
#define CHECK(c) do { if (c) ++g_ok; else { ++g_fail; std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

static NpcGiftMsg rt(const NpcGiftMsg& m, bool* ok = nullptr) {
    const std::vector<uint8_t> b = npcgift_encode(m);
    NpcGiftMsg out;
    const bool r = npcgift_decode(b.data(), b.size(), out);
    if (ok) *ok = r;
    return out;
}

static ItemFields item(const char* name, uint32_t a = 1) {
    ItemFields f;
    f.name = name;
    f.text = "";
    f.a = a; f.b = 2; f.c = 0x40; f.d = 7; f.e = 1; f.f = 300;
    return f;
}

static void test_codec() {
    // Un don, avec les capacites d'un chat.
    NpcGiftMsg d;
    d.op = kNpcDonate;
    d.npc = 0;
    d.favor = 1;
    d.cat = u8"Moustache [élu]";
    d.abilities = { "Fireball", "None", "Scratch", "None", "Thorns", "None" };
    bool ok = false;
    NpcGiftMsg r = rt(d, &ok);
    CHECK(ok);
    CHECK(r.op == kNpcDonate && r.npc == 0 && r.favor == 1 && r.cat == d.cat);
    CHECK(r.abilities == d.abilities);

    // Une recompense : pieces, nourriture, objets et leurs noms affiches.
    NpcGiftMsg g;
    g.op = kNpcGift;
    g.npc = 2;
    g.gift = 0x1122334455667788ull;
    g.coins = 50;
    g.food = 3;
    g.items.push_back(NpcGiftItem{ item("blankcollar"), u8"Collier vierge" });
    g.items.push_back(NpcGiftItem{ item("needle_fire", 0xFFFFFFFFu), u8"Aiguille de feu" });
    r = rt(g, &ok);
    CHECK(ok);
    CHECK(r.gift == g.gift && r.coins == 50 && r.food == 3 && r.items.size() == 2);
    CHECK(r.items[1].f.a == 0xFFFFFFFFu && r.items[1].f.f == 300 && r.items[1].label == u8"Aiguille de feu");
    CHECK(item_digest(r.items[0].f) == item_digest(g.items[0].f));

    // Un nom trop long est coupe sans casser un caractere UTF-8 (relecture de la
    // build 29) ; les limites des objets sont celles que mm_npcgift verifie.
    {
        NpcGiftMsg longname = d;
        longname.cat.clear();
        for (int i = 0; i < 100; ++i) longname.cat += u8"é";
        r = rt(longname, &ok);
        CHECK(ok && !r.cat.empty() && r.cat.size() < longname.cat.size() && r.cat.size() % 2 == 0);
        NpcGiftMsg longlabel = g;
        longlabel.items[0].label.clear();
        for (int i = 0; i < 100; ++i) longlabel.items[0].label += u8"€";   // 3 octets
        r = rt(longlabel, &ok);
        CHECK(ok && r.items[0].label.size() % 3 == 0 && r.items[0].label.size() <= 127);
        CHECK(kNpcGiftMaxName == 63 && kNpcGiftMaxText == 255);
    }

    // La reponse.
    NpcGiftMsg got;
    got.op = kNpcGot;
    got.npc = 2;
    got.gift = g.gift;
    got.money = 1;
    got.kept = 0x3;
    r = rt(got, &ok);
    CHECK(ok && r.gift == g.gift && r.money == 1 && r.kept == 0x3);

    NpcGiftMsg ready;
    ready.op = kNpcReady;
    ready.npc = 7;
    r = rt(ready, &ok);
    CHECK(ok && r.op == kNpcReady && r.npc == 7);

    // Refus : PNJ hors bornes, faveur nulle ou enorme, pieces negatives, op inconnue,
    // octets en trop, tronque, pas un kTeamNpc.
    NpcGiftMsg bad = d;
    bad.npc = 8;
    rt(bad, &ok);
    CHECK(!ok);
    bad = d;
    bad.favor = 0;
    rt(bad, &ok);
    CHECK(!ok);
    bad.favor = 1000;
    rt(bad, &ok);
    CHECK(!ok);
    bad = g;
    bad.coins = -5;
    rt(bad, &ok);
    CHECK(!ok);
    std::vector<uint8_t> b = npcgift_encode(d);
    b.push_back(0);
    NpcGiftMsg out;
    CHECK(!npcgift_decode(b.data(), b.size(), out));
    b = npcgift_encode(g);
    for (size_t cut = 0; cut < b.size(); ++cut) CHECK(!npcgift_decode(b.data(), cut, out));
    b = npcgift_encode(d);
    b[8] = 99;                                    // op inconnue
    CHECK(!npcgift_decode(b.data(), b.size(), out));
    b = npcgift_encode(d);
    b[5] = kTeamLoot;                             // un autre genre de message d'equipe
    CHECK(!npcgift_decode(b.data(), b.size(), out));
    CHECK(team_kind(b.data(), b.size()) == kTeamLoot);
    b = npcgift_encode(d);
    CHECK(team_kind(b.data(), b.size()) == kTeamNpc);
    // Un objet sans nom.
    bad = g;
    bad.items[0].f.name.clear();
    rt(bad, &ok);
    CHECK(!ok);
    // Trop de capacites : coupees a 6 a l'envoi.
    bad = d;
    bad.abilities.resize(9, "Extra");
    r = rt(bad, &ok);
    CHECK(ok && r.abilities.size() == kNpcGiftAbilities);
}

static void test_abilities() {
    std::vector<std::string> a = npcgift_abilities({ "Fireball", "", "bad name!", "None" });
    CHECK(a.size() == 6);
    CHECK(a[0] == "Fireball" && a[1] == "None" && a[2] == "None" && a[3] == "None" && a[4] == "None");
    a = npcgift_abilities({});
    CHECK(a.size() == 6 && a[5] == "None");
    a = npcgift_abilities({ "A", "B", "C", "D", "E", "F", "G" });
    CHECK(a.size() == 6 && a[5] == "F");
}

static void test_new_items() {
    std::vector<NpcStored> before = { { 10, item("a") }, { 11, item("b") } };
    std::vector<NpcStored> after  = { { 10, item("a") }, { 12, item("c") }, { 11, item("b") }, { 13, item("d") } };
    std::vector<size_t> n = npcgift_new_items(before, after);
    CHECK(n.size() == 2 && n[0] == 1 && n[1] == 3);
    // Un objet pris entre-temps (disparu) n'est pas nouveau ; rien de neuf : vide.
    n = npcgift_new_items(after, before);
    CHECK(n.empty());
    n = npcgift_new_items({}, after);
    CHECK(n.size() == 4);
}

static void test_text() {
    CHECK(npcgift_list_text(0, 0, {}) == "rien");
    CHECK(npcgift_list_text(1, 0, {}) == u8"1 pièce");
    CHECK(npcgift_list_text(50, 2, {}) == u8"50 pièces et 2 nourritures");
    CHECK(npcgift_list_text(50, 1, { u8"Collier vierge" }) == u8"50 pièces, 1 nourriture et Collier vierge");
    CHECK(npcgift_list_text(0, 0, { "A", "", "B" }) == "A et B");
    CHECK(npcgift_list_text(-3, -1, { "A" }) == "A");
}

static void test_donor() {
    NpcDonor d;
    // Un don qui ne remplit pas la jauge : rien.
    CHECK(!npcgift_after_donation(d, false, false, 1, "Lou", "tink_reward1"));
    CHECK(d.peer == kNpcNoDonor);
    // Le don de Lou la remplit : la recompense est a Lou.
    CHECK(npcgift_after_donation(d, false, true, 1, "Lou", "tink_reward1"));
    CHECK(d.peer == 1 && d.name == "Lou" && d.reward == "tink_reward1");
    // Un autre don, jauge deja pleine : elle reste a Lou.
    CHECK(!npcgift_after_donation(d, true, true, 2, "Max", "tink_reward1"));
    CHECK(d.peer == 1);
    CHECK(npcgift_reward_owner(d, "tink_reward1") == 1);
    // Plus la meme recompense visee (prise autrement) : a l'hote.
    CHECK(npcgift_reward_owner(d, "tink_reward2") == kNpcNoDonor);
    // L'hote remplit lui-meme une jauge : la sienne, meme si un donneur restait note.
    NpcDonor h;
    h.peer = 2; h.name = "Max"; h.reward = "frank_attic";
    CHECK(npcgift_after_donation(h, false, true, kNpcNoDonor, "", "frank_attic"));
    CHECK(h.peer == kNpcNoDonor && npcgift_reward_owner(h, "frank_attic") == kNpcNoDonor);
}

int main() {
    test_codec();
    test_abilities();
    test_new_items();
    test_text();
    test_donor();
    std::printf("%s (%d verifications, %d echec(s))\n", g_fail ? "ECHEC" : "ok", g_ok + g_fail, g_fail);
    return g_fail ? 1 : 0;
}
