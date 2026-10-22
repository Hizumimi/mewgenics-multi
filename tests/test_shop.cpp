// tests/test_shop.cpp -- Mewgenics Multi : les achats en boutique avec sa bourse,
// la partie portable (mm_shop_core, protocole 51).
#include "mm_shop_core.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// mm_team_merge.cpp (item_digest) ecrit au journal du mod.
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

static ShopMsg rt(const ShopMsg& m, bool* ok = nullptr) {
    const std::vector<uint8_t> b = shop_encode(m);
    ShopMsg out;
    const bool r = shop_decode(b.data(), b.size(), out);
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

static bool contains(const std::string& s, const char* part) { return s.find(part) != std::string::npos; }

static void test_codec() {
    bool ok = false;
    // Une demande d'achat.
    ShopMsg b;
    b.op = kShopBuy;
    b.node = 0x211f6ea51e365ae0ull;
    b.index = 3;
    b.digest = 0x1234567890abcdefull;
    b.price = 12;
    ShopMsg r = rt(b, &ok);
    CHECK(ok);
    CHECK(r.op == kShopBuy && r.node == b.node && r.index == 3 && r.digest == b.digest && r.price == 12);

    // La reponse, chaque resultat.
    for (uint8_t res = 0; res <= kShopResultLast; ++res) {
        ShopMsg d;
        d.op = kShopDone;
        d.node = 7;
        d.index = 1;
        d.result = (ShopResult)res;
        d.price = 30;
        d.purse = -4;
        r = rt(d, &ok);
        CHECK(ok && r.result == d.result && r.price == 30 && r.purse == -4 && r.node == 7 && r.index == 1);
    }

    // Un objet vendu, avec son nom (UTF-8).
    ShopMsg s;
    s.op = kShopSold;
    s.node = 99;
    s.index = 5;
    s.digest = 42;
    s.price = 18;
    s.buyer = 2;
    s.coins_before = 40; s.food_before = 3; s.coins_after = 22; s.food_after = 3;
    s.label = u8"Épée [rouillée]";
    r = rt(s, &ok);
    CHECK(ok);
    CHECK(r.buyer == 2 && r.coins_before == 40 && r.coins_after == 22 && r.food_before == 3 && r.food_after == 3);
    CHECK(r.label == s.label && r.digest == 42 && r.price == 18 && r.index == 5);
    // Un nom trop long est coupe sans casser un caractere.
    s.label.clear();
    for (int i = 0; i < 100; ++i) s.label += u8"é";
    r = rt(s, &ok);
    CHECK(ok && r.label.size() <= kShopMaxLabel && r.label.size() % 2 == 0);

    // A la victoire : les achats rentrent chez l'acheteur.
    ShopMsg g;
    g.op = kShopGive;
    g.seq = 4;
    g.gift = 0xfeedull;
    for (int i = 0; i < 3; ++i) {
        ShopGiveItem it;
        it.f = item(i == 1 ? "Book_Fireball" : "Sword", (uint32_t)i + 1);
        if (i == 1) it.f.text = "Fireball";
        it.label = i == 1 ? u8"Livre : Boule de feu" : u8"Épée";
        g.items.push_back(it);
    }
    r = rt(g, &ok);
    CHECK(ok && r.seq == 4 && r.gift == 0xfeedull && r.items.size() == 3);
    CHECK(r.items[1].f.name == "Book_Fireball" && r.items[1].f.text == "Fireball" && r.items[1].label == g.items[1].label);
    CHECK(r.items[2].f.a == 3 && r.items[2].f.f == 300 && r.items[2].f.e == 1);
    CHECK(item_digest(r.items[0].f) == item_digest(g.items[0].f));
    // Au plus kShopMaxItems objets.
    g.items.resize(kShopMaxItems + 5, g.items[0]);
    r = rt(g, &ok);
    CHECK(ok && r.items.size() == kShopMaxItems);

    // L'invite dit a l'hote si sa boutique est ouverte.
    ShopMsg h;
    h.op = kShopHere;
    h.node = 0x77;
    h.open = 1;
    r = rt(h, &ok);
    CHECK(ok && r.op == kShopHere && r.node == 0x77 && r.open == 1);
    h.open = 0;
    r = rt(h, &ok);
    CHECK(ok && r.open == 0);

    ShopMsg k;
    k.op = kShopGot;
    k.gift = 0xfeedull;
    k.kept = 0x5ull;
    r = rt(k, &ok);
    CHECK(ok && r.gift == 0xfeedull && r.kept == 5);
}

static void test_codec_refuses() {
    ShopMsg out;
    // Pas un kTeamShop.
    std::vector<uint8_t> b = shop_encode([]{ ShopMsg x; x.op = kShopBuy; x.node = 1; x.digest = 1; x.price = 1; return x; }());
    b[5] = kTeamNpc;
    CHECK(!shop_decode(b.data(), b.size(), out));
    // Tronque, ou trop long.
    ShopMsg s;
    s.op = kShopSold; s.node = 3; s.label = "abc";
    b = shop_encode(s);
    for (size_t n = 0; n < b.size(); ++n) CHECK(!shop_decode(b.data(), n, out));
    b.push_back(0);
    CHECK(!shop_decode(b.data(), b.size(), out));
    // Une case nulle, un prix negatif, un resultat inconnu, un op inconnu.
    ShopMsg z;
    z.op = kShopBuy; z.node = 0;
    b = shop_encode(z);
    CHECK(!shop_decode(b.data(), b.size(), out));
    z.node = 1; z.price = -1;
    b = shop_encode(z);
    CHECK(!shop_decode(b.data(), b.size(), out));
    ShopMsg d;
    d.op = kShopDone; d.node = 1;
    b = shop_encode(d);
    b[4 + 1 + 1 + 1 + 8 + 2] = kShopResultLast + 1;
    CHECK(!shop_decode(b.data(), b.size(), out));
    ShopMsg u;
    u.op = 9;
    b = shop_encode(u);
    CHECK(!shop_decode(b.data(), b.size(), out));
    // Un compte negatif dans kShopSold.
    s.coins_after = -1;
    b = shop_encode(s);
    CHECK(!shop_decode(b.data(), b.size(), out));
    // Un envoi sans numero, un objet sans nom.
    ShopMsg g;
    g.op = kShopGive; g.gift = 0;
    b = shop_encode(g);
    CHECK(!shop_decode(b.data(), b.size(), out));
    g.gift = 5;
    ShopGiveItem it;
    it.f = item("");
    g.items.push_back(it);
    b = shop_encode(g);
    CHECK(!shop_decode(b.data(), b.size(), out));
    CHECK(!shop_decode(nullptr, 10, out));
    // kShopHere : sans case, ou un drapeau inconnu.
    ShopMsg h;
    h.op = kShopHere; h.node = 0; h.open = 1;
    b = shop_encode(h);
    CHECK(!shop_decode(b.data(), b.size(), out));
    h.node = 5;
    b = shop_encode(h);
    b.back() = 2;
    CHECK(!shop_decode(b.data(), b.size(), out));
}

static void test_rules() {
    ShopMsg req;
    req.op = kShopBuy; req.node = 5; req.index = 2; req.digest = 77; req.price = 10;
    ShopLook look;
    look.shop_here = true; look.present = true; look.digest = 77; look.type = kShopTypeItem; look.price = 10;
    look.stock = 1;
    CHECK(shop_check(req, look, true, true, 10) == ShopResult::Bought);
    CHECK(shop_check(req, look, true, true, 9) == ShopResult::Poor);
    CHECK(shop_check(req, look, true, true, -3) == ShopResult::Poor);   // une dette
    CHECK(shop_check(req, look, false, true, 50) == ShopResult::NotRun);
    CHECK(shop_check(req, look, true, false, 50) == ShopResult::NoPurse);
    ShopLook l2 = look;
    l2.shop_here = false;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::NoShop);
    l2 = look; l2.present = false;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::NoItem);
    l2 = look; l2.digest = 78;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::NoItem);
    l2 = look; l2.stock = 0;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::SoldOut);
    l2 = look; l2.type = kShopTypeLevelUp;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::NotItem);
    l2.type = kShopTypeFamiliar;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::NotItem);
    l2.type = kShopTypeFood;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::Bought);
    l2.type = kShopTypeCoins;
    CHECK(shop_check(req, l2, true, true, 50) == ShopResult::Bought);
    // Le prix vu ne compte pas : celui de l'hote (un prix a change -> l'empreinte aussi).
    ShopMsg r2 = req;
    r2.price = 1;
    CHECK(shop_check(r2, look, true, true, 5) == ShopResult::Poor);

    CHECK(shop_guest_type(kShopTypeItem) && shop_guest_type(kShopTypeFood) && shop_guest_type(kShopTypeCoins));
    CHECK(!shop_guest_type(kShopTypeLevelUp) && !shop_guest_type(kShopTypeFamiliar) &&
          !shop_guest_type(kShopTypeCollar) && !shop_guest_type(kShopTypeFurniture) && !shop_guest_type(0));

    CHECK(shop_host_check(true, 10, 10) == ShopResult::Bought);
    CHECK(shop_host_check(true, 9, 10) == ShopResult::Poor);
    CHECK(shop_host_check(false, 0, 10) == ShopResult::NoPurse);
    CHECK(shop_host_check(false, 0, 0) == ShopResult::Bought);

    // Les empreintes : chaque champ compte.
    const uint64_t d0 = shop_item_digest(1, 10, 0, 555);
    CHECK(d0 != 0);
    CHECK(d0 == shop_item_digest(1, 10, 0, 555));
    CHECK(d0 != shop_item_digest(16, 10, 0, 555));
    CHECK(d0 != shop_item_digest(1, 11, 0, 555));
    CHECK(d0 != shop_item_digest(1, 10, 1, 555));
    CHECK(d0 != shop_item_digest(1, 10, 0, 556));
}

