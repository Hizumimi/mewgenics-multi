#pragma once
// mgmp_members.h -- Mewgenics Multi : qui joue un combat, et qui dirige chaque
// chat. Pur (aucun appel au jeu ni au reseau), teste par tests/test_members.cpp.
//
// POURQUOI. mgmp faisait jouer chaque combat a TOUS les joueurs de la session :
// la barriere d'entree attendait le CONTROL de chacun, et les chats d'un joueur
// lui revenaient tant qu'il etait connecte. Un invite rentre chez lui en pleine
// aventure (toujours connecte, mais plus dans la copie de l'hote) bloquait donc
// le combat suivant pour toujours -- journal du 17 octobre : « join barrier:
// still holding after 3000 poll(s) ». Et un joueur qui part en plein combat
// laissait ses chats sans personne pour les jouer.
//
// LA REGLE (protocole 48).
//   - L'hote choisit les MEMBRES d'un combat en entrant dans la case : lui, et
//     les invites qui sont dans son aventure (MM_PEER_RUN). Le masque part avec
//     ENTERNODE, pareil pour tous -- c'est sur lui que chacun fait le partage
//     (par proprietaire, ou par place), donc tout le monde trouve le meme.
//   - Un chat dont le proprietaire n'est pas membre est a l'hote.
//   - Pendant le combat les membres ne peuvent que diminuer : un membre qui se
//     deconnecte ou quitte l'aventure en sort, et l'hote prend ses chats
//     (members_takeover). L'hote renvoie alors son CONTROL, qui porte les
//     membres a jour EN MEME TEMPS que sa liste de chats : un invite ne compte
//     jamais une liste sans l'autre (sinon un chat serait compte deux fois, ou
//     zero, et la verification arreterait le combat).
//   - Un joueur qui revient en plein combat n'en redevient pas membre : il
//     regarde jusqu'au combat suivant.
//
// Les masques ont un bit par numero de joueur (1 << id ; kMaxPeers = 4, donc les
// bits 0 a 3). 0 veut dire « inconnu » : on retombe sur la regle de mgmp, tous
// les joueurs connectes.
#include <cstdint>

#include "mgmp_split.h"

namespace mgmp {

constexpr uint8_t kMemberNone = 0xFF;   // pas de joueur (chat de l'IA, ou inconnu)

inline uint8_t member_bit(uint8_t id) { return id < 8 ? (uint8_t)(1u << id) : 0; }
inline bool    member_in(uint8_t mask, uint8_t id) { return id < 8 && ((mask >> id) & 1u) != 0; }

inline uint8_t member_mask_of(const uint8_t* ids, uint32_t n) {
    uint8_t m = 0;
    for (uint32_t i = 0; i < n; ++i) m |= member_bit(ids[i]);
    return m;
}

inline uint32_t member_count(uint8_t mask) {
    uint32_t n = 0;
    for (uint8_t id = 0; id < 8; ++id) n += member_in(mask, id) ? 1u : 0u;
    return n;
}

// La place d'un joueur parmi les membres, par numero croissant ; kMemberNone
// s'il n'en est pas.
inline uint8_t member_pos(uint8_t mask, uint8_t id) {
    if (!member_in(mask, id)) return kMemberNone;
    uint8_t pos = 0;
    for (uint8_t i = 0; i < id; ++i) pos += member_in(mask, i) ? 1 : 0;
    return pos;
}

// Les membres, par numero croissant. Retourne combien ont ete ecrits.
inline uint32_t member_ids(uint8_t mask, uint8_t* out, uint32_t cap) {
    uint32_t n = 0;
    for (uint8_t id = 0; id < 8 && n < cap; ++id)
        if (member_in(mask, id)) out[n++] = id;
    return n;
}

// Hote : les membres maintenant, d'apres ceux de l'entree dans la case. Un
// membre en sort s'il n'est plus connecte, ou (quand l'entree a ete decidee
// d'apres l'aventure) s'il n'est plus dans l'aventure. L'hote reste toujours.
inline uint8_t members_now(uint8_t entry, uint8_t connected, uint8_t host_bit, uint8_t in_run,
                           bool check_run) {
    uint8_t now = entry & (connected | host_bit);
    if (check_run) now &= (uint8_t)(in_run | host_bit);
    return (uint8_t)(now | (entry & host_bit));
}

// Le partage par proprietaire. `owner[i]` : le joueur a qui est le chat i (-1
// si personne ne le connait : une recrue, un chat de l'hote hors liste). Un chat
// humain dont le proprietaire n'est pas membre va a l'hote ; un chat de l'IA
// n'est a personne (chacun le calcule).
inline void members_split_by_owner(const int* owner, const bool* human, uint32_t n, uint8_t members,
                                   uint8_t host, uint8_t* controller) {
    for (uint32_t i = 0; i < n; ++i) {
        if (!human[i]) { controller[i] = kMemberNone; continue; }
        const int o = owner[i];
        controller[i] = (o >= 0 && o < 8 && member_in(members, (uint8_t)o)) ? (uint8_t)o : host;
    }
}

// Le partage par place (mgmp, sans equipe mixte) : les chats humains, dans
// l'ordre du combat, en tranches contigues, une par membre dans l'ordre des
// numeros -- la regle de mgmp_split.h, sur les membres au lieu de la session.
inline void members_split_by_seat(const bool* human, uint32_t n, uint8_t members, uint8_t* controller) {
    uint8_t ids[8] = {};
    const uint32_t P = member_ids(members, ids, 8);
    uint32_t humans = 0;
    for (uint32_t i = 0; i < n; ++i) humans += human[i] ? 1u : 0u;
    uint32_t nth = 0;
    for (uint32_t i = 0; i < n; ++i) {
        controller[i] = kMemberNone;
        if (!human[i]) continue;
        for (uint32_t p = 0; p < P; ++p)
            if (split_owns(split_for(humans, P, p), nth)) { controller[i] = ids[p]; break; }
        ++nth;
    }
}

// Les chats des membres partis passent a l'hote. Retourne combien ont change.
inline uint32_t members_takeover(uint8_t* controller, const bool* human, uint32_t n, uint8_t departed,
                                 uint8_t host) {
    uint32_t moved = 0;
    for (uint32_t i = 0; i < n; ++i) {
        if (!human[i] || controller[i] == host || controller[i] == kMemberNone) continue;
        if (!member_in(departed, controller[i])) continue;
        controller[i] = host;
        ++moved;
    }
    return moved;
}

} // namespace mgmp
