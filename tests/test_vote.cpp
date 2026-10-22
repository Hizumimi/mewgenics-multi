// test_vote -- Mewgenics Multi : le vote du chemin, partie portable
// (src/session/mm_vote_core.cpp) :
//
//   - le tour : ouverture au premier vote, votants fixes, changement de vote,
//     resultat a l'unanimite, a la majorite, egalite tiree au sort ;
//   - le compte a rebours de 30 s : arrete tant qu'un votant n'est pas sur la
//     carte, absent ecarte apres 60 s d'affilee (jamais l'hote), retour d'un
//     ecarte, plafond de 3 minutes, pas de temps plafonne ;
//   - la duree du resultat (courte a l'unanimite) ;
//   - les votants qui partent (et leur vote), le tour abandonne sans vote ;
//   - le message MM_VOTE (aller-retour, refus des messages faux) ;
//   - la fenetre de chaque joueur (« Même chemin », « Changer »...) et son texte.

#include "mm_vote_core.h"
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

bool has(const std::string& s, const char* part) { return s.find(part) != std::string::npos; }

// Tirage au sort : rend toujours `pick` (modulo n), et compte les appels.
struct FixedRng { uint32_t pick = 0; int calls = 0; };
uint32_t fixed_rng(void* ctx, uint32_t n) {
    auto* r = (FixedRng*)ctx;
    ++r->calls;
    return n ? r->pick % n : 0;
}

constexpr uint32_t kHost = 1u << 0, kGuest = 1u << 1, kThird = 1u << 2;

// Avance le temps par pas de 100 ms, tous presents selon `present`.
bool run(VoteHost& h, uint64_t& now, uint32_t ms, uint32_t present, FixedRng& rng) {
    bool changed = false;
    for (uint32_t t = 0; t < ms; t += 100) {
        now += 100;
        h.set_present(present);
        if (h.tick(now, fixed_rng, &rng)) changed = true;
    }
    return changed;
}

void test_unanimous() {
    VoteHost h; h.set_never_drop(kHost);
    FixedRng rng;
    uint64_t now = 1000;
    check(h.state().phase == VotePhase::Idle, "au depart, pas de tour");
    check(h.cast(0, 7, 12, kHost | kGuest, now), "le premier vote ouvre le tour");
    check(h.state().phase == VotePhase::Voting, "tour ouvert");
    check(h.state().round == 1, "premier tour = 1");
    check(h.state().voters == (kHost | kGuest), "votants fixes a l'ouverture");
    check(h.state().remaining_ms == kVoteCountdownMs, "30 s au compteur");
    run(h, now, 1000, kHost | kGuest, rng);
    check(h.state().phase == VotePhase::Voting, "en attente du second vote");
    check(h.state().remaining_ms == kVoteCountdownMs - 1000, "le compte avance quand tous sont la");
    check(h.cast(1, 7, 12, 0, now), "le vote de l'invite");
    check(run(h, now, 100, kHost | kGuest, rng), "tout le monde a vote : resultat");
    check(h.state().phase == VotePhase::Result, "phase resultat");
    check(h.state().winner.node == 7 && h.state().winner.type == 12, "la case choisie");
    check(!h.state().drawn && rng.calls == 0, "pas de tirage a l'unanimite");
    check(!h.cast(1, 9, 5, 0, now), "plus de vote pendant le resultat");
    h.finish();
    check(h.state().phase == VotePhase::Idle && h.state().round == 1, "fini, le numero reste");
    check(h.cast(1, 3, 5, kHost | kGuest, now), "un invite peut ouvrir un tour");
    check(h.state().round == 2, "second tour = 2");
}

void test_tie_and_change() {
    VoteHost h; h.set_never_drop(kHost);
    FixedRng rng; rng.pick = 1;
    uint64_t now = 0;
    h.cast(0, 4, 5, kHost | kGuest | kThird, now);
    h.cast(1, 9, 12, 0, now);
    check(h.state().phase == VotePhase::Voting, "il manque un votant");
    check(h.cast(1, 4, 5, 0, now), "changer son vote");
    check(h.state().ballot[1].node == 4, "le vote change");
    check(h.cast(1, 9, 12, 0, now), "et le rechanger");
    check(h.cast(2, 2, 6, 0, now), "troisieme vote");
    run(h, now, 100, kHost | kGuest | kThird, rng);
    check(h.state().phase == VotePhase::Result, "tous ont vote");
    check(h.state().drawn, "trois cases a une voix : tirage");
    check(rng.calls == 1, "un seul tirage");
    check(h.state().tied_count == 3, "trois cases a egalite");
    check(h.state().tied[0].node == 2 && h.state().tied[1].node == 4 && h.state().tied[2].node == 9,
          "les cases a egalite, par index");
    check(h.state().winner.node == 4, "le tirage choisit la deuxieme (pick = 1)");

    VoteHost m; m.set_never_drop(kHost);
    FixedRng r2;
    now = 0;
    m.cast(0, 4, 5, kHost | kGuest | kThird, now);
    m.cast(1, 9, 12, 0, now);
    m.cast(2, 9, 12, 0, now);
    run(m, now, 100, kHost | kGuest | kThird, r2);
    check(m.state().winner.node == 9 && !m.state().drawn && r2.calls == 0, "la majorite l'emporte");
}

