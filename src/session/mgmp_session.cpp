// mgmp_session.cpp -- see mgmp_session.h.

#include "mgmp_cursor.h"
#include "mgmp_overlay.h"
#include "mgmp_session.h"
#include "mm_menu.h"   // Mewgenics Multi : noms des joueurs
#include "mgmp_net.h"
#include "mgmp_catsync.h"
#include "mgmp_invsync.h"
#include "mgmp_aim.h"
#include "mgmp_nodehash.h"
#include "mgmp_runhist.h"
#include "mgmp_lockstep.h"
#include "mgmp_follow.h"
#include "mgmp_choice.h"
#include "mgmp_savefile.h"
#include "mgmp_leave.h"
#include "mm_world.h"         // Mewgenics Multi : le monde partage
#include "mm_npc.h"           // Mewgenics Multi : les PNJ en direct
#include "mm_team.h"          // Mewgenics Multi : l'equipe mixte
#include "mm_nemesis.h"
#include "mm_embark.h"        // Mewgenics Multi : le depart ensemble
#include "mgmp_config.h"
#include "mgmp_hooks.h"     // hooks_install_late, for a session started from the panel
#include "mgmp_tuning.h"
#include "mgmp_rng.h"
#include "mgmp_log.h"

#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace mgmp {
namespace {

enum class Phase { Off, Waiting, HelloSent, Ready, Refused };

// A runtime host/join/disconnect asked for by the debug panel, applied at the
// top of session_update. See the note on session_request_host in the header for
// why it cannot be applied where it is asked for.
enum class Request : uint8_t { None, Host, Join, Disconnect };

struct State {
    Phase    phase = Phase::Off;
    bool     started = false;
    char     status[192] = "off";
    uint64_t gpak_hash = 0;
    uint64_t build_hash = 0;
    // One bit per message type already reported as dropped pre-Ready, so the
    // warning is one line and not one line per frame.
    uint32_t dropped_types = 0;

    Request  request = Request::None;
    char     req_addr[64] = {};
    uint16_t req_port = 0;
    char     last_action[128] = {};
};

State g;

uint64_t fnv1a(const void* p, size_t n, uint64_t h = 1469598103934665603ULL) {
    const uint8_t* b = (const uint8_t*)p;
    for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ULL; }
    return h;
}

// Identity of the shipped game data.
//
// NOT a content hash: resources.gpak is ~5 GB and hashing it would stall
// startup for minutes. This hashes the file SIZE plus the first 1 MiB, which
// covers the whole index -- u32 count then per entry {u16 namelen, char name[],
// u32 size} -- so any change to which assets exist, their names, or their sizes
// moves it. Two different builds, or a modded archive, will not collide in
// practice. It will NOT catch a byte edited inside a payload without changing
// its length; if that ever matters, hash the index properly rather than the
// first megabyte.
uint64_t hash_gpak() {
    wchar_t exe[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH)) return 0;
    wchar_t* slash = wcsrchr(exe, L'\\');
    if (!slash) return 0;
    *slash = 0;

    wchar_t path[MAX_PATH];
    _snwprintf_s(path, _TRUNCATE, L"%s\\resources.gpak", exe);

    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        log_line("SESSION", "!! resources.gpak not found next to the exe -- "
                            "data identity cannot be checked");
        return 0;
    }

    LARGE_INTEGER size{};
    GetFileSizeEx(h, &size);
    uint64_t acc = fnv1a(&size.QuadPart, sizeof(size.QuadPart));

    static uint8_t buf[1u << 20];
    DWORD got = 0;
    if (ReadFile(h, buf, sizeof(buf), &got, nullptr) && got)
        acc = fnv1a(buf, got, acc);
    CloseHandle(h);
    return acc;
}

// Identity of the executable: its size and PE timestamp. Enough to catch "one
// of us updated the game", which is the case that matters -- every address the
// mod hooks is pinned to one build.
uint64_t hash_build() {
    HMODULE base = GetModuleHandleW(nullptr);
    if (!base) return 0;
    auto* dos = (IMAGE_DOS_HEADER*)base;
    auto* nt  = (IMAGE_NT_HEADERS*)((uint8_t*)base + dos->e_lfanew);
    struct { uint32_t stamp, size; } id{
        nt->FileHeader.TimeDateStamp,
        nt->OptionalHeader.SizeOfImage
    };
    return fnv1a(&id, sizeof(id));
}

