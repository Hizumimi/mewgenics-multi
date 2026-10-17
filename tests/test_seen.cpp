// tests/test_seen.cpp -- Mewgenics Multi : les decisions de combat deja vues
// (mgmp_seen.h, protocole 50). Les ordres d'arrivee d'une decision relayee par
// l'hote et rejouee dans sa reprise, la reprise de ses propres decisions apres un
// retour, et l'oubli d'un combat.
#include "mgmp_seen.h"

#include <cstddef>
#include <cstdio>
#include <vector>

using namespace mgmp;

static int g_fail = 0, g_ok = 0;
#define CHECK(c) do { if (c) ++g_ok; else { ++g_fail; std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

using Seen = SeenActions<4, 4>;
constexpr uint64_t B1 = 0x211f6ea51e365ae0ull, B2 = 0x1234ull, B3 = 0x99ull, B4 = 0x77ull, B5 = 0x55ull;

// La file d'un joueur qui arrive : ce qu'il garde, dans l'ordre d'arrivee.
struct Joiner {
    Seen s;
    std::vector<std::pair<uint8_t, uint32_t>> queue;
    void recv(uint64_t b, uint8_t author, uint32_t seq) {
        if (!s.mark(b, author, seq)) queue.push_back({ author, seq });
    }
};

// Une decision dans la file (tour, auteur, rang), et la prise de pend_take : la
// plus ancienne selon seen_action_earlier, la premiere arrivee a egalite.
struct Dec { uint32_t turn; uint8_t author; uint32_t seq; };
static Dec take(std::vector<Dec>& q) {
    size_t best = 0;
    for (size_t i = 1; i < q.size(); ++i)
        if (seen_action_earlier(q[i].turn, q[i].author, q[i].seq, q[best].turn, q[best].author, q[best].seq))
            best = i;
    const Dec d = q[best];
    q.erase(q.begin() + (std::ptrdiff_t)best);
    return d;
}