void test_countdown_and_absence() {
    VoteHost h; h.set_never_drop(kHost);
    FixedRng rng;
    uint64_t now = 0;
    h.cast(0, 5, 5, kHost | kGuest, now);
    // L'invite n'est pas encore sur la carte : le compte ne bouge pas.
    run(h, now, 20000, kHost, rng);
    check(h.state().phase == VotePhase::Voting, "en attente de l'invite");
    check(!h.state().running, "compte arrete");
    check(h.state().remaining_ms == kVoteCountdownMs, "le compte n'a pas bouge");
    // Il arrive : le compte repart.
    run(h, now, 10000, kHost | kGuest, rng);
    check(h.state().running, "compte en cours");
    check(h.state().remaining_ms == kVoteCountdownMs - 10000, "10 s de compte");
    // 20 s de plus, sans son vote : abstention.
    run(h, now, 20000, kHost | kGuest, rng);
    check(h.state().phase == VotePhase::Result, "delai ecoule : resultat");
    check(h.state().winner.node == 5, "le seul vote l'emporte");

    // Absent 60 s : ecarte, puis le compte repart pour les autres.
    VoteHost a; a.set_never_drop(kHost);
    now = 0;
    a.cast(0, 5, 5, kHost | kGuest | kThird, now);
    bool dropped_seen = false;
    for (int i = 0; i < 700; ++i) {
        now += 100;
        a.set_present(kHost | kThird);
        if (a.tick(now, fixed_rng, &rng) && (a.state().dropped & kGuest)) dropped_seen = true;
    }
    check(dropped_seen, "l'absent est ecarte (et c'est publie)");
    check(a.state().dropped == kGuest, "seul l'invite absent est ecarte");
    check(a.state().phase == VotePhase::Voting, "pas encore le resultat");
    check(a.state().running, "le compte repart sans lui");
    check(a.state().remaining_ms < kVoteCountdownMs, "et avance");
    // Il revient et vote a temps : il compte.
    check(a.cast(1, 6, 5, 0, now), "l'ecarte revient voter");
    check(!(a.state().dropped & kGuest), "il n'est plus ecarte");
    run(a, now, 100, kHost | kGuest | kThird, rng);
    check(a.state().phase == VotePhase::Voting, "le troisieme n'a pas vote");
    check(a.cast(2, 6, 5, 0, now), "le troisieme vote");
    run(a, now, 100, kHost | kGuest | kThird, rng);
    check(a.state().phase == VotePhase::Result && a.state().winner.node == 6, "son vote compte");

    // Ecarte alors que les autres ont tous vote : le resultat tombe aussitot.
    VoteHost c; c.set_never_drop(kHost);
    now = 0;
    c.cast(0, 5, 5, kHost | kGuest | kThird, now);
    c.cast(2, 6, 5, 0, now);
    run(c, now, 59900, kHost | kThird, rng);
    check(c.state().phase == VotePhase::Voting, "59,9 s : on l'attend encore");
    run(c, now, 200, kHost | kThird, rng);
    check(c.state().phase == VotePhase::Result, "60 s : ecarte, les autres ont vote");
    check(c.state().drawn, "une voix chacun : tirage");

    // L'hote absent n'est jamais ecarte.
    VoteHost b; b.set_never_drop(kHost);
    now = 0;
    b.cast(1, 3, 5, kHost | kGuest, now);
    run(b, now, 100000, kGuest, rng);
    check(!(b.state().dropped & kHost), "l'hote n'est jamais ecarte");
    check(b.state().phase == VotePhase::Voting, "on l'attend");
    // ... sauf le plafond de 3 minutes.
    run(b, now, 90000, kGuest, rng);
    check(b.state().phase == VotePhase::Result, "3 minutes au plus");
    check(b.state().winner.node == 3, "le vote de l'invite l'emporte");
}