void set_status(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf_s(g.status, sizeof(g.status), _TRUNCATE, fmt, ap);
    va_end(ap);
}

// Mewgenics Multi : les raisons partent chez l'autre joueur et s'affichent dans
// son salon -- en francais (mm_menu les reconnait aussi en anglais, celles d'une
// version d'avant).
bool check_peer(const Hello& h, char* why, size_t why_size) {
    if (h.proto != kProtoVersion) {
        _snprintf_s(why, why_size, _TRUNCATE,
                    "version du mod différente (protocole %u d'un côté, %u de l'autre)",
                    h.proto, kProtoVersion);
        return false;
    }
    if (g.build_hash && h.build_hash && h.build_hash != g.build_hash) {
        _snprintf_s(why, why_size, _TRUNCATE,
                    "version du jeu différente (%016llx / %016llx)",
                    (unsigned long long)h.build_hash, (unsigned long long)g.build_hash);
        return false;
    }
    if (g.gpak_hash && h.gpak_hash && h.gpak_hash != g.gpak_hash) {
        _snprintf_s(why, why_size, _TRUNCATE,
                    "fichiers du jeu différents (resources.gpak %016llx / %016llx)",
                    (unsigned long long)h.gpak_hash, (unsigned long long)g.gpak_hash);
        return false;
    }
    return true;
}

Hello our_hello() {
    Hello h{};
    h.proto      = kProtoVersion;
    h.gpak_hash  = g.gpak_hash;
    h.build_hash = g.build_hash;
    // Mewgenics Multi : le pseudo Steam, affiche dans le salon des autres.
    // Le champ est indicatif (diagnostic) : le changer ne touche pas au protocole.
    _snprintf_s(h.name, sizeof(h.name), _TRUNCATE, "%s", mm_local_name());
    return h;
}

void send_our_hello() {
    net_send_hello(our_hello());
    g.phase = Phase::HelloSent;
}

void go_ready(const char* how) {
    // Idempotent. With one client this only ever ran once, so nothing guarded
    // it. With three, every accepted HELLO reaches here -- and re-running it
    // mid-run would re-init lockstep, follow, savefile, catsync and invsync
    // underneath a battle that is already in progress, wiping the roster
    // snapshot and the epoch that the whole desync check is keyed on. A player
    // joining must not disturb the players already playing.
    if (g.phase == Phase::Ready) return;
    g.phase = Phase::Ready;
    set_status("ready (%s)", how);
    log_line("SESSION", "handshake complete -- %s", how);
    lockstep_init();
    follow_init();
    savefile_init();
    leave_init();
    world_init();
    npc_init();
    team_init();
    embark_init();
    catsync_init();
    invsync_init();
    runhist_init();
    nodehash_init();
    aim_init();
    choice_init();
    cursor_init();
    overlay_init();
}

// Refuse one peer. On the host the session survives it -- the other players keep
// playing and the refused peer is simply never welcomed. On a client, being
// refused by the host is the end of the session, because the host is the only
// peer it has.
void refuse_peer(uint8_t peer, const char* why) {
    log_line("SESSION", "!! refusing peer %u: %s", (unsigned)peer, why);
    net_send_refuse_to(peer, why);
    if (net_role() != NetRole::Host) {
        g.phase = Phase::Refused;
        set_status("refused: %s", why);
    } else {
        // Mewgenics Multi (protocole 61) : hors de la liste des joueurs, et son lien
        // ferme 30 s plus tard (le temps de lire le refus). Avant, il restait
        // connecte : il comptait dans la liste (la nuit votee l'attendait, le salon
        // et Steam le comptaient) et gardait l'un des trois numeros.
        net_drop_peer(peer, kRefuseGraceMs);
    }
}

