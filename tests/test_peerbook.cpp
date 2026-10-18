// test_peerbook -- Mewgenics Multi (protocole 59) : le hash des cases (NODEHASH)
// compare chaque pair, a 2, 3 ou 4 joueurs.
//
// Le bogue fige ici : avec deux files communes a tous les pairs, l'echantillon du
// deuxieme invite ne trouvait plus le notre (retire a la premiere comparaison) et
// restait garde pour toujours ; apres 16, plus rien n'etait compare.
//
// Build: via CMake (tests/CMakeLists.txt), or standalone with
//   g++ -std=c++17 -Isrc/net tests/test_peerbook.cpp

#include "mgmp_peerbook.h"

#include <cstdio>
#include <cstdint>
#include <vector>

using namespace mgmp;

static int g_fail = 0;
static int g_checks = 0;

#define CHECK(cond, ...)                                                     \
    do {                                                                     \
        ++g_checks;                                                          \
        if (!(cond)) {                                                       \
            ++g_fail;                                                        \
            std::printf("FAIL %s:%d  ", __FILE__, __LINE__);                 \
            std::printf(__VA_ARGS__);                                        \
            std::printf("\n");                                               \
        }                                                                    \
    } while (0)

struct Sample {
    uint64_t value = 0;
};

struct Cmp {
    uint8_t  peer;
    uint64_t ours, theirs;
};

struct Log {
    std::vector<Cmp> v;
    void operator()(uint8_t p, const Sample& a, const Sample& b) { v.push_back({p, a.value, b.value}); }
    uint32_t with(uint8_t p) const {
        uint32_t n = 0;
        for (const Cmp& c : v) n += c.peer == p;
        return n;
    }
};

static Sample s(uint64_t v) { Sample x; x.value = v; return x; }