void test_step_cap() {
    VoteHost h; h.set_never_drop(kHost);
    FixedRng rng;
    uint64_t now = 0;
    h.cast(0, 5, 5, kHost | kGuest, now);
    h.set_present(kHost | kGuest);
    h.tick(now + 10000, fixed_rng, &rng);              // une image figee 10 s
    check(h.state().remaining_ms == kVoteCountdownMs - kVoteMaxStepMs, "un pas long ne compte que 500 ms");
}

void test_leaving() {
    VoteHost h; h.set_never_drop(kHost);
    FixedRng rng;
    uint64_t now = 0;
    h.cast(0, 5, 5, kHost | kGuest | kThird, now);
    h.cast(1, 8, 9, 0, now);
    check(h.keep_voters(kHost | kThird), "l'invite part");
    check(h.state().voters == (kHost | kThird), "il n'est plus votant");
    check(!h.state().ballot[1].valid(), "son vote part avec lui");
    check(!h.keep_voters(kHost | kThird), "rien de plus");
    check(!h.cast(1, 8, 9, kHost | kThird, now), "plus de vote de lui a ce tour");
    h.cast(2, 5, 5, 0, now);
    run(h, now, 100, kHost | kThird, rng);
    check(h.state().phase == VotePhase::Result && h.state().winner.node == 5, "les restants decident");

    // Le seul vote part : plus de tour.
    VoteHost g; g.set_never_drop(kHost);
    now = 0;
    g.cast(1, 8, 9, kHost | kGuest, now);
    g.keep_voters(kHost);
    check(run(g, now, 100, kHost, rng), "publie");
    check(g.state().phase == VotePhase::Idle, "tour abandonne faute de vote");
    check(g.state().round == 1, "le numero reste");
}

void test_late_joiner() {
    VoteHost h; h.set_never_drop(kHost);
    FixedRng rng;
    uint64_t now = 0;
    h.cast(0, 5, 5, kHost | kGuest, now);
    run(h, now, 1000, kHost | kGuest, rng);
    check(h.state().voters == (kHost | kGuest), "le troisieme n'est pas attendu");
    check(!h.cast(2, 5, 5, kHost | kGuest, now), "hors de l'aventure : refuse");
    check(h.cast(2, 6, 5, kHost | kGuest | kThird, now), "arrive dans l'aventure : son clic compte");
    check(h.state().voters == (kHost | kGuest | kThird), "il entre dans le tour");
    check(h.state().present & kThird, "il est sur la carte");
    h.cast(1, 6, 5, 0, now);
    run(h, now, 100, kHost | kGuest | kThird, rng);
    check(h.state().phase == VotePhase::Result && h.state().winner.node == 6, "sa voix fait la majorite");
}

void test_round_numbers() {
    VoteHost h;
    FixedRng rng;
    uint64_t now = 0;
    for (int i = 0; i < 300; ++i) {
        check(h.cast(0, 1, 5, kHost, now), "tour seul");
        check(h.state().round != 0, "jamais le tour 0");
        h.finish();
    }
    h.reset();
    h.cast(0, 1, 5, kHost, now);
    check(h.state().round != 0, "reset garde la suite des numeros");
    (void)rng;
}