// The catch-up a peer needs to arrive in the run where it stands now: the save,
// then the cats, inventory and history, then the current node, then the
// decisions already taken in the current battle. Run for a peer that has just
// been accepted, and (Mewgenics Multi) for a peer that was playing its own save
// and now asks to join the adventure in progress (MM_RUN_REQ).
void catchup_peer(uint8_t from) {
    // Mewgenics Multi : il revient -- s'il s'etait arrete dans un combat d'avant
    // (desynchronise), il redevient membre des combats, meme si l'hote est sur la
    // carte (pas de reprise de combat plus bas).
    lockstep_peer_back(from);

    // Mewgenics Multi : qui dirige quels chats dans l'aventure en cours, AVANT la
    // save -- le premier combat de l'arrivant en a besoin.
    team_catchup(from);

    // A peer that joined after the save was already published gets it now.
    // Without this it would be accepted and welcomed and then sit forever on a
    // save-selection screen whose input is suppressed, waiting for a broadcast
    // that already happened.
    savefile_catchup(from);

    // ...and the run state on top of it. THE SAVE IS NOT ENOUGH, in either
    // direction, which is why both of these happen and neither replaces the
    // other:
    //
    //   * a peer whose PROCESS restarted has nothing and needs the save -- it
    //     is the only way to construct a run, and it is what carries the map,
    //     whose per-node MapNode+0x118 seeds are what make two peers roll the
    //     same battle;
    //   * a peer that only lost the SOCKET still has its run, declines the
    //     save (see savefile_on_message) and needs exactly this: the cats and
    //     the inventory as they stand now, rather than at whatever node it was
    //     last told about.
    //
    // The forget() calls are load-bearing. Both publishers dedupe against the
    // last push, and that cache is per-RUN, not per-peer -- so without them a
    // reconnecting peer would be told only about whatever happened to change
    // since the last map node, and nothing about the rest.
    //
    // On the map, both publish by broadcast, so peers already in the session
    // are re-sent state they already have. That is idempotent (the client
    // resolves by cat id and applies over itself) and a reconnect is rare.
    // Mewgenics Multi : inside a node, the entry copies go to the joiner alone
    // (follow_catchup) -- see below.
    catsync_forget();
    invsync_forget();
    runhist_forget();

    // Mewgenics Multi (protocole 58) : DANS UNE CASE, PAS L'ETAT ACTUEL AVANT LA
    // CASE. Celui du milieu d'un combat n'est pas celui que le combat a ete
    // construit depuis : un arrivant qui l'appliquait avant d'y entrer tirait un
    // autre niveau (l'historique note le niveau deja tire, qui n'est plus tire) --
    // 17 octobre, 52 caracteres chez l'invite revenu contre 25 chez l'hote, et les
    // decisions rejouees dans un autre combat. follow_catchup envoie l'etat
    // d'entree de la case, puis la case ; l'etat d'apres viendra avec les poussees
    // de retour sur la carte (les caches viennent d'etre oublies : complet).
    //
    // Sur la carte, la derniere case est finie (et deja dans la save) : la
    // renvoyer ferait rentrer l'arrivant dans une case resolue, et ses decisions
    // resteraient a jamais dans sa file. L'etat actuel est alors le bon.
    //
    // Order is load-bearing and it is the same order follow_on_enter_node uses
    // for the live path: save, then cats and inventory, then the node. The joiner
    // acts on the node the moment it can, and a battle built before its cats
    // arrive is a battle built from the wrong cats.
    // follow_in_node (protocole 59) : pas follow_inside_node seul, qui s'efface
    // quand la carte tourne pendant un evenement -- l'arrivant prenait alors le
    // chemin « sur la carte », sans la case ni son etat d'entree.
    if (!follow_in_node() || !follow_catchup(from)) {
        catsync_publish("a peer joined or reconnected");
        invsync_publish("a peer joined or reconnected");
        // The used-event list belongs in the same burst and for the same reason:
        // a peer whose process restarted has the history that was in the save,
        // which is the run's state at the last checkpoint rather than now.
        runhist_publish("a peer joined or reconnected");
        // Mewgenics Multi (protocole 66) : la fin du dernier combat, deja passee --
        // un invite reconnecte dont la copie y attend encore doit la voir
        // (mgmp_orphan.h, regle 1).
        lockstep_resend_battle_end(from);
        return;
    }

    // The decisions already taken in the battle it is walking into. Must come
    // after the node -- these name a battle_id the peer does not recognise
    // until it has entered that node, and until then they would sit in its
    // pending queue as "a battle we have not reached". Which is survivable
    // (they are held, not dropped) but the ordering makes the log readable.
    lockstep_catchup(from);
}

