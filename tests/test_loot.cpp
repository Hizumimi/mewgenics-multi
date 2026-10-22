// Mewgenics Multi : le defile du butin (mm_loot_core) -- les regles, le message
// et le deroulement chez l'hote.
#include "mm_loot_core.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

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

static int g_fail = 0, g_checks = 0;
#define CHECK(c) do { ++g_checks; if (!(c)) { ++g_fail; std::printf("  ECHEC ligne %d : %s\n", __LINE__, #c); } } while (0)

using namespace mgmp;

static ItemFields item(const char* name, const char* text = "", uint32_t a = 0xFFFFFFFFu, uint32_t d = 1000) {
    ItemFields f;
    f.name = name; f.text = text; f.a = a; f.b = 3; f.c = 0x20; f.d = d; f.e = 255; f.f = 1;
    return f;
}

static std::string nm(uint8_t p) { return p == 0 ? "Hote" : p == 1 ? "Ami" : "Lou"; }

static void test_message() {
    std::printf("message kTeamLoot\n");
    LootMsg m;
    m.op = kLootStart; m.parade = 77; m.seq = 5; m.players = 0x7;
    m.items = { item("Pipe"), item("tk_Book", "Fireball", 2, 4000001) };
    std::vector<uint8_t> w = loot_encode(m);
    LootMsg back;
    CHECK(loot_decode(w.data(), w.size(), back));
    CHECK(back.op == kLootStart && back.parade == 77 && back.seq == 5 && back.players == 0x7);
    CHECK(back.items.size() == 2 && back.items[1].name == "tk_Book" && back.items[1].text == "Fireball");
    CHECK(back.items[1].a == 2 && back.items[1].d == 4000001 && back.items[1].e == 255 && back.items[1].f == 1);
    CHECK(item_digest(back.items[0]) == item_digest(m.items[0]));
    // Tronque : refuse ; un octet de trop : refuse ; une autre tete : refuse.
    CHECK(!loot_decode(w.data(), w.size() - 1, back));
    w.push_back(0);
    CHECK(!loot_decode(w.data(), w.size(), back));
    w.pop_back();
    std::vector<uint8_t> bad = w;
    bad[5] = kTeamItemOp;
    CHECK(!loot_decode(bad.data(), bad.size(), back));

    LootMsg c; c.op = kLootChoose; c.parade = 77; c.index = 1; c.choice = kLootNeed;
    w = loot_encode(c);
    CHECK(loot_decode(w.data(), w.size(), back) && back.op == kLootChoose && back.index == 1 && back.choice == kLootNeed);
    c.choice = 9;                                             // pas un choix
    w = loot_encode(c);
    CHECK(!loot_decode(w.data(), w.size(), back));

    LootMsg s; s.op = kLootState; s.parade = 77;
    s.state.index = 1; s.state.phase = LootPhase::Result; s.state.ms_left = 3500; s.state.players = 7;
    s.state.ready = 5; s.state.chosen = 0;
    s.state.result.decided = true; s.state.result.winner = 1; s.state.result.category = kLootNeed;
    s.state.result.rerolls = 1;
    s.state.result.rolls = { {0, kLootGreed, 0}, {1, kLootNeed, 87}, {2, kLootNeed, 42} };
    s.state.winners = { 2, 1, kLootUndecided };
    w = loot_encode(s);
    CHECK(loot_decode(w.data(), w.size(), back) && back.op == kLootState);
    CHECK(back.state.phase == LootPhase::Result && back.state.ms_left == 3500 && back.state.players == 7);
    CHECK(back.state.ready == 5 && back.state.result.decided && back.state.result.winner == 1);
    CHECK(back.state.result.rolls.size() == 3 && back.state.result.rolls[1].roll == 87);
    CHECK(back.state.winners.size() == 3 && back.state.winners[0] == 2 && back.state.winners[2] == kLootUndecided);
    CHECK(back.state.parade == 77);
    CHECK(team_kind(w.data(), w.size()) == kTeamLoot);
    // Un de au-dessus de 100, un joueur hors des numeros : refuse.
    s.state.result.rolls[1].roll = 101;
    w = loot_encode(s);
    CHECK(!loot_decode(w.data(), w.size(), back));
    s.state.result.rolls[1].roll = 87;
    s.state.result.rolls[2].peer = 8;
    w = loot_encode(s);
    CHECK(!loot_decode(w.data(), w.size(), back));

    LootMsg g; g.op = kLootGot; g.parade = 77; g.index = 3; g.ok = 1;
    w = loot_encode(g);
    CHECK(loot_decode(w.data(), w.size(), back) && back.op == kLootGot && back.index == 3 && back.ok == 1);
    LootMsg gv; gv.op = kLootGive; gv.parade = 78; gv.index = 2;
    w = loot_encode(gv);
    CHECK(loot_decode(w.data(), w.size(), back) && back.op == kLootGive && back.index == 2 && back.parade == 78);
    LootMsg rd; rd.op = kLootReady; rd.parade = 9; rd.ready = 1;
    w = loot_encode(rd);
    CHECK(loot_decode(w.data(), w.size(), back) && back.op == kLootReady && back.parade == 9 && back.ready == 1);
    rd.ready = 0;
    w = loot_encode(rd);
    CHECK(loot_decode(w.data(), w.size(), back) && back.ready == 0);
    w.back() = 2;                                             // ni 0 ni 1
    CHECK(!loot_decode(w.data(), w.size(), back));
    // Un defile sans objet : le message dit qu'il n'y en a pas.
    LootMsg e; e.op = kLootStart; e.parade = 80; e.seq = 1; e.players = 3;
    w = loot_encode(e);
    CHECK(loot_decode(w.data(), w.size(), back) && back.op == kLootStart && back.items.empty());
    // Une operation inconnue : refusee.
    LootMsg u; u.op = 42; u.parade = 1;
    w = loot_encode(u);
    CHECK(!loot_decode(w.data(), w.size(), back));
}