void test_message() {
    VoteState s;
    s.round = 42; s.phase = VotePhase::Result;
    s.voters = 0x07; s.present = 0x05; s.dropped = 0x02; s.running = false; s.remaining_ms = 12345;
    s.ballot[0] = {7, 12}; s.ballot[2] = {300, 5};
    s.winner = {300, 5}; s.drawn = true; s.tied_count = 2;
    s.tied[0] = {7, 12}; s.tied[1] = {300, 5};
    s.refused_slot = 2; s.refused_reason = kRefuseBlocked;
    MmMsg m;
    vote_encode_state(s, m);
    check(m.kind == MM_VOTE && m.a == kVoteState, "entete");
    VoteState d;
    check(vote_decode_state(m, d), "decode");
    check(d.round == 42 && d.phase == VotePhase::Result, "tour et phase");
    check(d.voters == 0x07 && d.present == 0x05 && d.dropped == 0x02, "masques");
    check(!d.running && d.drawn && d.remaining_ms == 12345, "drapeaux et temps");
    check(d.ballot[0].node == 7 && d.ballot[0].type == 12, "vote 0");
    check(!d.ballot[1].valid(), "pas de vote 1");
    check(d.ballot[2].node == 300 && d.ballot[2].type == 5, "vote 2");
    check(d.winner.node == 300 && d.tied_count == 2 && d.tied[1].node == 300, "resultat");
    check(d.refused_slot == 2 && d.refused_reason == kRefuseBlocked, "vote refuse");

    // Par le VRAI codec du reseau, pas seulement la structure : l'etat est
    // binaire, et passe en chaine il etait coupe au premier octet nul (le vote
    // 0 a la case 7 s'ecrit 07 00 -- tout le reste arrivait a zero chez
    // l'invite, 15 octobre).
    {
        uint8_t wire[256];
        const uint32_t n = enc_mm(wire, sizeof(wire), m);
        check(n > 0, "reseau : encode");
        Reader r(wire, n);
        check(r.u8v() == MSG_MM, "reseau : type");
        MmMsg back;
        check(dec_mm(r, back) && r.pos == r.len, "reseau : decode, tout lu");
        VoteState w;
        check(vote_decode_state(back, w), "reseau : etat decode");
        check(w.ballot[0].node == 7 && w.ballot[2].node == 300 && w.ballot[2].type == 5,
              "reseau : les votes apres un octet nul arrivent");
        check(w.winner.node == 300 && w.winner.type == 5 && w.tied_count == 2 && w.tied[1].node == 300,
              "reseau : le gagnant et l'egalite arrivent");
        check(w.refused_slot == 2 && w.refused_reason == kRefuseBlocked, "reseau : le refus arrive");
    }
    VoteState none;
    vote_encode_state(none, m);
    vote_decode_state(m, d);
    check(d.refused_slot == kVoteNobody && d.refused_reason == kRefuseNone, "aucun refus par defaut");
    check(std::string(vote_refusal_text(kRefuseOffMap)) == u8"l'hôte n'est pas sur la carte", "raison");

    MmMsg bad = m; bad.c = 9;
    check(!vote_decode_state(bad, d), "phase inconnue refusee");
    bad = m; ((uint8_t*)bad.text)[27] = 5;
    check(!vote_decode_state(bad, d), "trop de cases a egalite");
    bad = m; bad.a = kVoteCast;
    check(!vote_decode_state(bad, d), "pas un etat");

    VoteCast c; c.round = 3; c.node = 17; c.node_count = 40; c.seed_lo = 0xDEADBEEF; c.type = 13;
    vote_encode_cast(c, m);
    VoteCast e;
    check(vote_decode_cast(m, e), "decode vote");
    check(e.round == 3 && e.node == 17 && e.node_count == 40 && e.seed_lo == 0xDEADBEEF && e.type == 13,
          "vote aller-retour");
    c.node = 40;
    vote_encode_cast(c, m);
    check(!vote_decode_cast(m, e), "case hors de la carte refusee");
    vote_encode_here(true, m);
    check(m.kind == MM_VOTE && m.a == kVoteHere && m.c == 1, "presence");
}

std::string test_label(void*, uint16_t node, uint8_t type) {
    return std::string(vote_type_name(type)) + (node == 9 ? u8" (en bas)" : "");
}