int main() {
    // 2 joueurs, nous d'abord : compare une fois, un doublon ne recompare pas.
    {
        PeerBook<Sample> b; Log log;
        b.publish(100, 0, s(7), log);
        CHECK(log.v.empty(), "rien a comparer avant le pair");
        b.receive(1, 100, 0, s(7), log);
        CHECK(log.v.size() == 1 && log.v[0].peer == 1 && log.v[0].ours == 7 && log.v[0].theirs == 7,
              "compare au pair 1");
        b.receive(1, 100, 0, s(7), log);
        CHECK(log.v.size() == 1, "un doublon ne recompare pas (%zu)", log.v.size());
    }
    // 2 joueurs, le pair d'abord : garde, puis compare a notre publication.
    {
        PeerBook<Sample> b; Log log;
        b.receive(1, 100, 1, s(9), log);
        CHECK(log.v.empty() && b.held(1) == 1, "garde en attendant le notre");
        CHECK(b.publish(100, 1, s(8), log) == 1, "une comparaison a la publication");
        CHECK(log.v.size() == 1 && log.v[0].ours == 8 && log.v[0].theirs == 9, "les deux valeurs, dans l'ordre");
        CHECK(b.held(1) == 0, "plus rien de garde");
        b.receive(1, 100, 1, s(9), log);
        CHECK(log.v.size() == 1, "deja compare a ce pair");
    }
    // 3 joueurs (le bogue) : l'hote compare les DEUX invites.
    {
        PeerBook<Sample> b; Log log;
        b.publish(200, 0, s(1), log);
        b.receive(1, 200, 0, s(1), log);
        b.receive(2, 200, 0, s(1), log);
        CHECK(log.with(1) == 1 && log.with(2) == 1, "les deux invites compares (%u, %u)", log.with(1), log.with(2));
        CHECK(b.held(2) == 0, "rien de garde pour le second invite");
    }
    // 3 joueurs, un invite devant et un derriere.
    {
        PeerBook<Sample> b; Log log;
        b.receive(2, 300, 0, s(5), log);       // devant nous
        b.publish(300, 0, s(5), log);          // compare a 2
        b.receive(1, 300, 0, s(5), log);       // derriere nous : trouve le notre
        CHECK(log.with(1) == 1 && log.with(2) == 1, "devant et derriere compares");
    }
    // 4 joueurs, beaucoup de cases : chacun compare a chaque point, rien ne s'accumule.
    {
        PeerBook<Sample> b; Log log;
        for (uint64_t node = 1; node <= 50; ++node) {
            for (uint32_t pt = 0; pt < 2; ++pt) {
                b.receive(3, node, pt, s(node), log);   // le plus rapide
                b.publish(node, pt, s(node), log);
                b.receive(1, node, pt, s(node), log);
                b.receive(2, node, pt, s(node), log);
            }
        }
        CHECK(log.with(1) == 100 && log.with(2) == 100 && log.with(3) == 100,
              "100 comparaisons par pair (%u %u %u)", log.with(1), log.with(2), log.with(3));
        CHECK(b.held(1) == 0 && b.held(2) == 0 && b.held(3) == 0, "rien de garde");
        bool same = true;
        for (const Cmp& c : log.v) same = same && c.ours == c.theirs;
        CHECK(same, "les valeurs comparees sont celles du meme point");
    }
    // Des points que nous n'atteindrons jamais ne bloquent pas les suivants.
    {
        PeerBook<Sample> b; Log log;
        for (uint64_t i = 0; i < 40; ++i) b.receive(1, 1000 + i, 0, s(i), log);   // jamais atteints
        CHECK(b.held(1) == kBookDepth, "file du pair pleine (%u)", b.held(1));
        CHECK(b.evicted[1] == 40 - kBookDepth, "les plus anciens oublies (%u)", b.evicted[1]);
        b.receive(1, 5000, 0, s(42), log);
        b.publish(5000, 0, s(42), log);
        CHECK(log.with(1) == 1, "le point atteint est compare malgre la file pleine");
    }
    // Deux ecrans d'evenement dans une case (meme graine, meme point), le pair en
    // avance : le premier avec le premier, le deuxieme avec le deuxieme.
    {
        PeerBook<Sample> b; Log log;
        b.receive(1, 600, 1, s(10), log);
        b.receive(1, 600, 1, s(20), log);
        CHECK(b.held(1) == 2, "les deux gardes, dans l'ordre (%u)", b.held(1));
        b.publish(600, 1, s(10), log);
        b.publish(600, 1, s(20), log);
        CHECK(log.v.size() == 2 && log.v[0].ours == 10 && log.v[0].theirs == 10 &&
              log.v[1].ours == 20 && log.v[1].theirs == 20, "appaires dans l'ordre");
    }
    // Les memes deux ecrans, nous en avance.
    {
        PeerBook<Sample> b; Log log;
        b.publish(700, 1, s(1), log);
        b.publish(700, 1, s(3), log);
        CHECK(b.mine.count == 2, "les deux des notres gardes");
        b.receive(1, 700, 1, s(1), log);
        b.receive(1, 700, 1, s(3), log);
        CHECK(log.v.size() == 2 && log.v[0].ours == 1 && log.v[0].theirs == 1 &&
              log.v[1].ours == 3 && log.v[1].theirs == 3, "appaires dans l'ordre");
        b.receive(1, 700, 1, s(3), log);
        CHECK(log.v.size() == 2, "un troisieme du pair n'a plus rien en face");
        // Un autre pair, plus tard : compare aux deux aussi.
        b.receive(2, 700, 1, s(1), log);
        b.receive(2, 700, 1, s(3), log);
        CHECK(log.with(2) == 2 && log.v[2].ours == 1 && log.v[3].ours == 3, "le second pair aussi, dans l'ordre");
    }
    // Un pair qui part puis un autre qui reprend son numero : compare a nouveau.
    {
        PeerBook<Sample> b; Log log;
        b.publish(750, 0, s(5), log);
        b.receive(2, 750, 0, s(5), log);
        b.forget_peer(2);
        b.receive(2, 750, 0, s(5), log);
        CHECK(log.with(2) == 2, "le nouveau venu au meme numero est compare (%u)", log.with(2));
    }
    // Un pair tres en retard : au-dela de la profondeur, le notre est oublie (pas
    // de fausse comparaison), les points recents sont compares.
    {
        PeerBook<Sample> b; Log log;
        for (uint64_t i = 1; i <= 40; ++i) b.publish(i, 0, s(i), log);
        b.receive(1, 1, 0, s(1), log);
        CHECK(log.v.empty(), "trop ancien : rien a comparer");
        b.receive(1, 40, 0, s(40), log);
        CHECK(log.v.size() == 1 && log.v[0].ours == 40, "recent : compare");
    }
    // Un pair oublie (parti) : ses echantillons gardes partent avec lui.
    {
        PeerBook<Sample> b; Log log;
        b.receive(2, 800, 0, s(1), log);
        b.forget_peer(2);
        CHECK(b.held(2) == 0, "oublie");
        b.publish(800, 0, s(1), log);
        CHECK(log.v.empty(), "rien a comparer apres l'oubli");
    }
    // Un id hors limites ne casse rien.
    {
        PeerBook<Sample> b; Log log;
        CHECK(b.receive(200, 1, 0, s(1), log), "id hors limites ignore");
        CHECK(log.v.empty(), "rien");
    }

    std::printf("%d/%d checks passed\n", g_checks - g_fail, g_checks);
    return g_fail ? 1 : 0;
}