static void test_resolve() {
    std::printf("le gagnant d'un objet\n");
    LootRng rng(1234);
    for (int k = 0; k < 2000; ++k) {
        const uint8_t d = rng.d100();
        CHECK(d >= 1 && d <= 100);
    }
    // Besoin l'emporte sur Cupidite, quel que soit le de.
    for (int round = 0; round < 200; ++round) {
        uint8_t ch[kLootSlots] = { kLootGreed, kLootNeed, kLootPass };
        LootResult r = loot_resolve(ch, 0x7, rng);
        CHECK(r.decided && r.winner == 1 && r.category == kLootNeed);
        CHECK(r.rolls.size() == 3 && r.rolls[0].roll == 0 && r.rolls[1].roll >= 1 && r.rolls[2].roll == 0);
    }
    // Sans Besoin : entre les Cupidite ; le plus haut de.
    for (int round = 0; round < 200; ++round) {
        uint8_t ch[kLootSlots] = { kLootGreed, kLootPass, kLootGreed };
        LootResult r = loot_resolve(ch, 0x7, rng);
        CHECK(r.category == kLootGreed && (r.winner == 0 || r.winner == 2));
        const LootRoll& w = r.rolls[r.winner == 0 ? 0 : 2];
        const LootRoll& o = r.rolls[r.winner == 0 ? 2 : 0];
        CHECK(w.roll >= o.roll);                // le premier lancer, montre
        CHECK((w.roll == o.roll) == (r.rerolls > 0));
        CHECK(r.rolls[1].roll == 0);            // il a passe : pas de de
    }
    // Personne : il reste a l'hote ; pas de choix = passe ; joueur hors defile ignore.
    {
        uint8_t ch[kLootSlots] = { kLootPass, kLootNone, kLootPass, kLootNeed };
        LootResult r = loot_resolve(ch, 0x7, rng);
        CHECK(r.decided && r.winner == kLootNobody && r.category == kLootNone && r.rolls.size() == 3);
        CHECK(r.rolls[1].choice == kLootPass);
    }
    // Egalites en tete relancees entre les ex aequo seuls ; le gagnant a toujours
    // le plus haut premier lancer (jamais un perdant au-dessus de lui).
    {
        int ties = 0, lower_ties = 0;
        for (uint64_t seed = 1; seed < 20000; ++seed) {
            LootRng r2(seed);
            uint8_t ch[kLootSlots] = { kLootNeed, kLootNeed, kLootNeed };
            LootResult r = loot_resolve(ch, 0x7, r2);
            CHECK(r.winner <= 2);
            uint8_t best = 0; int at = 0;
            for (const LootRoll& x : r.rolls) {
                CHECK(x.roll >= 1 && x.roll <= 100);
                if (x.roll > best) { best = x.roll; at = 1; } else if (x.roll == best) ++at;
            }
            CHECK(r.rolls[r.winner].roll == best);
            CHECK((at > 1) == (r.rerolls > 0));
            ties += r.rerolls > 0;
            // Une egalite plus bas (pas en tete) ne relance rien.
            const uint8_t a = r.rolls[0].roll, b = r.rolls[1].roll, c = r.rolls[2].roll;
            if (at == 1 && (a == b || b == c || a == c)) { ++lower_ties; CHECK(r.rerolls == 0); }
        }
        CHECK(ties > 0 && lower_ties > 0);
    }
}