void test_view() {
    VoteState s;
    s.round = 1; s.phase = VotePhase::Voting; s.voters = kHost | kGuest; s.present = kHost | kGuest;
    s.running = true; s.remaining_ms = 24000;
    s.ballot[0] = {9, 5};
    check(vote_layout(s, 1) == VoteLayout::Open, "l'invite n'a pas vote : Même chemin");
    check(vote_layout(s, 0) == VoteLayout::Voted, "l'hote a vote : Changer");
    check(vote_layout(s, 2) == VoteLayout::None, "pas votant : rien");
    VoteBallot lead = vote_leader_excluding(s, 1);
    check(lead.node == 9 && lead.type == 5, "le chemin de l'hote");
    check(!vote_leader_excluding(s, 0).valid(), "l'hote : personne d'autre n'a vote");
    check(vote_layout(s, 1, VoteBallot{4, 12}) == VoteLayout::Voted, "son vote envoye compte");

    const std::string names[kVoteSlots] = { "Hizumi", "Ami" };
    std::string t = vote_text(s, 1, names, 23400, test_label, nullptr);
    check(has(t, u8"Choix du chemin"), "titre");
    check(has(t, u8"Hizumi : Combat (en bas)"), "le vote de l'hote, avec sa position");
    check(has(t, u8"Ami (toi) : pas encore voté"), "l'invite n'a pas vote");
    check(has(t, u8"À toi de choisir : encore 24 s."), "compte a rebours arrondi au-dessus");
    t = vote_text(s, 0, names, 24000, test_label, nullptr);
    check(has(t, u8"Hizumi (toi) : Combat"), "l'hote voit son vote");
    check(has(t, u8"En attente des autres : encore 24 s."), "il attend");
    t = vote_text(s, 1, names, 24000, test_label, nullptr, VoteBallot{4, 12});
    check(has(t, u8"Ami (toi) : Boutique"), "le vote envoye s'affiche tout de suite");

    s.present = kHost; s.running = false;
    t = vote_text(s, 0, names, 24000, test_label, nullptr);
    check(has(t, u8"Ami : pas encore sur la carte"), "absent");
    check(has(t, u8"En pause"), "compte arrete");
    s.dropped = kGuest;
    t = vote_text(s, 0, names, 24000, test_label, nullptr);
    check(has(t, u8"Ami : absent, ne vote pas"), "ecarte");

    // Deux votes pour la meme case : un chemin mene ; deux cases : personne.
    VoteState three;
    three.round = 2; three.phase = VotePhase::Voting; three.voters = kHost | kGuest | kThird;
    three.ballot[0] = {9, 5}; three.ballot[2] = {4, 12};
    check(vote_layout(three, 1) == VoteLayout::Choose, "egalite chez les autres : Choisir");
    three.ballot[2] = {9, 5};
    check(vote_layout(three, 1) == VoteLayout::Open, "les deux autres d'accord");

    VoteState r;
    r.round = 3; r.phase = VotePhase::Result; r.voters = kHost | kGuest;
    r.ballot[0] = {9, 5}; r.ballot[1] = {4, 12};
    r.winner = {4, 12}; r.drawn = true; r.tied_count = 2; r.tied[0] = {4, 12}; r.tied[1] = {9, 5};
    check(vote_layout(r, 0) == VoteLayout::Result, "resultat");
    t = vote_text(r, 0, names, 0, test_label, nullptr);
    check(has(t, u8"Égalité entre Boutique et Combat (en bas) :\nle sort choisit Boutique."), "tirage");
    r.drawn = false; r.tied_count = 0; r.ballot[0] = {4, 12};
    t = vote_text(r, 0, names, 0, test_label, nullptr);
    check(has(t, u8"Tout le monde a choisi Boutique."), "unanimite");
    r.ballot[1] = VoteBallot{};
    t = vote_text(r, 0, names, 0, test_label, nullptr);
    check(has(t, u8"Ami : pas de vote"), "abstention");
    check(has(t, u8"Boutique l'emporte."), "seul vote");
    t = vote_text(r, 0, nullptr, 0, nullptr, nullptr);
    check(has(t, u8"Joueur 2 : pas de vote"), "nom par defaut");
}

// L'absence se compte d'affilee : 40 s, de retour, 40 s -- pas ecarte ; puis
// 60 s d'un trait -- ecarte.
void test_absence_in_a_row() {
    VoteHost h; h.set_never_drop(kHost);
    FixedRng rng;
    uint64_t now = 0;
    h.cast(0, 5, 5, kHost | kGuest, now);
    run(h, now, 40000, kHost, rng);
    run(h, now, 1000, kHost | kGuest, rng);
    run(h, now, 40000, kHost, rng);
    check(!(h.state().dropped & kGuest), "absences coupees par un retour : pas ecarte");
    check(h.state().phase == VotePhase::Voting, "on l'attend encore");
    run(h, now, 20000, kHost, rng);
    check(h.state().phase == VotePhase::Result, "60 s d'affilee : ecarte, le vote de l'hote l'emporte");
}

// La duree du resultat : courte quand tout le monde a choisi la meme case.
void test_result_show() {
    VoteState s;
    s.phase  = VotePhase::Result;
    s.voters = kHost | kGuest;
    s.ballot[0] = {4, 12};
    s.ballot[1] = {4, 12};
    s.winner    = {4, 12};
    check(vote_unanimous(s) && vote_result_show_ms(s) == kVoteResultSameMs, "unanime : resultat court");
    s.ballot[1] = {7, 5};
    check(!vote_unanimous(s) && vote_result_show_ms(s) == kVoteResultMs, "majorite : resultat entier");
    s.ballot[1] = VoteBallot{};
    check(!vote_unanimous(s), "une abstention : pas unanime");
    s.ballot[1] = {4, 12};
    s.drawn = true;
    check(!vote_unanimous(s), "tirage au sort : pas unanime");
    s.drawn  = false;
    s.voters = kHost;
    s.ballot[1] = VoteBallot{};
    check(!vote_unanimous(s), "un seul votant : pas unanime");
}

} // namespace

int main() {
    test_unanimous();
    test_tie_and_change();
    test_countdown_and_absence();
    test_step_cap();
    test_leaving();
    test_late_joiner();
    test_round_numbers();
    test_message();
    test_view();
    test_absence_in_a_row();
    test_result_show();
    if (g_fail) { std::printf("%d echec(s)\n", g_fail); return 1; }
    std::printf("ok\n");
    return 0;
}
