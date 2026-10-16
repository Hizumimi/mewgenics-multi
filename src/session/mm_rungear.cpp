// mm_rungear.cpp -- voir mm_rungear.h.
#include "mm_rungear.h"
#include "mm_gear.h"
#include "mm_menu.h"
#include "mm_team.h"
#include "mm_team_merge.h"

#include "mgmp_addresses.h"
#include "mgmp_catsync.h"
#include "mgmp_choice.h"
#include "mgmp_follow.h"
#include "mgmp_hooks.h"
#include "mgmp_invsync.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"

#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_rungear.h ----------------------------------------------

constexpr uintptr_t kScr_Current = 0x108;   // u64, la cle du chat montre (InventoryScreen2)
constexpr uintptr_t kCat_Id      = 0x00;    // CatData : son id

// --- les delais et les tailles ----------------------------------------------------------

constexpr uint8_t  kPeers         = 8;
constexpr uint32_t kMaxScreens    = 4;
constexpr uint32_t kMaxRunKeys    = 16;
constexpr uint64_t kRevBeatMs     = 2000;   // hote : la revision, meme sans changement
constexpr uint64_t kAnswerWaitMs  = 20000;  // invite : sa photo sans reponse, au plus
constexpr uint64_t kChoiceQuietMs = 600;    // invite : pas de relecture juste apres un ecran de choix
constexpr uint32_t kStaleFrames   = 120;    // un ecran pas mis a jour depuis : verifier qu'il existe encore

// MM_RUNREV a = 1 (hote -> tous) : x = la revision, b = ces drapeaux, texte[0..7] =
// la graine de la derniere case ou l'hote est entre dans cette aventure (0 : aucune).
constexpr uint8_t  kRunRevMsg     = 1;
constexpr uint8_t  kRevSettled    = 0x01;   // il a pousse son etat d'apres cette case (ou du debut)
constexpr uint8_t  kRevHostBusy   = 0x02;   // son ecran des objets est ouvert

using FnScreenUpdate = void  (__fastcall*)(void* screen);
using FnScreenDtor   = void* (__fastcall*)(void* screen, uint32_t flags);
using FnNodeEnd      = void  (__fastcall*)(void* map);
FnScreenUpdate o_ScreenUpdate = nullptr;
FnScreenDtor   o_ScreenDtor   = nullptr;
FnNodeEnd      o_NodeEnd      = nullptr;

// Pourquoi un invite ne change rien sur un ecran (hors chats des autres).
enum class Why : uint8_t {
    None,
    Node,        // pas sur la carte, ou l'hote est entre dans une case a suivre
    Behind,      // des poussees de l'hote attendent (ou un autre ecran a la main)
    Answer,      // sa photo d'avant attend encore la reponse de l'hote
    Wait,        // l'hote n'a pas encore pousse son etat d'apres la case
    HostBusy,    // l'hote a son ecran des objets ouvert
    Unreadable,  // la photo d'ouverture ne se lit pas
};

// Ses chats (ceux de l'equipe qui sont a lui), le sac et la poubelle, tels que
// le jeu les serialise ; et leur empreinte.
struct Snap {
    bool       ok   = false;
    uint64_t   hash = 0;
    RunGearMsg msg;
};

struct Screen {
    void*    ptr      = nullptr;
    void*    vt       = nullptr;   // sa vtable a la premiere image (InventoryScreen2)
    uint32_t seen     = 0;         // la derniere image ou son update a tourne
    bool     snap_due = false;     // invite : photo d'ouverture a prendre (guest_pump)
    bool     session  = false;     // invite : ce qu'il y change partira chez l'hote
    bool     lazy_off = false;     // invite : change depuis l'ouverture, plus de session tardive
    Why      why      = Why::None;
    Snap     snap;                 // invite : l'etat a l'ouverture
};

// Un ecran ferme, que guest_pump ou rungear_hold_follow traite.
struct Closed {
    bool session = false;
    Snap snap;
};

// Hote : la derniere photo de chaque invite, pas encore traitee.
struct Pending {
    bool       have = false;
    RunGearMsg msg;
};

struct State {
    uint32_t frame  = 1;
    bool     active = false;          // une fois par image (rungear_pump)
    bool     told_hooks = false;

    // Les ecrans des objets ouverts chez ce joueur.
    Screen   screens[kMaxScreens];
    uint32_t n_screens  = 0;
    void*    last_screen = nullptr;   // le dernier vu : celui d'un clic
    Closed   closed[kMaxScreens];
    uint32_t n_closed   = 0;

    // « ce joueur peut-il toucher au chat montre ? », une fois par image
    uint32_t touch_frame  = 0;
    void*    touch_screen = nullptr;
    uint64_t touch_key    = 0;
    bool     touch_ok     = true;
    int      touch_owner  = -1;

    // Hote.
    uint32_t rev         = 0;
    uint64_t rev_sent_at = 0;
    uint8_t  flags_sent  = 0xFF;
    uint64_t node_seed   = 0;         // la derniere case ou il est entre (0 : aucune)
    bool     settled     = false;     // son etat d'apres cette case est parti
    bool     publish_now = false;     // son ecran vient de se fermer
    Pending  pend[kPeers];

