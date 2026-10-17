// test_members.cpp -- Mewgenics Multi : les membres d'un combat (mgmp_members.h).
//
// Le combat ne se joue qu'entre ses membres, choisis par l'hote a l'entree de la
// case ; un membre qui part laisse ses chats a l'hote. Ce test rejoue, pour 2 a
// 4 joueurs, ce que fait chacun dans mgmp_lockstep.cpp : le partage sur les
// membres de l'entree, la reprise par l'hote, l'adoption des membres de l'hote
// par un invite, et la verification (chaque chat humain dans la liste
// d'exactement un membre, sur les listes que chacun a recues).
#include "mgmp_members.h"

#include <cstdio>
#include <cstring>
#include <vector>

using namespace mgmp;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}

constexpr uint8_t kHost = 0;

// Un combat : ses chats (humain ou non, et a qui).
struct Battle {
    std::vector<bool> human;
    std::vector<int>  owner;        // -1 : inconnu (recrue) ou chat de l'IA
};

// Un CONTROL tel qu'il circule.
struct Control {
    bool                 have = false;
    std::vector<uint8_t> cats;
    uint8_t              members = 0;   // l'hote seulement
};

// Un joueur, comme mgmp_lockstep le tient.
struct Peer {
    uint8_t              self = 0;
    bool                 host = false;
    uint8_t              entry = 0;
    uint8_t              members = 0;
    bool                 from_host = false;
    std::vector<uint8_t> controller;
    std::vector<uint8_t> split_ctl;     // le partage de la photo (g.split_controller)
    std::vector<bool>    local;
    Control              got[8];        // les CONTROL recus (le sien compris)

    void split(const Battle& b, uint8_t entry_mask, bool by_owner) {
        entry = entry_mask;
        members = entry_mask;
        from_host = false;
        const uint32_t n = (uint32_t)b.human.size();
        controller.assign(n, kMemberNone);
        bool human[64] = {};
        int  owner[64] = {};
        for (uint32_t i = 0; i < n; ++i) { human[i] = b.human[i]; owner[i] = b.owner[i]; }
        if (by_owner) members_split_by_owner(owner, human, n, entry, kHost, controller.data());
        else          members_split_by_seat(human, n, entry, controller.data());
        split_ctl = controller;
        refresh(b);
    }
    // controller_from_members : le partage de la photo moins les membres sortis,
    // recalcule a chaque fois (jamais cumule).
    void from_members(const Battle& b, uint8_t mask) {
        controller = split_ctl;
        bool human[64] = {};
        for (size_t i = 0; i < b.human.size(); ++i) human[i] = b.human[i];
        const uint8_t gone = (uint8_t)(entry & ~mask);
        if (gone) members_takeover(controller.data(), human, (uint32_t)b.human.size(), gone, kHost);
        refresh(b);
    }
    void refresh(const Battle& b) {
        local.assign(b.human.size(), false);
        for (size_t i = 0; i < b.human.size(); ++i) local[i] = b.human[i] && controller[i] == self;
    }
    Control publish() const {
        Control c;
        c.have = true;
        for (size_t i = 0; i < local.size(); ++i) if (local[i]) c.cats.push_back((uint8_t)i);
        c.members = host ? members : 0;
        return c;
    }
    // Hote : membres maintenant ; reprend les chats des partis.
    uint32_t host_tick(const Battle& b, uint8_t connected, uint8_t in_run, bool check_run) {
        const uint8_t now  = members_now(entry, connected, member_bit(self), in_run, check_run);
        const uint8_t gone = (uint8_t)(members & ~now);
        if (!gone) return 0;
        const std::vector<bool> before = local;
        members = (uint8_t)(members & ~gone);
        from_members(b, members);
        uint32_t moved = 0;
        for (size_t i = 0; i < local.size(); ++i) moved += (!before[i] && local[i]) ? 1u : 0u;
        return moved;
    }
    // Invite : les membres annonces par l'hote.
    void apply_host(const Battle& b, uint8_t mask) {
        if (controller.empty()) return;     // pas dans ce combat (lockstep : snap_current)
        members = mask;
        from_host = true;
        from_members(b, mask);
    }
    // La verification de verify_control : -2 pas encore (une liste manque),
    // -1 bon, sinon le premier chat fautif.
    int tally(const Battle& b) const {
        if (!host && !from_host) return -2;
        // Un invite qui n'est pas membre : la liste de l'hote suffit (il regarde).
        if (!host && !member_in(members, self) && got[kHost].have) return -1;
        for (uint8_t p = 0; p < 8; ++p)
            if (member_in(members, p) && !got[p].have) return -2;
        for (size_t i = 0; i < b.human.size(); ++i) {
            if (!b.human[i]) continue;
            int c = 0;
            for (uint8_t p = 0; p < 8; ++p) {
                if (!member_in(members, p)) continue;
                for (uint8_t k : got[p].cats) if (k == i) ++c;
            }
            if (c != 1) return (int)i;
        }
        return -1;
    }
};

