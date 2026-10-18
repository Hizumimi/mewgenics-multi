// mgmp_net.cpp -- Winsock TCP transport. See mgmp_net.h for the design rule.

#include "mgmp_net.h"
#include "mgmp_log.h"
#include "mgmp_recvq.h"   // Mewgenics Multi (protocole 59) : la file qui grandit

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <cstdio>

#pragma comment(lib, "ws2_32.lib")

namespace mgmp {
namespace {

// The receive queue. Mewgenics Multi (protocole 59) : it GROWS now (256, then
// doubling up to kRecvQueueMax), and a cursor or aim message replaces the one
// from the same author still queued -- see mgmp_recvq.h. The original rule ("a
// turn never produces 256, so overflow is a bug worth counting") missed the
// game thread being FROZEN, not broken: a save or battle load stops the drain
// for seconds while the receive threads keep going, and the relayed cursors of
// three other players fill 256 slots in about four.

// The largest frame this build will accept. Everything except MSG_SAVEFILE is
// under 128 bytes; a save is ~45 KB, and the slack is for saves that grow.
constexpr uint32_t kMaxFrame = 1u << 20;
static_assert(kMaxFrame <= kMaxPayload, "frame cap must fit the protocol cap");

// One connection. On the host there is one per client and `id` is that client's
// peer id; on a client there is exactly one, links[0], and it is the host.
//
// Each link owns a receive thread for its whole lifetime, rather than one thread
// select()ing over all of them. Three sockets do not justify a readiness loop,
// and a thread per link keeps the per-link byte stream strictly ordered with no
// interleaving logic to get wrong -- which matters because the relay runs on
// this thread.
struct Link {
    SOCKET        sock   = INVALID_SOCKET;
    HANDLE        thread = nullptr;
    uint8_t       id     = kNoPeer;
    volatile LONG live   = 0;
    // Mewgenics Multi (protocole 61), hote : un pair refuse -- sa connexion se
    // ferme a cette heure (GetTickCount64), 0 sinon. D'ici la il n'est plus dans
    // la liste des joueurs, ne recoit plus rien et n'est plus ecoute : seulement
    // le temps de lire le refus (une version d'avant affiche « connexion coupee »
    // des que la connexion se ferme).
    volatile LONG64 drop_at = 0;
    // ... et l'heure ou elle a ete coupee (shutdown), 0 sinon : sans fin de son
    // cote apres kForceCloseMs (un jeu fige), elle est fermee pour de bon.
    volatile LONG64 cut_at  = 0;
};

struct State {
    NetRole  role  = NetRole::None;
    volatile LONG state = (LONG)NetState::Idle;

    SOCKET   listener = INVALID_SOCKET;

    Link     links[kMaxPeers];
    uint8_t  self     = kNoPeer;     // our own peer id; kHostPeer on the host

    HANDLE   accept_thread = nullptr;
    volatile LONG stop = 0;

    CRITICAL_SECTION cs;
    bool     cs_ready = false;

    RecvQueue<NetMsg> queue;
    bool     drop_said = false;   // le premier message jete, dit une fois
    uint32_t grown_said = 0;      // la plus grande capacite deja dite

    CRITICAL_SECTION send_cs;
    bool     send_cs_ready = false;

    // Membership, host-side. Rebroadcast to everyone whenever it changes.
    PeersMsg roster;
    bool     roster_valid = false;

    char     error[256] = {};
    NetStats stats;

    bool     wsa_up = false;