    // Invite.
    uint32_t rev_received = 0;
    uint32_t rev_applied  = 0;
    bool     have_rev     = false;    // un MM_RUNREV recu de l'hote
    uint8_t  host_flags   = 0;
    uint64_t host_seed    = 0;
    uint64_t entered_seed = 0;        // la derniere case de l'hote que ce joueur a suivie
    bool     adopted      = false;    // ... connue (suivie, ou reprise en arrivant)
    bool     session      = false;    // un ecran de session ouvert
    uint32_t session_base = 0;
    Snap     m0;                      // l'etat a l'ouverture de la derniere session
    uint32_t seq          = 0;
    bool     unconfirmed  = false;    // sa photo est partie, sans reponse encore
    uint32_t unconf_seq   = 0;
    uint64_t unconf_since = 0;
    bool     restore_due  = false;    // sa photo refusee : remettre m0, puis les poussees
    bool     told_hold    = false;

    // Les fenetres de ce joueur : une par raison et par ecran ouvert.
    uint32_t    told = 0;
    std::string notice;

    uint32_t sent = 0, received = 0, applied = 0, refused = 0, blocked = 0, drained = 0, restored = 0;
};

State g;

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

uint64_t fnv(const void* p, size_t n, uint64_t h) {
    const uint8_t* b = (const uint8_t*)p;
    for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 0x100000001B3ull; }
    return h;
}

bool is_host()  { return net_role() == NetRole::Host; }
bool is_guest() { return net_role() == NetRole::Client; }
int  self_id()  { return is_host() ? (int)kHostPeer : (int)net_self(); }

bool compute_active() {
    if (gear_role_active()) return false;               // le choix des objets du depart (mm_gear)
    if (!savefile_adventure_is_loaded()) return false;
    bool want = false;
    if (is_host())       want = mm_run_peers_mask() != 0;
    else if (is_guest()) want = savefile_in_coop() && mm_guest_in_run();
    if (!want) return false;
    // Sans ses deux crochets, ce module ne sait ni quand un ecran est ouvert ni
    // quand il se ferme : un invite attendrait sa fermeture pour toujours, ou
    // une relecture passerait sous un ecran ouvert. Mieux vaut l'ancien
    // comportement.
    if (!hooks_is_live(T_InvScreenUpdate) || !hooks_is_live(T_InvScreenDtor)) {
        if (!g.told_hooks) {
            g.told_hooks = true;
            log_line("SAC", "!! l'ecran des objets n'est pas suivi (crochets absents) : objets en aventure "
                            "eteints, chacun touche a tous les chats comme avant");
        }
        return false;
    }
    return true;
}

// L'equipe de l'aventure (MewDirector+1472).
bool run_keys(std::vector<uint64_t>& out) {
    out.clear();
    const void* const* slot = (const void* const*)addr_of_data(D_MewDirectorPtr);
    const void* dir = nullptr;
    if (!slot || !mem_read(slot, &dir, sizeof(dir)) || !dir) return false;
    uint32_t n = 0;
    const uint64_t* data = nullptr;
    if (!mem_read((const char*)dir + kDir_CatIdCount, &n, sizeof(n)) ||
        !mem_read((const char*)dir + kDir_CatIdData, &data, sizeof(data)))
        return false;
    if (n == 0 || n > kMaxRunKeys || !data) return false;
    out.resize(n);
    if (!mem_read(data, out.data(), n * sizeof(uint64_t))) { out.clear(); return false; }
    return true;
}

// Le proprietaire d'un chat de l'equipe : celui de l'equipe mixte, l'hote a
// defaut. -1 : pas de chat a cette cle.
int owner_of_key(uint64_t key) {
    void* cat = catsync_cat_by_key(key);
    uint64_t id = 0;
    if (!cat || !mem_read((const char*)cat + kCat_Id, &id, sizeof(id))) return -1;
    const int o = team_owner_of(id);
    return o < 0 ? (int)kHostPeer : o;
}

Screen* find_screen(void* ptr) {
    for (uint32_t i = 0; i < g.n_screens; ++i)
        if (g.screens[i].ptr == ptr) return &g.screens[i];
    return nullptr;
}

void tell(Why why, const std::string& text) {
    const uint32_t bit = 1u << (uint32_t)why;
    if (g.told & bit) return;
    g.told |= bit;
    g.notice = text;
}

// --- les ecrans des objets (les deux detours) ---------------------------------------------

void note_screen(void* screen) {
    g.last_screen = screen;
    if (Screen* known = find_screen(screen)) { known->seen = g.frame; return; }
    if (g.n_screens >= kMaxScreens) return;
    Screen& s = g.screens[g.n_screens++];
    s = Screen{};
    s.ptr      = screen;
    s.seen     = g.frame;
    s.snap_due = true;
    mem_read(screen, &s.vt, sizeof(s.vt));
    g.told = 0;
}

void forget_screen(void* screen) {
    for (uint32_t i = 0; i < g.n_screens; ++i) {
        if (g.screens[i].ptr != screen) continue;
        Screen& s = g.screens[i];
        if (s.session || s.snap.ok) {
            if (g.n_closed < kMaxScreens) {
                Closed& c = g.closed[g.n_closed++];
                c.session = s.session;
                c.snap    = std::move(s.snap);
            } else {
                log_line("SAC", "!! trop d'ecrans fermes a la fois -- l'un n'est pas verifie");
            }
        }
        if (s.session) g.session = false;
        g.screens[i] = std::move(g.screens[--g.n_screens]);
        g.screens[g.n_screens] = Screen{};
        if (is_host()) g.publish_now = true;
        break;
    }
    if (g.last_screen == screen) g.last_screen = g.n_screens ? g.screens[0].ptr : nullptr;
    if (g.touch_screen == screen) g.touch_frame = 0;
}

