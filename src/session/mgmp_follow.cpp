// mgmp_follow.cpp -- see mgmp_follow.h.

#include "mgmp_catsync.h"
#include "mgmp_choice.h"
#include "mgmp_invsync.h"
#include "mgmp_nodehash.h"
#include "mgmp_runhist.h"
#include "mgmp_follow.h"
#include "mgmp_marker.h"     // Mewgenics Multi : le pion arrive sur la case suivie
#include "mm_rungear.h"      // Mewgenics Multi : les objets en aventure
#include "mgmp_savefile.h"   // savefile_in_coop (Mewgenics Multi)
#include "mgmp_lockstep.h"
#include "mgmp_net.h"
#include "mgmp_config.h"
#include "mgmp_tuning.h"
#include "mgmp_mem.h"
#include "mgmp_log.h"
#include "mgmp_addresses.h"  // T_NodeEnd (Mewgenics Multi)
#include "mgmp_hooks.h"      // hooks_is_live (Mewgenics Multi : la fin de case)

#include <windows.h>
#include <cstdio>
#include <cstring>

namespace mgmp {
namespace {

// MapScreen's node vector, read off MapNode::Click and cross-checked against
// every other MapScreen function that walks it (update, generate_map,
// generate_auto_connections, generate_bonus_nodes, load_map, select_boss_level,
// TickNemesis all use the same +124/+128 pair). Standard CustomVector shape:
// { u32 capacity @ 0x78, u32 count @ 0x7C, MapNode** data @ 0x80 }.
constexpr uintptr_t kMap_NodeCount = 124;
constexpr uintptr_t kMap_NodeData  = 128;

// MapNode+0x118: the node's own 32-byte xoshiro256 seed, written by
// MapScreen::generate_map and copied into TLS+0x178 by EnterNode.
// MapNode+0x138: MapNodeType, from MapNode::str_to_type.
constexpr uintptr_t kNode_Seed = 0x118;
constexpr uintptr_t kNode_Type = 0x138;

// Mewgenics Multi : la maison (MapNodeType 4), et MapScreen+0x1E7 -- la maison
// prise avant la fin de la carte (voir kEnterEarlyHome dans mgmp_proto.h).
// Pose a 1 par la confirmation POPUP_TRY_RETURN_HOME_EARLY (0x140228341), a 0
// par le clic ordinaire (0x140228875), lu par EnterNode (0x1403927E3).
constexpr uint32_t  kNodeType_Home = 4;
constexpr uintptr_t kMap_EarlyHome = 0x1E7;

// MapNode::str_to_type, in enum order. 1 and 18 are three-character names whose
// string constants IDA did not render; they are not battle types and nothing
// here depends on knowing them.
const char* node_type_name(uint32_t t) {
    static const char* kNames[] = {
        "none", "?1", "enter", "exit", "home", "battle", "hard", "miniboss",
        "boss", "event", "optional_event", "special_event", "shop", "treasure",
        "furniturebox", "foodbox", "bonus", "empty", "?18"
    };
    return t < (sizeof(kNames) / sizeof(kNames[0])) ? kNames[t] : "?";
}

struct State {
    bool     on             = false;
    bool     is_client      = false;

    void*    map            = nullptr;   // latest MapScreen*, from its update

    // NODES THE HOST HAS ENTERED AND THIS PEER HAS NOT -- A QUEUE, NOT A SLOT.
    //
    // It was one slot, and a second ENTERNODE overwrote it in silence. That is
    // not a race to tolerate: it is the client SKIPPING A WHOLE NODE of the
    // run, and it happened live on 2026-08-25. The host entered node 6 (an
    // event), resolved it and walked on to node 17 faster than this peer's map
    // tick came round; node 17 replaced node 6 in the slot; the client never
    // entered node 6, never built its option list, and never applied the event.
    // Its log says `<- host entered node 6` with no `following host into node
    // 6` after it, and there was no other line anywhere to say a node had been
    // dropped.
    //
    // The run was already wrong at that point. What made it unrecoverable was
    // that node 6's CHOICE stayed held and landed on node 3 two nodes later --
    // see ChoiceMsg::node_seed.
    //
    // A queue is the whole fix, because the peers do not have to be on the same
    // node at the same moment -- only to walk the same nodes in the same order.
    // The battle layer already assumes exactly that: the join barrier exists so
    // a peer can arrive at a fight late. Trailing the host by a node is the same
    // situation one screen earlier.
    static constexpr uint32_t kMaxPendingNodes = 8;
    struct PendingNode {
        uint32_t index = 0;
        uint32_t type  = 0;
        uint32_t count = 0;
        uint64_t seed  = 0;
        uint64_t at    = 0;         // tick the host's choice arrived
        bool     held  = false;     // ...and whether we have said we are holding
        uint8_t  flags = 0;         // EnterNodeMsg::flags (kEnterEarlyHome)
        uint32_t epoch = 0;         // Mewgenics Multi : son rang parmi les ENTERNODE recus (follow_epoch)
    };
    PendingNode pending_q[kMaxPendingNodes];
    uint32_t    pending_head  = 0;
    uint32_t    pending_count = 0;

