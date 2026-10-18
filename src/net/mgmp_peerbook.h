#pragma once
// mgmp_peerbook -- Mewgenics Multi (protocole 59) : la comptabilite du hash des
// cases (NODEHASH) avec PLUSIEURS autres joueurs, en code pur pour etre testee
// sans le jeu (tests/test_peerbook.cpp).
//
// POURQUOI. mgmp_nodehash comparait avec deux files communes a tous les pairs :
// la notre, ou chaque echantillon etait RETIRE a la premiere comparaison, et
// celle des pairs, qui refusait tout une fois pleine. A deux joueurs, c'etait
// juste. A trois, l'echantillon du deuxieme invite ne trouvait plus le notre
// (deja pris par le premier) et restait dans la file des pairs pour toujours ;
// apres 16, plus rien n'etait compare, et la ligne « !! holding 16 node hashes »
// revenait a chaque case.
//
// MAINTENANT :
//   - le notre reste (une file qui oublie le plus ancien) et note a quels pairs
//     il a deja ete compare : chaque pair est compare UNE fois a chacun ;
//   - ceux des pairs arrives avant nous attendent dans une file PAR PAIR, qui
//     oublie le plus ancien quand elle est pleine (un point que nous n'atteindrons
//     jamais -- une case sautee -- ne bloque plus les suivants) ;
//   - un meme point plusieurs fois (deux ecrans d'evenement dans une case : meme
//     graine, meme point) : DANS L'ORDRE, le premier des notres avec le premier
//     des siens, le deuxieme avec le deuxieme -- comme les files d'avant.
//
// La cle est la paire (graine de la case, point), comme pour HashRing.

#include "mgmp_hashring.h"

#include <cstdint>

namespace mgmp {

constexpr uint8_t  kBookPeers = 8;    // les ids de pairs sont < kMaxPeers (4) ; marge
constexpr uint32_t kBookDepth = 32;   // 8 cases suivies d'avance x 2 points, et de la marge

template <typename Msg>
struct PeerBook {
    struct Mine {
        uint64_t battle_id = 0;   // la graine de la case
        uint32_t turn      = 0;   // le point (entree, evenement)
        Msg      m{};
        uint32_t done      = 0;   // les pairs deja compares a celui-ci
    };
    struct Theirs {
        uint64_t battle_id = 0;
        uint32_t turn      = 0;
        Msg      m{};
    };

    HashRing<Mine, kBookDepth>   mine;
    HashRing<Theirs, kBookDepth> theirs[kBookPeers];
    uint32_t                     evicted[kBookPeers] = {};

    void clear() {
        mine.clear();
        for (auto& r : theirs) r.clear();
        for (auto& e : evicted) e = 0;
    }

    // Notre echantillon a (cle, point). Chaque pair dont un echantillon pour ce
    // point attendait est compare maintenant, au plus ancien des siens --
    // pair(peer, le_notre, le_sien) --, et le notre est garde pour ceux qui
    // viendront. Rend le nombre de comparaisons faites.
    template <typename Pair>
    uint32_t publish(uint64_t key, uint32_t point, const Msg& m, Pair&& pair) {
        Mine e;
        e.battle_id = key;
        e.turn      = point;
        e.m         = m;
        uint32_t n = 0;
        for (uint8_t p = 0; p < kBookPeers; ++p) {
            Theirs t;
            if (!theirs[p].take(key, point, t)) continue;   // le plus ancien (ordre d'arrivee)
            e.done |= 1u << p;
            pair(p, e.m, t.m);
            ++n;
        }
        mine.push_evicting(e);
        return n;
    }

    // L'echantillon d'un pair. Compare tout de suite au plus ancien des notres
    // pour ce point qui ne lui a pas encore ete compare ; garde sinon. Rend false
    // si le garder a fait oublier un echantillon plus ancien de ce pair.
    template <typename Pair>
    bool receive(uint8_t from, uint64_t key, uint32_t point, const Msg& m, Pair&& pair) {
        if (from >= kBookPeers) return true;
        const uint32_t bit = 1u << from;
        for (uint32_t i = 0; i < mine.count; ++i) {
            Mine& e = mine.slot[i];
            if (e.battle_id != key || e.turn != point || (e.done & bit)) continue;
            e.done |= bit;
            pair(from, e.m, m);
            return true;
        }
        Theirs t;
        t.battle_id = key;
        t.turn      = point;
        t.m         = m;
        const bool full = theirs[from].full();
        theirs[from].push_evicting(t);
        if (full) ++evicted[from];
        return !full;
    }

    // Combien d'echantillons de ce pair attendent un point que nous n'avons pas atteint.
    uint32_t held(uint8_t from) const { return from < kBookPeers ? theirs[from].count : 0; }

    // Un pair parti : ses echantillons gardes partent avec lui, et celui qui
    // reprendra son numero sera compare a nouveau aux notres.
    void forget_peer(uint8_t from) {
        if (from >= kBookPeers) return;
        theirs[from].clear();
        evicted[from] = 0;
        for (uint32_t i = 0; i < mine.count; ++i) mine.slot[i].done &= ~(1u << from);
    }
};

} // namespace mgmp