    // Mewgenics Multi : connexion a l'hote en cours, sans bloquer le jeu.
    // connect() bloquant sur le thread du jeu le figeait ~20 s quand
    // l'adresse tapee ne repondait pas.
    SOCKET    pending = INVALID_SOCKET;
    ULONGLONG pending_deadline = 0;
};

State g;

constexpr ULONGLONG kConnectTimeoutMs = 8000;

void set_state(NetState s) { InterlockedExchange(&g.state, (LONG)s); }

void fail(const char* what, int err) {
    _snprintf_s(g.error, sizeof(g.error), _TRUNCATE, "%s failed (WSA %d)", what, err);
    set_state(NetState::Failed);
    log_line("NET", "!! %s", g.error);
}

bool wsa_init() {
    if (g.wsa_up) return true;
    WSADATA wsa{};
    int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (rc != 0) { fail("WSAStartup", rc); return false; }
    g.wsa_up = true;
    return true;
}

// Mewgenics Multi (protocole 59) : « le plus recent suffit » -- la position du
// curseur et la visee d'un joueur. Ni l'un ni l'autre ne possede de memoire : un
// message en file peut etre ecrase par le suivant du meme auteur.
bool latest_wins(uint8_t type) { return type == MSG_CURSOR || type == MSG_AIM; }

void queue_push(const NetMsg& m) {
    EnterCriticalSection(&g.cs);
    const RecvPush r = g.queue.push(m, latest_wins(m.type),
                                    [](const NetMsg& q, const NetMsg& n) {
                                        return q.type == n.type && q.from == n.from;
                                    });
    if (r == RecvPush::Dropped) {
        // Drop the newest rather than the oldest. Losing the oldest would
        // reorder the action stream, and an out-of-order ACTION is a desync;
        // a dropped one is at least detectable as a stall at the turn barrier.
        // Mewgenics Multi : seulement a kRecvQueueMax messages en attente -- le
        // jeu ne vide plus du tout la file.
        ++g.stats.dropped;
        const bool say = !g.drop_said;
        g.drop_said = true;
        const uint32_t queued = g.queue.count;
        LeaveCriticalSection(&g.cs);
        if (say)
            log_line("NET", "!! receive queue full (%u messages waiting) -- DROPPED a %s from peer %u; "
                            "the game has not read the network for a long time",
                     queued, msg_name(m.type), (unsigned)m.from);
        // The frame is ours by the time it reaches here, so dropping it means
        // freeing it -- a dropped SAVEFILE that leaked would be 45 KB gone with
        // nothing to say so.
        NetMsg dead = m;
        net_msg_release(dead);
        return;
    }
    ++g.stats.received;
    uint32_t grew_to = 0;
    if (r == RecvPush::Grown && g.queue.cap > kRecvQueueStart && g.queue.cap > g.grown_said) {
        g.grown_said = g.queue.cap;
        grew_to = g.queue.cap;
    }
    LeaveCriticalSection(&g.cs);
    if (grew_to)
        log_line("NET", "receive queue grew to %u slots -- the game was not reading the network "
                        "(a load?); nothing was lost", grew_to);
}

// --- link table -------------------------------------------------------------
//
// `live` is the lock-free publication flag that readers test (sends, relays,
// the roster). A slot's LIFECYCLE -- its socket, id, drop_at and `live` going
// up or down -- changes only under send_cs, the lock every send already takes:
// start_link fills a slot (accept thread), the slot's own link thread empties
// it (takes its socket, closes it, then clears `live`), and the game thread
// marks a refused peer (net_drop_peer) or ends its grace (pump_refused).
//
// Mewgenics Multi (protocole 61) : another thread does not CLOSE a live link's
// socket, it shuts it down, under send_cs: the peer ends its side, the owner's
// recv returns and the owner closes. Closing a handle under a thread still using
// it is how a handle VALUE gets reused by the next accept while the dying thread
// reads from it. Two exceptions: net_shutdown closes everything, after setting
// g.stop, which every receive loop tests before each recv; and a refused link
// still live kForceCloseMs after its cut (close_cut_locked, from pump_refused or
// evict_refused) -- its peer never ended its side, so its owner is parked in
// recv, which then fails, and recv_exact never reads again after an error.
//
// A slow peer must still not be able to stall the accept thread or another
// peer's relay beyond what a blocking send already does.

// Mewgenics Multi (protocole 61) : prend la socket d'un lien et la laisse
// INVALID_SOCKET, en une operation -- celui qui la recoit est le seul a la fermer.
SOCKET take_link_socket(Link& l) {
    static_assert(sizeof(SOCKET) == sizeof(PVOID), "SOCKET is pointer-sized on x64");
    return (SOCKET)InterlockedExchangePointer((PVOID volatile*)&l.sock, (PVOID)INVALID_SOCKET);
}

// Mewgenics Multi (protocole 61) : ce lien est celui d'un pair refuse (net_drop_peer).
bool link_dropping(Link& l) { return InterlockedCompareExchange64(&l.drop_at, 0, 0) != 0; }

// drop_at d'un lien refuse deja coupe (shutdown) : jamais plus « l'heure ».
constexpr LONG64 kDropDone = 0x7FFFFFFFFFFFFFFFLL;

// Un lien refuse coupe et toujours la apres ce delai : son pair ne ferme pas (un
// jeu fige qui ne lit plus) -- fermee ici. Son fil est alors bloque dans recv,
// qui rend une erreur ; et recv_exact ne relit jamais apres une erreur.
constexpr uint64_t kForceCloseMs = 10000;

// Sous send_cs : coupe (shutdown seulement) le lien refuse du slot `i`. Le pair
// recoit la fin de connexion et ferme la sienne ; le fil du lien sort alors de
// recv, ferme la socket et libere le slot.
void cut_refused_locked(int i, const char* why) {
    Link& l = g.links[i];
    if (l.sock != INVALID_SOCKET) shutdown(l.sock, SD_BOTH);
    InterlockedExchange64(&l.drop_at, kDropDone);
    InterlockedExchange64(&l.cut_at, (LONG64)GetTickCount64());
    log_line("NET", "peer %u (refused) -- connection cut (%s)", (unsigned)l.id, why);
}

// Sous send_cs : ferme pour de bon le lien refuse et deja coupe du slot `i`.
void close_cut_locked(int i, const char* why) {
    Link& l = g.links[i];
    const SOCKET s = take_link_socket(l);
    if (s != INVALID_SOCKET) closesocket(s);
    InterlockedExchange64(&l.cut_at, 0);
    log_line("NET", "peer %u (refused) -- connection closed (%s)", (unsigned)l.id, why);
}

// Hote, sur le fil du jeu (net_poll) : la fin du delai des pairs refuses.
void pump_refused() {
    if (g.role != NetRole::Host || !g.send_cs_ready) return;
    const uint64_t now = GetTickCount64();
    for (int i = 0; i < kMaxPeers; ++i) {
        Link& l = g.links[i];
        const LONG64 at  = InterlockedCompareExchange64(&l.drop_at, 0, 0);
        const LONG64 cut = InterlockedCompareExchange64(&l.cut_at, 0, 0);
        const bool due_cut   = at && at != kDropDone && now >= (uint64_t)at;
        const bool due_close = at == kDropDone && cut && now >= (uint64_t)cut + kForceCloseMs;
        if (!due_cut && !due_close) continue;
        EnterCriticalSection(&g.send_cs);
        // Relu sous le verrou : le meme lien (start_link remet drop_at et cut_at a
        // 0), encore la.
        if (InterlockedCompareExchange(&l.live, 0, 0) &&
            InterlockedCompareExchange64(&l.drop_at, 0, 0) == at &&
            InterlockedCompareExchange64(&l.cut_at, 0, 0) == cut) {
            if (due_cut) cut_refused_locked(i, "the 30 s to read the refusal are over");
            else         close_cut_locked(i, "it did not end on its side");
        }
        LeaveCriticalSection(&g.send_cs);
    }
}

int link_slot_of(uint8_t peer) {
    for (int i = 0; i < kMaxPeers; ++i)
        if (InterlockedCompareExchange(&g.links[i].live, 0, 0) && g.links[i].id == peer)
            return i;
    return -1;
}

int free_link_slot() {
    for (int i = 0; i < kMaxPeers; ++i)
        if (!InterlockedCompareExchange(&g.links[i].live, 0, 0)) return i;
    return -1;
}

// Read exactly n bytes, or fail. Returns false on close/error/stop.
bool recv_exact(SOCKET s, uint8_t* p, uint32_t n) {
    uint32_t got = 0;
    while (got < n) {
        if (InterlockedCompareExchange(&g.stop, 0, 0)) return false;
        int r = recv(s, (char*)(p + got), (int)(n - got), 0);
        if (r == 0)  return false;                  // orderly shutdown
        // Mewgenics Multi (protocole 61) : toute erreur finit le lien, WSAEINTR
        // compris -- sous Windows il ne vient que d'une socket fermee par un autre
        // fil (net_shutdown), et la relire serait lire un handle peut-etre deja
        // celui d'une autre connexion.
        if (r < 0) return false;
        got += (uint32_t)r;
    }
    return true;
}

bool decode_into(const uint8_t* buf, uint32_t len, NetMsg& m) {
    if (len < 1) return false;
    Reader r(buf, len);
    m.type = r.u8v();
    switch (m.type) {
        case MSG_HELLO:   return dec_hello(r, m.hello);
        case MSG_WELCOME: return dec_welcome(r, m.welcome);
        case MSG_ACTION:  return dec_action(r, m.action);
        case MSG_HASH:    return dec_hash(r, m.hash);
        case MSG_CONTROL: return dec_control(r, m.control);
        case MSG_CURSOR:  return dec_cursor(r, m.cursor);
        case MSG_AIM:     return dec_aim(r, m.aim);
        case MSG_ENTERNODE: return dec_enter_node(r, m.enter_node);
        case MSG_CHOICE:    return dec_choice(r, m.choice);
        case MSG_SAVEFILE: return dec_savefile(r, m.savefile);
        // Mewgenics Multi : le meme cadre, dans le meme champ (mm_world.cpp).
        case MSG_WORLD:    return dec_savefile(r, m.savefile);
        case MSG_NPC:      return dec_savefile(r, m.savefile);   // (mm_npc.cpp)
        case MSG_TEAM:     return dec_savefile(r, m.savefile);   // (mm_team.cpp)
        case MSG_CATDATA:  return dec_catdata(r, m.catdata);
        case MSG_INVENTORY: return dec_inventory(r, m.inventory);
        case MSG_RUNHIST:   return dec_runhist(r, m.runhist);
        case MSG_STATEDUMP: return dec_statedump(r, m.statedump);
        case MSG_NODEHASH:  return dec_nodehash(r, m.nodehash);
        case MSG_HOSTLEFT:  return dec_hostleft(r, m.hostleft);
        case MSG_MM:        return dec_mm(r, m.mm);
        case MSG_PEERS:   return dec_peers(r, m.peers);
        case MSG_HALT:    return dec_halt(r, m.halt);
        case MSG_REFUSE:  r.str(m.refuse, sizeof(m.refuse)); return r.ok;
        case MSG_PING:    return true;
        default:          return false;
    }
}

// Write one framed message on one socket. `from` is stamped into the envelope:
// our own id when we author a message, the ORIGINATOR's id when the host relays
// one.
bool send_framed(SOCKET s, uint8_t from, const uint8_t* payload, uint32_t len) {
    if (s == INVALID_SOCKET || len == 0) return false;

    uint32_t n = len + kEnvelopeBytes;
    EnterCriticalSection(&g.send_cs);
    bool ok = true;
    const char* parts[3] = { (const char*)&n, (const char*)&from, (const char*)payload };
    int         sizes[3] = { 4, (int)kEnvelopeBytes, (int)len };
    for (int i = 0; i < 3 && ok; ++i) {
        int off = 0;
        while (off < sizes[i]) {
            int r = send(s, parts[i] + off, sizes[i] - off, 0);
            if (r <= 0) { ok = false; break; }
            off += r;
        }
    }
    if (ok) { ++g.stats.sent; g.stats.bytes_sent += n + 4; }
    LeaveCriticalSection(&g.send_cs);
    return ok;
}

// Mewgenics Multi (protocole 61) : envoyer sur un lien. `live`, drop_at et la
// socket relus sous send_cs, le verrou du cycle de vie des slots : la socket ne
// peut pas etre fermee (et son handle repris par une autre connexion) entre la
// lecture et l'envoi. 1 envoye, 0 echec, -1 lien absent ou refuse.
int send_link(Link& l, uint8_t from, const uint8_t* payload, uint32_t len) {
    EnterCriticalSection(&g.send_cs);
    int r = -1;
    if (InterlockedCompareExchange(&l.live, 0, 0) && !link_dropping(l) && l.sock != INVALID_SOCKET)
        r = send_framed(l.sock, from, payload, len) ? 1 : 0;   // send_cs se reprend
    LeaveCriticalSection(&g.send_cs);
    return r;
}

// Which messages the host passes on to the other clients.
//
// Only the ones a CLIENT can author and another client needs. Everything else
// is either host-authored and already going to everyone (SAVEFILE, CATDATA,
// INVENTORY, ENTERNODE, PEERS, WELCOME) or strictly point to point (HELLO,
// REFUSE). Relaying a host-authored message would deliver it twice.
bool relayed(uint8_t type) {
    switch (type) {
        case MSG_ACTION:
        case MSG_HASH:
        case MSG_CONTROL:
        case MSG_CURSOR:
        // AIM, like CURSOR, is authored by whichever peer is aiming and is for
        // everyone else to look at.
        case MSG_AIM:
        case MSG_HALT:
        // NODEHASH is symmetric -- every peer authors its own -- so with more
        // than two players a client's has to reach the other clients, exactly
        // like the per-turn HASH above it.
        case MSG_NODEHASH:
            return true;
        default:
            return false;
    }
}

void host_relay(uint8_t from, const uint8_t* payload, uint32_t len) {
    for (int i = 0; i < kMaxPeers; ++i) {
        Link& l = g.links[i];
        if (!InterlockedCompareExchange(&l.live, 0, 0)) continue;
        if (l.id == from) continue;              // not back to its author
        send_link(l, from, payload, len);        // Mewgenics Multi : pas un pair refuse
    }
}

// Build the current membership and tell everyone, including ourselves.
//
// Each client gets its own copy because `you` differs; the host queues one
// locally so the session layer learns the membership through exactly the same
// path on both sides rather than by reaching into the transport.
//
// Called on the accept thread when a peer arrives and on a link's own thread
// when it leaves. Both are transport threads, never the game thread.
void host_publish_roster() {
    if (g.role != NetRole::Host) return;

    // Mewgenics Multi (protocole 61) : une arrivee (fil d'acceptation) et un
    // depart (fil du lien) au meme instant publiaient chacun leur liste, et
    // l'ordre des deux files pouvait laisser la plus ancienne en dernier -- un
    // joueur parti encore compte, ou un arrivant oublie. Une liste a la fois :
    // la derniere calculee est la derniere envoyee. send_cs, que chaque envoi
    // prend deja (meme fil : il se reprend sans attendre).
    EnterCriticalSection(&g.send_cs);
    PeersMsg roster{};
    roster.ids[roster.count++] = kHostPeer;
    for (uint8_t want = 0; want < 255 && roster.count < kMaxPeers; ++want) {
        for (int i = 0; i < kMaxPeers; ++i) {
            Link& l = g.links[i];
            if (!InterlockedCompareExchange(&l.live, 0, 0)) continue;
            if (l.id != want || l.id == kHostPeer) continue;
            if (link_dropping(l)) continue;      // Mewgenics Multi : refuse, plus un joueur
            roster.ids[roster.count++] = l.id;   // ascending by construction
        }
    }

    g.roster = roster;
    g.roster_valid = true;

    for (int i = 0; i < kMaxPeers; ++i) {
        Link& l = g.links[i];
        if (!InterlockedCompareExchange(&l.live, 0, 0)) continue;
        if (link_dropping(l)) continue;
        PeersMsg mine = roster;
        mine.you = l.id;
        uint8_t p[64];
        uint32_t n = enc_peers(p, sizeof(p), mine);
        if (n) send_framed(l.sock, kHostPeer, p, n);
    }

    NetMsg self{};
    self.type      = MSG_PEERS;
    self.from      = kHostPeer;
    self.peers     = roster;
    self.peers.you = kHostPeer;
    queue_push(self);
    LeaveCriticalSection(&g.send_cs);
}

// The receive loop for ONE link. Owns that link's socket for its lifetime.
void recv_loop(Link& link) {
    // Sized for MSG_SAVEFILE, which is the only frame that is not a few hundred
    // bytes. Heap rather than stack: a receive thread's stack is not the place
    // for a megabyte, and now there is one of these per peer, so it cannot be
    // `static` any more either.
    uint8_t* buf = (uint8_t*)malloc(kMaxFrame);
    if (!buf) { log_line("NET", "!! could not allocate a receive buffer"); return; }

    for (;;) {
        if (InterlockedCompareExchange(&g.stop, 0, 0)) break;

        uint32_t len = 0;
        if (!recv_exact(link.sock, (uint8_t*)&len, 4)) break;
        if (len < kEnvelopeBytes || len > kMaxFrame) {
            log_line("NET", "!! bad frame length %u from peer %u -- closing",
                     len, (unsigned)link.id);
            break;
        }
        if (!recv_exact(link.sock, buf, len)) break;
        // Mewgenics Multi (protocole 61) : un pair refuse n'est plus ecoute (ni
        // relaye, ni mis dans la file) -- la trame est lue en entier, pour rester
        // cale sur le flux, puis laissee.
        if (g.role == NetRole::Host && link_dropping(link)) continue;

        uint8_t         from    = buf[0];
        const uint8_t*  payload = buf + kEnvelopeBytes;
        uint32_t        plen    = len - kEnvelopeBytes;
        if (plen == 0) continue;                 // envelope with no message

        // A client may only speak for itself. Believing a forged `from` would
        // let one peer inject another's decisions, so the host overwrites it
        // with the id it handed that socket rather than trusting what arrived.
        if (g.role == NetRole::Host) from = link.id;

        // Relay BEFORE decoding and queuing. Decoding can allocate and the game
        // thread may be mid-frame; the other clients should not wait on either.
        if (g.role == NetRole::Host && relayed(payload[0]))
            host_relay(from, payload, plen);

        NetMsg m{};
        if (!decode_into(payload, plen, m)) {
            // Mewgenics Multi (protocole 61) : un curseur ou une visee hors des
            // bornes ne coupe plus personne. L'hote relaie AVANT de decoder : une
            // seule visee mal formee fermait le lien de son auteur chez l'hote, et
            // celui de chaque autre invite vers l'hote. Ils ne portent qu'un
            // dessin ; on les laisse tomber (dit une fois).
            if (payload[0] == MSG_CURSOR || payload[0] == MSG_AIM) {
                static volatile LONG said = 0;
                if (InterlockedExchange(&said, 1) == 0)
                    log_line("NET", "!! undecodable %s frame from peer %u, %u bytes -- dropped (only a drawing; "
                                    "said once)", msg_name(payload[0]), (unsigned)from, plen);
                continue;
            }
            log_line("NET", "!! undecodable %s frame from peer %u, %u bytes -- closing",
                     msg_name(payload[0]), (unsigned)from, plen);
            break;
        }
        m.from = from;
        g.stats.bytes_received += len + 4;
        queue_push(m);
    }

    free(buf);

    // Mewgenics Multi (protocole 61) : la socket d'abord, PUIS le drapeau. Dans
    // l'autre ordre, une connexion acceptee entre les deux prenait ce lien libre,
    // et cette fermeture fermait SA socket. Le numero est lu avant : une fois le
    // drapeau baisse, le lien peut deja etre celui d'un nouvel arrivant.
    const uint8_t id = link.id;
    // Sous send_cs (le cycle de vie d'un slot, voir la table des liens) ; pas
    // pendant net_shutdown, qui a deja pris et ferme chaque socket et detruira
    // le verrou apres l'attente des fils.
    const bool stopping = InterlockedCompareExchange(&g.stop, 0, 0) != 0;
    if (!stopping) EnterCriticalSection(&g.send_cs);
    {
        const SOCKET s = take_link_socket(link);
        if (s != INVALID_SOCKET) closesocket(s);
    }
    InterlockedExchange(&link.live, 0);
    if (!stopping) LeaveCriticalSection(&g.send_cs);

    if (!InterlockedCompareExchange(&g.stop, 0, 0)) {
        log_line("NET", "peer %u disconnected", (unsigned)id);
        if (g.role == NetRole::Host) {
            // One client leaving is not the end of the session for the others.
            // Whether the RUN can continue is the session layer's call; the
            // transport only reports the membership change.
            host_publish_roster();
        } else {
            set_state(NetState::Closed);
        }
    }
}

DWORD WINAPI link_thread(LPVOID param) {
    recv_loop(*(Link*)param);
    return 0;
}

void nodelay_on(SOCKET s) {
    // Turn off Nagle. A turn-based game sends one small message and then waits
    // for the reply, which is precisely the pattern Nagle's 200 ms delayed-ACK
    // interaction punishes hardest -- it would add latency to every decision
    // for no throughput gain.
    BOOL nodelay = TRUE;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
}

bool start_link(int slot, SOCKET s, uint8_t id) {
    Link& l = g.links[slot];
    // Mewgenics Multi : le fil de la connexion d'avant sur ce lien (fini, ou qui
    // finit) -- sa poignee etait perdue a chaque reconnexion.
    if (l.thread) { CloseHandle(l.thread); l.thread = nullptr; }
    nodelay_on(s);
    EnterCriticalSection(&g.send_cs);            // Mewgenics Multi : le cycle de vie du slot
    l.sock = s;
    l.id   = id;
    InterlockedExchange64(&l.drop_at, 0);        // avant `live`
    InterlockedExchange64(&l.cut_at, 0);
    InterlockedExchange(&l.live, 1);
    LeaveCriticalSection(&g.send_cs);
    l.thread = CreateThread(nullptr, 0, link_thread, &l, 0, nullptr);
    if (!l.thread) {
        EnterCriticalSection(&g.send_cs);
        const SOCKET t = take_link_socket(l);
        InterlockedExchange(&l.live, 0);
        LeaveCriticalSection(&g.send_cs);
        if (t != INVALID_SOCKET) closesocket(t);
        fail("CreateThread", (int)GetLastError());
        return false;
    }
    return true;
}

// Lowest free id rather than a monotonic counter, so an id is always below
// kMaxPeers and every per-peer table in the mod can be a flat array indexed by
// it. Reuse is safe because the roster is republished on every membership
// change, and because position -- not id -- is what the control split is
// derived from.
uint8_t lowest_free_id() {
    for (uint8_t cand = kHostPeer + 1; cand < kMaxPeers; ++cand)
        if (link_slot_of(cand) < 0) return cand;
    return kNoPeer;
}

// Mewgenics Multi (protocole 61) : un joueur arrive et toutes les places sont
// prises, dont une par un pair refuse qui lit encore son refus -- le joueur passe
// avant lui. Vrai si une place s'est liberee (le fil du refuse a ferme).
bool evict_refused() {
    int victim = -1;
    EnterCriticalSection(&g.send_cs);
    // Un lien pas encore coupe d'abord ; sinon un lien deja coupe et toujours la
    // (son pair ne ferme pas) : ferme pour de bon.
    for (int pass = 0; pass < 2 && victim < 0; ++pass) {
        for (int i = 0; i < kMaxPeers && victim < 0; ++i) {
            Link& l = g.links[i];
            if (!InterlockedCompareExchange(&l.live, 0, 0) || !link_dropping(l)) continue;
            const bool cut = InterlockedCompareExchange64(&l.drop_at, 0, 0) == kDropDone;
            if (pass == 0 && !cut) cut_refused_locked(i, "a player is joining a full session");
            else if (pass == 1)    close_cut_locked(i, "a player is joining a full session");
            else                   continue;
            victim = i;
        }
    }
    LeaveCriticalSection(&g.send_cs);
    if (victim < 0) return false;
    for (int t = 0; t < 100; ++t) {                  // une seconde au plus
        if (!InterlockedCompareExchange(&g.links[victim].live, 0, 0)) return true;
        if (InterlockedCompareExchange(&g.stop, 0, 0)) return false;
        Sleep(10);
    }
    return false;
}

DWORD WINAPI accept_thread(LPVOID) {
    set_state(NetState::Listening);
    log_line("NET", "listening for up to %u peer(s) on the host", (unsigned)(kMaxPeers - 1));

    for (;;) {
        if (InterlockedCompareExchange(&g.stop, 0, 0)) break;

        SOCKET c = accept(g.listener, nullptr, nullptr);
        if (c == INVALID_SOCKET) {
            if (!InterlockedCompareExchange(&g.stop, 0, 0))
                fail("accept", WSAGetLastError());
            break;
        }

        uint8_t id   = lowest_free_id();
        int     slot = free_link_slot();
        if ((slot < 0 || id == kNoPeer) && evict_refused()) {
            id   = lowest_free_id();
            slot = free_link_slot();
        }
        if (slot < 0 || id == kNoPeer) {
            // Refuse politely rather than dropping the socket: a player who
            // turns up to a full session should be told, not left staring at a
            // connect that appears to succeed and then does nothing.
            uint8_t p[128];
            uint32_t n = enc_refuse(p, sizeof(p), "partie complète (4 joueurs au plus)");
            if (n) send_framed(c, kHostPeer, p, n);
            log_line("NET", "!! refused a connection -- the session already has "
                            "%u peer(s), the maximum", (unsigned)kMaxPeers);
            closesocket(c);
            continue;
        }

        if (!start_link(slot, c, id)) continue;

        log_line("NET", "peer %u connected", (unsigned)id);
        set_state(NetState::Connected);
        host_publish_roster();
    }
    return 0;
}

void ensure_cs() {
    if (!g.cs_ready)      { InitializeCriticalSection(&g.cs);      g.cs_ready = true; }
    if (!g.send_cs_ready) { InitializeCriticalSection(&g.send_cs); g.send_cs_ready = true; }
}

} // namespace

// ---------------------------------------------------------------------------

bool net_host(uint16_t port) {
    if (g.role != NetRole::None) return false;
    ensure_cs();
    if (!wsa_init()) return false;

    g.listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g.listener == INVALID_SOCKET) { fail("socket", WSAGetLastError()); return false; }