// Accept (or refuse) one peer's HELLO.
//
// Pulled out of session_update's poll loop because that loop STOPS RUNNING once
// this peer is Ready -- session_update returns early and lockstep_pump drains
// the queue instead. A HELLO arriving after that fell through lockstep's switch
// into `default: break;` and was discarded in silence, so the second client to
// join a host was never accepted, never welcomed, and never told why. It sat
// there having printed "connected" while the host printed nothing after "peer N
// connected".
//
// So this must be reachable from BOTH drain sites, which is what
// session_on_hello exists for.
void handle_hello(uint8_t from, const Hello& h) {
    char why[160];
    if (!check_peer(h, why, sizeof(why))) {
        // Refuse THAT peer, not the session. With one client those were the
        // same thing; with three they are not, and dropping everyone because a
        // latecomer showed up on the wrong build would be a worse bug than the
        // one being refused.
        refuse_peer(from, why);
        return;
    }
    log_line("SESSION", "peer %u '%s' accepted (proto %u)",
             (unsigned)from, h.name[0] ? h.name : "?", h.proto);
    mm_on_peer_hello(from, h.name);

    if (net_role() != NetRole::Host) return;

    // Answer with OUR hello, to this peer specifically.
    //
    // The broadcast in session_update only fires while the host is still in
    // Phase::Waiting, so it reaches the FIRST client and nobody after it. A
    // second player would then never see the host's build and gpak hashes and
    // would skip the compatibility check entirely -- the check that caught a
    // stale DLL presenting itself as a turn-0 desync. Every client must get
    // this, whatever phase the host is in.
    net_send_hello_to(from, our_hello());

    // The host owns the run, so it publishes the simulation stream. The client
    // does not need to reproduce the host's RNG *history*, only to arrive at
    // the same state -- and entering a battle does not re-seed (measured across
    // four launches, all entering at s0=967e2d6d328620b1).
    Welcome w{};
    if (uint64_t* s = rng_global_stream())
        for (int i = 0; i < 4; ++i) w.rng_state[i] = s[i];
    const Config& cfg = config();
    // Each peer's own net_control governs that peer. This list is the HOST's,
    // sent so the client can cross-check it: two cats claimed by both sides
    // would have both players driving one cat and is refused on receipt. A cat
    // claimed by neither is not detectable here -- it shows up as that cat
    // never acting, which is at least diagnosable from the roster both peers
    // print at battle start.
    w.cat_count = 0;
    for (uint32_t i = 0; i < cfg.net_control_count && i < 32; ++i)
        w.cats[w.cat_count++] = cfg.net_control[i];
    // To that peer only. A broadcast would re-welcome the players already in
    // the battle and hand them a fresh RNG state mid-run.
    net_send_welcome_to(from, w);

    // Ordering matters: go_ready is what arms the savefile module on the very
    // first peer, and it is a no-op on every peer after that. Either way it has
    // to have happened before the catch-up send.
    go_ready("host");

    // Tout ce qu'il faut pour le mettre a niveau : voir session_catchup_peer.
    catchup_peer(from);
    // Mewgenics Multi : et le monde de l'hote, qu'il reprendra dans sa save.
    world_catchup(from);
    // Et l'etat de ses PNJ, recopie dans sa maison des qu'il y sera.
    npc_catchup(from);
    // Et a qui est la 4e place (protocole 58) -- apres le WELCOME : son module de
    // l'equipe n'ecoute qu'une fois la session prete chez lui.
    team_slot_hello(from);
}

} // namespace

// ---------------------------------------------------------------------------

void session_on_hello(uint8_t from, const Hello& h) { handle_hello(from, h); }

void session_catchup_peer(uint8_t peer) {
    if (net_role() != NetRole::Host || !net_active()) return;
    log_line("SESSION", "catching peer %u up to the adventure in progress (it asked to join)",
             (unsigned)peer);
    catchup_peer(peer);
}

