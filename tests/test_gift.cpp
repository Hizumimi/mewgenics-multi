// test_gift -- Mewgenics Multi : les dons entre joueurs (protocole 62), la partie
// portable (src/session/mm_gift_core.cpp, et les dons entre bourses de
// src/session/mm_purse_core.cpp) :
//
//   - le message kTeamGift (un don, sa reponse, son refus) et ce qu'il refuse ;
//   - les noms coupes sans casser un caractere UTF-8 ;
//   - les objets ajoutes au rangement tel que le jeu l'ecrit ;
//   - ce que fait celui qui recoit (garder, attendre, refuser) ;
//   - le don d'une bourse a une autre (le compte de l'aventure ne change pas) et
//     les messages MM_PURSE a = 4, 5, 6 ;
//   - les textes en francais (elisions comprises).

#include "mm_gift_core.h"
#include "mm_purse_core.h"
#include "mm_team_merge.h"
#include "mgmp_proto.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// mm_team_merge.cpp (team_item_record, item_digest) ecrit au journal du mod.
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

namespace {

int g_ok   = 0;
int g_fail = 0;

void check(bool ok, const char* what) {
    if (ok) ++g_ok;
    else {
        std::printf("FAIL: %s\n", what);
        ++g_fail;
    }
}

ItemFields item(const char* name, uint32_t a = 1) {
    ItemFields f;
    f.name = name;
    f.text = "";
    f.a = a; f.b = 2; f.c = 0x40; f.d = 7; f.e = 1; f.f = 3;
    return f;
}

bool same(const ItemFields& x, const ItemFields& y) {
    return x.name == y.name && x.text == y.text && x.a == y.a && x.b == y.b && x.c == y.c && x.d == y.d &&
           x.e == y.e && x.f == y.f;
}

GiftMsg roundtrip(const GiftMsg& m, bool* ok) {
    const std::vector<uint8_t> b = gift_encode(m);
    GiftMsg out;
    *ok = gift_decode(b.data(), b.size(), out);
    return out;
}

void test_codec() {
    GiftMsg m;
    m.op     = kGiftOffer;
    m.id     = 0xDEADBEEF;
    m.giver  = 2;
    m.target = 0;
    m.coins  = 20;
    m.food   = 5;
    GiftItem a;
    a.f     = item("Snack", 3);
    a.f.text = "Fireball";
    a.label = u8"Casse-croûte";
    m.items.push_back(a);
    GiftItem b;
    b.f     = item("Sword");
    b.label = u8"Épée";
    m.items.push_back(b);
    m.giver_name = u8"Élise";
    bool ok = false;
    const GiftMsg r = roundtrip(m, &ok);
    check(ok, "un don se relit");
    check(r.op == kGiftOffer && r.id == 0xDEADBEEF && r.giver == 2 && r.target == 0, "tete du don");
    check(r.coins == 20 && r.food == 5, "pieces et nourriture");
    check(r.items.size() == 2 && same(r.items[0].f, a.f) && same(r.items[1].f, b.f), "les objets");
    check(r.items[0].label == u8"Casse-croûte" && r.items[1].label == u8"Épée", "leurs noms");
    check(r.giver_name == u8"Élise", "le nom du donneur");

    // Un don d'or seul.
    GiftMsg c;
    c.op = kGiftOffer; c.id = 1; c.giver = 0; c.target = 1; c.coins = 1;
    roundtrip(c, &ok);
    check(ok, "or seul");

    // Les reponses.
    GiftMsg got;
    got.op = kGiftGot; got.id = 7; got.giver = 1; got.target = 3;
    const GiftMsg rg = roundtrip(got, &ok);
    check(ok && rg.op == kGiftGot && rg.id == 7 && rg.giver == 1 && rg.target == 3, "reponse : garde");
    GiftMsg no;
    no.op = kGiftRefused; no.id = 8; no.giver = 1; no.target = 0; no.why = kGiftWhyBusy;
    const GiftMsg rn = roundtrip(no, &ok);
    check(ok && rn.op == kGiftRefused && rn.why == kGiftWhyBusy, "reponse : refuse et pourquoi");

    // Ce qui est refuse.
    GiftMsg self = c;
    self.target = self.giver;
    roundtrip(self, &ok);
    check(!ok, "un don a soi-meme");
    GiftMsg empty = c;
    empty.coins = 0;
    roundtrip(empty, &ok);
    check(!ok, "un don vide");
    GiftMsg bad = no;
    bad.why = 0;
    roundtrip(bad, &ok);
    check(!ok, "un refus sans raison");
    bad.why = kGiftWhyLast + 1;
    roundtrip(bad, &ok);
    check(!ok, "un refus a raison inconnue");

    std::vector<uint8_t> raw = gift_encode(c);
    raw.push_back(0);
    GiftMsg out;
    check(!gift_decode(raw.data(), raw.size(), out), "un octet de trop");
    raw = gift_encode(m);
    check(!gift_decode(raw.data(), raw.size() - 1, out), "un octet de moins");
    check(!gift_decode(raw.data(), 6, out), "la tete seule");
    check(!gift_decode(nullptr, 0, out), "rien");

    // Des quantites hors des bornes (ecrites a la main : l'encodeur les borne).
    raw = gift_encode(c);
    // tete MSG_TEAM (4 + 1 + 1), op, id (4), donneur, destinataire : les pieces a 13.
    const int32_t neg = -5;
    memcpy(raw.data() + 13, &neg, 4);
    check(!gift_decode(raw.data(), raw.size(), out), "des pieces negatives");
    const int32_t big = kGiftMaxMoney + 1;
    memcpy(raw.data() + 13, &big, 4);
    check(!gift_decode(raw.data(), raw.size(), out), "trop de pieces");
    // Un op inconnu.
    raw = gift_encode(got);
    raw[6] = 9;
    check(!gift_decode(raw.data(), raw.size(), out), "un op inconnu");
    // Un autre message d'equipe.
    raw = gift_encode(got);
    raw[5] = kTeamShop;
    check(!gift_decode(raw.data(), raw.size(), out), "pas un kTeamGift");

    // L'encodeur borne : pas plus de 16 objets, les montants dans les bornes.
    GiftMsg many = c;
    for (int i = 0; i < 20; ++i) {
        GiftItem it;
        it.f = item("Rock");
        many.items.push_back(it);
    }
    many.coins = kGiftMaxMoney * 3;
    const GiftMsg rm = roundtrip(many, &ok);
    check(ok && rm.items.size() == kGiftMaxItems && rm.coins == kGiftMaxMoney, "16 objets et le plafond");

    // Les noms trop longs : coupes sans casser un caractere UTF-8.
    GiftMsg longname = c;
    std::string l;
    for (int i = 0; i < 100; ++i) l += u8"é";        // 200 octets
    longname.giver_name = l;
    const GiftMsg rl = roundtrip(longname, &ok);
    check(ok && rl.giver_name.size() <= kGiftMaxLabel && rl.giver_name.size() % 2 == 0, "nom coupe en UTF-8");
}

void test_storage() {
    // Un rangement vide (absent) : les objets ajoutes, comptes.
    std::vector<uint8_t> blob;
    std::string why;
    check(gift_storage_append(blob, { item("Snack"), item("Sword", 2) }, &why), "ajout a un rangement vide");
    std::vector<ItemFields> got;
    check(gift_storage_items(blob, got) && got.size() == 2, "deux objets");
    check(got.size() == 2 && same(got[0], item("Snack")) && same(got[1], item("Sword", 2)), "les memes objets");
    uint32_t count = 0;
    memcpy(&count, blob.data(), 4);
    check(count == 2, "le nombre en tete");

    // Un rangement deja plein : les nouveaux a la fin.
    check(gift_storage_append(blob, { item("Gem") }, &why), "ajout a la suite");
    check(gift_storage_items(blob, got) && got.size() == 3 && got[2].name == "Gem", "trois objets, le dernier a la fin");
    // Rien a ajouter : rien ne change.
    const std::vector<uint8_t> keep = blob;
    check(gift_storage_append(blob, {}, &why) && blob == keep, "rien a ajouter");

    // Un rangement illisible : refuse, et laisse tel quel.
    std::vector<uint8_t> broken = blob;
    broken.resize(broken.size() - 3);
    const std::vector<uint8_t> before = broken;
    check(!gift_storage_append(broken, { item("Gem") }, &why) && broken == before, "rangement illisible : tel quel");
    check(!why.empty(), "et pourquoi");
    // Un objet sans nom : refuse.
    std::vector<uint8_t> b2 = keep;
    check(!gift_storage_append(b2, { item("") }, &why) && b2 == keep, "objet sans nom");
    // Le rangement vide du jeu : 4 octets a zero.
    std::vector<uint8_t> zero(4, 0);
    check(gift_storage_items(zero, got) && got.empty(), "rangement vide du jeu");
    check(gift_storage_append(zero, { item("Snack") }, &why) && gift_storage_items(zero, got) && got.size() == 1,
          "ajout au rangement vide du jeu");
}

void test_receiver() {
    uint8_t why = 0;
    check(gift_receiver_step(true, false, 0, why) == GiftStep::Apply && why == 0, "dans sa maison : garde");
    check(gift_receiver_step(false, false, 0, why) == GiftStep::Refuse && why == kGiftWhyNotHome,
          "pas dans sa maison : refuse");
    check(gift_receiver_step(true, true, 1000, why) == GiftStep::Wait, "un ecran ouvert : attend");
    check(gift_receiver_step(true, true, kGiftWaitMs, why) == GiftStep::Refuse && why == kGiftWhyBusy,
          "un ecran ouvert trop longtemps : refuse");
    check(gift_receiver_step(false, true, 0, why) == GiftStep::Refuse && why == kGiftWhyNotHome,
          "pas dans sa maison passe avant l'ecran");

    check(gift_clamp(10, 50) == 10, "clamp : ce qu'il a");
    check(gift_clamp(80, 50) == 50, "clamp : pas plus qu'il n'a");
    check(gift_clamp(-3, 50) == 0 && gift_clamp(5, 0) == 0, "clamp : rien");
    check(gift_clamp(5000000, 9000000) == kGiftMaxMoney, "clamp : le plafond");
}

void test_purse_transfer() {
    PurseBook b;
    b.set_players((1u << 0) | (1u << 1) | (1u << 2), 0);
    b.share(30, 9);                          // 10 pieces et 3 nourritures chacun
    check(b.at(1).coins == 10 && b.at(2).food == 3, "le depart partage");
    check(b.transfer(1, 2, 4, 1), "un don de bourse");
    check(b.at(1).coins == 6 && b.at(1).food == 2 && b.at(2).coins == 14 && b.at(2).food == 4, "les deux bourses");
    check(b.total_coins() == 30 && b.total_food() == 9, "le compte de l'aventure ne change pas");
    check(!b.transfer(1, 2, 7, 0), "plus qu'il n'a : refuse");
    check(!b.transfer(1, 1, 1, 0), "a soi-meme : refuse");
    check(!b.transfer(1, 2, 0, 0), "rien : refuse");
    check(!b.transfer(1, 2, -1, 0), "negatif : refuse");
    check(!b.transfer(1, 9, 1, 0) && !b.transfer(9, 1, 1, 0), "numero hors limites : refuse");
    check(b.at(1).coins == 6 && b.at(2).coins == 14, "un refus ne change rien");
    // Une dette ne se donne pas.
    b.credit(1, -10, 0);
    check(b.at(1).coins == -4 && !b.transfer(1, 0, 1, 0), "une bourse negative ne donne rien");
    // A l'hote, meme sans chat.
    check(b.transfer(2, 0, 14, 0) && b.at(0).coins == 24 && b.at(2).coins == 0, "a l'hote");
}

void test_purse_messages() {
    MmMsg m;
    uint8_t to = 0, seq = 0, giver = 0, why = 0;
    int32_t c = 0, f = 0;
    purse_encode_give_req(m, 2, 15, 3, 9);
    check(m.kind == MM_PURSE && m.a == kPurseGiveReq, "demande : MM_PURSE a = 4");
    check(purse_decode_give_req(m, to, c, f, seq) && to == 2 && c == 15 && f == 3 && seq == 9, "demande relue");
    purse_encode_give_req(m, 2, 0, 0, 1);
    check(!purse_decode_give_req(m, to, c, f, seq), "demande vide : refusee");
    purse_encode_give_req(m, 2, -4, 0, 1);
    check(!purse_decode_give_req(m, to, c, f, seq), "demande negative : refusee");
    purse_encode_give_req(m, 2, kPurseGiveMax + 1, 0, 1);
    check(!purse_decode_give_req(m, to, c, f, seq), "demande trop grosse : refusee");
    purse_encode_give_req(m, kPurseSlots, 1, 0, 1);
    check(!purse_decode_give_req(m, to, c, f, seq), "demande a un numero hors limites : refusee");

    purse_encode_given(m, 1, 0, 7, 0, 0);
    check(purse_decode_given(m, giver, to, c, f, seq) && giver == 1 && to == 0 && c == 7 && f == 0 && seq == 0,
          "don fait relu (numero 0 : celui de l'hote)");
    purse_encode_given(m, 3, 2, 0, 4, 200);
    check(purse_decode_given(m, giver, to, c, f, seq) && seq == 200 && f == 4, "don fait : le numero de la demande");
    purse_encode_given(m, 2, 2, 1, 0, 1);
    check(!purse_decode_given(m, giver, to, c, f, seq), "don a soi-meme : refuse");
    // Sur le fil : le texte (le numero) survit a la coupe des zeros de fin.
    uint8_t wire[128] = {};
    purse_encode_given(m, 1, 2, 5, 0, 0);
    const uint32_t n = enc_mm(wire, sizeof(wire), m);
    Reader rd(wire + 1, n - 1);
    MmMsg back;
    check(n && dec_mm(rd, back) && purse_decode_given(back, giver, to, c, f, seq) && seq == 0 && giver == 1,
          "don fait, numero 0, sur le fil");

    purse_encode_give_no(m, 1, kPurseNoEnough, 9);
    check(purse_decode_give_no(m, giver, why, seq) && giver == 1 && why == kPurseNoEnough && seq == 9, "refus relu");
    purse_encode_give_no(m, 1, 0, 9);
    check(!purse_decode_give_no(m, giver, why, seq), "refus sans raison");
    purse_encode_give_no(m, 1, kPurseNoLast + 1, 9);
    check(!purse_decode_give_no(m, giver, why, seq), "refus a raison inconnue");
    // Les anciens messages ne passent pas pour des dons.
    MmMsg ledger;
    PurseLine line;
    line.peer = 1;
    purse_encode_lines(ledger, kPurseLedger, 1, &line, 1, 0, 0);
    check(!purse_decode_given(ledger, giver, to, c, f, seq) && !purse_decode_give_req(ledger, to, c, f, seq),
          "les bourses ne sont pas un don");
}

void test_text() {
    check(gift_list_text(20, 5, { u8"Casse-croûte" }) == u8"20 pièces, 5 nourritures et Casse-croûte", "liste");
    check(gift_list_text(1, 0, {}) == u8"1 pièce", "une piece");
    check(gift_list_text(0, 1, { "A", "B" }) == u8"1 nourriture, A et B", "nourriture et objets");
    check(gift_list_text(0, 0, {}) == "rien", "rien");

    check(gift_why_text(kGiftWhyNotHome, "Ami") == u8"Ami n'est pas dans sa maison", "raison : maison");
    check(gift_why_text(kGiftWhyFailed, "Ami") == u8"le jeu d'Ami n'a pas pu les ranger", "raison : elision");
    check(gift_why_text(kGiftWhyFailed, "Bob") == u8"le jeu de Bob n'a pas pu les ranger", "raison : sans elision");
    check(gift_why_text(kGiftWhyGone, "Bob") == u8"Bob n'est plus dans la partie", "raison : parti");
    check(gift_received_text(u8"Élise", u8"3 pièces", true) ==
              u8"Cadeau d'Élise\n\nÉlise t'a donné 3 pièces.\nC'est dans ta maison, et ta save est à jour.",
          "fenetre de celui qui recoit");
    check(gift_received_text("[Bob]", u8"3 pièces", false) ==
              u8"Cadeau de (Bob)\n\n(Bob) t'a donné 3 pièces.\nC'est dans ta maison.",
          "fenetre de celui qui recoit : nom sans balise");
    check(gift_done_text("Ami", u8"3 pièces", 0, false) == u8"Ami a bien reçu 3 pièces : c'est parti de ta maison.",
          "le donneur : fait");
    check(gift_done_text("Ami", "Gem", 1, true).find(u8"Finalement, Ami a bien reçu Gem") == 0 &&
              gift_done_text("Ami", "Gem", 1, true).find(u8"1 objet n'a pas pu") != std::string::npos,
          "le donneur : en retard, un objet reste");
    check(gift_refused_text("Ami", kGiftWhyNotHome) ==
              u8"Le don à Ami n'est pas passé : Ami n'est pas dans sa maison. Rien n'a quitté ta maison.",
          "le donneur : refuse");

    check(purse_given_line(false, true, "Ami", "", 20, 0) == u8"Ami t'a donné 20 pièces : c'est dans ta bourse.",
          "bourse : recu");
    check(purse_given_line(true, false, "", "Bob", 0, 2) == u8"Tu as donné 2 nourritures à Bob : c'est dans sa bourse.",
          "bourse : donne");
    check(purse_given_line(false, false, "Ami", "Bob", 1, 1) == u8"Ami a donné 1 pièce et 1 nourriture à Bob.",
          "bourse : les autres");
    check(purse_give_no_text(kPurseNoEnough) == u8"ta bourse n'en a pas autant", "bourse : refus");
}

} // namespace

int main() {
    test_codec();
    test_storage();
    test_receiver();
    test_purse_transfer();
    test_purse_messages();
    test_text();
    std::printf("%s (%d verifications, %d echec(s))\n", g_fail ? "ECHEC" : "ok", g_ok + g_fail, g_fail);
    return g_fail ? 1 : 0;
}
