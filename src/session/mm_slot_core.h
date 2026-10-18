#pragma once
// mm_slot_core -- Mewgenics Multi (protocole 58) : la 4e place a 3 joueurs.
//
// L'equipe a toujours 4 places. A deux : 2 chats chacun ; a quatre : 1 chacun.
// A trois, chacun emmene 1 chat et la 4e place va au premier qui met un
// deuxieme chat dans sa boite de depart -- il la garde tant qu'il en a deux.
// Sans volontaire, l'equipe part a 3 chats.
//
// L'hote arbitre (mm_team) : chaque joueur lui dit quand sa boite passe a deux
// chats ou plus, puis en dessous (MM_SLOT a = 1) ; l'hote garde l'ordre des
// demandes et donne la place au premier encore present, a 3 joueurs seulement
// (MM_SLOT a = 2, a tous). Un joueur parti perd sa demande ; revenu, il la
// refait (en dernier).
//
// Sans dependance au jeu ni a Windows : teste par tests/test_slot.cpp.

#include <cstdint>

namespace mgmp {

constexpr uint8_t kSlotNobody = 0xFF;

struct SlotArbiter {
    uint8_t order[8] = {};          // les joueurs qui la veulent, par ordre de demande
    uint8_t count    = 0;
    uint8_t owner    = kSlotNobody; // a qui elle est (kSlotNobody : personne)

    bool wants(uint8_t p) const {
        for (uint8_t i = 0; i < count; ++i)
            if (order[i] == p) return true;
        return false;
    }

    // Sa boite a deux chats ou plus. Une demande deja faite garde son rang.
    void claim(uint8_t p) {
        if (p >= 8 || wants(p) || count >= 8) return;
        order[count++] = p;
    }

    // Sa boite n'a plus deux chats, ou il est parti.
    void release(uint8_t p) {
        uint8_t n = 0;
        for (uint8_t i = 0; i < count; ++i)
            if (order[i] != p) order[n++] = order[i];
        count = n;
    }

    void reset() { count = 0; owner = kSlotNobody; }

    // `players` en session, `present` leur masque (bit = numero). Les absents
    // perdent leur demande ; a 3 joueurs, la place va a la premiere demande ;
    // sinon a personne. Vrai si elle change de main.
    bool settle(int players, uint32_t present) {
        uint8_t n = 0;
        for (uint8_t i = 0; i < count; ++i)
            if (order[i] < 32 && (present & (1u << order[i]))) order[n++] = order[i];
        count = n;
        const uint8_t next = (players == 3 && count) ? order[0] : kSlotNobody;
        const bool changed = next != owner;
        owner = next;
        return changed;
    }
};

// Le nombre de chats qu'emmene le joueur `peer`, a `players` joueurs, la 4e place
// etant a `owner`. Seul (ou sans session) : toute l'equipe (`team_size`).
inline int slot_quota(uint8_t peer, int players, uint8_t owner, int team_size) {
    if (players <= 1) return team_size;
    if (players == 2) return 2;
    if (players == 3) return peer == owner ? 2 : 1;
    return 1;
}

} // namespace mgmp