namespace {

void note_action(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _vsnprintf_s(g.last_action, sizeof(g.last_action), _TRUNCATE, fmt, ap);
    va_end(ap);
    log_line("SESSION", "%s", g.last_action);
}

// Open a socket in one role and arm the handshake. The one path into a live
// session, whether the ini asked for it at launch or somebody pressed a button
// forty minutes in -- so a runtime connect cannot drift from the launch one.
bool begin(bool host, const char* addr, uint16_t port) {
    g.started       = true;
    g.dropped_types = 0;   // a fresh session gets a fresh set of warnings

    // BEFORE the socket, because five modules and six hooks read the role and
    // not the socket. A session started from the panel in a process launched
    // with role = off used to leave config().net_role saying "off" forever:
    // net_role() was right, so cursors, follow, choice and lockstep worked, and
    // savefile, catsync, invsync, runhist and aim quietly never armed. The
    // client connected, saw the host's mouse, and never received the run.
    //
    // Doing it here rather than in apply_request covers session_start() too,
    // where it is a no-op because the file already said host or client.
    if (config_set_role(host)) {
        log_line("SESSION", "role set to %s from the menu (mgmp.json said 'off')",
                 host ? "host" : "client");
        // Mewgenics Multi installe les crochets coop des le lancement (le role
        // se choisit dans le menu) : en temps normal il n'y a plus rien a
        // ajouter ici, et zero n'est pas une erreur.
        const int late = hooks_install_late();
        if (late)
            log_line("SESSION", "installed %d hook(s) that role = off had skipped",
                     late);
    }

    // Computed once per process, not per session: hashing the gpak index is
    // ~1 MiB of file read and neither hash can change while the process lives.
    if (!g.gpak_hash && !g.build_hash) {
        g.gpak_hash  = hash_gpak();
        g.build_hash = hash_build();
        log_line("SESSION", "data identity: gpak %016llx build %016llx",
                 (unsigned long long)g.gpak_hash, (unsigned long long)g.build_hash);
    }

    if (!(host ? net_host(port) : net_join(addr, port))) {
        set_status("failed: %s", net_error());
        g.phase = Phase::Off;
        // Left started = false so another attempt is possible. A failed dial is
        // the single most likely thing to want to retry, and the reconnect
        // testing this exists for consists mostly of retrying.
        g.started = false;
        return false;
    }

    g.phase = Phase::Waiting;
    if (host) set_status("hosting on port %u", (unsigned)port);
    else      set_status("joining %s:%u", addr, (unsigned)port);
    return true;
}

// Apply whatever the panel asked for, at the top of a frame rather than in the
// middle of one. See the header note on session_request_host.
void apply_request() {
    const Request req = g.request;
    if (req == Request::None) return;
    g.request = Request::None;

    // Always tear down first, whichever way round. Host-while-hosting and
    // join-while-joined both mean "start over" -- that is what the button is
    // for during a reconnect test -- and net_host would otherwise fail to bind
    // a port this process still holds.
    if (g.started || net_active()) {
        session_shutdown();
        if (req == Request::Disconnect)
            note_action("disconnected on request from the panel");
    } else if (req == Request::Disconnect) {
        note_action("disconnect requested, but there was no session");
        return;
    }

    if (req == Request::Disconnect) return;

    const bool host = (req == Request::Host);
    if (begin(host, g.req_addr, g.req_port)) {
        if (host) note_action("hosting on port %u (from the panel)", (unsigned)g.req_port);
        else      note_action("dialling %s:%u (from the panel)", g.req_addr,
                              (unsigned)g.req_port);
    } else {
        note_action("!! could not %s: %s", host ? "host" : "connect", net_error());
    }
}

} // namespace

bool session_start() {
    if (g.started) return g.phase != Phase::Off;

    const Config& cfg = config();
    bool host   = _stricmp(cfg.net_role, "host") == 0;
    bool client = _stricmp(cfg.net_role, "client") == 0;
    if (!host && !client) {
        // The default. No socket is opened -- but the panel's connect controls
        // can still open one later, which is why session_start's `started`
        // latch is no longer set on this path.
        set_status("off");
        return false;
    }

    return begin(host, cfg.net_addr, cfg.net_port);
}

void session_request_host(uint16_t port) {
    g.request  = Request::Host;
    g.req_port = port;
}

void session_request_join(const char* addr, uint16_t port) {
    g.request  = Request::Join;
    g.req_port = port;
    _snprintf_s(g.req_addr, sizeof(g.req_addr), _TRUNCATE, "%s", addr ? addr : "");
}

void session_request_disconnect() { g.request = Request::Disconnect; }

bool        session_request_pending() { return g.request != Request::None; }
const char* session_last_action()     { return g.last_action; }

void session_shutdown() {
    if (!g.started) return;
    cursor_shutdown();
    overlay_shutdown();
    savefile_shutdown();
    leave_shutdown();
    world_shutdown();
    npc_shutdown();
    embark_shutdown();
    team_shutdown();
    nemesis_shutdown();               // Mewgenics Multi : la Nemesis a plusieurs
    catsync_shutdown();
    invsync_shutdown();
    runhist_shutdown();
    nodehash_shutdown();
    aim_shutdown();
    follow_shutdown();
    choice_shutdown();
    lockstep_shutdown();
    net_shutdown();
    g.phase = Phase::Off;
    g.started = false;
}