void __fastcall h_ScreenUpdate(void* screen) {
    o_ScreenUpdate(screen);
    if (screen) note_screen(screen);
}

void* __fastcall h_ScreenDtor(void* screen, uint32_t flags) {
    if (screen) forget_screen(screen);
    return o_ScreenDtor(screen, flags);
}

// La fin d'une case (T_NodeEnd) : d'abord le jeu, puis le suivi le note.
void __fastcall h_NodeEnd(void* map) {
    o_NodeEnd(map);
    follow_on_node_end();
}

// Un ecran dont l'update ne tourne plus depuis un moment : s'il n'est plus un
// InventoryScreen2 (sa vtable a change -- detruit sans passer par le slot 5),
// on l'oublie, sinon un invite attendrait sa fermeture pour toujours avant de
// suivre l'hote. S'il en est toujours un (cache par une autre fenetre), il reste.
void drop_vanished_screens() {
    for (uint32_t i = 0; i < g.n_screens;) {
        Screen& s = g.screens[i];
        void* vt = nullptr;
        if (g.frame - s.seen < kStaleFrames ||
            (mem_read(s.ptr, &vt, sizeof(vt)) && vt && vt == s.vt)) { ++i; continue; }
        log_line("SAC", "!! un ecran des objets a disparu sans son destructeur (%u images sans update) -- oublie",
                 g.frame - s.seen);
        forget_screen(s.ptr);
    }
}

// --- les messages ---------------------------------------------------------------------------

uint8_t host_flags_now() {
    return (uint8_t)((g.settled ? kRevSettled : 0) | (g.n_screens ? kRevHostBusy : 0));
}

void send_rev(uint64_t now) {
    MmMsg m{};
    m.kind = MM_RUNREV;
    m.a    = kRunRevMsg;
    m.b    = host_flags_now();
    m.x    = g.rev;
    static_assert(sizeof(m.text) >= sizeof(g.node_seed), "MmMsg::text trop petit");
    memcpy(m.text, &g.node_seed, sizeof(g.node_seed));
    net_send_mm(m);
    g.rev_sent_at = now;
    g.flags_sent  = m.b;
}

// Hote : ce qui a change chez lui part ; la revision avance si quelque chose est parti.
bool publish_changes(const char* why, uint64_t now) {
    const uint32_t cats = catsync_publish(why);
    const bool     inv  = invsync_publish(why);
    if (!cats && !inv) return false;
    ++g.rev;
    send_rev(now);
    return true;
}

// Hote : tout repart, meme ce qui n'a pas change chez lui -- un invite dont la
// photo est refusee revient ainsi a l'etat de l'hote. Sans faire avancer la
// revision : ses vrais changements sont partis juste avant (publish_changes),
// ceci n'est que le meme etat renvoye. Sinon, a deux invites, chaque refus
// rendrait perimee la session de l'autre.
void resend_all(const char* why, uint64_t now) {
    catsync_forget();
    invsync_forget();
    catsync_publish(why);
    invsync_publish(why);
    send_rev(now);
}

void send_ack(uint8_t peer, uint32_t seq, uint8_t result) {
    RunGearAck a;
    a.seq    = seq;
    a.result = result;
    a.rev    = g.rev;
    if (!team_send(team_rungear_ack_encode(a), peer))
        log_line("SAC", "!! reponse a %s non envoyee", mm_peer_name(peer).c_str());
}

// --- hote -------------------------------------------------------------------------------------

// La reponse part apres les poussees : l'invite les a quand il la lit.
void refuse(uint8_t peer, const RunGearMsg& m, uint8_t result, const char* why, uint64_t now) {
    ++g.refused;
    log_line("SAC", "photo %u de %s refusee : %s -- tout lui est renvoye", m.seq, mm_peer_name(peer).c_str(), why);
    resend_all("photo d'un invite refusee", now);
    send_ack(peer, m.seq, result);
}

void process_photo(uint8_t peer, uint64_t now) {
    Pending& x = g.pend[peer];
    x.have = false;
    const RunGearMsg& m = x.msg;
    if (m.base != g.rev) {
        refuse(peer, m, kRunGearStale,
               fmt("faite sur la revision %u, l'aventure en est a la %u", m.base, g.rev).c_str(), now);
        return;
    }
    // Le sac et la poubelle d'abord (de quoi les remettre), puis ses chats --
    // qui se remettent eux-memes s'ils echouent.
    std::vector<uint8_t> bag0, trash0;
    if (!invsync_serialize_bucket(kInvBucketBackpack, bag0) ||
        !invsync_serialize_bucket(kInvBucketTrash, trash0)) {
        refuse(peer, m, kRunGearInvalid, "le sac de l'hote ne se lit pas", now);
        return;
    }
    if (!invsync_load_bucket(kInvBucketBackpack, m.backpack) ||
        !invsync_load_bucket(kInvBucketTrash, m.trash)) {
        invsync_load_bucket(kInvBucketBackpack, bag0);
        invsync_load_bucket(kInvBucketTrash, trash0);
        refuse(peer, m, kRunGearInvalid, "son sac n'a pas pu etre relu", now);
        return;
    }
    std::string why;
    if (!gear_apply_cats(peer, m.cats, why)) {
        invsync_load_bucket(kInvBucketBackpack, bag0);
        invsync_load_bucket(kInvBucketTrash, trash0);
        refuse(peer, m, kRunGearInvalid, why.c_str(), now);
        return;
    }
    ++g.applied;
    log_line("SAC", "photo %u de %s prise : %zu chat(s), sac %zu octets, poubelle %zu octets", m.seq,
             mm_peer_name(peer).c_str(), m.cats.size(), m.backpack.size(), m.trash.size());
    publish_changes("photo d'un invite prise", now);
    send_ack(peer, m.seq, kRunGearApplied);
}