// Envoie le CONTROL de `from` a tous (et a lui-meme).
void broadcast(std::vector<Peer>& peers, size_t from, const Battle& b) {
    const Control c = peers[from].publish();
    for (Peer& p : peers) {
        p.got[peers[from].self] = c;
        if (!p.host && peers[from].host && c.members) p.apply_host(b, c.members);
    }
}

std::vector<Peer> make_peers(std::initializer_list<uint8_t> ids) {
    std::vector<Peer> v;
    for (uint8_t id : ids) {
        Peer p;
        p.self = id;
        p.host = id == kHost;
        v.push_back(p);
    }
    return v;
}

uint32_t local_count(const Peer& p) {
    uint32_t n = 0;
    for (bool l : p.local) n += l ? 1u : 0u;
    return n;
}

} // namespace

int main() {
    printf("-- les masques --\n");
    {
        const uint8_t ids[3] = { 0, 1, 3 };
        const uint8_t m = member_mask_of(ids, 3);
        check(m == 0x0B, "mask of {0,1,3}");
        check(member_count(m) == 3, "three members");
        check(member_pos(m, 0) == 0 && member_pos(m, 1) == 1 && member_pos(m, 3) == 2, "positions by id");
        check(member_pos(m, 2) == kMemberNone, "a non-member has no position");
        uint8_t out[8] = {};
        check(member_ids(m, out, 8) == 3 && out[2] == 3, "ids in order");
        check(!member_in(m, 9) && member_bit(9) == 0, "ids past 7 are nobody");
    }

    printf("\n-- l'hote : qui reste membre --\n");
    {
        // Entree {0,1,2}, tous connectes, tous dans l'aventure.
        check(members_now(0x07, 0x07, 0x01, 0x06, true) == 0x07, "everyone still here");
        // 1 deconnecte.
        check(members_now(0x07, 0x05, 0x01, 0x06, true) == 0x05, "a disconnected member leaves");
        // 2 rentre chez lui (connecte, plus dans l'aventure).
        check(members_now(0x07, 0x07, 0x01, 0x02, true) == 0x03, "a member back home leaves");
        // Entree d'apres les connectes seulement (masque inconnu) : l'aventure ne compte pas.
        check(members_now(0x07, 0x07, 0x01, 0x00, false) == 0x07, "no run check without a host-chosen entry");
        // L'hote reste meme si la liste des connectes ne le dit pas encore.
        check(members_now(0x03, 0x00, 0x01, 0x00, true) == 0x01, "the host always stays");
        // Celui qui revient n'en redevient pas membre.
        check(members_now(0x01, 0x03, 0x01, 0x02, true) == 0x01, "a returning player is not a member again");
    }

    printf("\n-- partage par proprietaire --\n");
    {
        // 5 entrees : 2 chats de l'hote, 1 de l'invite, 1 ennemi (IA), 1 recrue.
        Battle b;
        b.human = { true, true, true, false, true };
        b.owner = { 0, 0, 1, -1, -1 };
        auto peers = make_peers({ 0, 1 });
        for (Peer& p : peers) p.split(b, 0x03, true);
        check(peers[0].controller[0] == 0 && peers[0].controller[2] == 1, "each owner gets their cats");
        check(peers[0].controller[3] == kMemberNone, "an AI cat is nobody's");
        check(peers[0].controller[4] == 0, "an unknown cat (a recruit) is the host's");
        broadcast(peers, 0, b);
        broadcast(peers, 1, b);
        check(peers[0].tally(b) == -1 && peers[1].tally(b) == -1, "both tallies agree");

        // L'invite n'est pas dans l'aventure a l'entree : tout est a l'hote.
        auto solo = make_peers({ 0, 1 });
        solo[0].split(b, 0x01, true);
        check(solo[0].controller[2] == 0 && local_count(solo[0]) == 4, "an absent owner's cat is the host's");
        broadcast(solo, 0, b);
        check(solo[0].tally(b) == -1, "the host alone is enough (no wait for the absent guest)");
    }

    printf("\n-- un membre part en plein combat (2 joueurs) --\n");
    {
        Battle b;
        b.human = { true, true, true, true };
        b.owner = { 0, 0, 1, 1 };
        auto peers = make_peers({ 0, 1 });
        for (Peer& p : peers) p.split(b, 0x03, true);
        broadcast(peers, 0, b);
        broadcast(peers, 1, b);
        check(peers[0].tally(b) == -1, "agreed before the departure");
        // L'invite rentre chez lui : MM_PEER_RUN 0, toujours connecte.
        const uint32_t moved = peers[0].host_tick(b, 0x03, 0x00, true);
        check(moved == 2 && local_count(peers[0]) == 4, "the host takes the guest's two cats");
        check(peers[0].members == 0x01, "the guest is no longer a member");
        broadcast(peers, 0, b);
        check(peers[0].tally(b) == -1, "the host's tally holds with its new claims");
        check(peers[0].host_tick(b, 0x03, 0x02, true) == 0, "coming back mid-battle changes nothing");
    }

    printf("\n-- 3 joueurs : B part, A recoit la mise a jour de l'hote --\n");
    {
        Battle b;
        b.human = { true, true, true, false };
        b.owner = { 0, 1, 2, -1 };
        auto peers = make_peers({ 0, 1, 2 });   // hote, A, B
        for (Peer& p : peers) p.split(b, 0x07, true);
        broadcast(peers, 0, b);
        broadcast(peers, 1, b);
        broadcast(peers, 2, b);
        check(peers[1].tally(b) == -1, "A agrees with everyone");
        // B se deconnecte.
        check(peers[0].host_tick(b, 0x03, 0x06, true) == 1, "the host takes B's cat");
        broadcast(peers, 0, b);                 // CONTROL de l'hote, membres {0,1}
        check(peers[1].members == 0x03 && peers[1].from_host, "A adopts the host's members");
        check(peers[1].controller[2] == 0, "A sees B's cat as the host's");
        check(peers[1].tally(b) == -1, "A's tally: no cat counted twice (B's old list is ignored)");
        check(local_count(peers[1]) == 1, "A keeps its own cat");
    }

    printf("\n-- 3 joueurs : B part avant d'avoir annonce sa liste --\n");
    {
        Battle b;
        b.human = { true, true, true };
        b.owner = { 0, 1, 2 };
        auto peers = make_peers({ 0, 1, 2 });
        for (Peer& p : peers) p.split(b, 0x07, true);
        broadcast(peers, 0, b);                 // l'hote annonce (membres {0,1,2})
        broadcast(peers, 1, b);                 // A annonce ; B n'annonce jamais
        check(peers[1].tally(b) == -2, "A waits for B's list");
        check(peers[0].tally(b) == -2, "the host waits for B's list");
        peers[0].host_tick(b, 0x03, 0x06, true);   // B deconnecte
        // Entre la reprise et sa nouvelle annonce, la liste de l'hote est perimee :
        // c'est pourquoi members_tick annonce avant de verifier, sans rien entre.
        check(peers[0].tally(b) == 2, "the host's own old list would not cover B's cat");
        broadcast(peers, 0, b);
        check(peers[0].tally(b) == -1, "the host's barrier opens without B");
        check(peers[1].tally(b) == -1, "A's barrier opens with the host's update");
    }

    printf("\n-- un joueur qui arrive en plein combat regarde --\n");
    {
        Battle b;
        b.human = { true, true, true };
        b.owner = { 0, 0, 1 };
        auto peers = make_peers({ 0, 1 });
        peers[0].split(b, 0x01, true);          // a l'entree, seul l'hote etait la
        broadcast(peers, 0, b);
        peers[1].split(b, 0x01, true);          // l'ENTERNODE renvoye : membres {0}
        peers[1].apply_host(b, peers[0].members);
        check(local_count(peers[1]) == 0, "the late player drives nothing");
        broadcast(peers, 1, b);
        check(peers[1].tally(b) == -1, "its tally only counts the members");
        check(peers[0].tally(b) == -1, "the host never waits for it");
    }

    printf("\n-- revenu sans avoir quitte le combat (reconnexion) --\n");
    {
        Battle b;
        b.human = { true, true };
        b.owner = { 0, 1 };
        auto peers = make_peers({ 0, 1 });
        for (Peer& p : peers) p.split(b, 0x03, true);
        broadcast(peers, 0, b);
        broadcast(peers, 1, b);
        peers[0].host_tick(b, 0x01, 0x00, true);   // deconnecte : l'hote reprend
        check(local_count(peers[1]) == 1, "before the update, the guest still thinks it drives its cat");
        broadcast(peers, 0, b);                 // lockstep_catchup : l'hote redit son CONTROL
        check(local_count(peers[1]) == 0, "after it, the guest lets go of its cat");
        check(local_count(peers[0]) == 2, "the host drives both");
    }

    printf("\n-- un masque perime de l'hote, puis le bon (relecture du 17 octobre) --\n");
    {
        Battle b;
        b.human = { true, true, true };
        b.owner = { 0, 1, 2 };
        auto peers = make_peers({ 0, 1, 2 });
        for (Peer& p : peers) p.split(b, 0x07, true);
        // Un CONTROL de l'hote avec les membres d'un autre combat (sans A) arrive...
        peers[1].apply_host(b, 0x05);
        check(local_count(peers[1]) == 0, "a stale mask without A takes A's cat away for now");
        // ... puis le vrai : A retrouve son chat (le partage est recalcule, pas cumule).
        peers[1].apply_host(b, 0x07);
        check(local_count(peers[1]) == 1 && peers[1].controller[1] == 1, "the right mask gives it back");
        for (size_t i = 0; i < peers.size(); ++i) broadcast(peers, i, b);
        check(peers[0].tally(b) == -1 && peers[1].tally(b) == -1 && peers[2].tally(b) == -1,
              "everyone agrees afterwards");
    }

    printf("\n-- celui qui regarde n'attend pas la liste de chaque membre --\n");
    {
        Battle b;
        b.human = { true, true, true };
        b.owner = { 0, 1, 2 };
        auto peers = make_peers({ 0, 1, 2 });
        peers[0].split(b, 0x03, true);          // membres : l'hote et A ; B arrive en plein combat
        peers[1].split(b, 0x03, true);
        broadcast(peers, 0, b);
        broadcast(peers, 1, b);
        peers[2].split(b, 0x03, true);          // B : l'ENTERNODE renvoye ; il ne recoit que la liste de l'hote
        peers[2].got[1] = Control{};            // celle de A ne lui est jamais renvoyee
        peers[2].apply_host(b, peers[0].members);
        check(local_count(peers[2]) == 0, "B drives nothing");
        check(peers[2].tally(b) == -1, "B's barrier opens on the host's list alone");
    }

    printf("\n-- partage par place (sans equipe mixte) --\n");
    {
        Battle b;
        b.human = { true, true, false, true, true };
        b.owner = { -1, -1, -1, -1, -1 };
        auto peers = make_peers({ 0, 1, 3 });   // le joueur 2 manque : les places, pas les ids
        for (Peer& p : peers) p.split(b, 0x0B, false);
        check(peers[0].controller[0] == 0 && peers[0].controller[1] == 0, "4 cats over 3: the host takes 2");
        check(peers[0].controller[3] == 1 && peers[0].controller[4] == 3, "then one each, by position");
        for (size_t i = 0; i < peers.size(); ++i) broadcast(peers, i, b);
        check(peers[1].tally(b) == -1 && peers[2].tally(b) == -1, "everyone agrees");
        peers[0].host_tick(b, 0x03, 0x0A, true);   // le joueur 3 se deconnecte
        broadcast(peers, 0, b);
        check(peers[0].controller[4] == 0 && peers[1].tally(b) == -1, "its cat goes to the host, the rest stays put");
    }

    // L'IA du jeu chez l'hote (protocole 52) : les chats d'un absent, et eux seuls.
    auto ai_of = [](const Peer& p, const Battle& b, std::vector<bool>& ai) {
        const uint32_t n = (uint32_t)b.human.size();
        bool human[64] = {}, flags[64] = {};
        int  owner[64] = {};
        for (uint32_t i = 0; i < n; ++i) { human[i] = b.human[i]; owner[i] = b.owner[i]; }
        const uint32_t c = members_ai_cats(p.controller.data(), p.split_ctl.data(), owner, human, n, kHost, flags);
        ai.assign(flags, flags + n);
        return c;
    };

    printf("\n-- l'IA joue les chats d'un absent (par proprietaire) --\n");
    {
        // 2 chats de l'hote, 2 de l'invite, un ennemi, une recrue.
        Battle b;
        b.human = { true, true, true, true, false, true };
        b.owner = { 0, 0, 1, 1, -1, -1 };
        auto peers = make_peers({ 0, 1 });
        for (Peer& p : peers) p.split(b, 0x03, true);
        std::vector<bool> ai;
        check(ai_of(peers[0], b, ai) == 0, "everyone here: the AI plays nothing");
        peers[0].host_tick(b, 0x01, 0x00, true);   // l'invite se deconnecte
        check(ai_of(peers[0], b, ai) == 2 && ai[2] && ai[3], "the guest leaves: the AI plays its two cats");
        check(!ai[0] && !ai[1], "never the host's own cats");
        check(!ai[4], "never an enemy");
        check(!ai[5] && peers[0].local[5], "a recruit with no known owner stays with the host's player");

        // A l'entree, l'invite n'etait pas dans l'aventure : ses chats, des le debut.
        auto solo = make_peers({ 0, 1 });
        solo[0].split(b, 0x01, true);
        check(ai_of(solo[0], b, ai) == 2 && ai[2] && ai[3] && !ai[5], "an owner absent at entry: the AI from the start");
    }

    printf("\n-- l'IA, 3 joueurs : seul l'absent est remplace --\n");
    {
        Battle b;
        b.human = { true, true, true };
        b.owner = { 0, 1, 2 };
        auto peers = make_peers({ 0, 1, 2 });
        for (Peer& p : peers) p.split(b, 0x07, true);
        peers[0].host_tick(b, 0x03, 0x02, true);   // B rentre chez lui
        std::vector<bool> ai;
        check(ai_of(peers[0], b, ai) == 1 && ai[2] && !ai[1], "B's cat to the AI, A keeps playing its own");
        // Chez A, le chat de B est a l'hote (ses decisions arrivent de l'hote) :
        // l'IA ne tourne que chez l'hote (mgmp_lockstep ne fait ce calcul que la).
        broadcast(peers, 0, b);
        check(peers[1].controller[2] == kHost && !peers[1].local[2], "A waits for the host's decisions for B's cat");
    }

    printf("\n-- l'IA, partage par place : un membre parti --\n");
    {
        Battle b;
        b.human = { true, true, true, true };
        b.owner = { -1, -1, -1, -1 };
        auto peers = make_peers({ 0, 1 });
        for (Peer& p : peers) p.split(b, 0x03, false);
        std::vector<bool> ai;
        check(ai_of(peers[0], b, ai) == 0, "no owners known, everyone here: nothing for the AI");
        peers[0].host_tick(b, 0x01, 0x00, true);
        check(ai_of(peers[0], b, ai) == 2 && ai[2] && ai[3] && !ai[0], "the departed seat's cats go to the AI");
    }

    printf("\n%s (%d failure%s)\n", g_fail ? "FAILED" : "PASSED", g_fail, g_fail == 1 ? "" : "s");
    return g_fail ? 1 : 0;
}