static void test_texts() {
    for (uint8_t res = 1; res <= kShopResultLast; ++res) {
        const std::string t = shop_refusal_text((ShopResult)res, false, 12, 5);
        CHECK(!t.empty() && contains(t, u8"Boutique"));
        CHECK(t.find('[') == std::string::npos);
    }
    CHECK(shop_refusal_text(ShopResult::Bought, false, 1, 1).empty());
    const std::string poor = shop_refusal_text(ShopResult::Poor, false, 12, 5);
    CHECK(contains(poor, u8"12 pièces") && contains(poor, u8"en a 5"));
    CHECK(contains(shop_refusal_text(ShopResult::Poor, false, 1, -3), u8"1 pièce ;"));
    CHECK(contains(shop_refusal_text(ShopResult::Poor, false, 1, -3), u8"en a 0"));
    CHECK(contains(shop_refusal_text(ShopResult::Poor, true, 12, 5), u8"toi aussi"));
    CHECK(shop_refusal_text(ShopResult::NoPurse, true, 1, 1) != shop_refusal_text(ShopResult::NoPurse, false, 1, 1));

    const std::string w1 = shop_wait_text("Ami", false), w2 = shop_wait_text("Ami et Lou", true);
    CHECK(contains(w1, u8"Ami n'a pas fini") && contains(w1, u8"attends-le") && contains(w1, u8"quand même"));
    CHECK(contains(w2, u8"Ami et Lou n'ont pas fini") && contains(w2, u8"attends-les"));
    CHECK(contains(shop_local_failure_text(), u8"rien n'a été acheté"));
    CHECK(contains(shop_refusal_text(ShopResult::Busy, false, 3, 9), u8"Réessaie"));

    CHECK(shop_amount_label(kShopTypeFood, 5) == u8"5 nourritures");
    CHECK(shop_amount_label(kShopTypeFood, 1) == u8"1 nourriture");
    CHECK(shop_amount_label(kShopTypeCoins, 10) == u8"10 pièces");
    CHECK(shop_amount_label(kShopTypeCoins, 1) == u8"1 pièce");
    CHECK(shop_amount_label(kShopTypeItem, 3).empty());
}

