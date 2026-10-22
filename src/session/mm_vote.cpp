// mm_vote.cpp -- voir mm_vote.h.
#include "mm_vote.h"
#include "mm_purse.h"
#include "mm_vote_core.h"

#include "mgmp_addresses.h"
#include "mgmp_follow.h"
#include "mgmp_log.h"
#include "mgmp_marker.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_savefile.h"
#include "mm_menu.h"

#include <windows.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>

namespace mgmp {
namespace {

// --- la carte (offsets : voir mm_vote.h) ------------------------------------------

constexpr uintptr_t kMap_NodeCount = 0x7C;    // u32, MapScreen
constexpr uintptr_t kMap_NodeData  = 0x80;    // MapNode**
constexpr uintptr_t kNode_Seed     = 0x118;
constexpr uintptr_t kNode_Type     = 0x138;
constexpr uintptr_t kNode_Visual   = 0x160;
constexpr uintptr_t kNode_Visited  = 0x168;
constexpr uintptr_t kNode_Map      = 0x170;
constexpr uintptr_t kNode_Button   = 0x178;
constexpr uintptr_t kVisual_X      = 0x70;    // float
constexpr uintptr_t kVisual_Y      = 0x74;
constexpr uintptr_t kBtn_Active    = 0x0E;
constexpr uintptr_t kBtn_Hidden    = 0x0F;
constexpr uintptr_t kBtn_Visible   = 0x10;
constexpr uintptr_t kBtn_HoverOnly = 0x5A;
// Le pion fige (la Nemesis marche) : MapMarker::update efface alors son etape
// et sa destination a chaque image (0x140221FF5) ; un clic n'y survit pas.
constexpr uintptr_t kMarker_Frozen = 0x78;
// L'etat du bouton : kBtn_State / kBtnState_Disabled (mgmp_addresses.h).

// --- les delais -------------------------------------------------------------------

// Le resultat reste affiche : vote_result_show_ms (mm_vote_core).
// Apres le resultat, au plus ce temps de carte qui tourne (pas en pause) pour
// rejouer le clic et y entrer : un filet, la fin normale vient des evenements.
constexpr uint64_t kMoveMapMs      = 30000;
constexpr uint64_t kRefusedShowMs  = 6000;    // invite : « ton vote n'a pas ete pris »
constexpr uint64_t kPublishMs      = 2000;    // l'etat du tour, meme sans changement
constexpr uint64_t kHereMs         = 3000;    // invite : sa presence, meme sans changement
constexpr uint64_t kHereStaleMs    = 8000;    // hote : une presence plus vieille ne compte plus
constexpr uint64_t kPendingMs      = 3000;    // invite : son vote envoye, en attendant l'hote
constexpr uint64_t kMapTickMs      = 500;     // la carte tourne encore
constexpr uint64_t kDismissPickMs  = 12000;   // « Choisir » : la fenetre revient au plus tard
constexpr uint64_t kDismissVotedMs = 8000;    // « Changer »

uint32_t slot_bit(uint8_t slot) { return slot < kVoteSlots ? 1u << slot : 0u; }

struct Here { bool on = false; uint64_t at = 0; };

struct State {
    // La carte de ce joueur.
    void*    map      = nullptr;
    uint64_t map_tick = 0;
    uint64_t popup_seen_at = 0;            // la fenetre du vote ouverte, la derniere fois

    // Hote.
    VoteHost     host;
    std::mt19937 rng;
    bool         rng_ready  = false;
    uint64_t     result_at  = 0;
    bool         applied    = false;      // clic du jeu rejoue
    uint64_t     applied_at = 0;
    uint64_t     result_map_ms = 0;       // temps de carte qui tourne depuis le resultat
    void*        round_map  = nullptr;    // la carte du tour (MapScreen)
    uint64_t     last_pub   = 0;
    uint32_t     pub_present = 0;
    bool         pub_running = false;
    Here         here[kVoteSlots];

    // Invite.
    VoteState  gs;                         // dernier etat recu de l'hote
    bool       have_gs        = false;
    uint64_t   gs_at          = 0;
    VoteBallot pending;                    // son vote envoye, pas encore dans l'etat
    uint64_t   pending_at     = 0;
    uint64_t   result_seen_at = 0;
    int        here_sent      = -1;
    uint64_t   here_sent_at   = 0;
    uint64_t   refused_at     = 0;            // son vote refuse par l'hote...
    uint8_t    refused_reason = 0;            // ... et pourquoi
    bool       notice_closed  = false;        // ... et il a ferme la fenetre qui le dit

    // La fenetre de ce joueur.
    uint8_t    dismiss_round  = 0;
    uint64_t   dismiss_until  = 0;
    uint16_t   dismiss_node   = kVoteNoNode;  // son vote quand il l'a fermee
    uint8_t    result_closed  = 0;            // tour dont il a ferme le resultat