void host_pump(uint64_t now) {
    if (g.rev == 0) {
        g.rev       = 1;
        g.node_seed = 0;
        g.settled   = false;
        send_rev(now);
    }
    // Sur la carte, entre deux cases, sans ecran des objets ouvert : rien ne
    // tient les objets du sac ni ceux des chats.
    if (follow_on_map() && !follow_inside_node() && !follow_node_live() && !g.n_screens) {
        // De retour sur la carte (ou au debut de l'aventure) : son etat d'apres
        // la case part, une fois. Jusque-la, un invite n'ouvre pas de session --
        // sa photo serait faite avant cette poussee et refusee.
        if (!g.settled) {
            g.settled = true;
            const bool any = publish_changes(g.node_seed ? "de retour sur la carte" : "debut de l'aventure", now);
            if (!any) send_rev(now);
            log_line("SAC", "etat de l'hote %s (revision %u) : les invites peuvent changer leurs objets",
                     any ? "pousse" : "inchange", g.rev);
        }
        // Ses propres changements d'abord, et toujours avant de prendre une
        // photo : une photo d'invite faite avant eux ne passe pas par-dessus (la
        // revision avance, elle est refusee).
        bool photo = false;
        for (uint8_t p = 1; p < kPeers; ++p) photo = photo || g.pend[p].have;
        if (g.publish_now || photo) {
            const bool closed = g.publish_now;
            g.publish_now = false;
            publish_changes(closed ? "l'hote a ferme le sac" : "avant une photo d'invite", now);
        }
        for (uint8_t p = 1; p < kPeers; ++p)
            if (g.pend[p].have) process_photo(p, now);
    }
    // Apres les poussees : un invite ne doit pas lire « sac de l'hote ferme »
    // avant ce qu'il y a change.
    if (host_flags_now() != g.flags_sent || now - g.rev_sent_at >= kRevBeatMs) send_rev(now);
}

// --- invite -----------------------------------------------------------------------------------

bool take_photo(Snap& s) {
    s = Snap{};
    RunGearMsg& m = s.msg;
    if (!invsync_serialize_bucket(kInvBucketBackpack, m.backpack) ||
        !invsync_serialize_bucket(kInvBucketTrash, m.trash))
        return false;
    std::vector<uint64_t> keys;
    if (!run_keys(keys)) return false;
    const int self = self_id();
    for (uint64_t key : keys) {
        if (owner_of_key(key) != self) continue;
        GearCat c;
        c.key = key;
        if (!catsync_serialize_cat(catsync_cat_by_key(key), c.data) || c.data.empty()) return false;
        m.cats.push_back(std::move(c));
    }
    uint64_t h = fnv(m.backpack.data(), m.backpack.size(), 0xCBF29CE484222325ull);
    h = fnv(m.trash.data(), m.trash.size(), h);
    for (const GearCat& c : m.cats) {
        h = fnv(&c.key, sizeof(c.key), h);
        h = fnv(c.data.data(), c.data.size(), h);
    }
    s.hash = h;
    s.ok   = true;
    return true;
}

// Remet ses chats, le sac et la poubelle comme dans `s`. Jamais sous un ecran
// des objets ouvert (ses boites tiennent ces objets).
bool restore(const Snap& s, const char* why) {
    if (!s.ok || g.n_screens) {
        log_line("SAC", "!! %s : rien a remettre%s", why, g.n_screens ? " (un ecran est ouvert)" : "");
        return false;
    }
    bool ok = invsync_load_bucket(kInvBucketBackpack, s.msg.backpack) &&
              invsync_load_bucket(kInvBucketTrash, s.msg.trash);
    std::string err;
    if (!gear_apply_cats((uint8_t)self_id(), s.msg.cats, err)) {
        ok = false;
        log_line("SAC", "!! ses chats ne reviennent pas (%s)", err.c_str());
    }
    ++g.restored;
    log_line("SAC", "%s : %s", why, ok ? "ses chats et le sac reviennent comme a l'ouverture"
                                       : "!! RETOUR INCOMPLET -- ses objets peuvent differer de ceux de l'hote");
    return ok;
}

// Remet `target` tout de suite ; ou, si un autre ecran des objets s'est deja
// rouvert (ses boites tiennent les objets), a sa fermeture : sa photo
// d'ouverture devient `target`, et un ecran sans session dont l'etat differe de
// sa photo revient a elle en se fermant.
void restore_or_defer(const Snap& target, const char* why) {
    if (!g.n_screens) { restore(target, why); return; }
    for (uint32_t i = 0; i < g.n_screens; ++i) {
        Screen& s = g.screens[i];
        s.snap     = target;
        s.snap_due = false;
        s.session  = false;
        s.lazy_off = true;
        s.why      = Why::Behind;
    }
    g.session = false;
    log_line("SAC", "%s : un ecran est deja rouvert, ses objets reviendront a sa fermeture", why);
}

