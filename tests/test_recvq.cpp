// test_recvq -- Mewgenics Multi (protocole 59) : la file de reception ne perd
// plus un message parce que le jeu a fige quelques secondes.
//
// Le bogue fige ici : 256 places fixes, le plus recent jete en silence. Pendant
// un chargement, les curseurs des autres joueurs (20 par seconde chacun, relayes
// par l'hote) remplissaient la file, et la decision de combat ou l'ENTERNODE
// suivant etait perdu.
//
// Build: via CMake (tests/CMakeLists.txt), or standalone with
//   g++ -std=c++17 -Isrc/net tests/test_recvq.cpp

#include "mgmp_recvq.h"

#include <cstdio>
#include <cstdint>

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

enum : uint8_t { kAction = 1, kCursor = 2, kAim = 3, kEnter = 4 };

struct Msg {
    uint8_t  type = 0;
    uint8_t  from = 0;
    uint32_t seq  = 0;
};

static bool same_kind(const Msg& a, const Msg& b) { return a.type == b.type && a.from == b.from; }
static bool latest_wins(uint8_t t) { return t == kCursor || t == kAim; }

static RecvPush push(RecvQueue<Msg>& q, uint8_t type, uint8_t from, uint32_t seq) {
    Msg m; m.type = type; m.from = from; m.seq = seq;
    return q.push(m, latest_wins(type), same_kind);
}

int main() {
    // Ordre garde, a travers les tours de la file circulaire.
    {
        RecvQueue<Msg> q;
        uint32_t next = 0, got = 0;
        bool ordered = true;
        for (int round = 0; round < 10; ++round) {
            for (int i = 0; i < 200; ++i) push(q, kAction, 1, next++);
            Msg m;
            for (int i = 0; i < 150; ++i) { q.pop(m); ordered = ordered && m.seq == got++; }
        }
        Msg m;
        while (q.pop(m)) ordered = ordered && m.seq == got++;
        CHECK(ordered && got == next, "ordre garde (%u/%u)", got, next);
    }
    // Un gel de 10 s a 4 joueurs : 600 curseurs, 30 visees, et les vrais messages.
    {
        RecvQueue<Msg> q;
        uint32_t seq = 0;
        for (int tick = 0; tick < 200; ++tick) {           // 10 s a 20 par seconde
            for (uint8_t p = 1; p <= 3; ++p) push(q, kCursor, p, seq++);
            if (tick % 20 == 0) push(q, kAim, 1, seq++);
            if (tick % 10 == 0) push(q, kAction, (uint8_t)(1 + tick % 3), seq++);
        }
        push(q, kEnter, 0, seq++);
        // 3 curseurs (un par joueur) + 1 visee + 20 decisions + 1 ENTERNODE.
        CHECK(q.count == 25, "les curseurs et visees fondus (%u en file)", q.count);
        Msg m; uint32_t actions = 0, cursors = 0, aims = 0, enters = 0, last_action = 0;
        bool action_order = true;
        while (q.pop(m)) {
            if (m.type == kAction) { action_order = action_order && m.seq >= last_action; last_action = m.seq; ++actions; }
            if (m.type == kCursor) ++cursors;
            if (m.type == kAim) ++aims;
            if (m.type == kEnter) ++enters;
        }
        CHECK(actions == 20 && enters == 1, "aucun vrai message perdu (%u decisions, %u entree)", actions, enters);
        CHECK(cursors == 3 && aims == 1, "le dernier curseur de chacun (%u), la derniere visee (%u)", cursors, aims);
        CHECK(action_order, "decisions dans l'ordre");
    }
    // Le dernier curseur gagne : la valeur est celle du plus recent.
    {
        RecvQueue<Msg> q;
        push(q, kCursor, 2, 10);
        push(q, kAction, 1, 11);
        CHECK(push(q, kCursor, 2, 12) == RecvPush::Replaced, "remplace");
        Msg m;
        q.pop(m);
        CHECK(m.type == kCursor && m.seq == 12, "le plus recent, a la place de l'ancien (%u)", m.seq);
        q.pop(m);
        CHECK(m.type == kAction && m.seq == 11, "puis la decision");
    }
    // Un arrivant en plein combat : 512 decisions d'un coup, toutes gardees.
    {
        RecvQueue<Msg> q;
        bool grown = false;
        for (uint32_t i = 0; i < 512 + 40; ++i) grown = (push(q, kAction, 0, i) == RecvPush::Grown) || grown;
        CHECK(grown && q.count == 552 && q.cap >= 552, "la file a grandi (%u en file, %u places)", q.count, q.cap);
        Msg m; uint32_t n = 0; bool ordered = true;
        while (q.pop(m)) ordered = ordered && m.seq == n++;
        CHECK(ordered && n == 552, "toutes, dans l'ordre (%u)", n);
    }
    // Grandir au milieu d'un tour de file garde l'ordre.
    {
        RecvQueue<Msg> q;
        for (uint32_t i = 0; i < 200; ++i) push(q, kAction, 0, i);
        Msg m;
        for (uint32_t i = 0; i < 150; ++i) q.pop(m);        // head avance
        for (uint32_t i = 200; i < 600; ++i) push(q, kAction, 0, i);   // repasse le bout, puis grandit
        uint32_t expect = 150; bool ordered = true;
        while (q.pop(m)) ordered = ordered && m.seq == expect++;
        CHECK(ordered && expect == 600, "ordre garde apres agrandissement (%u)", expect);
    }
    // Plafond : au-dela, le plus recent est refuse (et la file reste intacte).
    {
        RecvQueue<Msg> q;
        q.max_cap = 512;
        uint32_t dropped = 0;
        for (uint32_t i = 0; i < 600; ++i)
            if (push(q, kAction, 0, i) == RecvPush::Dropped) ++dropped;
        CHECK(q.count == 512 && dropped == 88, "512 gardes, 88 refuses (%u, %u)", q.count, dropped);
        Msg m; q.pop(m);
        CHECK(m.seq == 0, "le plus ancien d'abord");
        // Plein, un curseur deja en file se remplace encore.
        RecvQueue<Msg> c;
        c.max_cap = 256;
        push(c, kCursor, 1, 0);
        for (uint32_t i = 1; i < 256; ++i) push(c, kAction, 0, i);
        CHECK(push(c, kCursor, 1, 999) == RecvPush::Replaced, "plein, le curseur se remplace");
        CHECK(push(c, kAction, 0, 1000) == RecvPush::Dropped, "plein, une decision est refusee");
    }
    // Vider puis remettre a zero.
    {
        RecvQueue<Msg> q;
        for (uint32_t i = 0; i < 300; ++i) push(q, kAction, 0, i);
        uint32_t seen = 0;
        for (uint32_t i = 0; i < q.count; ++i) seen += q.at(i).seq == i;
        CHECK(seen == 300, "at() dans l'ordre (%u)", seen);
        q.reset();
        CHECK(q.count == 0 && q.cap == 0 && q.slot == nullptr, "remise a zero");
        Msg m;
        CHECK(!q.pop(m), "vide");
        push(q, kAction, 0, 7);
        CHECK(q.pop(m) && m.seq == 7, "reutilisable");
    }

    std::printf("%d/%d checks passed\n", g_checks - g_fail, g_checks);
    return g_fail ? 1 : 0;
}