static void test_host() {
    std::printf("le defile chez l'hote\n");
    LootRng rng(99);
    LootHost h;
    uint64_t t = 1000000;
    h.start(5, 3, { item("Pipe"), item("Horns"), item("ShoeHorn") }, 0x6, 0, t);   // l'hote ajoute
    CHECK(h.running() && h.players() == 0x7 && h.state(t).phase == LootPhase::Waiting);
    CHECK(!h.tick(t + 10, rng));                       // l'hote n'est pas pret
    h.set_ready(0, true, t + 100);
    CHECK(!h.tick(t + 200, rng));                      // on attend les invites
    h.set_ready(1, true, t + 300);
    CHECK(!h.tick(t + 400, rng));
    // 3 minutes apres l'hote, on commence sans Lou.
    CHECK(h.tick(t + 100 + kLootReadyWaitMs, rng));
    t += 100 + kLootReadyWaitMs;
    LootState s = h.state(t);
    CHECK(s.phase == LootPhase::Choosing && s.index == 0 && s.ms_left == kLootChoiceMs);
    // Lou n'est pas pret : seuls l'hote et Ami sont attendus.
    CHECK(h.choose(1, 0, kLootNeed));
    CHECK(!h.choose(1, 0, kLootGreed));                // deja choisi
    CHECK(!h.choose(1, 1, kLootGreed));                // pas l'objet en cours
    CHECK(!h.choose(4, 0, kLootGreed));                // pas du defile
    CHECK(!h.tick(t + 10, rng));
    CHECK(h.state(t + 10).chosen == 0x2);
    CHECK(h.choose(0, 0, kLootGreed));
    CHECK(h.tick(t + 20, rng));                        // tous les prets ont choisi
    s = h.state(t + 20);
    CHECK(s.phase == LootPhase::Result && s.result.winner == 1 && s.result.category == kLootNeed);
    CHECK(!h.tick(t + 20 + kLootResultMs - 1, rng));
    CHECK(h.tick(t + 20 + kLootResultMs, rng));        // objet suivant
    t += 20 + kLootResultMs;
    s = h.state(t);
    CHECK(s.phase == LootPhase::Choosing && s.index == 1 && s.chosen == 0);
    // Lou devient pret en cours d'objet, et choisit.
    h.set_ready(2, true, t + 5);
    CHECK(h.choose(2, 1, kLootGreed));
    // 30 s sans choix de l'hote ni d'Ami : ils passent.
    CHECK(!h.tick(t + kLootChoiceMs - 1, rng));
    CHECK(h.tick(t + kLootChoiceMs, rng));
    s = h.state(t + kLootChoiceMs);
    CHECK(s.result.winner == 2 && s.result.category == kLootGreed);
    t += kLootChoiceMs;
    CHECK(h.tick(t + kLootResultMs, rng));
    t += kLootResultMs;
    // L'hote ne voit plus le defile : le temps s'arrete.
    CHECK(h.state(t).index == 2);
    h.set_ready(0, false, t);
    CHECK(!h.tick(t + 60000, rng));
    CHECK(h.state(t + 60000).phase == LootPhase::Choosing);
    h.set_ready(0, true, t + 60000);
    CHECK(!h.tick(t + 60000 + 1000, rng));             // il reste du temps
    // Ami quitte la session : plus attendu ; personne ne veut l'objet.
    h.gone(1);
    CHECK(h.choose(0, 2, kLootPass) && h.choose(2, 2, kLootPass));
    CHECK(h.tick(t + 60000 + 2000, rng));
    s = h.state(t + 60000 + 2000);
    CHECK(s.result.winner == kLootNobody && s.players == 0x5);
    CHECK(h.tick(t + 60000 + 2000 + kLootResultMs, rng));
    CHECK(h.finished() && !h.running());
    CHECK(h.results().size() == 3 && h.results()[0].winner == 1 && h.results()[1].winner == 2 &&
          h.results()[2].winner == kLootNobody);
    // Tous prets d'un coup : la fenetre d'attente reste 2,5 s.
    LootHost w;
    w.start(9, 1, { item("Pipe") }, 0x2, 0, t);
    w.set_ready(0, true, t + 10);
    w.set_ready(1, true, t + 20);
    CHECK(!w.tick(t + 10 + kLootIntroMs - 1, rng) && w.state(t + 10 + kLootIntroMs - 1).phase == LootPhase::Waiting);
    CHECK(w.tick(t + 10 + kLootIntroMs, rng) && w.state(t + 10 + kLootIntroMs).phase == LootPhase::Choosing);
    // Rien : pas de defile.
    LootHost e;
    e.start(6, 1, {}, 0x2, 0, t);
    CHECK(!e.running() && !e.finished());
    // L'hote commence sans attendre : le premier objet tout de suite.
    LootHost f;
    f.start(7, 1, { item("Pipe") }, 0x2, 0, t);
    CHECK(!f.tick(t + 10, rng) && f.state(t + 10).phase == LootPhase::Waiting);
    CHECK(f.start_now(t + 20) && f.state(t + 20).phase == LootPhase::Choosing);
    CHECK(f.state(t + 20).ms_left == kLootChoiceMs && (f.ready() & 1));
    CHECK(!f.start_now(t + 30));                       // plus l'attente
    // Ami n'est pas pret : seul l'hote est attendu ; il passe, personne.
    CHECK(f.choose(0, 0, kLootPass) && f.tick(t + 40, rng));
    CHECK(f.state(t + 40).result.winner == kLootNobody);
    CHECK(f.state(t + 40).winners.size() == 1 && f.state(t + 40).winners[0] == kLootNobody);
    // Un invite pas pret peut quand meme choisir (il devient pret).
    LootHost g2;
    g2.start(8, 1, { item("Pipe"), item("Horns") }, 0x2, 0, t);
    g2.start_now(t);
    CHECK(g2.state(t).winners.size() == 2 && g2.state(t).winners[0] == kLootUndecided);
    CHECK(g2.choose(1, 0, kLootGreed) && (g2.ready() & 2));
    CHECK(!g2.tick(t + 5, rng));                       // l'hote n'a pas choisi
    CHECK(g2.choose(0, 0, kLootPass) && g2.tick(t + 6, rng));
    CHECK(g2.state(t + 6).result.winner == 1 && g2.state(t + 6).result.category == kLootGreed);
}