void session_update() {
    // BEFORE the early return below, deliberately: a Host or Join pressed with
    // no session at all arrives with phase == Off, which is precisely the state
    // that return exists to skip.
    apply_request();

    if (g.phase == Phase::Off || g.phase == Phase::Refused) return;

    // Send our HELLO as soon as the socket is up. Both sides do this
    // unconditionally, so neither has to know who speaks first.
    if (g.phase == Phase::Waiting && net_state() == NetState::Connected)
        send_our_hello();

    if (g.phase == Phase::Ready) {
        lockstep_pump();
        // The host's save is published from here rather than at the click,
        // because the click routinely happens before a peer has connected --
        // and, on a new game, before the file it names exists on disk.
        savefile_pump();
        // Watches the loaded-scene list on both roles: the host for its own
        // departure, the client for its arrival back on the menu.
        leave_pump();
        // Mewgenics Multi : l'hote renvoie son monde quand sa save change.
        world_pump();
        // Mewgenics Multi : les PNJ en direct (l'hote lit, l'invite recopie).
        npc_pump();
        // Mewgenics Multi : l'equipe mixte (fenetres, joueurs partis).
        team_pump();
        return;
    }

    // Handshake messages are drained here rather than in lockstep_pump, so
    // lockstep never has to know about a state it cannot act in.
    NetMsg m{};
    while (net_poll(m)) {
        switch (m.type) {
            case MSG_HELLO:
                handle_hello(m.from, m.hello);
                break;

            case MSG_MM:
                mm_on_message(m.from, m.mm);
                break;

            // Mewgenics Multi : une diffusion du monde de l'hote qui arrive
            // pendant la presentation. Sans effet : l'hote renvoie sa derniere
            // copie juste apres WELCOME (world_catchup).
            case MSG_WORLD:
                break;
            // Pareil pour l'etat des PNJ : renvoye juste apres WELCOME (npc_catchup).
            case MSG_NPC:
                break;
            // L'equipe mixte ne parle qu'une fois la session prete ; les
            // proprietaires de l'aventure en cours suivent WELCOME (team_catchup).
            case MSG_TEAM:
                break;


            // Membership changed: someone joined or left. Logged on every peer
            // because the control split is derived from this list, so a session
            // that splits cats wrongly is diagnosed by comparing these lines
            // before anything else.
            case MSG_PEERS: {
                char list[64] = {};
                int  off = 0;
                for (uint8_t i = 0; i < m.peers.count && off < (int)sizeof(list) - 8; ++i)
                    off += _snprintf_s(list + off, sizeof(list) - off, _TRUNCATE,
                                       "%s%u", i ? "," : "", (unsigned)m.peers.ids[i]);
                log_line("SESSION", "peers: %u in session [%s] -- we are peer %u%s",
                         (unsigned)m.peers.count, list, (unsigned)m.peers.you,
                         m.peers.you == kHostPeer ? " (host)" : "");
                break;
            }

            case MSG_WELCOME:
                if (net_role() == NetRole::Client) {
                    // Refuse an overlapping split before anything else: two
                    // players driving one cat produces two decisions for one
                    // brain, which is a desync on the very first turn either
                    // of them acts.
                    const Config& cfg = config();
                    for (uint8_t i = 0; i < m.welcome.cat_count; ++i) {
                        for (uint32_t j = 0; j < cfg.net_control_count; ++j) {
                            if (m.welcome.cats[i] != cfg.net_control[j]) continue;
                            char why[160];
                            _snprintf_s(why, sizeof(why), _TRUNCATE,
                                        "réglage net.control : le chat %u est demandé des deux côtés (mgmp.json)",
                                        (unsigned)m.welcome.cats[i]);
                            refuse_peer(m.from, why);
                            return;
                        }
                    }
                    log_line("SESSION", "host controls %u cat(s)", m.welcome.cat_count);

                    // The host's simulation stream is REPORTED, not adopted.
                    //
                    // Adopting it here would be wrong: the handshake happens
                    // whenever both instances are up -- at the main menu, most
                    // likely -- and the battle starts much later. Between the
                    // two, each peer consumes TLS+0x178 independently (cat
                    // generation, cosmetics, menu rolls), so a state copied at
                    // connect is stale long before the first roll that matters.
                    // Worse, writing another peer's state into a live stream
                    // mid-menu perturbs whatever the local game was mid-way
                    // through.
                    //
                    // The first milestone gets its shared seed the documented
                    // way instead: copy the host's save file. Entering a battle
                    // does not re-seed -- measured across four launches, all
                    // entering at s0=967e2d6d328620b1 -- so the seed travels
                    // with the save. What this comparison buys is that a
                    // MISMATCH is reported here, at connect, with an actionable
                    // message, rather than as an inexplicable desync on turn 3.
                    // Reported, never compared. The handshake completes during
                    // the loading screen -- measured: both peers connected
                    // before either had loaded a save -- so at this moment
                    // neither stream has been seeded from a save file yet and a
                    // difference here means nothing at all. An earlier version
                    // warned "the two saves are not the same file" here, which
                    // was alarming and wrong on both counts.
                    //
                    // The comparison that does mean something is the per-turn
                    // HASH, whose rng_hash covers this same 32-byte state at a
                    // point where both peers are in the same battle.
                    if (uint64_t* s = rng_global_stream())
                        log_line("SESSION", "sim stream at connect: ours s0=%016llx, "
                                 "host s0=%016llx (informational -- no save is "
                                 "loaded yet; the turn hash is the real check)",
                                 (unsigned long long)s[0],
                                 (unsigned long long)m.welcome.rng_state[0]);
                    go_ready("client");
                    // STOP DRAINING HERE.
                    //
                    // go_ready does not end this loop, and everything after
                    // WELCOME in the host's burst -- SAVEFILE, CATDATA,
                    // INVENTORY -- has no case in this switch. They would fall
                    // into `default` and be dropped, while this peer was
                    // already Ready and perfectly able to handle them.
                    //
                    // Measured 2026-08-26: a client reconnecting to a host
                    // mid-adventure sat on "waiting for the host to choose a
                    // save file" while its own byte counter showed the 73728
                    // byte save had arrived. Breaking out lets lockstep_pump,
                    // which owns the Ready path, take the rest of the queue on
                    // the very next line.
                    net_msg_release(m);
                    lockstep_pump();
                    return;
                }
                break;

            case MSG_REFUSE:
                // Mewgenics Multi (protocole 61) : chez l'hote, le refus d'UN
                // invite (une autre version) ne ferme pas le salon -- avant, le
                // premier ami mal a jour mettait l'hote en « refuse » pour toute la
                // session, et les suivants n'avaient jamais de reponse.
                if (net_role() == NetRole::Host) {
                    log_line("SESSION", "!! peer %u refused this host: %s -- dropped, the lobby stays open",
                             (unsigned)m.from, m.refuse);
                    net_drop_peer(m.from, kRefuseGraceMs);
                    break;
                }
                g.phase = Phase::Refused;
                set_status("peer refused: %s", m.refuse);
                log_line("SESSION", "!! peer refused us: %s", m.refuse);
                return;

            // NOT silent, deliberately.
            //
            // This is the second time a quiet `default` in a drain loop cost a
            // session: once when a second client's HELLO fell through
            // lockstep_pump's, and again when a reconnecting client's SAVEFILE
            // fell through this one. A message this peer is not ready for is a
            // fact worth one line -- and exactly one, because the sender may
            // repeat it.
            default:
                if (m.type < 32 && !(g.dropped_types & (1u << m.type))) {
                    g.dropped_types |= (1u << m.type);
                    log_line("SESSION", "!! dropped a type-%u message from peer %u "
                                        "that arrived before this peer was ready "
                                        "-- if the run misbehaves, this is why",
                             (unsigned)m.type, (unsigned)m.from);
                }
                break;
        }
        // MSG_SAVEFILE owns memory, and it CAN arrive here: the host's
        // savefile_catchup sends from inside its HELLO handler, which is well
        // before this peer reaches Ready. The comment that used to sit here
        // said the opposite and was the premise the drop above relied on.
        net_msg_release(m);
    }

    // Mewgenics Multi (protocole 61) : un refus garde sa raison -- l'hote ferme la
    // connexion apres son REFUSE (net_drop_peer ; une partie complete, tout de
    // suite, souvent avant cette ligne).
    if (g.phase == Phase::Refused) return;
    if (net_state() == NetState::Failed) set_status("failed: %s", net_error());
    else if (net_state() == NetState::Closed) set_status("peer disconnected");
}

bool        session_ready()  { return g.phase == Phase::Ready; }
const char* session_status() { return g.status; }

} // namespace mgmp