const char* const kTextCancelledNode =
    u8"Objets\n\nL'hôte est entré dans une case pendant que\n"
    u8"ton sac était ouvert : tes changements sont annulés.\n"
    u8"Refais-les sur la carte.";
const char* const kTextNoAnswer =
    u8"Objets\n\nL'hôte n'a pas répondu : tes changements\n"
    u8"sont annulés. Refais-les dans un moment.";
const char* const kTextUnsent =
    u8"Objets\n\nTes changements n'ont pas pu partir chez l'hôte :\n"
    u8"ils sont annulés (le journal du mod dit pourquoi).";

// Les ecrans fermes : la photo part (session), ou ses objets reviennent comme a
// l'ouverture. Avec une case a suivre, une photo ne serait jamais prise (l'hote
// est entre apres sa revision) : ses objets reviennent tout de suite.
void process_closed(uint64_t now) {
    const bool node = follow_pending_count() != 0;
    for (uint32_t i = 0; i < g.n_closed; ++i) {
        Closed& c = g.closed[i];
        Snap cur;
        if (!take_photo(cur)) {
            log_line("SAC", "!! photo illisible a la fermeture");
            if (c.session) {
                restore_or_defer(c.snap, "ecran ferme, photo illisible");
                tell(Why::Unreadable, kTextUnsent);
            }
            continue;
        }
        if (cur.hash == c.snap.hash) {
            if (c.session) log_line("SAC", "ecran des objets ferme : rien n'a change");
            continue;
        }
        if (!c.session) {
            // Grise, rien ne devait bouger -- un tri du sac, par exemple. L'hote
            // ne le saura pas : il revient comme a l'ouverture. Pas dans une
            // case : ses propres effets ont pu changer le sac, et son ecran en
            // tient peut-etre les objets ; la poussee de l'hote d'apres la case
            // les remettra.
            if (follow_on_map() && !follow_node_live())
                restore_or_defer(c.snap, "ecran sans session ferme, le sac avait bouge (tri ?)");
            else
                log_line("SAC", "ecran ferme dans une case, le sac a bouge : laisse tel quel");
            continue;
        }
        if (node) {
            restore_or_defer(c.snap, "ecran ferme apres l'entree de l'hote dans une case");
            tell(Why::Node, kTextCancelledNode);
            continue;
        }
        cur.msg.seq  = ++g.seq;
        cur.msg.base = g.session_base;
        if (!team_send(team_rungear_encode(cur.msg), kHostPeer)) {
            log_line("SAC", "!! photo %u non envoyee", cur.msg.seq);
            restore_or_defer(c.snap, "photo non envoyee");
            tell(Why::Unreadable, kTextUnsent);
            continue;
        }
        ++g.sent;
        g.m0           = std::move(c.snap);
        g.unconfirmed  = true;
        g.unconf_seq   = cur.msg.seq;
        g.unconf_since = now;
        log_line("SAC", "-> photo %u : %zu chat(s), sac %zu octets, poubelle %zu octets (revision %u)",
                 cur.msg.seq, cur.msg.cats.size(), cur.msg.backpack.size(), cur.msg.trash.size(),
                 cur.msg.base);
    }
    for (uint32_t i = 0; i < g.n_closed; ++i) g.closed[i] = Closed{};
    g.n_closed = 0;
}

// Sa photo en attente ne sera pas prise (une case, pas de reponse) : ses objets
// reviennent comme a l'ouverture de la session.
void drop_unconfirmed(const char* why, const char* text) {
    g.unconfirmed = false;
    restore(g.m0, why);
    g.m0 = Snap{};
    tell(Why::Answer, text);
}

bool host_settled_for_me() {
    return g.have_rev && g.adopted && (g.host_flags & kRevSettled) && g.host_seed == g.entered_seed;
}

bool held_pushes() { return catsync_pending_count() != 0 || invsync_has_pending(); }

// Ce qui empeche un invite d'ouvrir une session maintenant.
Why guest_block() {
    if (!follow_on_map() || follow_pending_count() != 0 || follow_node_live()) return Why::Node;
    if (g.unconfirmed) return Why::Answer;
    if (g.restore_due || held_pushes() || g.session) return Why::Behind;
    if (!host_settled_for_me()) return Why::Wait;
    if (g.host_flags & kRevHostBusy) return Why::HostBusy;
    return Why::None;
}

const char* why_name(Why w) {
    switch (w) {
        case Why::None:       return "rien";
        case Why::Node:       return "pas sur la carte, ou une case de l'hote a suivre";
        case Why::Behind:     return "des poussees de l'hote attendent";
        case Why::Answer:     return "sa photo d'avant attend la reponse de l'hote";
        case Why::Wait:       return "l'hote n'a pas encore pousse son etat d'apres la case";
        case Why::HostBusy:   return "l'hote a son sac ouvert";
        case Why::Unreadable: return "photo d'ouverture illisible";
    }
    return "?";
}

void sync_rev_applied() {
    if (!held_pushes()) g.rev_applied = g.rev_received;
}

void start_session(Screen& s, const char* how) {
    s.session      = true;
    s.why          = Why::None;
    g.session      = true;
    g.m0           = s.snap;
    g.session_base = g.rev_applied;
    log_line("SAC", "ecran des objets : session ouverte %s (revision %u)", how, g.session_base);
}