    // SO_REUSEADDR so a crashed session's TIME_WAIT does not lock the port for
    // the next run. During desync hunting the game gets restarted constantly.
    BOOL reuse = TRUE;
    setsockopt(g.listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    sockaddr_in a{};
    a.sin_family      = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port        = htons(port);
    if (bind(g.listener, (sockaddr*)&a, sizeof(a)) == SOCKET_ERROR) {
        fail("bind", WSAGetLastError()); closesocket(g.listener);
        g.listener = INVALID_SOCKET; return false;
    }
    if (listen(g.listener, kMaxPeers) == SOCKET_ERROR) {
        fail("listen", WSAGetLastError()); closesocket(g.listener);
        g.listener = INVALID_SOCKET; return false;
    }

    g.role = NetRole::Host;
    g.self = kHostPeer;
    log_line("NET", "hosting on port %u", (unsigned)port);

    InterlockedExchange(&g.stop, 0);
    g.accept_thread = CreateThread(nullptr, 0, accept_thread, nullptr, 0, nullptr);
    if (!g.accept_thread) { fail("CreateThread", (int)GetLastError()); return false; }
    return true;
}

// La socket est connectee : retour en mode bloquant (le fil de reception de
// start_link attend dans recv), puis meme suite que l'ancien net_join.
bool finish_connect(SOCKET s) {
    u_long blocking = 0;
    ioctlsocket(s, FIONBIO, &blocking);
    set_state(NetState::Connected);
    log_line("NET", "connected");
    return start_link(0, s, kHostPeer);
}

void drop_pending(const char* what, int err) {
    fail(what, err);
    closesocket(g.pending);
    g.pending = INVALID_SOCKET;
    // Plus rien en cours : une nouvelle tentative (net_join) redevient possible.
    g.role = NetRole::None;
}

// Une interrogation sans attente de la connexion en cours.
void net_pump_connect() {
    if (g.pending == INVALID_SOCKET) return;
    fd_set w, e;
    FD_ZERO(&w); FD_ZERO(&e);
    FD_SET(g.pending, &w);
    FD_SET(g.pending, &e);
    timeval now{0, 0};
    const int r = select(0, nullptr, &w, &e, &now);
    if (r == SOCKET_ERROR) { drop_pending("select", WSAGetLastError()); return; }
    if (r == 0) {
        if (GetTickCount64() > g.pending_deadline) drop_pending("connect", WSAETIMEDOUT);
        return;
    }
    if (FD_ISSET(g.pending, &e)) {
        int err = 0;
        int len = sizeof(err);
        getsockopt(g.pending, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
        drop_pending("connect", err ? err : WSAECONNREFUSED);
        return;
    }
    if (FD_ISSET(g.pending, &w)) {
        SOCKET s = g.pending;
        g.pending = INVALID_SOCKET;
        finish_connect(s);
    }
}

bool net_join(const char* addr, uint16_t port) {
    if (g.role != NetRole::None) return false;
    ensure_cs();
    if (!wsa_init()) return false;

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { fail("socket", WSAGetLastError()); return false; }

    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port   = htons(port);
    if (inet_pton(AF_INET, addr, &a.sin_addr) != 1) {
        _snprintf_s(g.error, sizeof(g.error), _TRUNCATE, "bad address '%s'", addr);
        set_state(NetState::Failed);
        log_line("NET", "!! %s", g.error);
        closesocket(s);
        return false;
    }

    set_state(NetState::Connecting);
    log_line("NET", "connecting to %s:%u", addr, (unsigned)port);

    // Mewgenics Multi : connexion non bloquante. Le role est Client des
    // maintenant (net_role() / net_active() le disent pendant la connexion) ;
    // net_pump_connect, appele a chaque image par net_poll, termine le travail.
    u_long nonblocking = 1;
    ioctlsocket(s, FIONBIO, &nonblocking);

    g.role = NetRole::Client;
    // Our own id is not known until PEERS arrives; until then we can still send,
    // because the host overwrites the envelope's `from` with the id it handed
    // this socket regardless of what we put there.
    g.self = kNoPeer;
    InterlockedExchange(&g.stop, 0);

    if (connect(s, (sockaddr*)&a, sizeof(a)) == 0)
        return finish_connect(s);

    const int err = WSAGetLastError();
    if (err != WSAEWOULDBLOCK) {
        fail("connect", err);
        closesocket(s);
        g.role = NetRole::None;
        return false;
    }
    g.pending = s;
    g.pending_deadline = GetTickCount64() + kConnectTimeoutMs;
    return true;
}

// A client learns its own id from the PEERS message, when net_poll hands it to
// the game thread (below), so the transport and the session layer agree on it.
uint8_t net_self() { return g.self; }

void net_drop_peer(uint8_t peer, uint32_t after_ms) {
    if (g.role != NetRole::Host || peer == kHostPeer || !g.send_cs_ready) return;
    // Hors de la liste des joueurs tout de suite ; la connexion, elle, est coupee
    // par pump_refused a l'heure dite (au prochain net_poll pour 0). La premiere
    // heure dite reste. Sous send_cs : le slot de ce numero ne peut pas changer
    // de connexion entre la recherche et la marque.
    EnterCriticalSection(&g.send_cs);
    const int slot = link_slot_of(peer);
    if (slot >= 0) {
        const LONG64 at = (LONG64)(GetTickCount64() + after_ms);
        if (InterlockedCompareExchange64(&g.links[slot].drop_at, at, 0) == 0) {
            log_line("NET", "peer %u refused -- out of the roster now, connection closed in %u s",
                     (unsigned)peer, after_ms / 1000);
            host_publish_roster();               // meme fil : send_cs se reprend
        }
    }
    LeaveCriticalSection(&g.send_cs);
}
uint8_t net_peer_count() { return g.roster_valid ? g.roster.count : 0; }

// Our INDEX in the sorted membership list, not our id.
//
// The two differ once anyone has disconnected: a session that loses peer 1
// keeps ids {0,2,3} while the positions become {0,1,2} (the next arrival takes
// the lowest free id, 1, again -- see accept_thread). The control split is
// computed from position precisely so that the departure of one player does
// not silently hand another player somebody else's cats.
uint8_t net_peer_pos() {
    if (!g.roster_valid) return 0;
    for (uint8_t i = 0; i < g.roster.count; ++i)
        if (g.roster.ids[i] == g.self) return i;
    return 0;
}

bool net_peer_ids(uint8_t* out, uint8_t cap) {
    if (!g.roster_valid || !out) return false;
    for (uint8_t i = 0; i < g.roster.count && i < cap; ++i) out[i] = g.roster.ids[i];
    return true;
}

void net_shutdown() {
    if (g.pending != INVALID_SOCKET) { closesocket(g.pending); g.pending = INVALID_SOCKET; }
    if (g.role == NetRole::None) return;
    InterlockedExchange(&g.stop, 1);

    // Shut the sockets down before waiting: every receive thread is parked in
    // recv() and will not notice a flag until the call returns. Closing the
    // listener is what releases the accept thread.
    if (g.listener != INVALID_SOCKET) { closesocket(g.listener); g.listener = INVALID_SOCKET; }
    for (int i = 0; i < kMaxPeers; ++i) {
        const SOCKET s = take_link_socket(g.links[i]);
        if (s != INVALID_SOCKET) { shutdown(s, SD_BOTH); closesocket(s); }
    }

    if (g.accept_thread) {
        if (WaitForSingleObject(g.accept_thread, 2000) == WAIT_TIMEOUT)
            log_line("NET", "!! accept thread did not exit in 2 s");
        CloseHandle(g.accept_thread);
        g.accept_thread = nullptr;
    }
    for (int i = 0; i < kMaxPeers; ++i) {
        Link& l = g.links[i];
        if (!l.thread) continue;
        if (WaitForSingleObject(l.thread, 2000) == WAIT_TIMEOUT)
            log_line("NET", "!! receive thread for peer %u did not exit in 2 s",
                     (unsigned)l.id);
        CloseHandle(l.thread);
        l.thread = nullptr;
        InterlockedExchange(&l.live, 0);
    }

    NetStats s = net_stats();
    log_line("NET", "closed: %u sent / %u received / %u dropped, %llu B out / %llu B in",
             s.sent, s.received, s.dropped,
             (unsigned long long)s.bytes_sent, (unsigned long long)s.bytes_received);

    // Anything still queued is ours to free. Only some frames own memory (see
    // NetMsg); releasing every one keeps that knowledge in net_msg_release.
    if (g.cs_ready) {
        EnterCriticalSection(&g.cs);
        for (uint32_t i = 0; i < g.queue.count; ++i) net_msg_release(g.queue.at(i));
        g.queue.reset();
        g.drop_said  = false;
        g.grown_said = 0;
        LeaveCriticalSection(&g.cs);
    }

    if (g.cs_ready)      { DeleteCriticalSection(&g.cs);      g.cs_ready = false; }
    if (g.send_cs_ready) { DeleteCriticalSection(&g.send_cs); g.send_cs_ready = false; }
    if (g.wsa_up)        { WSACleanup(); g.wsa_up = false; }

    g.role = NetRole::None;
    g.self = kNoPeer;
    g.roster_valid = false;
    set_state(NetState::Idle);
}

NetState    net_state() { return (NetState)InterlockedCompareExchange(&g.state, 0, 0); }
NetRole     net_role()  { return g.role; }
const char* net_error() { return g.error; }

bool net_active() {
    if (g.role == NetRole::None) return false;
    NetState s = net_state();
    return s != NetState::Failed && s != NetState::Closed && s != NetState::Idle;
}

// Send to everyone we are connected to.
//
// On a client that is one socket, the host, and this is the old behaviour
// unchanged. On the host it is every live client -- which is what makes every
// existing host-authored call site (SAVEFILE, CATDATA, INVENTORY, ENTERNODE,
// HASH, CONTROL) reach all the players without any of them being edited.
//
// Returns true if it reached at least one peer. A host with no clients yet
// returns false, which every caller already treats as "nothing sent".
bool net_send(const uint8_t* payload, uint32_t len) {
    if (len == 0) return false;
    bool any = false, failed = false;
    for (int i = 0; i < kMaxPeers; ++i) {
        Link& l = g.links[i];
        if (!InterlockedCompareExchange(&l.live, 0, 0)) continue;
        const int r = send_link(l, g.self, payload, len);   // Mewgenics Multi : pas un pair refuse
        if (r > 0) any = true;
        else if (r == 0) failed = true;
    }
    if (failed) log_line("NET", "!! send failed (WSA %d)", WSAGetLastError());
    return any;
}

bool net_send_peer(uint8_t peer, const uint8_t* payload, uint32_t len) {
    int slot = link_slot_of(peer);
    if (slot < 0) return false;
    // Mewgenics Multi : pas un pair refuse ; et l'id relu sous le verrou (send_link
    // le prend) -- le slot a pu changer de connexion depuis link_slot_of.
    EnterCriticalSection(&g.send_cs);
    const bool same = g.links[slot].id == peer;
    const int  r    = same ? send_link(g.links[slot], g.self, payload, len) : -1;
    LeaveCriticalSection(&g.send_cs);
    return r > 0;
}

// Every encoder writes into a stack buffer and hands the result to net_send.
// 512 bytes covers every phase-4 message; RUNSTATE will need its own path.
#define MGMP_SEND_WITH(encoder, arg)                       \
    uint8_t p[512];                                        \
    uint32_t n = encoder(p, sizeof(p), arg);               \
    return n && net_send(p, n)

bool net_send_hello  (const Hello& h)     { MGMP_SEND_WITH(enc_hello,   h); }
bool net_send_welcome(const Welcome& w)   { MGMP_SEND_WITH(enc_welcome, w); }
bool net_send_action (const ActionMsg& a) { MGMP_SEND_WITH(enc_action,  a); }
bool net_send_hash   (const HashMsg& h)   { MGMP_SEND_WITH(enc_hash,    h); }
bool net_send_halt   (const HaltMsg& h)   { MGMP_SEND_WITH(enc_halt,    h); }
bool net_send_control(const ControlMsg& c) { MGMP_SEND_WITH(enc_control, c); }
bool net_send_cursor (const CursorMsg& c)  { MGMP_SEND_WITH(enc_cursor,  c); }
bool net_send_aim    (const AimMsg& m)     { MGMP_SEND_WITH(enc_aim,     m); }
bool net_send_enter_node(const EnterNodeMsg& m) { MGMP_SEND_WITH(enc_enter_node, m); }
bool net_send_choice(const ChoiceMsg& m) { MGMP_SEND_WITH(enc_choice, m); }
bool net_send_nodehash(const NodeHashMsg& m) { MGMP_SEND_WITH(enc_nodehash, m); }
// Not in relayed() above: host-authored, so net_send already reaches every
// client and a relay would deliver it twice.
bool net_send_hostleft(const HostLeftMsg& m) { MGMP_SEND_WITH(enc_hostleft, m); }
bool net_send_mm(const MmMsg& m) { MGMP_SEND_WITH(enc_mm, m); }
bool net_send_refuse (const char* reason) { MGMP_SEND_WITH(enc_refuse,  reason); }

#undef MGMP_SEND_WITH

bool net_send_hello_to(uint8_t peer, const Hello& h) {
    uint8_t p[512];
    uint32_t n = enc_hello(p, sizeof(p), h);
    return n && net_send_peer(peer, p, n);
}

// To one peer, for replaying a battle already under way to a joiner. A
// broadcast would re-inject decisions into peers that already applied them --
// pend_take cannot tell a replay from a live decision, correctly, because
// there is no difference except who still needs it.
bool net_send_action_to(uint8_t peer, const ActionMsg& a) {
    uint8_t p[512];
    uint32_t n = enc_action(p, sizeof(p), a);
    return n && net_send_peer(peer, p, n);
}

// To one peer: telling a joiner where the run is standing. A broadcast would
// re-drive peers that are already in that node back into it.
bool net_send_enter_node_to(uint8_t peer, const EnterNodeMsg& m) {
    uint8_t p[512];
    uint32_t n = enc_enter_node(p, sizeof(p), m);
    return n && net_send_peer(peer, p, n);
}

// Mewgenics Multi (protocole 58) : le hash du tour 0 du combat en cours, a celui
// qui arrive en plein combat -- il sait tout de suite si son combat est le meme.
bool net_send_hash_to(uint8_t peer, const HashMsg& h) {
    uint8_t p[512];
    uint32_t n = enc_hash(p, sizeof(p), h);
    return n && net_send_peer(peer, p, n);
}

bool net_send_welcome_to(uint8_t peer, const Welcome& w) {
    uint8_t p[512];
    uint32_t n = enc_welcome(p, sizeof(p), w);
    return n && net_send_peer(peer, p, n);
}

bool net_send_refuse_to(uint8_t peer, const char* reason) {
    uint8_t p[512];
    uint32_t n = enc_refuse(p, sizeof(p), reason);
    return n && net_send_peer(peer, p, n);
}

namespace {
// Shared by the broadcast and the point-to-point form so the frame cap check
// and the malloc live in exactly one place.
bool send_savefile_frame(const SaveFileMsg& m, bool broadcast, uint8_t peer,
                         uint8_t type = MSG_SAVEFILE) {
    uint32_t need = savefile_frame_size(m);
    if (need > kMaxFrame) {
        log_line("NET", "!! save file is %u bytes, over the %u-byte frame cap",
                 m.size, kMaxFrame);
        return false;
    }
    uint8_t* p = (uint8_t*)malloc(need);
    if (!p) return false;
    uint32_t n = type == MSG_WORLD ? enc_world(p, need, m)
               : type == MSG_NPC   ? enc_npc(p, need, m)
               : type == MSG_TEAM  ? enc_team(p, need, m)
                                   : enc_savefile(p, need, m);
    bool ok = n && (broadcast ? net_send(p, n) : net_send_peer(peer, p, n));
    free(p);
    return ok;
}
} // namespace

bool net_send_savefile(const SaveFileMsg& m) {
    return send_savefile_frame(m, true, kNoPeer);
}

// To one peer. A player who joins after the host has already published needs
// the save, but the peers already in the run must NOT be sent it again -- on a
// client that arrives as a fresh redirect-and-load of the whole run.
bool net_send_savefile_to(uint8_t peer, const SaveFileMsg& m) {
    return send_savefile_frame(m, false, peer);
}

// Mewgenics Multi : la save de l'hote comme reference du monde (MSG_WORLD).
bool net_send_world(const SaveFileMsg& m) {
    return send_savefile_frame(m, true, kNoPeer, MSG_WORLD);
}

bool net_send_world_to(uint8_t peer, const SaveFileMsg& m) {
    return send_savefile_frame(m, false, peer, MSG_WORLD);
}

// Mewgenics Multi : l'etat des PNJ de l'hote en direct (MSG_NPC).
bool net_send_npc(const SaveFileMsg& m) {
    return send_savefile_frame(m, true, kNoPeer, MSG_NPC);
}

bool net_send_npc_to(uint8_t peer, const SaveFileMsg& m) {
    return send_savefile_frame(m, false, peer, MSG_NPC);
}

// Mewgenics Multi : l'equipe mixte (MSG_TEAM). Pas dans relayed() : d'un invite,
// net_send n'atteint que l'hote, et c'est lui seul que ces chats concernent.
bool net_send_team(const SaveFileMsg& m) {
    return send_savefile_frame(m, true, kNoPeer, MSG_TEAM);
}

bool net_send_team_to(uint8_t peer, const SaveFileMsg& m) {
    return send_savefile_frame(m, false, peer, MSG_TEAM);
}

// Mewgenics Multi (protocole 58) : `to` = kNoPeer pour tous, ou un seul joueur --
// l'etat d'entree de la case en cours, pour celui qui arrive en pleine case
// (follow_catchup).
static bool send_catdata_frame(const CatDataMsg& m, uint8_t to) {
    uint32_t need = catdata_frame_size(m);
    if (need > kMaxFrame) {
        log_line("NET", "!! serialized cat is %u bytes, over the %u-byte frame cap",
                 m.size, kMaxFrame);
        return false;
    }
    uint8_t* p = (uint8_t*)malloc(need);
    if (!p) return false;
    uint32_t n = enc_catdata(p, need, m);
    bool ok = n && (to == kNoPeer ? net_send(p, n) : net_send_peer(to, p, n));
    free(p);
    return ok;
}

bool net_send_catdata(const CatDataMsg& m)                 { return send_catdata_frame(m, kNoPeer); }
bool net_send_catdata_to(uint8_t peer, const CatDataMsg& m) { return send_catdata_frame(m, peer); }

static bool send_runhist_frame(const RunHistMsg& m, uint8_t to) {
    uint32_t need = runhist_frame_size(m);
    if (need > kMaxFrame) {
        log_line("NET", "!! serialized run history is %u bytes, over the %u-byte "
                        "frame cap", m.size, kMaxFrame);
        return false;
    }
    uint8_t* p = (uint8_t*)malloc(need);
    if (!p) return false;
    uint32_t n = enc_runhist(p, need, m);
    bool ok = n && (to == kNoPeer ? net_send(p, n) : net_send_peer(to, p, n));
    free(p);
    return ok;
}

bool net_send_runhist(const RunHistMsg& m)                 { return send_runhist_frame(m, kNoPeer); }
bool net_send_runhist_to(uint8_t peer, const RunHistMsg& m) { return send_runhist_frame(m, peer); }

bool net_send_statedump(const StateDumpMsg& m) {
    uint32_t need = statedump_frame_size(m);
    if (need > kMaxFrame) {
        log_line("NET", "!! state dump is %u bytes, over the %u-byte frame cap",
                 need, kMaxFrame);
        return false;
    }
    uint8_t* p = (uint8_t*)malloc(need);
    if (!p) return false;
    uint32_t n = enc_statedump(p, need, m);
    bool ok = n && net_send(p, n);
    free(p);
    return ok;
}

static bool send_inventory_frame(const InventoryMsg& m, uint8_t to) {
    uint32_t need = inventory_frame_size(m);
    if (need > kMaxFrame) {
        log_line("NET", "!! serialized inventory is %u bytes, over the %u-byte "
                        "frame cap", need, kMaxFrame);
        return false;
    }
    uint8_t* p = (uint8_t*)malloc(need);
    if (!p) return false;
    uint32_t n = enc_inventory(p, need, m);
    bool ok = n && (to == kNoPeer ? net_send(p, n) : net_send_peer(to, p, n));
    free(p);
    return ok;
}

bool net_send_inventory(const InventoryMsg& m)                 { return send_inventory_frame(m, kNoPeer); }
bool net_send_inventory_to(uint8_t peer, const InventoryMsg& m) { return send_inventory_frame(m, peer); }

void net_msg_release(NetMsg& m) {
    if (m.savefile.data) { free(m.savefile.data); m.savefile.data = nullptr; }
    if (m.catdata.data)  { free(m.catdata.data);  m.catdata.data  = nullptr; }
    if (m.runhist.data)  { free(m.runhist.data);  m.runhist.data  = nullptr; }
    if (m.statedump.data){ free(m.statedump.data);m.statedump.data= nullptr; }
    for (uint32_t i = 0; i < kInvBuckets; ++i)
        if (m.inventory.data[i]) { free(m.inventory.data[i]); m.inventory.data[i] = nullptr; }
}

bool net_poll(NetMsg& out) {
    if (!g.cs_ready) return false;
    net_pump_connect();
    pump_refused();
    EnterCriticalSection(&g.cs);
    // Hand the allocation over rather than sharing it: pop zeroes the slot, so
    // the queue never holds a second owner of one pointer (a double free waiting
    // for a busy session).
    bool got = g.queue.pop(out);
    LeaveCriticalSection(&g.cs);

    // Learn our own id here rather than in the session layer, so the transport's
    // idea of `self` -- which it stamps into every outgoing envelope -- can
    // never lag behind what the rest of the mod believes.
    if (got && out.type == MSG_PEERS && out.peers.you != kNoPeer) {
        g.self = out.peers.you;
        g.roster = out.peers;
        g.roster_valid = true;
    }
    return got;
}

NetStats net_stats() {
    NetStats s{};
    if (!g.cs_ready) return s;
    EnterCriticalSection(&g.cs);
    s = g.stats;
    LeaveCriticalSection(&g.cs);
    return s;
}

} // namespace mgmp