int main() {
    // Relai d'abord, puis la reprise qui la contient : une seule fois.
    {
        Joiner j;
        j.recv(B1, 2, 5);                                   // relai de l'hote (joueur 2, sa 5e)
        for (uint32_t q = 1; q <= 5; ++q) j.recv(B1, 2, q); // la reprise : ses 1..5
        CHECK(j.queue.size() == 5);
        int fives = 0;
        for (auto& e : j.queue) fives += e.second == 5;
        CHECK(fives == 1);
    }
    // La reprise ne contient pas encore la decision relayee (l'hote ne l'avait pas
    // traitee) : elle est gardee, rien ne manque.
    {
        Joiner j;
        j.recv(B1, 2, 5);
        for (uint32_t q = 1; q <= 4; ++q) j.recv(B1, 2, q);
        CHECK(j.queue.size() == 5);
    }
    // Reprise d'abord, relai ensuite.
    {
        Joiner j;
        for (uint32_t q = 1; q <= 3; ++q) j.recv(B1, 0, q);
        j.recv(B1, 0, 3);
        j.recv(B1, 0, 4);
        CHECK(j.queue.size() == 4);
    }
    // Les auteurs et les combats ne se confondent pas.
    {
        Seen s;
        CHECK(!s.mark(B1, 0, 1));
        CHECK(!s.mark(B1, 1, 1));
        CHECK(!s.mark(B2, 0, 1));
        CHECK(s.mark(B1, 0, 1));
        CHECK(s.mark(B2, 0, 1));
    }
    // Sans numero (ancien format, auteur inconnu) : jamais un double.
    {
        Seen s;
        CHECK(!s.mark(B1, 0xFF, 3) && !s.mark(B1, 0xFF, 3));
        CHECK(!s.mark(B1, 1, 0) && !s.mark(B1, 1, 0));
        CHECK(!s.mark(0, 1, 2) && !s.mark(0, 1, 2));
        CHECK(!s.mark(B1, 1, 70000) && !s.mark(B1, 1, 70000));
    }
    // Ses propres decisions : le rang suit ; apres un depart (oubli du combat)
    // puis la reprise qui les lui rend, il ne reprend pas un rang deja pris.
    {
        Seen s;
        CHECK(s.next(B1, 1) == 1);
        s.mark(B1, 1, s.next(B1, 1));
        s.mark(B1, 1, s.next(B1, 1));
        s.mark(B1, 1, s.next(B1, 1));
        CHECK(s.next(B1, 1) == 4);
        s.forget(B1);                                        // il quitte l'aventure
        CHECK(s.next(B1, 1) == 1);
        for (uint32_t q = 1; q <= 3; ++q) CHECK(!s.mark(B1, 1, q));   // la reprise : les siennes
        CHECK(s.next(B1, 1) == 4);
        CHECK(!s.mark(B1, 1, 4));
    }
    // Oublier tout ; un combat nouveau prend la place du plus ancien.
    {
        Seen s;
        s.mark(B1, 0, 1);
        s.forget(0);
        CHECK(!s.mark(B1, 0, 1));
        s.mark(B2, 0, 1);
        s.mark(B3, 0, 1);
        s.mark(B4, 0, 1);                                    // 4 combats : B1..B4
        CHECK(s.mark(B1, 0, 1));                             // encore la
        s.mark(B5, 0, 1);                                    // B1 sort
        CHECK(!s.mark(B1, 0, 1));                            // revu comme neuf
    }
    // Beaucoup de decisions : les rangs au-dela de 64 (plusieurs mots).
    {
        Seen s;
        for (uint32_t q = 1; q <= 600; ++q) CHECK(!s.mark(B1, 3, q));
        for (uint32_t q = 1; q <= 600; ++q) CHECK(s.mark(B1, 3, q));
        CHECK(s.next(B1, 3) == 601);
    }
    // L'ordre (relecture de la build 29) : le relai du tour 4 arrive avant la
    // reprise des tours 0 a 3 ; le chat les joue quand meme dans l'ordre.
    {
        Joiner j;
        const uint32_t turn_of[6] = { 0, 0, 1, 2, 3, 4 };      // rang -> tour
        j.recv(B1, 2, 5);
        for (uint32_t q = 1; q <= 5; ++q) j.recv(B1, 2, q);
        CHECK(j.queue.size() == 5 && j.queue.front().second == 5);   // l'ordre d'arrivee : [5,1,2,3,4]
        std::vector<Dec> q;
        for (auto& e : j.queue) q.push_back(Dec{ turn_of[e.second], e.first, e.second });
        for (uint32_t want = 1; want <= 5; ++want) CHECK(take(q).seq == want);
        CHECK(q.empty());
    }
    // Plusieurs decisions au meme tour (une capacite, puis la fin du tour) : le rang.
    {
        std::vector<Dec> q = { { 3, 1, 9 }, { 3, 1, 8 }, { 2, 1, 7 } };
        CHECK(take(q).seq == 7);
        CHECK(take(q).seq == 8);
        CHECK(take(q).seq == 9);
    }
    // Le chat change de main dans un tour (l'hote prend les chats d'un parti) :
    // deux auteurs au meme tour -- l'ordre d'arrivee.
    {
        std::vector<Dec> q = { { 5, 0, 2 }, { 5, 1, 1 } };
        CHECK(take(q).author == 0);
        CHECK(take(q).author == 1);
    }
    // Sans numero (ancien format) : le tour, puis l'arrivee.
    {
        std::vector<Dec> q = { { 4, 0xFF, 0 }, { 3, 0xFF, 0 }, { 4, 0xFF, 0 } };
        CHECK(take(q).turn == 3);
        CHECK(take(q).turn == 4 && q.size() == 1);
        CHECK(!seen_action_earlier(4, 0xFF, 0, 4, 0xFF, 0));
        CHECK(!seen_action_earlier(4, 1, 0, 4, 1, 3) && !seen_action_earlier(4, 1, 3, 4, 1, 0));
        CHECK(seen_action_earlier(4, 1, 2, 4, 1, 3) && !seen_action_earlier(4, 1, 3, 4, 1, 2));
    }
    std::printf("%s (%d verifications, %d echec(s))\n", g_fail ? "ECHEC" : "ok", g_ok + g_fail, g_fail);
    return g_fail ? 1 : 0;
}