void guest_pump(uint64_t now) {
    // Les ecrans fermes d'abord : si l'un doit revenir comme a l'ouverture et
    // qu'un autre s'est deja rouvert, celui-ci herite de la photo a remettre.
    if (g.n_closed) process_closed(now);
    // Les ecrans qui viennent de s'ouvrir : la photo d'ouverture (avant tout
    // clic), puis session ou non.
    for (uint32_t i = 0; i < g.n_screens; ++i) {
        Screen& s = g.screens[i];
        if (!s.snap_due) continue;
        s.snap_due = false;
        if (!take_photo(s.snap)) {
            s.why = Why::Unreadable;
            log_line("SAC", "!! photo d'ouverture illisible : rien ne change sur cet ecran");
            continue;
        }
        const Why w = guest_block();
        if (w == Why::None) start_session(s, "sur la carte");
        else {
            s.why = w;
            log_line("SAC", "ecran des objets ouvert sans session : %s", why_name(w));
        }
    }
    if (g.unconfirmed && !g.n_screens) {
        if (follow_pending_count())
            drop_unconfirmed("l'hote est entre dans une case avant de repondre a la photo", kTextCancelledNode);
        else if (now - g.unconf_since > kAnswerWaitMs)
            drop_unconfirmed("pas de reponse de l'hote a la photo", kTextNoAnswer);
    }
    if (g.restore_due && !g.n_screens) {
        g.restore_due = false;
        restore(g.m0, "photo refusee par l'hote");
        g.m0 = Snap{};
    }
    // Les poussees de l'hote, des que c'est sans danger : sur la carte, pas en
    // chemin vers une case (le suivi les applique alors lui-meme), pas juste
    // apres un ecran de choix (une montee de niveau peut encore s'y appliquer),
    // aucun ecran des objets ouvert, aucune photo sans reponse.
    if (held_pushes() && !g.n_screens && !g.unconfirmed && !g.restore_due && follow_on_map() &&
        follow_pending_count() == 0 && !follow_node_live() && !choice_screen_recent(kChoiceQuietMs)) {
        catsync_apply_pending("sur la carte");
        invsync_apply_pending("sur la carte");
        ++g.drained;
    }
    sync_rev_applied();
    // Un ecran ouvert trop tot (l'hote pas encore revenu, son sac ouvert...) :
    // la session s'ouvre d'elle-meme quand plus rien ne l'empeche, si rien n'a
    // bouge depuis l'ouverture.
    if (!g.session) {
        for (uint32_t i = 0; i < g.n_screens; ++i) {
            Screen& s = g.screens[i];
            if (s.session || s.snap_due || !s.snap.ok || s.lazy_off) continue;
            if (guest_block() != Why::None) continue;
            Snap cur;
            if (!take_photo(cur) || cur.hash != s.snap.hash) {
                s.lazy_off = true;
                log_line("SAC", "ecran des objets : le sac a bouge depuis l'ouverture, pas de session tardive");
                continue;
            }
            start_session(s, "en cours d'ecran (plus rien ne l'empechait)");
            break;
        }
    }
}

void reset_run() {
    for (uint8_t p = 0; p < kPeers; ++p) g.pend[p] = Pending{};
    // Un ecran deja ouvert : sa photo d'ouverture sera prise maintenant.
    for (uint32_t i = 0; i < g.n_screens; ++i) {
        g.screens[i].session  = false;
        g.screens[i].snap     = Snap{};
        g.screens[i].snap_due = true;
        g.screens[i].lazy_off = false;
        g.screens[i].why      = Why::None;
    }
    for (uint32_t i = 0; i < g.n_closed; ++i) g.closed[i] = Closed{};
    g.n_closed    = 0;
    g.session     = false;
    g.unconfirmed = false;
    g.restore_due = false;
    g.m0          = Snap{};
    g.rev         = 0;
    g.settled     = false;
    g.node_seed   = 0;
    g.publish_now = false;
    g.flags_sent  = 0xFF;
}

} // namespace

// ---------------------------------------------------------------------------------------------

bool rungear_binding(Target t, void** detour, void*** original) {
    if (t == T_InvScreenUpdate) {
        *detour   = (void*)&h_ScreenUpdate;
        *original = (void**)&o_ScreenUpdate;
        return true;
    }
    if (t == T_InvScreenDtor) {
        *detour   = (void*)&h_ScreenDtor;
        *original = (void**)&o_ScreenDtor;
        return true;
    }
    if (t == T_NodeEnd) {
        *detour   = (void*)&h_NodeEnd;
        *original = (void**)&o_NodeEnd;
        return true;
    }
    return false;
}

bool rungear_active() { return g.active; }

bool rungear_screen_open() { return g.n_screens != 0; }

bool rungear_hold_pushes() {
    return g.n_screens != 0 || (g.active && (g.unconfirmed || g.restore_due));
}