    // Journal.
    uint8_t    told_blocked   = 0;            // « vote retenu » deja ecrit pour ce tour

};

State g;

// Un nouveau tour : ce que ce joueur avait ferme concernait le precedent (les
// numeros de tour reviennent apres 255).
void new_round_seen() {
    g.dismiss_round = 0;
    g.dismiss_until = 0;
    g.dismiss_node  = kVoteNoNode;
    g.result_closed = 0;
    g.told_blocked  = 0;
    g.refused_at    = 0;
}

using FnNodeClick = void (__fastcall*)(void* self);
FnNodeClick o_NodeClick = nullptr;

// --- lire la carte ----------------------------------------------------------------

template <class T> bool rd(const void* base, uintptr_t off, T& out) {
    return base && mem_read((const uint8_t*)base + off, &out, sizeof(out));
}

uint32_t node_count(void* map) {
    uint32_t n = 0;
    if (!rd(map, kMap_NodeCount, n) || n > 4096) return 0;
    return n;
}

void* node_at(void* map, uint32_t index) {
    void* data = nullptr;
    if (index >= node_count(map) || !rd(map, kMap_NodeData, data) || !data) return nullptr;
    void* node = nullptr;
    mem_read((const uint8_t*)data + index * sizeof(void*), &node, sizeof(node));
    return node;
}

bool index_of_node(void* map, void* node, uint32_t& out) {
    const uint32_t n = node_count(map);
    void* data = nullptr;
    if (!n || !rd(map, kMap_NodeData, data) || !data) return false;
    for (uint32_t i = 0; i < n; ++i) {
        void* p = nullptr;
        if (!mem_read((const uint8_t*)data + i * sizeof(void*), &p, sizeof(p))) return false;
        if (p == node) { out = i; return true; }
    }
    return false;
}

uint8_t node_type(void* node) {
    uint32_t t = 0;
    rd(node, kNode_Type, t);
    return t < 256 ? (uint8_t)t : 0;
}

uint32_t node_seed_lo(void* node) {
    uint32_t s = 0;
    rd(node, kNode_Seed, s);
    return s;
}

void* node_map(void* node) {
    void* m = nullptr;
    rd(node, kNode_Map, m);
    return m;
}

bool node_pos(void* node, float& x, float& y) {
    void* vis = nullptr;
    return rd(node, kNode_Visual, vis) && vis && rd(vis, kVisual_X, x) && rd(vis, kVisual_Y, y) &&
           std::isfinite(x) && std::isfinite(y);
}

// La case peut etre choisie : celle que le joueur pourrait cliquer (sans la
// pause de la scene, voir mm_vote.h).
bool node_votable(void* node) {
    const uint8_t type = node_type(node);
    if (type < 3 || type > 17) return false;
    uint8_t visited = 1;
    if (!rd(node, kNode_Visited, visited) || visited) return false;
    void* btn = nullptr;
    if (!rd(node, kNode_Button, btn) || !btn) return false;
    uint8_t active = 0, hidden = 1, visible = 0, hover_only = 1;
    int32_t state = kBtnState_Disabled;
    if (!rd(btn, kBtn_Active, active) || !rd(btn, kBtn_Hidden, hidden) ||
        !rd(btn, kBtn_Visible, visible) || !rd(btn, kBtn_HoverOnly, hover_only) ||
        !rd(btn, kBtn_State, state))
        return false;
    return visible && !hidden && active && !hover_only && state != kBtnState_Disabled;
}

// La carte de ce joueur, si elle est la : elle tourne, ou la fenetre du vote
// la met en pause.
void* live_map() {
    if (!g.map) return nullptr;
    const uint64_t now = GetTickCount64();
    if (g.map_tick && now - g.map_tick < kMapTickMs) return g.map;
    // La fenetre met la carte en pause ; a sa fermeture, la carte peut ne
    // reprendre qu'une image plus tard : un court delai evite de se dire
    // absent le temps d'une image.
    if (mm_vote_popup_open()) g.popup_seen_at = now;
    if (g.popup_seen_at && now - g.popup_seen_at < kMapTickMs && mm_scene_alive("Map")) return g.map;
    return nullptr;
}

bool local_on_map() { return live_map() != nullptr; }

// --- les noms -----------------------------------------------------------------------

std::string safe_name(const char* s) {
    std::string out = s ? s : "";
    for (char& c : out)
        if (c == '[') c = '(';
        else if (c == ']') c = ')';
        else if (c == '\n' || c == '\r') c = ' ';
    return out;
}

void fill_names(std::string* names) {
    const uint8_t self = net_self();
    for (uint8_t i = 0; i < kVoteSlots; ++i)
        names[i] = i == self ? safe_name(mm_local_name()) : mm_peer_name(i);
}

// Le nom d'une case : son type, et sa place quand plusieurs cases du meme type
// sont proposees (« Combat (en haut) »). Les coordonnees sont celles de l'image
// de la case (y vers le bas, comme le SWF).
std::string node_label(void*, uint16_t index, uint8_t type) {
    std::string name = vote_type_name(type);
    void* map = live_map();
    if (!map) return name;
    struct P { uint32_t i; float x, y; };
    P ps[12];
    int n = 0, me = -1;
    const uint32_t count = node_count(map);
    for (uint32_t i = 0; i < count && n < 12; ++i) {
        void* node = node_at(map, i);
        if (!node || node_type(node) != type) continue;
        if (i != index && !node_votable(node)) continue;
        float x = 0, y = 0;
        if (!node_pos(node, x, y)) continue;
        if (i == index) me = n;
        ps[n++] = P{ i, x, y };
    }
    if (n < 2 || me < 0) return name;
    float minx = ps[0].x, maxx = ps[0].x, miny = ps[0].y, maxy = ps[0].y;
    for (int k = 1; k < n; ++k) {
        minx = ps[k].x < minx ? ps[k].x : minx;  maxx = ps[k].x > maxx ? ps[k].x : maxx;
        miny = ps[k].y < miny ? ps[k].y : miny;  maxy = ps[k].y > maxy ? ps[k].y : maxy;
    }
    const bool vertical = (maxy - miny) >= (maxx - minx);
    int rank = 0;
    for (int k = 0; k < n; ++k) {
        if (k == me) continue;
        const float a = vertical ? ps[k].y : ps[k].x, b = vertical ? ps[me].y : ps[me].x;
        if (a < b || (a == b && ps[k].i < ps[me].i)) ++rank;
    }
    const char* word = nullptr;
    char buf[16];
    if (n == 2)      word = vertical ? (rank ? u8"en bas" : u8"en haut") : (rank ? u8"à droite" : u8"à gauche");
    else if (n == 3) {
        static const char* kV[] = { u8"en haut", u8"au milieu", u8"en bas" };
        static const char* kH[] = { u8"à gauche", u8"au milieu", u8"à droite" };
        word = vertical ? kV[rank] : kH[rank];
    } else {
        std::snprintf(buf, sizeof(buf), u8"n°%d", rank + 1);
        word = buf;
    }
    return name + " (" + word + ")";
}

// --- l'hote ---------------------------------------------------------------------------

uint32_t rng_pick(void* ctx, uint32_t n) {
    auto* r = (std::mt19937*)ctx;
    if (!n) return 0;
    return std::uniform_int_distribution<uint32_t>(0, n - 1)(*r);
}

void ensure_rng() {
    if (g.rng_ready) return;
    LARGE_INTEGER qpc{};
    QueryPerformanceCounter(&qpc);
    const uint32_t parts[5] = { (uint32_t)GetTickCount64(), (uint32_t)qpc.LowPart,
                                (uint32_t)qpc.HighPart, (uint32_t)(uintptr_t)&g,
                                (uint32_t)GetCurrentProcessId() };
    std::seed_seq seq(parts, parts + 5);
    g.rng.seed(seq);
    g.rng_ready = true;
}

uint32_t host_voters() {
    return slot_bit(net_self()) | mm_run_peers_mask();
}

// Qui est sur la carte : l'hote d'apres la sienne, les invites d'apres leur
// dernier kVoteHere.
uint32_t host_presence(uint64_t now) {
    const uint8_t self = net_self();
    uint32_t present = local_on_map() ? slot_bit(self) : 0;
    for (uint8_t i = 0; i < kVoteSlots; ++i)
        if (i != self && g.here[i].on && now - g.here[i].at < kHereStaleMs) present |= slot_bit(i);
    return present;
}

void publish_state(uint64_t now, uint8_t refused_slot = kVoteNobody,
                   uint8_t refused_reason = kRefuseNone) {
    VoteState out = g.host.state();
    out.refused_slot   = refused_slot;
    out.refused_reason = refused_reason;
    MmMsg m;
    vote_encode_state(out, m);
    net_send_mm(m);
    g.last_pub    = now;
    g.pub_present = g.host.state().present;
    g.pub_running = g.host.state().running;
}

// Fin du tour (deplacement fait ou abandonne) : tout le monde le sait.
void host_finish(uint64_t now) {
    g.host.finish();
    g.applied       = false;
    g.result_map_ms = 0;
    g.round_map     = nullptr;
    publish_state(now);
}

const char* phase_name(VotePhase p) {
    switch (p) {
        case VotePhase::Voting: return "vote";
        case VotePhase::Result: return "resultat";
        default:                return "aucun";
    }
}

void log_result(const VoteState& s) {
    std::string votes;
    for (uint8_t i = 0; i < kVoteSlots; ++i) {
        if (!(s.voters & slot_bit(i))) continue;
        char buf[48];
        if (s.ballot[i].valid())
            std::snprintf(buf, sizeof(buf), " %u:%u", (unsigned)i, (unsigned)s.ballot[i].node);
        else
            std::snprintf(buf, sizeof(buf), " %u:-", (unsigned)i);
        votes += buf;
    }
    log_line("VOTE", "tour %u : case %u (%s) l'emporte%s -- votes%s",
             (unsigned)s.round, (unsigned)s.winner.node, vote_type_name(s.winner.type),
             s.drawn ? " au tirage au sort" : "", votes.c_str());
}

// Le clic de l'hote sur une case : son vote.
bool host_click(void* node, uint64_t now) {
    void* map = node_map(node);
    uint32_t index = 0;
    if (!index_of_node(map, node, index)) {
        log_line("VOTE", "!! clic sur une case hors de la carte -- laisse au jeu");
        return false;
    }
    const VoteState& s = g.host.state();
    if (s.phase == VotePhase::Result) {
        if (g.told_blocked != s.round) {
            g.told_blocked = s.round;
            log_line("VOTE", "clic de l'hote retenu : le chemin est deja choisi (tour %u)",
                     (unsigned)s.round);
        }
        return true;
    }
    const uint8_t type = node_type(node);
    const bool opened = s.phase == VotePhase::Idle;
    if (!g.host.cast(net_self(), (uint16_t)index, type, host_voters(), now)) {
        log_line("VOTE", "!! vote de l'hote refuse (case %u)", index);
        return true;
    }
    if (opened) {
        new_round_seen();
        g.round_map = map;
    }
    // L'etat publie dit tout de suite qui est la (et si le compte avance).
    g.host.set_present(host_presence(now) | slot_bit(net_self()));
    float x = 0, y = 0;
    const bool pos = node_pos(node, x, y);
    log_line("VOTE", "%s tour %u : l'hote vote pour la case %u (%s)%s",
             opened ? "nouveau" : "", (unsigned)g.host.state().round, index, vote_type_name(type),
             pos ? "" : " -- position introuvable");
    if (pos) log_line("VOTE", "  case %u a (%.0f, %.0f)", index, x, y);
    publish_state(now);
    return true;
}

// Fin du resultat : l'hote fait le clic du jeu sur la case gagnante. Elle a
// ete verifiee au vote (le clic de l'hote vient du jeu, celui d'un invite est
// passe par node_votable) et rien n'a bouge depuis : il suffit qu'elle soit
// toujours la, pas encore visitee. Si la lecture des drapeaux du bouton etait
// fausse, seuls les votes des invites seraient refuses -- le jeu ne serait pas
// bloque pour autant.
void host_apply(void* map, uint64_t now) {
    const VoteState& s = g.host.state();
    void* node = node_at(map, s.winner.node);
    uint8_t visited = 1;
    if (!node || node_type(node) != s.winner.type || !rd(node, kNode_Visited, visited) || visited) {
        log_line("VOTE", "!! tour %u : la case %u n'est plus possible -- pas de deplacement",
                 (unsigned)s.round, (unsigned)s.winner.node);
        host_finish(now);
        return;
    }
    // Le lambda ne lit que sa capture +8, la case (voir mm_vote.h).
    struct FakeCapture { void* vtable; void* node; } cap{ nullptr, node };
    g.applied    = true;
    g.applied_at = now;
    log_line("VOTE", "tour %u : clic du jeu rejoue sur la case %u (%s)",
             (unsigned)s.round, (unsigned)s.winner.node, vote_type_name(s.winner.type));
    o_NodeClick(&cap);
}

void host_pump(uint64_t now) {
    if (!savefile_run_published() || !net_active()) {
        if (g.host.state().phase != VotePhase::Idle) {
            log_line("VOTE", "l'hote n'est plus en aventure -- tour %u abandonne",
                     (unsigned)g.host.state().round);
            host_finish(now);
        }
        return;
    }
    const uint8_t self = net_self();
    g.host.set_never_drop(slot_bit(self));
    bool changed = false;
    const VotePhase before = g.host.state().phase;

    if (before == VotePhase::Voting) {
        if (g.host.keep_voters(host_voters())) {
            log_line("VOTE", "tour %u : un votant a quitte l'aventure (votants 0x%X)",
                     (unsigned)g.host.state().round, g.host.state().voters);
            changed = true;
        }
        g.host.set_present(host_presence(now));
        const uint32_t dropped = g.host.state().dropped;
        ensure_rng();
        if (g.host.tick(now, rng_pick, &g.rng)) changed = true;
        const VoteState& s = g.host.state();
        if (s.dropped != dropped)
            log_line("VOTE", "tour %u : absent depuis 60 s, plus attendu (ecartes 0x%X)",
                     (unsigned)s.round, s.dropped);
        if (s.phase == VotePhase::Result) {
            g.result_at     = now;
            g.applied       = false;
            g.result_map_ms = 0;
            log_result(s);
        } else if (s.phase == VotePhase::Idle) {
            log_line("VOTE", "tour %u abandonne : plus aucun vote", (unsigned)s.round);
            g.round_map = nullptr;
        }
    }

    const VoteState& s = g.host.state();
    if (changed || s.present != g.pub_present || s.running != g.pub_running ||
        (s.phase != VotePhase::Idle && now - g.last_pub >= kPublishMs))
        publish_state(now);
}

void host_on_cast(uint8_t from, const MmMsg& m) {
    VoteCast c;
    if (!vote_decode_cast(m, c)) {
        log_line("VOTE", "!! vote illisible de %u", (unsigned)from);
        return;
    }
    const uint64_t now = GetTickCount64();
    if (!savefile_run_published()) return;
    const uint32_t voters = host_voters();
    if (!(voters & slot_bit(from))) {
        log_line("VOTE", "vote de %u ignore : pas dans l'aventure", (unsigned)from);
        return;
    }
    const VoteState& s = g.host.state();
    if (s.phase == VotePhase::Result) {
        log_line("VOTE", "vote de %u pour la case %u ignore : le chemin est deja choisi",
                 (unsigned)from, (unsigned)c.node);
        publish_state(now, from, kRefuseDecided);
        return;
    }
    // Un vote fait pendant un tour qui n'est plus celui-ci (0 : l'invite ne
    // connaissait aucun tour en cours, il en ouvre un ou rejoint celui-ci).
    if (c.round != 0 && (s.phase != VotePhase::Voting || c.round != s.round)) {
        log_line("VOTE", "vote de %u pour la case %u ignore : fait au tour %u, fini",
                 (unsigned)from, (unsigned)c.node, (unsigned)c.round);
        publish_state(now, from, kRefuseStale);
        return;
    }
    void* map = live_map();
    // Mewgenics Multi : sa carte en pause (ecran des objets, une fenetre du jeu)
    // n'a pas bouge : le vote est pris. live_map reste la presence (le compte
    // a rebours l'attend) et le clic rejoue attend que la carte tourne.
    if (!map && g.map && mm_scene_alive("Map") && !follow_in_node()) map = g.map;
    const char* why = nullptr;
    uint8_t reason = kRefuseNone;
    void* node = nullptr;
    if (!map)                                     { why = "l'hote n'est pas sur la carte"; reason = kRefuseOffMap; }
    else if (node_count(map) != c.node_count)     { why = "pas la meme carte (nombre de cases)"; reason = kRefuseOtherMap; }
    else if (!(node = node_at(map, c.node)))      { why = "case introuvable"; reason = kRefuseOtherMap; }
    else if (node_seed_lo(node) != c.seed_lo)     { why = "pas la meme case (graine)"; reason = kRefuseOtherMap; }
    else if (node_type(node) != c.type)           { why = "pas le meme type de case"; reason = kRefuseOtherMap; }
    else if (!node_votable(node))                 { why = "case pas cliquable chez l'hote"; reason = kRefuseBlocked; }
    if (why) {
        log_line("VOTE", "vote de %u pour la case %u refuse : %s", (unsigned)from,
                 (unsigned)c.node, why);
        publish_state(now, from, reason);
        return;
    }
    const bool opened = s.phase == VotePhase::Idle;
    if (!g.host.cast(from, c.node, c.type, voters, now)) {
        log_line("VOTE", "vote de %u pour la case %u refuse par le tour", (unsigned)from,
                 (unsigned)c.node);
        publish_state(now, from, kRefuseStale);
        return;
    }
    if (opened) {
        new_round_seen();
        g.round_map = map;
    }
    g.host.set_present(host_presence(now) | slot_bit(from));
    log_line("VOTE", "%s tour %u : %u vote pour la case %u (%s)", opened ? "nouveau" : "",
             (unsigned)g.host.state().round, (unsigned)from, (unsigned)c.node,
             vote_type_name(c.type));
    publish_state(now);
}

// --- l'invite -------------------------------------------------------------------------

bool guest_click(void* node, uint64_t now) {
    if (follow_pending_count()) return true;          // l'hote est deja entre : on le suit
    if (g.have_gs && g.gs.phase == VotePhase::Result) {
        if (g.told_blocked != g.gs.round) {
            g.told_blocked = g.gs.round;
            log_line("VOTE", "clic retenu : le chemin est deja choisi (tour %u)",
                     (unsigned)g.gs.round);
        }
        return true;
    }
    void* map = node_map(node);
    uint32_t index = 0;
    if (!index_of_node(map, node, index)) {
        log_line("VOTE", "!! clic sur une case hors de la carte -- retenu");
        return true;
    }
    VoteCast c;
    // Le tour en cours, s'il en voit un ; sinon 0 (il en ouvre un, ou rejoint
    // celui que l'hote vient d'ouvrir).
    c.round      = (g.have_gs && g.gs.phase == VotePhase::Voting) ? g.gs.round : 0;
    c.node       = (uint16_t)index;
    c.node_count = (uint16_t)node_count(map);
    c.seed_lo    = node_seed_lo(node);
    c.type       = node_type(node);
    MmMsg m;
    vote_encode_cast(c, m);
    if (!net_send_mm(m)) {
        log_line("VOTE", "!! vote pour la case %u : envoi impossible", index);
        return true;
    }
    g.pending.node = c.node;
    g.pending.type = c.type;
    g.pending_at   = now;
    log_line("VOTE", "vote envoye a l'hote : case %u (%s)", index, vote_type_name(c.type));
    return true;
}

void guest_send_vote(const VoteBallot& b, uint64_t now) {
    void* map = live_map();
    void* node = map ? node_at(map, b.node) : nullptr;
    if (!node) {
        log_line("VOTE", "!! « Même chemin » : case %u introuvable sur la carte", (unsigned)b.node);
        return;
    }
    guest_click(node, now);
}

void guest_clear() {
    g.have_gs    = false;
    g.gs         = VoteState{};
    g.pending    = VoteBallot{};
    g.here_sent  = -1;
}

void guest_pump(uint64_t now) {
    if (!mm_guest_in_run()) {
        if (g.have_gs || g.pending.valid()) guest_clear();
        g.here_sent = -1;
        return;
    }
    const bool on = local_on_map() && !follow_pending_count();
    if ((int)on != g.here_sent || now - g.here_sent_at >= kHereMs) {
        MmMsg m;
        vote_encode_here(on, m);
        if (net_send_mm(m)) {
            if ((int)on != g.here_sent && (g.here_sent >= 0 || on))
                log_line("VOTE", "ce joueur %s la carte", on ? "est sur" : "n'est plus sur");
            g.here_sent    = on ? 1 : 0;
            g.here_sent_at = now;
        }
    }
    if (g.pending.valid() && now - g.pending_at > kPendingMs) g.pending = VoteBallot{};
}

void guest_on_state(const MmMsg& m) {
    VoteState s;
    if (!vote_decode_state(m, s)) {
        log_line("VOTE", "!! etat du vote illisible");
        return;
    }
    const uint64_t now = GetTickCount64();
    const bool new_round = !g.have_gs || g.gs.round != s.round;
    if (new_round && s.phase != VotePhase::Idle) new_round_seen();
    if (s.phase == VotePhase::Voting && (new_round || g.gs.phase != VotePhase::Voting))
        log_line("VOTE", "tour %u ouvert par l'hote", (unsigned)s.round);
    if (s.phase == VotePhase::Result && (new_round || g.gs.phase != VotePhase::Result)) {
        g.result_seen_at = now;
        log_result(s);
    }
    if (s.phase == VotePhase::Idle && g.have_gs && g.gs.phase != VotePhase::Idle)
        log_line("VOTE", "tour %u fini", (unsigned)s.round);
    g.gs      = s;
    g.gs_at   = now;
    g.have_gs = true;
    const uint8_t self = net_self();
    if (s.refused_slot == self && s.refused_reason != kRefuseNone) {
        // La fenetre du vote le dira (celle du tour, rouverte meme s'il l'avait
        // fermee pour cliquer ; sinon une fenetre a part).
        g.refused_at     = now;
        g.refused_reason = s.refused_reason;
        g.notice_closed  = false;
        g.dismiss_until  = 0;
        g.pending        = VoteBallot{};
        log_line("VOTE", "vote refuse par l'hote : %s", vote_refusal_text(s.refused_reason));
    }
    if (s.phase != VotePhase::Voting ||
        (self < kVoteSlots && g.pending.valid() && s.ballot[self].node == g.pending.node))
        g.pending = VoteBallot{};
}

// --- le detour ------------------------------------------------------------------------

enum class Mode { Off, Host, Guest };

Mode mode() {
    const NetRole r = net_role();
    if (r == NetRole::Host) {
        if (!net_active() || !savefile_run_published()) return Mode::Off;
        if (!mm_run_peers_mask() && g.host.state().phase == VotePhase::Idle) return Mode::Off;
        return Mode::Host;
    }
    if (r == NetRole::Client) return mm_guest_in_run() ? Mode::Guest : Mode::Off;
    return Mode::Off;
}

void __fastcall h_NodeClick(void* self) {
    void* node = nullptr;
    if (self && mem_read((const uint8_t*)self + 8, &node, sizeof(node)) && node) {
        const Mode md = mode();
        const uint64_t now = GetTickCount64();
        if (md == Mode::Host  && host_click(node, now))  return;
        if (md == Mode::Guest && guest_click(node, now)) return;
    }
    o_NodeClick(self);
}

const VoteState* current_state(Mode md) {
    if (md == Mode::Host) return &g.host.state();
    if (md == Mode::Guest && g.have_gs) return &g.gs;
    return nullptr;
}

VoteBallot own_pending(Mode md) {
    return md == Mode::Guest ? g.pending : VoteBallot{};
}

uint32_t shown_remaining(Mode md, const VoteState& s) {
    if (md == Mode::Host || !s.running) return s.remaining_ms;
    const uint64_t since = GetTickCount64() - g.gs_at;
    return since >= s.remaining_ms ? 0 : (uint32_t)(s.remaining_ms - since);
}

void cast_from_popup(Mode md, const VoteBallot& b) {
    const uint64_t now = GetTickCount64();
    if (md == Mode::Host) {
        if (g.host.cast(net_self(), b.node, b.type, host_voters(), now)) {
            log_line("VOTE", "tour %u : l'hote prend le meme chemin, case %u",
                     (unsigned)g.host.state().round, (unsigned)b.node);
            publish_state(now);
        }
        return;
    }
    guest_send_vote(b, now);
}

} // namespace

// ---------------------------------------------------------------------------

bool vote_binding(Target t, void** detour, void*** original) {
    if (t == T_MapNodeClick) {
        *detour   = (void*)&h_NodeClick;
        *original = (void**)&o_NodeClick;
        return true;
    }
    return false;
}

void vote_pump() {
    const uint64_t now = GetTickCount64();
    switch (net_role()) {
        case NetRole::Host:
            host_pump(now);
            break;
        case NetRole::Client:
            guest_pump(now);
            break;
        default:
            if (g.host.state().phase != VotePhase::Idle) g.host.reset();
            if (g.have_gs) guest_clear();
            break;
    }
}

void vote_map_update(void* map) {
    const uint64_t now  = GetTickCount64();
    const uint64_t step = g.map_tick && now > g.map_tick ? now - g.map_tick : 0;
    g.map      = map;
    g.map_tick = now;
    if (net_role() != NetRole::Host || !o_NodeClick) return;
    const VoteState& s = g.host.state();
    if (s.phase == VotePhase::Idle) return;
    // Mewgenics Multi (protocole 57) : pas pendant une case ou le combat de la
    // Nemesis (sa transition laisse la carte tourner, et le rappel de la marche de
    // la Nemesis vient de liberer le pion) : un clic rejoue ferait marcher le pion
    // sous la transition.
    if (follow_inside_node() || follow_node_live()) return;

    // Une autre carte (le chapitre suivant) : le tour etait celui de l'autre.
    if (g.round_map && map != g.round_map) {
        log_line("VOTE", "tour %u abandonne : nouvelle carte", (unsigned)s.round);
        host_finish(now);
        return;
    }
    if (s.phase != VotePhase::Result) return;

    // Le temps ne compte que carte en marche : un menu ou une fenetre par-dessus
    // ne fait pas perdre le choix.
    g.result_map_ms += step < 100 ? step : 100;
    if (g.result_map_ms > kMoveMapMs) {
        log_line("VOTE", "!! tour %u : %s -- le vote reprend", (unsigned)s.round,
                 g.applied ? "le pion n'est entre dans aucune case" : "clic du jeu impossible");
        host_finish(now);
        return;
    }
    void* marker = nullptr;
    uint8_t frozen = 0;
    if (rd(map, kMk_MapMarker, marker) && marker) rd(marker, kMarker_Frozen, frozen);
    if (!g.applied) {
        if (now - g.result_at < vote_result_show_ms(s)) return;
        if (mm_vote_popup_open() || mm_game_prompt_open()) return;
        if (frozen) return;                     // la Nemesis marche : le clic serait efface
        host_apply(map, now);
        return;
    }
    if (frozen) {
        // Fige apres le clic rejoue : sa destination vient d'etre effacee. On le
        // rejouera une fois le pion libre.
        log_line("VOTE", "tour %u : le pion est fige (Nemesis), le clic sera rejoue",
                 (unsigned)s.round);
        g.applied = false;
        return;
    }
    // Le clic est rejoue : le pion marche (destination posee) ou la
    // confirmation du jeu est ouverte (la carte est alors en pause). Ni l'un ni
    // l'autre : il s'est arrete sans entrer -- confirmation refusee, ou la
    // Nemesis sur le chemin.
    if (now - g.applied_at < 100 || mm_game_prompt_open()) return;
    void* target = map;
    if (!marker || !rd(marker, kMk_Target, target)) return;
    if (target) return;
    log_line("VOTE", "tour %u : le pion n'est pas parti (confirmation refusee ?) -- le vote reprend",
             (unsigned)s.round);
    host_finish(now);
}

void vote_on_enter_node() {
    const uint64_t now = GetTickCount64();
    if (net_role() == NetRole::Host) {
        const VoteState& s = g.host.state();
        if (s.phase == VotePhase::Idle) return;
        if (s.phase == VotePhase::Result && g.applied)
            log_line("VOTE", "tour %u : le pion est entre dans la case", (unsigned)s.round);
        else
            log_line("VOTE", "tour %u abandonne : une case est entree pendant le %s",
                     (unsigned)s.round, phase_name(s.phase));
        host_finish(now);
        return;
    }
    g.pending = VoteBallot{};
}

void vote_on_nemesis_fight() {
    if (net_role() != NetRole::Host) return;
    const VoteState& s = g.host.state();
    // Un resultat deja tombe se joue apres le combat (vote_map_update) ; un vote
    // encore ouvert se reglerait pendant le combat, sans les invites.
    if (s.phase != VotePhase::Voting) return;
    log_line("VOTE", "tour %u abandonne : le combat de la Nemesis commence pendant le vote",
             (unsigned)s.round);
    host_finish(GetTickCount64());
}

void vote_on_mm(uint8_t from, const MmMsg& m) {
    const NetRole r = net_role();
    switch (m.a) {
        case kVoteCast:
            if (r == NetRole::Host && from < kVoteSlots && from != net_self()) host_on_cast(from, m);
            break;
        case kVoteHere:
            if (r == NetRole::Host && from < kVoteSlots && from != net_self()) {
                const bool on = m.c != 0;
                if (on != g.here[from].on)
                    log_line("VOTE", "%u %s la carte", (unsigned)from, on ? "est sur" : "n'est plus sur");
                g.here[from].on = on;
                g.here[from].at = GetTickCount64();
            }
            break;
        case kVoteState:
            // Hors de l'aventure (dans sa propre save), l'etat ne le concerne pas.
            if (r == NetRole::Client && from == 0 && mm_guest_in_run()) guest_on_state(m);
            break;
        default:
            log_line("VOTE", "message de vote inconnu (%u) de %u", (unsigned)m.a, (unsigned)from);
            break;
    }
}

// Le layout de la fenetre a part qui dit a un invite que son vote n'a pas ete
// pris, quand il n'a pas de tour a afficher.
constexpr uint32_t kNoticeLayout = 0xFF05;

bool vote_popup(VotePopup& out, uint32_t open_layout) {
    const Mode md = mode();
    if (md == Mode::Off) return false;
    if (md == Mode::Guest && follow_pending_count()) return false;   // emmene dans une case
    if (!local_on_map()) return false;
    const uint8_t self = net_self();
    const VoteState* s = current_state(md);
    const VoteBallot own = own_pending(md);
    VoteLayout lay = (s && s->phase != VotePhase::Idle) ? vote_layout(*s, self, own)
                                                        : VoteLayout::None;
    if (lay == VoteLayout::None) {
        const uint64_t now = GetTickCount64();
        if (md != Mode::Guest || !g.refused_at || g.notice_closed ||
            now - g.refused_at >= kRefusedShowMs)
            return false;
        out.layout = kNoticeLayout;
        out.text   = std::string(u8"Choix du chemin\n\n[c:red]Ton vote n'a pas été pris : ") +
                     vote_refusal_text(g.refused_reason) + u8".[/c]";
        out.yes    = u8"D'accord";
        out.no     = nullptr;
        return true;
    }
    // Pas encore vote : la fenetre ouverte garde ses boutons quand les votes
    // des autres font ou defont un chemin en tete (elle ne clignote pas).
    const VoteLayout was = (VoteLayout)(open_layout & 0xFF);
    if ((open_layout >> 8) == s->round &&
        (was == VoteLayout::Open || was == VoteLayout::Choose) &&
        (lay == VoteLayout::Open || lay == VoteLayout::Choose))
        lay = was;
    const uint64_t now = GetTickCount64();
    if (lay == VoteLayout::Result) {
        const uint64_t at = md == Mode::Host ? g.result_at : g.result_seen_at;
        if (now - at >= vote_result_show_ms(*s)) return false;
        if (g.result_closed == s->round) return false;
    } else if (g.dismiss_round == s->round && now < g.dismiss_until) {
        const uint16_t cur = own.valid() ? own.node : s->ballot[self].node;
        if (cur == g.dismiss_node) return false;          // pas de nouveau vote depuis
    }
    // Ce joueur est sur la carte, puisqu'il la regarde : l'etat de l'hote le
    // dira a son prochain envoi.
    VoteState view = *s;
    view.present |= slot_bit(self);
    if (view.phase == VotePhase::Voting)
        view.running = ((view.voters & ~view.dropped) & ~view.present) == 0;
    std::string names[kVoteSlots];
    fill_names(names);
    out.layout = ((uint32_t)s->round << 8) | (uint32_t)lay;
    out.text   = vote_text(view, self, names, shown_remaining(md, *s), node_label, nullptr, own);
    out.text  += purse_vote_line(true);      // trouvailles et bourse de l'aventure (mm_purse) : la fenetre
                                             // s'ouvre ou se met a jour avec ce texte
    if (md == Mode::Guest && g.refused_at && now - g.refused_at < kRefusedShowMs &&
        lay != VoteLayout::Result)
        out.text += std::string(u8"\n[c:red]Ton vote n'a pas été pris : ") +
                    vote_refusal_text(g.refused_reason) + ".[/c]";
    out.yes    = nullptr;
    out.no     = nullptr;
    switch (lay) {
        case VoteLayout::Open:   out.yes = u8"Même chemin"; out.no = u8"Choisir"; break;
        case VoteLayout::Choose: out.yes = u8"Choisir"; break;
        case VoteLayout::Voted:  out.yes = u8"Changer"; break;
        case VoteLayout::Result: out.yes = u8"D'accord"; break;
        default: break;
    }
    return true;
}

void vote_popup_button(uint32_t layout, bool yes) {
    if (layout == kNoticeLayout) {
        g.notice_closed = true;
        return;
    }
    const Mode md = mode();
    const VoteState* s = current_state(md);
    const uint8_t round = (uint8_t)(layout >> 8);
    const VoteLayout lay = (VoteLayout)(layout & 0xFF);
    if (!s || s->round != round) return;                  // un autre tour depuis
    const uint8_t self = net_self();
    const VoteBallot own = own_pending(md);
    const uint64_t now = GetTickCount64();
    auto dismiss = [&](uint64_t ms) {
        g.dismiss_round = round;
        g.dismiss_until = now + ms;
        g.dismiss_node  = own.valid() ? own.node : (self < kVoteSlots ? s->ballot[self].node : kVoteNoNode);
    };
    switch (lay) {
        case VoteLayout::Open:
            if (yes) {
                const VoteBallot lead = s->phase == VotePhase::Voting
                                            ? vote_leader_excluding(*s, self) : VoteBallot{};
                if (lead.valid()) cast_from_popup(md, lead);
                // Plus de chemin en tete entre-temps : rien de vote, et la
                // fenetre revient (avec « Choisir »).
                return;
            }
            dismiss(kDismissPickMs);
            break;
        case VoteLayout::Choose:
            dismiss(kDismissPickMs);
            break;
        case VoteLayout::Voted:
            dismiss(kDismissVotedMs);
            break;
        case VoteLayout::Result:
            g.result_closed = round;
            break;
        default:
            break;
    }
}

} // namespace mgmp