    // Mewgenics Multi (invite) : les ENTERNODE recus depuis le lancement, gardes
    // ou refuses. Une poussee gardee (chats, sac, historique) porte la valeur
    // qu'il avait a son arrivee : arrivee avant l'ENTERNODE de rang E, elle est
    // de l'etat d'avant cette case ; arrivee apres, de l'etat d'apres -- voir
    // follow_epoch.
    uint32_t    epoch = 0;

    // The node this peer is standing in, remembered so a peer that connects
    // afterwards can be told where the run is. ENTERNODE used to be published
    // once, at the moment of entry, and nothing kept it -- so a reconnecting
    // peer loaded the save, landed on the map and waited forever for a message
    // that had already been sent. Measured 2026-08-26.
    bool     have_here      = false;
    uint32_t here_index     = 0;

    // Last tick MapScreen::update ran. See follow_on_map: this is what tells
    // the save flush whether the run is between nodes or inside one.
    uint64_t map_tick       = 0;

    // Mewgenics Multi (hote) : dans une case en ce moment. Mis a l'entree,
    // efface quand la carte tourne de nouveau (plus d'une seconde apres
    // l'entree : la carte peut encore tourner une image ou deux pendant la
    // transition). have_here, lui, reste sur la derniere case pour toujours.
    bool     inside         = false;
    uint64_t inside_at      = 0;

    // Mewgenics Multi : la case que CE joueur joue en ce moment (l'hote : celle
    // ou il est entre ; l'invite : celle ou il a suivi), de l'EnterNode a la fin
    // de case (0x140392FC0, T_NodeEnd) puis l'image de carte suivante. La carte
    // tourne encore pendant la transition d'entree (« EventInTransition »,
    // 0.25 s, puis le rappel qui l'arrete et cree la scene de la case), et par
    // moments pendant un evenement : follow_on_map n'y voit pas une case. Rien
    // de l'etat d'APRES la case ne doit s'appliquer avant sa fin ici --
    // l'evenement rejoue le choix de l'hote par-dessus.
    //   Les cases 5 a 15 (combats, evenements, boutique, tresor, boites)
    // passent toutes par la fin de case ; les autres (sortie vers le chapitre
    // suivant, maison...) jamais : pour elles, et sans le crochet, la premiere
    // image de carte plus d'une seconde apres l'entree. La fin de case que
    // MapScreen::update appelle lui-meme (Espace sur un pion au repos,
    // 0x14038F4EC) ne compte pas, sauf dans l'image de l'entree.
    bool     live           = false;
    bool     live_ended     = false;
    bool     live_waits_end = false;  // une case 5 a 15, crochet de fin de case en place
    uint32_t live_epoch     = 0;      // invite : le rang de son ENTERNODE
    uint64_t live_at        = 0;
    uint32_t live_end_ticks = 0;
    uint32_t live_entry_seq = 0;      // l'appel de MapScreen::update ou elle a commence
    uint32_t map_ticks      = 0;      // les images de carte, comptees
    uint32_t map_update_seq = 0;      // les appels de MapScreen::update (h_MapUpdate)
    bool     in_map_update  = false;  // ... dans l'original en ce moment
    uint64_t map_gap_at     = 0;      // la derniere image de carte apres un arret

    // A jump asked for by the debug panel, applied at the tail of the next
    // MapScreen::update -- the same site the follow path enters nodes from.
    bool     jump_pending   = false;
    uint32_t jump_index     = 0;
    uint32_t here_count     = 0;
    uint32_t here_type      = 0;
    uint64_t here_seed      = 0;

    // Mewgenics Multi : la prochaine case est la premiere depuis qu'une
    // aventure a ete chargee chez ce joueur (kNodeHashRngFresh) -- pose au
    // debut d'une session, quand une aventure commence chez l'hote et quand
    // une copie arrive chez l'invite (follow_forget_map). Pas a chaque
    // nouvelle carte : apres une sortie (chapitre suivant), les deux flux
    // viennent de la graine de la case de sortie et doivent s'accorder.
    bool     next_fresh     = true;

    uint32_t entered        = 0;         // nodes followed, for the shutdown line
    uint32_t published      = 0;
    uint32_t suppressed     = 0;
    bool     warned_no_map  = false;