static void test_texts() {
    std::printf("les textes\n");
    CHECK(std::string(loot_choice_word(kLootNeed)) == u8"Besoin");
    CHECK(std::string(loot_choice_word(kLootGreed)) == u8"Cupidité");
    CHECK(std::string(loot_choice_word(kLootPass)) == u8"Passer");
    LootResult r;
    r.rolls = { {0, kLootPass, 0}, {1, kLootNeed, 42}, {2, kLootNeed, 87} };
    CHECK(loot_rolls_text(r, nm) == "Lou 87, Ami 42");
}

static std::string nm2(uint8_t p) { return p == 0 ? "Hote" : p == 1 ? "Ami" : p == 2 ? "Lou" : "Max"; }

static LootView view(uint8_t self, LootPhase ph) {
    LootView v;
    v.self = self; v.host = 0; v.is_host = self == 0;
    v.st.phase = ph; v.st.players = 0x7; v.st.ready = 0x7; v.st.ms_left = 25000; v.st.index = 1;
    v.item_names = { "Pipe", "Corne", "Bottes" };
    v.item_descs = { "", "+1 CHA", "" };
    v.st.winners = { kLootUndecided, kLootUndecided, kLootUndecided };
    v.name = nm2;
    return v;
}

static bool has(const std::string& s, const char* part) {
    if (s.find(part) != std::string::npos) return true;
    std::printf("  texte sans [%s] :\n%s\n", part, s.c_str());
    return false;
}