static void test_index_and_ledger() {
    uint16_t ix = 0;
    CHECK(shop_index_of(0x1000, 0x1000 + 0xC0 * 4, 0x1000 + 0xC0 * 2, 0xC0, ix) && ix == 2);
    CHECK(shop_index_of(0x1000, 0x1000 + 0xC0 * 4, 0x1000, 0xC0, ix) && ix == 0);
    CHECK(!shop_index_of(0x1000, 0x1000 + 0xC0 * 4, 0x1000 + 0xC0 * 4, 0xC0, ix));   // la fin
    CHECK(!shop_index_of(0x1000, 0x1000 + 0xC0 * 4, 0x1000 + 0xC0 + 8, 0xC0, ix));    // au milieu
    CHECK(!shop_index_of(0x1000, 0x1000 + 0xC0 * 4, 0x0F00, 0xC0, ix));
    CHECK(!shop_index_of(0x2000, 0x1000, 0x1800, 0xC0, ix));
    CHECK(!shop_index_of(0x1000, 0x2000, 0x1000, 0, ix));

    ShopLedger L;
    CHECK(L.owner(5) == -1);
    L.add(5, 50, 2, "Sword");
    L.add(9, 90, 0, "Shield");
    CHECK(L.owner(5) == 2 && L.owner(9) == 0 && L.size() == 2 && L.all()[1].digest == 90);
    // Un second exemplaire du meme objet (meme id), achete par un autre : un achat de plus.
    L.add(5, 50, 3, "Sword");
    CHECK(L.owner(5) == 2 && L.size() == 3 && L.all()[2].slot == 3);
    L.clear();
    CHECK(L.owner(5) == -1 && L.size() == 0);

    CHECK(shop_count({ 1, 2, 3 }, 2) == 1);
    CHECK(shop_count({ 4, 1, 4 }, 4) == 2);
    CHECK(shop_count({}, 4) == 0);

    // La victoire : chaque achat prend un exemplaire du rangement, une seule fois.
    std::vector<ShopOwned> owned(3);
    owned[0].id = 5; owned[0].slot = 2;
    owned[1].id = 9; owned[1].slot = 0;
    owned[2].id = 5; owned[2].slot = 3;
    std::vector<int> m = shop_match_owned(owned, { 7, 5, 9, 5 });
    CHECK(m.size() == 3 && m[0] == 1 && m[1] == 2 && m[2] == 3);
    // Un exemplaire equipe sur un chat (pas au rangement) : le dernier achat n'a rien.
    m = shop_match_owned(owned, { 5, 9 });
    CHECK(m[0] == 0 && m[1] == 1 && m[2] == -1);
    m = shop_match_owned(owned, {});
    CHECK(m[0] == -1 && m[1] == -1 && m[2] == -1);

    // L'hote a recharge l'aventure : nouveaux ids dans le sac, memes empreintes.
    owned[0].digest = 50; owned[1].digest = 90; owned[2].digest = 50;
    std::vector<ShopOwned> r = owned;
    CHECK(shop_rekey_owned(r, { 21, 22, 23, 24 }, { 70, 50, 90, 50 }) == 3);
    CHECK(r[0].id == 22 && r[1].id == 23 && r[2].id == 24 && r[0].slot == 2 && r[2].slot == 3);
    // Un id exact garde son exemplaire ; l'autre achat prend le second par empreinte.
    r = owned;
    CHECK(shop_rekey_owned(r, { 31, 5, 9 }, { 50, 50, 90 }) == 1);
    CHECK(r[0].id == 5 && r[1].id == 9 && r[2].id == 31);
    // Rien de meme empreinte dans le sac : l'achat garde son id (il reste chez l'hote).
    r = owned;
    CHECK(shop_rekey_owned(r, { 41 }, { 77 }) == 0 && r[0].id == 5 && r[2].id == 5);
    // Sans empreinte (0), pas de recalage.
    r = owned;
    r[1].digest = 0;
    CHECK(shop_rekey_owned(r, { 41 }, { 0 }) == 0 && r[1].id == 9);
}

int main() {
    test_codec();
    test_codec_refuses();
    test_rules();
    test_texts();
    test_index_and_ledger();
    std::printf("%s (%d verifications, %d echec(s))\n", g_fail ? "ECHEC" : "ok", g_ok + g_fail, g_fail);
    return g_fail ? 1 : 0;
}
