// test_purse -- Mewgenics Multi : les bourses de l'aventure, partie portable
// (src/session/mm_purse_core.cpp) :
//
//   - ce qu'un evenement donne ou coute a un chat va a son proprietaire ;
//   - le reste est partage a parts egales entre les joueurs qui ont des chats,
//     les unites qui restent de la division a la bourse la plus maigre (gain)
//     ou la plus garnie (perte), l'hote d'abord a egalite ;
//   - la somme des bourses reste le compte de l'aventure ;
//   - la fin : chaque invite paye recoit sa bourse positive, dans la limite de
//     ce que l'aventure contient, l'hote garde le reste ;
//   - le message MM_PURSE (lignes, trouvaille) et ses refus ;
//   - le texte des quantites en francais.

#include "mm_purse_core.h"
#include "mgmp_proto.h"

#include <cstdio>
#include <cstring>
#include <string>

using namespace mgmp;

namespace {

int g_fail = 0;

void check(bool ok, const char* what) {
    if (!ok) { std::printf("FAIL: %s\n", what); ++g_fail; }
}

void test_book() {
    PurseBook b;
    b.set_players((1u << 0) | (1u << 1), 0);
    b.reset();
    check(b.players() == 1u, "reset : l'hote seul");
    b.set_players((1u << 0) | (1u << 1), 0);
    check(b.players() == 3u, "deux joueurs");

    // Les provisions du depart (8 pieces, 25 de nourriture) : partagees.
    b.share(8, 25);
    check(b.at(0).coins == 4 && b.at(1).coins == 4, "8 pieces : 4 et 4");
    check(b.at(0).food == 13 && b.at(1).food == 12, "25 de nourriture : 13 (hote, le reste) et 12");

    // Un chat de l'invite trouve 15 pieces : a lui seul.
    b.credit(1, 15, 0);
    check(b.at(1).coins == 19 && b.at(0).coins == 4, "trouvaille de l'invite");
    // Un chat de l'hote trouve 3 de nourriture.
    b.credit(0, 0, 3);
    check(b.at(0).food == 16, "trouvaille de l'hote");

    // La boutique : 7 pieces depensees, partagees (-3 chacun, et la derniere a
    // la bourse la plus garnie, celle de l'invite).
    b.share(-7, 0);
    check(b.at(0).coins == 1 && b.at(1).coins == 15, "depense partagee : -3 hote, -4 invite");
    check(b.total_coins() == 16 && b.total_food() == 28, "somme = compte de l'aventure");

    // Un joueur hors des joueurs : rien ; numero invalide : a l'hote.
    b.credit(9, 2, 0);
    check(b.at(0).coins == 3, "numero invalide : a l'hote");

    // La fin : l'invite recoit 15, l'hote garde le reste.
    PursePayout p = b.settle(1u << 1, b.total_coins(), b.total_food());
    check(p.coins[1] == 15 && p.food[1] == 12, "l'invite rapporte sa bourse");
    check(p.host_coins == 3 && p.host_food == 16, "l'hote garde le reste");
    check(p.coins[0] == 0, "l'hote n'est jamais dans les lignes");

    // Un invite non paye (parti) : sa bourse reste a l'hote.
    p = b.settle(0, b.total_coins(), b.total_food());
    check(p.coins[1] == 0 && p.host_coins == 18 && p.host_food == 28, "invite parti : tout a l'hote");

    // L'aventure contient moins que la bourse de l'invite (hote en negatif).
    PurseBook c;
    c.set_players(3u, 0);
    c.credit(1, 30, 5);
    c.share(-20, 0);     // l'hote -10, l'invite -10 : 20 pour l'invite, -10 pour l'hote
    check(c.at(1).coins == 20 && c.at(0).coins == -10, "hote en negatif");
    p = c.settle(1u << 1, c.total_coins(), c.total_food());
    check(p.coins[1] == 10 && p.host_coins == 0, "plafonne a ce que contient l'aventure");
    check(p.food[1] == 5 && p.host_food == 0, "nourriture");

    // Bourse negative : rien, et l'hote garde tout.
    PurseBook d;
    d.set_players(3u, 0);
    d.share(10, 0);
    d.credit(1, -8, 0);
    p = d.settle(1u << 1, d.total_coins(), d.total_food());
    check(p.coins[1] == 0 && p.host_coins == 2, "bourse negative : zero, l'hote garde ce qui reste");

    // Trois joueurs, l'hote numero 0 ; un gain de 10 : 3, 3, 3 et 1 de reste
    // (egalite : l'hote) ; une perte de 5 : -1 chacun, puis -1 a l'hote et -1 au
    // joueur 1 (egalite, dans l'ordre).
    PurseBook e;
    e.set_players((1u << 0) | (1u << 1) | (1u << 3), 0);
    e.share(10, -5);
    check(e.at(0).coins == 4 && e.at(1).coins == 3 && e.at(3).coins == 3 && e.at(2).coins == 0, "trois joueurs");
    check(e.at(0).food == -2 && e.at(1).food == -2 && e.at(3).food == -1, "perte partagee");
    check(e.total_coins() == 10 && e.total_food() == -5, "somme exacte");

    // Les restes ne vont pas toujours au meme : deux combats de 5 nourritures,
    // 3 + 2 puis 2 + 3.
    PurseBook f;
    f.set_players(3u, 0);
    f.share(0, 5);
    check(f.at(0).food == 3 && f.at(1).food == 2, "premier combat : le reste a l'hote (egalite)");
    f.share(0, 5);
    check(f.at(0).food == 5 && f.at(1).food == 5, "second combat : le reste a l'invite (le plus maigre)");
    // Une depense impaire : la derniere unite a la bourse la plus garnie.
    f.credit(1, 0, 4);
    f.share(0, -3);
    check(f.at(0).food == 4 && f.at(1).food == 7, "depense : -1 hote, -2 invite (plus garni)");

    // Un seul joueur : tout a l'hote.
    PurseBook h;
    h.set_players(1u, 0);
    h.share(7, -3);
    check(h.at(0).coins == 7 && h.at(0).food == -3, "seul : tout a l'hote");
    // L'hote n'est pas le numero 0.
    PurseBook k;
    k.set_players((1u << 2) | (1u << 5), 5);
    check(k.players() == ((1u << 2) | (1u << 5)) && k.host() == 5, "hote numero 5");
    k.share(1, 0);
    check(k.at(5).coins == 1 && k.at(2).coins == 0, "egalite : l'hote d'abord");
    k.share(1, 0);
    check(k.at(5).coins == 1 && k.at(2).coins == 1, "puis le plus maigre");

    // L'hote sans chat dans l'equipe : il ne partage pas, mais recoit ce qui va
    // a un numero hors limites, et le reste a la fin.
    PurseBook n;
    n.set_players(1u << 1, 0);
    check(n.players() == (1u << 1), "l'hote ne partage pas");
    n.share(10, 5);
    check(n.at(1).coins == 10 && n.at(1).food == 5 && n.at(0).coins == 0, "tout a l'invite");
    n.credit(9, 3, 0);
    check(n.at(0).coins == 3, "numero hors limites : a l'hote");
    p = n.settle(1u << 1, n.total_coins(), n.total_food());
    check(p.coins[1] == 10 && p.food[1] == 5 && p.host_coins == 3 && p.host_food == 0, "fin : l'hote garde le sien");
    // Aucun joueur : l'hote seul.
    PurseBook z;
    z.set_players(0, 0);
    check(z.players() == 1u, "aucun : l'hote");
}

void test_message() {
    PurseLine in[3];
    in[0].peer = 0; in[0].coins = 12;  in[0].food = 3;
    in[1].peer = 1; in[1].coins = -4;  in[1].food = 25;
    in[2].peer = 3; in[2].coins = 70000; in[2].food = 0;
    MmMsg m;
    purse_encode_lines(m, kPurseLedger, 7, in, 3, 78, 28);
    check(m.kind == MM_PURSE && m.a == kPurseLedger && m.b == 7 && m.c == 3, "en-tete des lignes");
    check((int32_t)m.x == 78 && (int32_t)m.y == 28, "compte de l'aventure");

    // Sur le fil, puis relu.
    uint8_t wire[256];
    const uint32_t n = enc_mm(wire, sizeof(wire), m);
    check(n > 0, "encode sur le fil");
    Reader r(wire, n);
    MmMsg back;
    check(r.u8v() == MSG_MM && dec_mm(r, back), "decode du fil");
    PurseLine out[kPurseMaxLines];
    check(purse_decode_lines(back, out, kPurseMaxLines) == 3, "trois lignes");
    check(out[0].peer == 0 && out[0].coins == 12 && out[0].food == 3, "ligne 1");
    check(out[1].peer == 1 && out[1].coins == -4 && out[1].food == 25, "ligne 2 (negatif)");
    check(out[2].peer == 3 && out[2].coins == 70000 && out[2].food == 0, "ligne 3 (zeros a la fin)");

    // Refus : un numero de joueur hors limites, trop de lignes, un autre genre.
    MmMsg bad = back;
    ((uint8_t*)bad.text)[9] = 9;
    check(purse_decode_lines(bad, out, kPurseMaxLines) == 0, "joueur hors limites refuse");
    bad = back; bad.c = 8;
    check(purse_decode_lines(bad, out, kPurseMaxLines) == 0, "trop de lignes refuse");
    bad = back; bad.a = kPurseFind;
    check(purse_decode_lines(bad, out, kPurseMaxLines) == 0, "pas des lignes");
    check(purse_decode_lines(back, out, 2) == 0, "plus de lignes que la place");

    // La fin : meme format.
    purse_encode_lines(m, kPursePayout, 7, in + 1, 2, 5, 6);
    check(purse_decode_lines(m, out, kPurseMaxLines) == 2 && out[0].peer == 1, "lignes de la fin");

    // Une trouvaille, avec un nom accentue trop long coupe proprement.
    std::string name = u8"Frédéric";
    MmMsg f;
    purse_encode_find(f, 1, 15, 2, name);
    uint8_t owner = 0; int32_t c = 0, fo = 0; std::string got;
    check(purse_decode_find(f, owner, c, fo, got), "trouvaille relue");
    check(owner == 1 && c == 15 && fo == 2 && got == name, "trouvaille : joueur, quantites, nom");
    std::string longname;
    for (int i = 0; i < 40; ++i) longname += u8"é";      // 80 octets
    purse_encode_find(f, 1, 1, 0, longname);
    check(purse_decode_find(f, owner, c, fo, got) && got.size() <= 63 && got.size() % 2 == 0,
          "nom coupe entre deux caracteres");
    // Le prix d'un choix : le drapeau passe.
    uint8_t flags = 0;
    purse_encode_find(f, 1, -10, 0, "Minou", kFindSpent);
    check(purse_decode_find(f, owner, c, fo, got, &flags) && flags == kFindSpent && c == -10, "drapeau « dépensé »");
    purse_encode_find(f, 1, 3, 0, "Minou");
    check(purse_decode_find(f, owner, c, fo, got, &flags) && flags == 0, "sans drapeau");
    f.b = 8;
    check(!purse_decode_find(f, owner, c, fo, got), "joueur hors limites refuse");
}

void test_text() {
    check(purse_amount_text(12, 3) == u8"12 pièces et 3 nourritures", "pieces et nourriture");
    check(purse_amount_text(1, 0) == u8"1 pièce", "une piece");
    check(purse_amount_text(0, 5) == u8"5 nourritures", "nourriture seule");
    check(purse_amount_text(0, 1) == u8"1 nourriture", "une nourriture");
    check(purse_amount_text(0, 0) == "rien", "rien");
    check(purse_amount_text(-3, -1) == "rien", "negatif : rien");

    check(purse_balance_text(-11, 13) == u8"-11 pièces et 13 nourritures", "dette en pieces");
    check(purse_balance_text(-1, 0) == u8"-1 pièce", "une piece de dette");
    check(purse_balance_text(3, -2) == u8"3 pièces et -2 nourritures", "dette en nourriture");
    check(purse_balance_text(0, 0) == "rien", "bourse vide");
    check(purse_balance_text(12, 3) == purse_amount_text(12, 3), "positive : le meme texte");

    check(purse_of("Bob") == "de Bob", "de Bob");
    check(purse_of("Ami") == "d'Ami", "d'Ami");
    check(purse_of("hugo") == "d'hugo", "d'hugo");
    check(purse_of(u8"Élise") == u8"d'Élise", "d'Elise (accent majuscule)");
    check(purse_of(u8"éric") == u8"d'éric", "d'eric (accent minuscule)");
    check(purse_of(u8"Çelik") == u8"de Çelik", "C cedille : pas d'elision");
    check(purse_of(u8"Ñandu") == u8"de Ñandu", "N tilde : pas d'elision");
    check(purse_of("xX_Gamer") == "de xX_Gamer", "consonne");
    check(purse_of("") == "de ?", "nom vide");
    check(purse_of("Yves") == "d'Yves", "y devant une consonne : elision");
    check(purse_of("Yannick") == "de Yannick", "y devant une voyelle : pas d'elision");
    check(purse_of("yo") == "de yo", "y devant une voyelle (minuscule)");
    check(purse_of("[c:red]Bob") == "de (c:red)Bob", "pas de balise dans un nom");
    check(purse_safe("A[b]\nc") == "A(b) c", "nom nettoye");

    check(purse_battle_line(12, 0, true, "") == u8"En combat, tes chats ont trouvé 12 pièces : c'est pour ta bourse.",
          "combat, a soi");
    check(purse_battle_line(0, 3, false, "Ami") ==
          u8"En combat, les chats d'Ami ont trouvé 3 nourritures : c'est pour sa bourse.", "combat, a un autre");
    check(purse_battle_line(-4, 0, false, "Bob") ==
          u8"En combat, les chats de Bob ont perdu 4 pièces : c'est pris sur sa bourse.", "combat, perte");

    check(purse_find_line(u8"Frédéric", 15, 0, true, "Bob") == u8"Frédéric a trouvé 15 pièces : c'est pour ta bourse.",
          "trouvaille a soi");
    check(purse_find_line(u8"Frédéric", 0, 3, false, "Ami") ==
          u8"Frédéric a trouvé 3 nourritures : c'est pour la bourse d'Ami.", "trouvaille d'un autre");
    check(purse_find_line("Minou", -5, 0, false, "Bob") == u8"Minou a perdu 5 pièces : c'est pris sur la bourse de Bob.",
          "perte d'un autre");
    check(purse_find_line("Minou", 3, -2, true, "") ==
          u8"Minou a trouvé 3 pièces et perdu 2 nourritures : ça compte pour ta bourse.", "gain et perte");
    check(purse_find_line("", 1, 0, true, "") == u8"Un chat a trouvé 1 pièce : c'est pour ta bourse.", "sans nom");
    check(purse_find_line("Mi[nou]", 1, 0, false, "B[o]b") ==
          u8"Mi(nou) a trouvé 1 pièce : c'est pour la bourse de B(o)b.", "noms nettoyes");
    check(purse_find_line("Minou", -10, 0, true, "", true) == u8"Minou a dépensé 10 pièces : c'est pris sur ta bourse.",
          "prix d'un choix");
    check(purse_find_line("Minou", 5, 0, true, "", true) == u8"Minou a trouvé 5 pièces : c'est pour ta bourse.",
          "« dépensé » ne change pas un gain");
}

} // namespace

int main() {
    test_book();
    test_message();
    test_text();
    if (g_fail) { std::printf("%d echec(s)\n", g_fail); return 1; }
    std::printf("ok\n");
    return 0;
}