    CRITICAL_SECTION cs;
    bool cs_ready = false;
};

State g;

struct Guard {
    Guard()  { if (g.cs_ready) EnterCriticalSection(&g.cs); }
    ~Guard() { if (g.cs_ready) LeaveCriticalSection(&g.cs); }
};

// --- the pending-node queue -------------------------------------------------
//
// All three are called with the Guard held. `front` is only valid while
// `pending_any()` is true.

bool pending_any() { return g.pending_count != 0; }

State::PendingNode& pending_front() {
    return g.pending_q[g.pending_head];
}

void pending_pop() {
    if (!g.pending_count) return;
    g.pending_head = (g.pending_head + 1) % State::kMaxPendingNodes;
    --g.pending_count;
}

void pending_clear() { g.pending_head = 0; g.pending_count = 0; }

bool read_nodes(void* map, uint32_t& count, void*& data) {
    count = 0; data = nullptr;
    if (!map) return false;
    if (!mem_read((const uint8_t*)map + kMap_NodeCount, &count, sizeof(count))) return false;
    if (!mem_read((const uint8_t*)map + kMap_NodeData,  &data,  sizeof(data)))  return false;
    // A map with no nodes, or an implausible count, means we are reading the
    // wrong object -- say nothing rather than index into it.
    return data != nullptr && count > 0 && count < 4096;
}

void* node_at(void* map, uint32_t index) {
    uint32_t count = 0; void* data = nullptr;
    if (!read_nodes(map, count, data) || index >= count) return nullptr;
    void* node = nullptr;
    if (!mem_read((const uint8_t*)data + index * sizeof(void*), &node, sizeof(node))) return nullptr;
    return node;
}

bool index_of_node(void* map, void* node, uint32_t& out) {
    uint32_t count = 0; void* data = nullptr;
    if (!read_nodes(map, count, data)) return false;
    for (uint32_t i = 0; i < count; ++i) {
        void* p = nullptr;
        if (!mem_read((const uint8_t*)data + i * sizeof(void*), &p, sizeof(p))) return false;
        if (p == node) { out = i; return true; }
    }
    return false;
}

uint32_t node_type(void* node) {
    uint32_t t = 0;
    if (node) mem_read((const uint8_t*)node + kNode_Type, &t, sizeof(t));
    return t;
}

uint64_t node_seed0(void* node) {
    uint64_t s = 0;
    if (node) mem_read((const uint8_t*)node + kNode_Seed, &s, sizeof(s));
    return s;
}

} // namespace

// ---------------------------------------------------------------------------

void follow_init() {
    if (!g.cs_ready) { InitializeCriticalSection(&g.cs); g.cs_ready = true; }
    g.on        = config().net_follow;
    g.is_client = (net_role() == NetRole::Client);
    g.map       = nullptr;
    g.next_fresh = true;         // Mewgenics Multi : la premiere case de la session
    pending_clear();
    if (!g.on) { log_line("FOLLOW", "map following disabled by net_follow = 0"); return; }
    log_line_lvl(LogLevel::Trace, "FOLLOW", "armed -- %s",
             g.is_client ? "following the host's map choices; local map input is suppressed"
                         : "publishing this peer's map choices");
    if (config().net_follow_delay_ms && g.is_client)
        log_line("FOLLOW", "!! TEST SETTING ACTIVE: net_follow_delay_ms = %u. This peer"
                           " will lag every node by that long ON PURPOSE, to exercise"
                           " the join barrier. Set it to 0 for a real session.",
                 config().net_follow_delay_ms);
    else if (config().net_follow_delay_ms)
        log_line("FOLLOW", "net_follow_delay_ms = %u is set but this peer is the host,"
                           " which never follows -- it will have no effect",
                 config().net_follow_delay_ms);
}

void follow_shutdown() {
    if (!g.on) return;
    log_line_lvl(LogLevel::Trace, "FOLLOW",
             "done: %u published, %u followed, %u local click(s) suppressed",
             g.published, g.entered, g.suppressed);
    g.on = false;
    if (g.cs_ready) { DeleteCriticalSection(&g.cs); g.cs_ready = false; }
}

// Mewgenics Multi : seulement dans la copie de l'aventure de l'hote. Dans sa
// propre save, la carte de l'invite est a lui.
bool follow_suppresses_local_input() { return g.on && g.is_client && savefile_in_coop(); }

// Where this peer is now. Recorded on BOTH peers -- the host so it can tell a
// joiner, the client so its own log says where it thinks it is.
void remember_node(uint32_t index, uint32_t count, uint32_t type, uint64_t seed) {
    const bool moved = !g.have_here || g.here_seed != seed;
    g.have_here  = true;
    g.here_index = index;
    g.here_count = count;
    g.here_type  = type;
    g.here_seed  = seed;

    // Tell the choice layer where the run now is. It holds a choice that has
    // not found its screen yet, and a choice belongs to exactly one node -- see
    // ChoiceMsg::node_seed for the run this cost.
    //
    // The seed is PUSHED rather than pulled. mgmp_choice could ask
    // follow_here_seed() when it applies, but that would have it take this
    // module's lock while holding its own, and this call already runs the other
    // way round -- the classic inversion. Handing the value over means neither
    // module ever waits on the other.
    if (moved) choice_on_node_entered(seed);

    // The meta layer's own hash, sampled at the one moment both peers reach
    // identically: immediately before EnterNode. Deliberately taken on BOTH
    // sides from the same function, because a hash whose two halves are
    // computed at different points in the frame compares two different things
    // and reports a mismatch that is entirely ours.
    //
    // Note this is BEFORE the publishes below, which is what makes the two
    // comparable: the host describes the run it is about to push, and the
    // client -- which applied that push when it popped, ahead of its own map
    // tick -- describes the same bytes.
    //
    // Mewgenics Multi : la premiere case d'une carte qui vient d'etre chargee
    // chez ce joueur (une aventure qui commence, la copie de l'invite, une
    // reprise) ne compare pas le flux : il vient du chargement, pas d'une case.
    if (moved) {
        const bool fresh = g.next_fresh;
        g.next_fresh = false;
        nodehash_on_node(seed, index, fresh);
    }
}

// Mewgenics Multi : ce joueur entre dans une case de ce type (voir `live`).
static void start_live(uint32_t type, uint32_t epoch) {
    g.live           = true;
    g.live_ended     = false;
    g.live_epoch     = epoch;
    g.live_at        = GetTickCount64();
    g.live_entry_seq = g.map_update_seq;
    g.live_waits_end = type >= 5 && type <= 15 && hooks_is_live(T_NodeEnd);
}

bool follow_on_enter_node(void* map_screen, void* node, bool* sent) {
    if (sent) *sent = false;
    if (!g.on || !net_active() || !map_screen || !node) return true;

    Guard guard;
    g.map = map_screen;                    // the click proves the pointer is live

    uint32_t index = 0;
    bool     known = index_of_node(map_screen, node, index);
    uint32_t type  = node_type(node);

    // --- the client: swallow it ------------------------------------------
    //
    // The host owns the run, so a click here is a second opinion nobody asked
    // for -- and acting on it would put the two peers on different nodes, which
    // no amount of battle-layer lockstep can recover from. Swallowing rather
    // than disabling the UI keeps the change to one function.
    // Mewgenics Multi : dans sa propre save, la carte est a l'invite (il ne
    // publie rien pour autant : il n'est pas l'hote).
    if (g.is_client && !savefile_in_coop()) return true;
    if (g.is_client) {
        ++g.suppressed;
        log_line("FOLLOW", "suppressed local map input: node %u (%s) -- the host"
                           " drives the run",
                 known ? index : 0xFFFFFFFFu, node_type_name(type));
        return false;
    }

    // --- the host: publish it --------------------------------------------
    if (!known) {
        // Nothing to send that the client could resolve. Do not block the host
        // over it: a run that continues single-player is better than one that
        // stops, and the client will notice it is on a different node the
        // moment a battle starts and the hashes disagree.
        log_line("FOLLOW", "!! entered a node that is not in MapScreen's vector"
                           " -- cannot publish it; the client will not follow");
        return true;
    }

    uint32_t count = 0; void* data = nullptr;
    read_nodes(map_screen, count, data);

    // Battle identity first of all, and before the publish: from here on this
    // peer's ACTION/HASH/CONTROL name this node, and the client establishes the
    // same id from the same field of the same node in follow_map_update.
    const uint64_t seed = node_seed0(node);
    lockstep_enter_battle(seed);
    remember_node(index, count, type, seed);
    g.inside    = true;
    g.inside_at = GetTickCount64();
    start_live(type, 0);

    // Cats FIRST, node second. TCP is ordered and the client applies a CATDATA
    // the moment it pops, while it defers the node entry to the tail of
    // MapScreen::update -- so sending in this order is what guarantees the
    // battle is built from the host's cats rather than from the client's stale
    // ones. Reversing these two lines reintroduces the bug this closes.
    // Mewgenics Multi : en aventure ensemble, la poussee d'entree est entiere
    // (mm_rungear) -- ce qu'un invite aurait de different est remis avant la case.
    rungear_before_node_push();
    catsync_publish("entering a node");
    // ...and the other half of the same equip. Order between these two does not
    // matter the way it does against ENTERNODE -- an equip has already moved
    // the item out of the inventory and onto the cat by the time either is
    // serialized, so neither push depends on the other -- but both must land
    // BEFORE the node, and the client applies each the moment it pops.
    invsync_publish("entering a node");
    // ...and the used-event list, which is the third piece of run state and the
    // one nothing pushed until now. Same ordering requirement as the other two:
    // BEFORE the node, because it is what the next event will be rolled from.
    runhist_publish("entering a node");
    // Mewgenics Multi : la revision de l'aventure avance (les objets en
    // aventure, mm_rungear) -- apres les poussees qu'elle couvre, avant la case.
    rungear_on_node_entered(seed);

    EnterNodeMsg m{};
    m.index      = index;
    m.node_count = count;
    m.type       = type;
    m.seed0      = seed;
    // Mewgenics Multi : la maison prise avant la fin de la carte (kEnterEarlyHome).
    uint8_t early = 0;
    if (type == kNodeType_Home &&
        mem_read((const uint8_t*)map_screen + kMap_EarlyHome, &early, sizeof(early)) && early)
        m.flags |= kEnterEarlyHome;
    if (net_send_enter_node(m)) {
        ++g.published;
        if (sent) *sent = true;
        log_line("FOLLOW", "-> node %u/%u (%s) seed0=%016llx%s",
                 index, count, node_type_name(type), (unsigned long long)m.seed0,
                 (m.flags & kEnterEarlyHome) ? " (retour anticipe)" : "");
    }
    return true;
}

// A peer connected or reconnected: tell it which node the run is standing in.
//
// Without this the whole save/cats/inventory catch-up leaves the joiner on the
// map with nothing to do, because ENTERNODE is otherwise published exactly once
// -- at the instant of entry -- and a peer that was not connected then never
// hears about it. That was the "client loads the save and the battle never
// activates" report.
bool follow_on_map() {
    if (!g.map_tick) return false;
    // 250 ms: MapScreen::update does not tick on the frame a node is entered,
    // and a single missed frame is not a reason to refuse a flush. A quarter
    // second is far shorter than any node and far longer than any frame.
    return (GetTickCount64() - g.map_tick) < 250;
}

uint32_t follow_node_count() {
    if (!g.map) return 0;
    uint32_t count = 0; void* data = nullptr;
    if (!read_nodes(g.map, count, data)) return 0;
    return count;
}

bool follow_node_info(uint32_t index, uint32_t& type, uint64_t& seed0,
                      const char** type_name) {
    if (!g.map) return false;
    void* node = node_at(g.map, index);
    if (!node) return false;
    type  = node_type(node);
    seed0 = node_seed0(node);
    if (type_name) *type_name = node_type_name(type);
    return true;
}

bool follow_current_node(uint32_t& index) {
    if (!g.have_here) return false;
    index = g.here_index;
    return true;
}

// Deliberately unlocked. It is one naturally-aligned u64 written by the game
// thread in remember_node and read by the game thread when the host stamps a
// CHOICE, so the lock would only buy a lock-ordering hazard against the module
// that wants the value. See remember_node for the other half of that argument.
uint64_t follow_here_seed() { return g.have_here ? g.here_seed : 0; }

// --- where the run is standing, read from the GAME rather than remembered ----
//
// follow_current_node above reports the node this MODULE watched somebody
// enter. That is the wrong answer for the case that matters most: a run
// reloaded from disk has entered nothing this session, so it reports nothing --
// and a reloaded run parked on an unresolved node is exactly when you need to
// be told which node that is.
//
// glaiel::MapNode::Click sets the current node with
//
//     *(MapNode+0x170 -> MapScreen) + 0xA0) + 0x60 = node
//
// and guards its own entry on the same read. MapScreen+0xA0 is the map MARKER
// (sub_14038DE60 calls MapMarker::CanReachNode on it and derives a facing from
// it), and it carries two node slots: +0x50, which is where the marker IS, and
// +0x60, which is what Click selected.
//
// BOTH ARE VALIDATED AGAINST THE NODE VECTOR RATHER THAN TRUSTED. index_of_node
// only reports an index when the pointer is genuinely an element of
// MapScreen+0x80, so a wrong offset yields "unknown" instead of a confident
// wrong number -- the same rule the per-turn state hash follows. That is what
// makes reading two undocumented slots an acceptable risk here.
constexpr uintptr_t kMap_Marker      = 0xA0;
constexpr uintptr_t kMarker_AtNode   = 0x50;
constexpr uintptr_t kMarker_Selected = 0x60;

bool read_marker_node(uintptr_t slot, uint32_t& index) {
    if (!g.map) return false;
    const void* marker = nullptr;
    if (!mem_read((const uint8_t*)g.map + kMap_Marker, &marker, sizeof(marker)) || !marker)
        return false;
    void* node = nullptr;
    if (!mem_read((const uint8_t*)marker + slot, &node, sizeof(node)) || !node)
        return false;
    return index_of_node(g.map, node, index);
}

bool follow_marker_node(uint32_t& index)   { return read_marker_node(kMarker_AtNode,   index); }
bool follow_selected_node(uint32_t& index) { return read_marker_node(kMarker_Selected, index); }

void follow_request_jump(uint32_t index) {
    g.jump_pending = true;
    g.jump_index   = index;
}

void follow_catchup(uint8_t peer) {
    if (!g.on || g.is_client || !net_active()) return;

    Guard guard;
    if (!g.have_here) {
        log_line("FOLLOW", "peer %u joined before this peer entered any node --"
                           " nothing to catch it up to; the next node is published"
                           " normally", (unsigned)peer);
        return;
    }

    EnterNodeMsg m{};
    m.index      = g.here_index;
    m.node_count = g.here_count;
    m.type       = g.here_type;
    m.seed0      = g.here_seed;
    if (!net_send_enter_node_to(peer, m)) return;

    log_line("FOLLOW", "-> re-sent the CURRENT node %u/%u (%s) seed0=%016llx to"
                       " peer %u (it joined after we entered it)",
             m.index, m.node_count, node_type_name(m.type),
             (unsigned long long)m.seed0, (unsigned)peer);
}

bool follow_inside_node() { return g.inside; }

uint32_t follow_epoch() {
    Guard guard;
    return g.epoch;
}

uint32_t follow_note_arrival() {
    // Le thread du jeu seulement (la boucle des messages, mgmp_lockstep) ; le
    // verrou ne coute rien et garde le compteur juste si cela changeait.
    static volatile LONG s_arrival = 0;
    return (uint32_t)InterlockedIncrement(&s_arrival);
}

uint32_t follow_apply_bound() {
    if (!g.on) return UINT32_MAX;
    Guard guard;
    if (g.pending_count) return pending_front().epoch;
    // Dans la case suivie : ce qui est arrive apres son ENTERNODE attend sa fin.
    if (g.live && g.is_client) return g.live_epoch;
    return UINT32_MAX;
}

uint32_t follow_pending_count() {
    if (!g.on) return 0;
    Guard guard;
    return g.pending_count;
}

void follow_drop_pending() {
    Guard guard;
    if (g.pending_count)
        log_line("FOLLOW", "dropped %u queued node(s) from before the host's save that "
                           "just arrived -- that save already has them",
                 g.pending_count);
    pending_clear();
    g.next_fresh = true;         // une copie neuve : sa premiere case est fraiche
    g.live       = false;
}

void follow_forget_map() {
    Guard guard;
    g.next_fresh = true;
    g.live       = false;
}

void follow_on_node_end() {
    Guard guard;
    if (!g.live) return;
    // Appelee par MapScreen::update lui-meme (Espace sur un pion au repos) apres
    // l'image de l'entree : ce n'est pas la fin de cette case.
    if (g.in_map_update && g.map_update_seq != g.live_entry_seq) {
        static bool said = false;
        if (!said) {
            said = true;
            log_line("FOLLOW", "end of node called from the map's own update (Space?) while a node "
                               "is still being entered -- not taken as its end");
        }
        return;
    }
    g.live_ended     = true;
    g.live_end_ticks = g.map_ticks;
}

void follow_note_map_update(bool inside_original) {
    Guard guard;
    if (inside_original) ++g.map_update_seq;
    g.in_map_update = inside_original;
}

bool follow_node_live() {
    Guard guard;
    return g.live;
}

void follow_on_message(const EnterNodeMsg& m) {
    if (!g.on) return;
    Guard guard;
    if (!g.is_client) {
        // Symmetric message, asymmetric authority: a host that receives one is
        // talking to a peer that thinks it is the host too.
        log_line("FOLLOW", "!! received a node choice while hosting -- both peers"
                           " believe they own the run");
        return;
    }
    // Mewgenics Multi : un invite qui joue dans sa propre save et ne rejoint
    // pas l'aventure n'a rien a suivre. Les cases s'empileraient (et la file
    // pleine les marquerait « sautees » pour la couche des choix) ; il recevra
    // la case actuelle en rejoignant (session_catchup_peer).
    if (!savefile_in_coop() && !savefile_join_armed()) return;
    // A full queue means this peer is eight nodes behind, which no amount of
    // waiting is going to fix. Refuse the newest and SAY SO: the run is already
    // wrong, and the one thing that must not happen is what used to -- losing a
    // node without a line in the log. Dropping the newest rather than the
    // oldest keeps the queue contiguous, the same rule the peer-hash ring
    // follows for the same reason.
    ++g.epoch;
    if (g.pending_count >= State::kMaxPendingNodes) {
        log_line("FOLLOW", "!! %u nodes already queued and the host entered another"
                           " (node %u) -- REFUSING it. This peer is too far behind"
                           " to follow the run and the two are no longer the same"
                           " run.",
                 g.pending_count, m.index);
        choice_on_node_skipped(m.seed0);
        return;
    }

    State::PendingNode& p =
        g.pending_q[(g.pending_head + g.pending_count) % State::kMaxPendingNodes];
    ++g.pending_count;
    p.index = m.index;
    p.type  = m.type;
    p.count = m.node_count;
    p.seed  = m.seed0;
    p.flags = m.flags;
    p.epoch = g.epoch;
    // Timed from ARRIVAL, not from the first map update that sees it: the point
    // of the delay is to put a known gap between the host entering a node and
    // this peer entering it, and the map screen may not be ready for a while
    // either way.
    p.at    = GetTickCount64();
    p.held  = false;
    log_line("FOLLOW", "<- host entered node %u/%u (%s) seed0=%016llx%s",
             m.index, m.node_count, node_type_name(m.type),
             (unsigned long long)m.seed0,
             g.pending_count > 1 ? " -- QUEUED, this peer is still behind" : "");
}

void* follow_map_update(void* map_screen) {
    if (!g.on || !map_screen) return nullptr;

    Guard guard;
    g.map = map_screen;

    // The map is ticking, so the run is BETWEEN nodes. This is the stamp the
    // save flush reads -- see follow_on_map.
    const uint64_t prev_tick = g.map_tick;
    g.map_tick = GetTickCount64();
    if (g.inside && g.map_tick - g.inside_at > 1000) g.inside = false;
    // Mewgenics Multi : la case de ce joueur est finie a la premiere image de
    // carte apres sa fin de case (cases 5 a 15) ; pour les autres, plus d'une
    // seconde apres l'entree. Filet : une carte qui tourne sans arret depuis
    // 10 s (aucune scene de case n'en fait autant) sans fin de case vue.
    if (!prev_tick || g.map_tick - prev_tick > 200) g.map_gap_at = g.map_tick;
    ++g.map_ticks;
    if (g.live) {
        bool done = false;
        if (!g.live_waits_end) {
            done = g.map_tick - g.live_at > 1000;
        } else if (g.live_ended && g.map_ticks > g.live_end_ticks) {
            done = true;
        } else if (g.map_tick - g.live_at >= 10000 && g.map_tick - g.map_gap_at >= 10000) {
            done = true;
            log_line("FOLLOW", "!! no end of node seen, but the map has been running for 10 s -- "
                               "taking the node as finished");
        }
        if (done) g.live = false;
    }

    // --- a jump asked for by the debug panel --------------------------------
    //
    // Handled before the follow path and returning early, because the two are
    // different intentions and letting a jump fall through into the pending
    // follow would enter two nodes on one tick.
    if (g.jump_pending) {
        g.jump_pending = false;
        const uint32_t index = g.jump_index;

        uint32_t count = 0; void* data = nullptr;
        if (!read_nodes(map_screen, count, data)) return nullptr;   // not ready

        void* node = node_at(map_screen, index);
        if (!node) {
            log_line("FOLLOW", "!! panel asked to enter node %u but this map has"
                               " %u node(s) -- ignored", index, count);
            return nullptr;
        }

        // Loud, unconditionally, and on both peers' behalf: this is the debug
        // panel EDITING THE RUN. Everything else in this module reacts to a
        // decision a player made by clicking the map.
        log_line("FOLLOW", "!! PANEL JUMP: entering node %u (%s) seed0=%016llx by"
                           " request from the debug panel -- this is not a move the"
                           " player made on the map",
                 index, node_type_name(node_type(node)),
                 (unsigned long long)node_seed0(node));

        // Mirror h_EnterNode exactly: publish through the ordinary path first,
        // then let the caller run the original. Calling o_EnterNode alone would
        // bypass the detour and the client would never hear about it -- the same
        // property the follow path relies on in the other direction.
        bool sent = false;
        if (!follow_on_enter_node(map_screen, node, &sent)) return nullptr;
        // Mewgenics Multi : sans marche, le pion doit arriver sur la case pour
        // que la fin de case la marque visitee (voir mgmp_marker.h).
        marker_arrive(map_screen, node);
        return node;
    }

    // Mewgenics Multi : jamais dans la propre save de l'invite -- les cases de
    // l'hote sont celles de SA course, pas de celle-ci.
    if (g.is_client && !savefile_in_coop()) return nullptr;
    if (!pending_any()) return nullptr;
    // Mewgenics Multi : pas tant que son ecran des objets est ouvert (ses
    // boites tiennent les objets que la case relirait) ; et ce que l'hote ne
    // prendra jamais revient comme avant, ici, avant de suivre (mm_rungear).
    if (g.is_client && rungear_hold_follow()) return nullptr;
    State::PendingNode& p = pending_front();

    // --- the deliberate late-join delay (test knob) -------------------------
    //
    // Holding the node here rather than dropping it is what makes this a test
    // of the join barrier and not just a broken client: the host walks into the
    // battle alone, this peer arrives net_follow_delay_ms later, and the barrier
    // is what has to have kept the host from playing turns in the meantime.
    // Everything downstream -- the seed check, the type check -- still runs when
    // the hold expires, so the delay changes WHEN we follow and nothing else.
    if (const uint32_t delay = config().net_follow_delay_ms) {
        const uint64_t waited = GetTickCount64() - p.at;
        if (waited < delay) {
            if (!p.held) {
                p.held = true;
                log_line("FOLLOW", "!! net_follow_delay_ms = %u -- deliberately holding"
                                   " node %u for %u ms before following. This is a TEST"
                                   " setting: it manufactures the late-join gap.",
                         delay, p.index, delay);
            }
            return nullptr;
        }
        if (p.held)
            log_line("FOLLOW", "delay elapsed (%llu ms) -- following now",
                     (unsigned long long)waited);
    }

    uint32_t count = 0; void* data = nullptr;
    if (!read_nodes(map_screen, count, data)) return nullptr;   // not ready yet

    // Same map, or the index means nothing. Both peers generate the map from
    // the same save, so a differing count is a real problem and not a race.
    if (p.count && count != p.count) {
        log_line("FOLLOW", "!! host's map has %u nodes, ours has %u -- not"
                           " following; the two runs are not the same run",
                 p.count, count);
        const uint64_t lost = p.seed;
        pending_pop();
        choice_on_node_skipped(lost);
        return nullptr;
    }

    void* node = node_at(map_screen, p.index);
    if (!node) {
        log_line("FOLLOW", "!! node %u is out of range on our map (%u nodes)",
                 p.index, count);
        const uint64_t lost = p.seed;
        pending_pop();
        choice_on_node_skipped(lost);
        return nullptr;
    }

    // Identity check, and it is the strong one: the node's stored seed is the
    // 32 bytes EnterNode is about to load into the simulation stream, so if it
    // differs the battle would start from a different RNG state and desync on
    // its first roll. Catching it here names the cause; catching it at the
    // first turn hash would only say "they disagree".
    uint64_t seed = node_seed0(node);
    if (p.seed && seed != p.seed) {
        log_line("FOLLOW", "!! node %u seed is %016llx here but %016llx on the"
                           " host -- refusing to enter; the maps differ",
                 p.index, (unsigned long long)seed,
                 (unsigned long long)p.seed);
        const uint64_t lost = p.seed;
        pending_pop();
        choice_on_node_skipped(lost);
        return nullptr;
    }

    uint32_t type = node_type(node);
    if (p.type != type)
        log_line("FOLLOW", "!! node %u is '%s' here but '%s' on the host --"
                           " entering anyway, the seed matched",
                 p.index, node_type_name(type), node_type_name(p.type));

    const uint32_t entering = p.index;
    const uint8_t  flags    = p.flags;
    const uint32_t epoch    = p.epoch;
    pending_pop();
    start_live(type, epoch);
    ++g.entered;
    log_line("FOLLOW", "following host into node %u (%s)%s",
             entering, node_type_name(type),
             pending_any() ? " -- more nodes still queued" : "");

    // Battle identity, established without negotiating anything: this is the
    // same node the host entered, so `seed` is the same 64 bits it read. The
    // host does the matching call in follow_on_enter_node. Note this is the
    // CLIENT's only chance to make it -- the follow path calls o_EnterNode, the
    // MinHook trampoline, which bypasses the h_EnterNode detour entirely.
    lockstep_enter_battle(seed);

    // Everything the host published for this node lands HERE, and it must land
    // BEFORE remember_node, which is where the meta hash is taken.
    //
    // Two separate reasons, and both were measured wrong on 2026-08-26.
    //
    // The apply point itself: this is the last moment before EnterNode, so it
    // is still in time for the battle or shop the node is about to open; and it
    // is inside MapScreen::update, so the inventory apply's clear-and-rebuild
    // cannot free Equipment out from under an inventory screen this peer had
    // open when the host clicked. See the note above invsync_apply_pending.
    //
    // The ORDER: the host takes its node hash after publishing, so the hash
    // describes the run it just pushed. With these calls below remember_node
    // the client hashed the run it had BEFORE the push -- one node stale --
    // and reported an inventory mismatch that was entirely ours. That is
    // exactly the "a hash whose two halves are computed at different points in
    // the frame compares two different things" failure remember_node's own
    // comment warns about.
    //
    // Mewgenics Multi : seulement ce qui est arrive AVANT l'ENTERNODE de cette
    // case (rang `epoch`). Ce que l'hote a pousse apres -- de retour sur la
    // carte, ou en entrant dans la case suivante pendant que ce joueur etait
    // en retard -- est l'etat d'APRES cette case : applique avant elle, la case
    // le referait par-dessus (un evenement rejoue chez ce joueur donnerait deux
    // fois). Il reste garde jusqu'apres la case.
    invsync_apply_pending_before(epoch, "about to follow the host into a node");
    catsync_apply_pending_before(epoch, "about to follow the host into a node");
    runhist_apply_pending_before(epoch, "about to follow the host into a node");
    rungear_on_followed(seed);

    remember_node(entering, count, type, seed);

    // A shift-held EnterNode takes a completely different branch (the very
    // first thing it does is SDL_GetScancodeFromKey(SDLK_LSHIFT) and, if down,
    // tail-calls sub_1403923F0 instead). That is local keyboard state, so the
    // client would diverge from the host purely by holding a key. Cheap to
    // notice, so say so rather than let it be mysterious.
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        log_line("FOLLOW", "!! LEFT SHIFT is down -- EnterNode branches on it and"
                           " this peer will not take the host's path");

    // Mewgenics Multi : le pion de ce joueur n'a pas marche jusqu'a la case
    // (ses clics sont des votes, ou il n'a pas clique). Il y arrive maintenant,
    // comme a la fin d'une marche : sinon la fin de case marquerait visitee la
    // case ou il est reste, et sa carte n'avancerait plus (mgmp_marker.h).
    marker_arrive(map_screen, node);

    // Mewgenics Multi : la maison, comme chez l'hote -- prise avant la fin de la
    // carte ou non (MapScreen+0x1E7, que seul le clic ou la confirmation de
    // l'hote posent ; EnterNode le lit dans la branche de la maison).
    if (type == kNodeType_Home) {
        const uint8_t early = (flags & kEnterEarlyHome) ? 1 : 0;
        mem_write((uint8_t*)map_screen + kMap_EarlyHome, &early, sizeof(early));
        if (early) log_line("FOLLOW", "retour anticipe a la maison, comme chez l'hote");
    }

    return node;
}

} // namespace mgmp
