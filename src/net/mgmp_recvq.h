#pragma once
// mgmp_recvq -- Mewgenics Multi (protocole 59) : la file de reception de
// mgmp_net, en code pur pour etre testee sans sockets (tests/test_recvq.cpp).
//
// POURQUOI. La file d'origine avait 256 places fixes et jetait en silence le
// message le plus recent une fois pleine (seulement compte). Elle se remplit
// quand le jeu ne la vide plus : un chargement de save ou de combat fige le jeu
// plusieurs secondes, pendant lesquelles les fils de reception continuent. Or les
// curseurs partent jusqu'a 20 fois par seconde par joueur et l'hote les relaie a
// tous : a 4 joueurs, 60 par seconde, 256 en 4 s. Le message suivant -- une
// decision de combat, un ENTERNODE, un choix, les chats -- etait perdu, et la
// partie avec. Un arrivant en plein combat recoit aussi d'un coup jusqu'a 512
// decisions rejouees.
//
// MAINTENANT :
//   - un message « le plus recent suffit » (curseur, visee) remplace le sien
//     encore en file, du meme auteur, au lieu d'en prendre une place ;
//   - la file double quand elle est pleine, jusqu'a kRecvQueueMax ;
//   - au-dela seulement, le plus recent est jete (et l'appelant le dit).
//
// Pas de verrou ici : mgmp_net tient le sien autour de chaque appel.

#include <cstdint>
#include <new>

namespace mgmp {

constexpr uint32_t kRecvQueueStart = 256;
constexpr uint32_t kRecvQueueMax   = 16384;

enum class RecvPush { Queued, Grown, Replaced, Dropped };

template <typename T>
struct RecvQueue {
    T*       slot  = nullptr;
    uint32_t cap   = 0;
    uint32_t head  = 0;
    uint32_t count = 0;
    uint32_t peak  = 0;
    uint32_t max_cap = kRecvQueueMax;

    RecvQueue() = default;
    RecvQueue(const RecvQueue&) = delete;
    RecvQueue& operator=(const RecvQueue&) = delete;
    ~RecvQueue() { delete[] slot; }

    // Double la capacite (kRecvQueueStart la premiere fois), dans l'ordre.
    bool grow() {
        const uint32_t ncap = cap ? cap * 2 : kRecvQueueStart;
        if (ncap > max_cap || ncap < cap) return false;
        T* n = new (std::nothrow) T[ncap];
        if (!n) return false;
        for (uint32_t i = 0; i < count; ++i) n[i] = slot[(head + i) % cap];
        delete[] slot;
        slot = n;
        cap  = ncap;
        head = 0;
        return true;
    }

    // `coalesce` : un message « le plus recent suffit ». S'il y en a deja un que
    // `same(en_file, m)` reconnait, m le remplace a sa place (l'ancien ne doit rien
    // posseder : il est ecrase, pas libere). Sinon m va au bout, la file grandit
    // s'il le faut ; Dropped seulement a kRecvQueueMax -- m n'est alors pas pris.
    template <typename Same>
    RecvPush push(const T& m, bool coalesce, Same&& same) {
        if (coalesce) {
            for (uint32_t i = 0; i < count; ++i) {
                T& q = slot[(head + i) % cap];
                if (same(q, m)) {
                    q = m;
                    return RecvPush::Replaced;
                }
            }
        }
        bool grown = false;
        if (count >= cap) {
            if (!grow()) return RecvPush::Dropped;
            grown = true;
        }
        slot[(head + count) % cap] = m;
        ++count;
        if (count > peak) peak = count;
        return grown ? RecvPush::Grown : RecvPush::Queued;
    }

    // Le plus ancien. La place est remise a zero : elle ne possede plus rien
    // (ce qu'il possedait est maintenant a `out`).
    bool pop(T& out) {
        if (!count) return false;
        out = slot[head];
        slot[head] = T{};
        head = (head + 1) % cap;
        --count;
        return true;
    }

    // Pour vider (et liberer ce que possedent les messages restants) : le i-eme
    // message encore en file.
    T& at(uint32_t i) { return slot[(head + i) % cap]; }

    // Oublie tout et rend la memoire. L'appelant a libere ce que les messages
    // possedaient.
    void reset() {
        delete[] slot;
        slot  = nullptr;
        cap   = 0;
        head  = 0;
        count = 0;
    }
};

} // namespace mgmp
