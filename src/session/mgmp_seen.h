#pragma once
// mgmp_seen.h -- Mewgenics Multi : les decisions de combat deja vues (protocole
// 50). Pur (aucun appel au jeu ni au reseau), teste par tests/test_seen.cpp.
//
// POURQUOI. Une decision d'un invite arrive aux autres de deux facons : l'hote la
// relaie des qu'il la recoit (sur le fil du reseau), et la rejoue dans la reprise
// d'un joueur qui arrive en plein combat (sa liste des decisions de ce combat,
// tenue sur le fil du jeu). Selon le moment, celui qui arrive la recoit deux fois,
// une fois, ou d'abord par le relai puis par la reprise -- et une decision jouee
// deux fois desynchronise son combat (relecture du 17 octobre). Les jeter par
// position (« tout ce qui precede la reprise ») en perdait une sur deux.
//
// LA REGLE. Chaque decision porte son auteur (le numero du joueur qui l'a prise)
// et son rang parmi les siennes dans ce combat (1, 2, ...). Chacun note les
// (combat, auteur, rang) qu'il a vus et jette un double, dans quelque ordre qu'ils
// arrivent. Le rang de sa propre decision suit le plus grand qu'il ait vu de
// lui-meme dans ce combat : apres une reprise qui lui rend ses decisions d'avant
// (il etait parti, il revient), il ne reprend pas un rang deja pris.
//
// L'ORDRE (relecture de la build 29). Jeter les doubles ne remet pas les
// decisions dans l'ordre : celui qui rejoint un combat recoit les decisions en
// direct (relayees) des son clic, avant la reprise des plus anciennes. La file
// est alors [5, 1, 2, 3, 4] pour un meme chat, et prendre la premiere arrivee
// lui ferait jouer au tour 0 la decision du tour N. On prend donc, pour un chat,
// la plus ancienne : seen_action_earlier.
#include <cstdint>
#include <vector>

namespace mgmp {

// Deux decisions du meme chat dans la file : la premiere est-elle a jouer avant ?
// Celle du tour le plus ancien d'abord ; au meme tour et du meme auteur, le plus
// petit rang ; sinon (sans numero, ou deux auteurs -- le chat a change de main
// dans ce tour) : non, l'ordre d'arrivee decide.
inline bool seen_action_earlier(uint32_t turn_a, uint8_t author_a, uint32_t seq_a,
                                uint32_t turn_b, uint8_t author_b, uint32_t seq_b) {
    if (turn_a != turn_b) return turn_a < turn_b;
    if (author_a == author_b && author_a != 0xFF && seq_a && seq_b) return seq_a < seq_b;
    return false;
}

template <int kPeers, int kBattles>
class SeenActions {
public:
    static constexpr uint64_t kNone   = 0;        // pas de combat (kNoBattle)
    static constexpr uint32_t kMaxSeq = 65536;    // au-dela : pas de controle (garde-fou)

    // Vrai si cette decision a deja ete vue (rien ne change) ; sinon elle est
    // notee. Sans numero (auteur hors bornes, rang 0) : jamais un double.
    bool mark(uint64_t battle, uint8_t author, uint32_t seq) {
        if (battle == kNone || author >= kPeers || seq == 0 || seq >= kMaxSeq) return false;
        Slot& s = slot(battle);
        std::vector<uint64_t>& b = s.bits[author];
        const size_t   w   = seq / 64;
        const uint64_t bit = 1ull << (seq % 64);
        if (b.size() <= w) b.resize(w + 1, 0);
        if (b[w] & bit) return true;
        b[w] |= bit;
        if (seq > s.max[author]) s.max[author] = seq;
        return false;
    }

    // Le rang de la prochaine decision de `author` dans ce combat.
    uint32_t next(uint64_t battle, uint8_t author) {
        if (battle == kNone || author >= kPeers) return 0;
        return slot(battle).max[author] + 1;
    }

    // Oublie ce combat (kNone : tous).
    void forget(uint64_t battle) {
        for (Slot& s : slots_)
            if (battle == kNone || s.battle == battle) s = Slot{};
    }

private:
    struct Slot {
        uint64_t              battle = kNone;
        uint32_t              max[kPeers] = {};
        std::vector<uint64_t> bits[kPeers];
    };

    // Le combat, ou une place neuve a la place du plus ancien.
    Slot& slot(uint64_t battle) {
        for (Slot& s : slots_)
            if (s.battle == battle) return s;
        Slot& s = slots_[next_++ % kBattles];
        s = Slot{};
        s.battle = battle;
        return s;
    }

    Slot     slots_[kBattles];
    uint32_t next_ = 0;
};

} // namespace mgmp