bool rungear_can_touch(void* screen) {
    if (!g.active || !screen) return true;
    uint64_t key = 0;
    if (!mem_read((const char*)screen + kScr_Current, &key, sizeof(key))) return is_host();
    if (g.touch_frame == g.frame && g.touch_screen == screen && g.touch_key == key) return g.touch_ok;
    g.touch_frame  = g.frame;
    g.touch_screen = screen;
    g.touch_key    = key;
    const int owner = owner_of_key(key);
    g.touch_owner  = owner;
    bool ok;
    if (owner < 0)                ok = is_host();             // pas de chat montre : l'hote, comme le jeu
    else if (owner != self_id())  ok = false;
    else if (is_guest()) {
        // Un invite : dans une session, et tant que l'hote n'est pas entre dans
        // une case (ce qu'il changerait serait annule).
        const Screen* s = find_screen(screen);
        ok = s && s->session && follow_pending_count() == 0 && !follow_node_live();
    } else                        ok = true;
    g.touch_ok = ok;
    return ok;
}

bool rungear_on_click(void* button) {
    if (!g.active || !button) return false;
    char name[64] = {};
    if (!mem_read_std_string((const char*)button + kBtn_Name, name, sizeof(name))) return false;
    if (strncmp(name, "InventoryButton_", 16) != 0 && strncmp(name, "EquippedButton_", 15) != 0) return false;
    void* screen = g.last_screen;
    if (!screen || !find_screen(screen) || rungear_can_touch(screen)) return false;
    const bool other = g.touch_owner >= 0 && g.touch_owner != self_id();
    const Screen* s = find_screen(screen);
    Why why = Why::None;
    if (!other && s) {
        why = s->snap_due ? Why::None : s->why;
        if (s->session) why = Why::Node;          // une session, mais une case de l'hote a suivre
    }
    if (++g.blocked <= 3 || g.blocked % 20 == 0)
        log_line("SAC", "clic retenu (%s) : %s", name,
                 other ? "ce chat n'est pas a ce joueur" : why_name(why));
    if (other) {
        tell(Why::None, fmt(u8"Objets\n\nCe chat est à %s : seul son propriétaire\n"
                            u8"peut changer ses objets.", mm_peer_name((uint8_t)g.touch_owner).c_str()));
        return true;
    }
    switch (why) {
        case Why::Node:
            if (follow_pending_count()) break;      // il va suivre l'hote : rien a lire apres coup
            tell(why, u8"Objets\n\nPendant une case, les objets ne changent que\n"
                      u8"chez l'hôte. Équipe tes chats sur la carte.");
            break;
        case Why::Behind:
            tell(why, u8"Objets\n\nLe sac vient de changer chez l'hôte :\n"
                      u8"ferme-le et rouvre-le pour le voir à jour.");
            break;
        case Why::Answer:
            tell(why, u8"Objets\n\nTes changements d'avant attendent encore\n"
                      u8"la réponse de l'hôte : rouvre ton sac dans un instant.");
            break;
        case Why::Wait:
            tell(why, u8"Objets\n\nL'hôte n'est pas encore revenu sur la carte :\n"
                      u8"attends-le, ton sac se débloquera tout seul.");
            break;
        case Why::HostBusy:
            tell(why, u8"Objets\n\nL'hôte a son sac ouvert :\n"
                      u8"attends qu'il le ferme.");
            break;
        case Why::Unreadable:
            tell(why, u8"Objets\n\nLe sac ne se lit pas chez toi :\n"
                      u8"le journal du mod dit pourquoi.");
            break;
        case Why::None:
            break;
    }
    return true;
}

void rungear_pump() {
    ++g.frame;
    drop_vanished_screens();
    const bool was = g.active;
    g.active = compute_active();
    if (was && !g.active) {
        log_line("SAC", "fin des objets en aventure : %u photo(s) envoyee(s), %u recue(s), %u prise(s), "
                        "%u refusee(s), %u clic(s) retenu(s), %u relecture(s) sur la carte, %u retour(s) en arriere",
                 g.sent, g.received, g.applied, g.refused, g.blocked, g.drained, g.restored);
        reset_run();
    } else if (!was && g.active) {
        log_line("SAC", "objets en aventure : chacun n'equipe que ses chats, %s",
                 is_host() ? "l'hote prend les photos des invites" : "ce qu'il change part chez l'hote");
        reset_run();
        g.told = 0;
        if (is_guest()) {
            // La case ou l'hote en est, telle qu'il la dit : ce joueur arrive
            // dans l'aventure telle qu'elle est maintenant.
            g.adopted      = g.have_rev;
            g.entered_seed = g.host_seed;
        }
    }
    if (!savefile_adventure_is_loaded()) {
        g.rev_received = g.rev_applied = 0;
        g.have_rev = g.adopted = false;
    }
    if (!g.notice.empty() && !mm_info_busy() && !g.n_screens) {
        // Jamais au moment ou il va suivre l'hote : la fenetre arreterait sa
        // carte, et l'hote l'attendrait. Elle attend la carte d'apres la case.
        const bool show = is_host() ? (follow_on_map() && !follow_inside_node() && !follow_node_live())
                                    : (follow_on_map() && follow_pending_count() == 0 && !follow_node_live());
        if (show) {
            mm_show_info(g.notice, kInfoYieldToFollow);
            g.notice.clear();
        }
    }
    if (!g.active) return;
    const uint64_t now = GetTickCount64();
    if (is_host()) host_pump(now);
    else           guest_pump(now);
}