static void test_screen() {
    std::printf("les fenetres\n");
    // Pas du defile, ou pas d'objet : pas de fenetre.
    LootView v = view(3, LootPhase::Choosing);
    CHECK(loot_screen(v).layout == kLayNone);
    v = view(1, LootPhase::Choosing);
    v.item_names.clear();
    CHECK(loot_screen(v).layout == kLayNone);
    v = view(1, LootPhase::Idle);
    CHECK(loot_screen(v).layout == kLayNone);
    // L'attente : l'hote peut commencer, l'invite attend.
    v = view(0, LootPhase::Waiting);
    v.st.ready = 0x3; v.st.ms_left = 160000;
    LootScreen s = loot_screen(v);
    CHECK(s.layout == kLayWait && std::string(s.yes) == "Commencer" && !s.no);
    CHECK(has(s.text, u8"3 objets du sac à partager entre toi, Ami et Lou."));
    CHECK(has(s.text, u8"En attente de Lou... (2:40)"));
    v = view(1, LootPhase::Waiting);
    v.st.ready = 0x2;                                   // l'hote ne voit pas le defile
    s = loot_screen(v);
    CHECK(std::string(s.yes) == u8"D'accord" && has(s.text, u8"En attente de Hote..."));
    v.st.ready = 0x1;                                   // l'hote est la ; lui pas encore dit, Lou absent
    s = loot_screen(v);
    CHECK(has(s.text, u8"En attente de Lou...") && s.text.find("Ami") == std::string::npos);
    v.st.ready = 0x5;
    s = loot_screen(v);
    CHECK(has(s.text, u8"Tout le monde est là"));
    // Choisir, en deux temps.
    v = view(1, LootPhase::Choosing);
    s = loot_screen(v);
    CHECK(s.layout == kLayChoose && std::string(s.yes) == "Je le veux" && std::string(s.no) == "Passer");
    CHECK(has(s.text, u8"Partage du butin : objet 2 sur 3") && has(s.text, "Corne\n+1 CHA") &&
          has(s.text, "(25 s)"));
    v.step2 = true;
    s = loot_screen(v);
    CHECK(s.layout == kLayNeedGreed && std::string(s.yes) == "Besoin" && std::string(s.no) == u8"Cupidité");
    // Choisi : on attend les joueurs prets ; un absent passe.
    v.step2 = false; v.my_choice = kLootNeed; v.st.ready = 0x3; v.st.chosen = 0;
    s = loot_screen(v);
    CHECK(s.layout == kLayChosen && has(s.text, u8"Ton choix : Besoin."));
    CHECK(has(s.text, u8"En attente de Hote (25 s)."));
    CHECK(has(s.text, u8"Lou n'est pas là : son tour passe."));
    v.my_choice = kLootNone; v.st.chosen = 0x2; v.st.ready = 0x7;
    s = loot_screen(v);
    CHECK(s.layout == kLayChosen && has(s.text, u8"Ton choix est fait.") && has(s.text, "Hote et Lou"));
    v.st.chosen = 0x7;
    s = loot_screen(v);
    CHECK(has(s.text, u8"Le résultat arrive"));
    v.st.chosen = 0x2; v.st.ready = 0x6;                // l'hote ne voit plus le defile
    s = loot_screen(v);
    CHECK(has(s.text, u8"En attente de Hote..."));
    // Le resultat.
    v = view(2, LootPhase::Result);
    v.st.result.decided = true; v.st.result.winner = 1; v.st.result.category = kLootNeed; v.st.result.rerolls = 1;
    v.st.result.rolls = { {0, kLootPass, 0}, {1, kLootNeed, 87}, {2, kLootNeed, 87} };
    s = loot_screen(v);
    CHECK(s.layout == kLayResult && has(s.text, u8"Ami l'emporte (Besoin)."));
    CHECK(has(s.text, u8"Dés : Ami 87, toi 87 (égalité départagée)."));
    CHECK(has(s.text, u8"Besoin : Ami et toi ; Passer : Hote."));
    v.self = 1;
    s = loot_screen(v);
    CHECK(has(s.text, u8"Tu l'emportes (Besoin) !"));
    v.st.result.winner = kLootNobody; v.st.result.category = kLootNone; v.st.result.rerolls = 0;
    v.st.result.rolls = { {0, kLootPass, 0}, {1, kLootPass, 0}, {2, kLootPass, 0} };
    s = loot_screen(v);
    CHECK(has(s.text, u8"Personne n'en veut : il reste chez Hote.") && s.text.find(u8"Dés") == std::string::npos);
    v.self = 0; v.is_host = true;
    s = loot_screen(v);
    CHECK(has(s.text, u8"il reste chez toi."));
    // La fin.
    v = view(1, LootPhase::Done);
    v.st.winners = { 1, 2, kLootNobody };
    s = loot_screen(v);
    CHECK(s.layout == kLaySummary && has(s.text, "Tu remportes : Pipe.") && has(s.text, "Lou remporte : Corne."));
    CHECK(has(s.text, u8"Personne n'en a voulu (restent chez Hote) : Bottes."));
    CHECK(has(s.text, u8"Ton butin t'attend dans ton rangement"));
    v.at_home = true;
    s = loot_screen(v);
    CHECK(has(s.text, u8"la fenêtre suivante") && s.text.find("retourne au menu") == std::string::npos);
    // Gagne mais pas donne (l'hote ne l'avait plus) : dit a part.
    v.at_home = false; v.track_received = true; v.received = 0;
    s = loot_screen(v);
    CHECK(has(s.text, u8"Remporté, mais resté chez Hote : Pipe.") && s.text.find("Tu remportes") == std::string::npos);
    CHECK(s.text.find(u8"Ton butin t'attend") == std::string::npos);
    v.received = 1;                                     // l'objet 0 (Pipe) donne
    s = loot_screen(v);
    CHECK(has(s.text, "Tu remportes : Pipe.") && s.text.find(u8"Remporté, mais") == std::string::npos);
    v = view(0, LootPhase::Done);
    v.st.winners = { 1, 0, kLootNobody };
    s = loot_screen(v);
    CHECK(has(s.text, "Tu remportes : Corne.") && has(s.text, "Ami remporte : Pipe."));
    CHECK(has(s.text, "(restent chez toi) : Bottes.") && has(s.text, "quitte ton rangement"));
    v.st.winners = { kLootUndecided, kLootUndecided, kLootUndecided };
    s = loot_screen(v);
    CHECK(has(s.text, u8"Aucun objet n'a été partagé."));
    // Le temps, les listes.
    CHECK(loot_time_text(25000) == "25 s" && loot_time_text(24001) == "25 s" && loot_time_text(0) == "0 s");
    CHECK(loot_time_text(160000) == "2:40" && loot_time_text(60000) == "1:00");
    CHECK(loot_join({}).empty() && loot_join({ "A" }) == "A" && loot_join({ "A", "B" }) == "A et B" &&
          loot_join({ "A", "B", "C" }) == "A, B et C");
}

int main() {
    test_message();
    test_resolve();
    test_host();
    test_texts();
    test_screen();
    if (g_fail) { std::printf("%d echec(s) sur %d verifications\n", g_fail, g_checks); return 1; }
    std::printf("ok (%d verifications)\n", g_checks);
    return 0;
}