void rungear_on_node_entered(uint64_t seed) {
    if (!g.active || !is_host()) return;
    const uint64_t now = GetTickCount64();
    ++g.rev;
    g.node_seed = seed;
    g.settled   = false;
    // Une photo d'avant cette case ne vaut plus : ses chats y auront joue. Tout
    // repart une fois (l'etat d'avant la case), puis les reponses.
    bool any = false;
    for (uint8_t p = 1; p < kPeers; ++p) any = any || g.pend[p].have;
    if (any) resend_all("photo d'un invite refusee : l'hote entre dans une case", now);
    else     send_rev(now);
    for (uint8_t p = 1; p < kPeers; ++p) {
        if (!g.pend[p].have) continue;
        g.pend[p].have = false;
        ++g.refused;
        log_line("SAC", "photo %u de %s refusee : l'hote entre dans une case", g.pend[p].msg.seq,
                 mm_peer_name(p).c_str());
        send_ack(p, g.pend[p].msg.seq, kRunGearNode);
    }
}

void rungear_on_followed(uint64_t seed) {
    if (!is_guest()) return;
    g.entered_seed = seed;
    g.adopted      = true;
}

bool rungear_hold_follow() {
    if (!g.active || !is_guest()) return false;
    if (g.n_screens) {
        if (!g.told_hold) {
            g.told_hold = true;
            log_line("SAC", "l'hote est entre dans une case : ce joueur le suit des que son ecran des objets "
                            "est ferme");
        }
        return true;
    }
    g.told_hold = false;
    // Avant de suivre : ce qui ne sera jamais pris chez l'hote revient comme a
    // l'ouverture -- sinon la case commencerait avec des objets qu'il n'a pas.
    const uint64_t now = GetTickCount64();
    if (g.n_closed) process_closed(now);
    if (g.unconfirmed)
        drop_unconfirmed("l'hote est entre dans une case avant de repondre a la photo", kTextCancelledNode);
    if (g.restore_due) {
        g.restore_due = false;
        restore(g.m0, "photo refusee par l'hote");
        g.m0 = Snap{};
    }
    return false;
}

void rungear_on_mm(uint8_t from, const MmMsg& m) {
    if (m.kind != MM_RUNREV || m.a != kRunRevMsg) return;
    if (!is_guest() || from != kHostPeer) return;
    g.rev_received = m.x;
    g.host_flags   = m.b;
    memcpy(&g.host_seed, m.text, sizeof(g.host_seed));
    g.have_rev     = true;
    if (g.active && !g.adopted) {
        g.adopted      = true;
        g.entered_seed = g.host_seed;
    }
    sync_rev_applied();
}

void rungear_on_team(uint8_t from, const uint8_t* data, size_t size) {
    const uint8_t kind = team_kind(data, size);
    if (kind == kTeamRunGear) {
        RunGearMsg m;
        if (!is_host() || from == kHostPeer || from >= kPeers || !team_rungear_decode(data, size, m)) {
            log_line("SAC", "!! photo inattendue ou illisible de %u -- ignoree", (unsigned)from);
            return;
        }
        ++g.received;
        log_line("SAC", "<- photo %u de %s : %zu chat(s), revision %u (l'aventure en est a la %u)", m.seq,
                 mm_peer_name(from).c_str(), m.cats.size(), m.base, g.rev);
        if (!g.active) {
            refuse(from, m, kRunGearStale, "plus d'aventure ensemble chez l'hote", GetTickCount64());
            return;
        }
        if (g.pend[from].have)
            log_line("SAC", "la photo %u de %s, pas encore traitee, est remplacee", g.pend[from].msg.seq,
                     mm_peer_name(from).c_str());
        g.pend[from].have = true;
        g.pend[from].msg  = std::move(m);
        return;
    }
    if (kind == kTeamRunGearAck) {
        RunGearAck a;
        if (!is_guest() || from != kHostPeer || !team_rungear_ack_decode(data, size, a)) {
            log_line("SAC", "!! reponse inattendue ou illisible -- ignoree");
            return;
        }
        const bool mine = g.unconfirmed && a.seq == g.unconf_seq;
        if (!mine) {
            log_line("SAC", "<- reponse a la photo %u (%s) : deja traitee ici", a.seq,
                     a.result == kRunGearApplied ? "prise" : "refusee");
            return;
        }
        g.unconfirmed = false;
        if (a.result == kRunGearApplied) {
            log_line("SAC", "<- photo %u prise par l'hote (revision %u)", a.seq, a.rev);
            g.m0 = Snap{};
            return;
        }
        log_line("SAC", "<- photo %u refusee par l'hote (%s) : l'etat de l'hote revient", a.seq,
                 a.result == kRunGearStale ? "l'aventure avait change"
                 : a.result == kRunGearNode ? "il est entre dans une case" : "invalide");
        // Ses objets d'abord comme a l'ouverture, puis les poussees de l'hote
        // (arrivees avant cette reponse) par-dessus.
        g.restore_due = true;
        if (a.result == kRunGearNode) { tell(Why::Node, kTextCancelledNode); return; }
        tell(Why::Behind, a.result == kRunGearStale
            ? std::string(u8"Objets\n\nTes changements n'ont pas été pris : le sac\n"
                          u8"a changé chez l'hôte pendant ce temps.\n"
                          u8"Tes chats et le sac reviennent comme chez lui :\nrecommence.")
            : std::string(u8"Objets\n\nTes changements n'ont pas été pris\n"
                          u8"(le journal du mod dit pourquoi).\n"
                          u8"Tes chats et le sac reviennent comme chez l'hôte."));
    }
}

} // namespace mgmp
