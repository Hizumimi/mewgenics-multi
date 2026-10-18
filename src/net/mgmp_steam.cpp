// mgmp_steam.cpp -- Mewgenics Multi (protocole 59) : jouer ensemble par Steam.
// Voir mgmp_steam.h : un tuyau entre une connexion Steam P2P et une connexion
// TCP locale, plus « Rejoindre la partie » dans la liste d'amis Steam.

#include "mgmp_steam.h"
#include "mgmp_config.h"
#include "mgmp_log.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

namespace mgmp {
namespace {

// --- l'API « flat » de Steamworks : seulement ce qui sert ici -----------------------
//
// Des fonctions C exportees par steam_api64.dll (steam_api_flat.h). Sur x64 il n'y
// a qu'une convention d'appel ; une reference C++ (const SteamNetworkingIdentity&)
// passe comme un pointeur.

typedef uint32_t HConn;              // HSteamNetConnection (0 : aucune)
typedef uint32_t HListen;            // HSteamListenSocket (0 : aucun)
typedef int32_t  EResult;

constexpr EResult kResultOK            = 1;
constexpr EResult kResultLimitExceeded = 25;

// ESteamNetworkingConnectionState
constexpr int kStateNone          = 0;
constexpr int kStateConnecting    = 1;
// (2 : FindingRoute -- une connexion qui se cherche un chemin, rien a faire)
constexpr int kStateConnected     = 3;
constexpr int kStateClosedByPeer  = 4;
constexpr int kStateProblem       = 5;

// ESteamNetworkingConfigValue
constexpr int kCfgSendBufferSize = 9;
constexpr int kCfgCallbackStatus = 201;   // k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged

constexpr int kSendReliableNoNagle = 8 | 1;   // k_nSteamNetworkingSend_Reliable | NoNagle

constexpr int kCbJoinRequested = 337;    // GameRichPresenceJoinRequested_t (k_iSteamFriendsCallbacks + 37)
constexpr int kCbConnStatus    = 1221;   // SteamNetConnectionStatusChangedCallback_t

// Les tailles des structures des rappels (pack 8 sous Windows) : Steam ne s'en sert
// que pour verifier ; seuls les premiers champs sont lus ici.
constexpr int kSizeJoinRequested = 8 + 256;       // CSteamID + char[k_cchMaxRichPresenceValueLength]
constexpr int kSizeConnStatus    = 8 + 696 + 8;   // m_hConn (+pad), SteamNetConnectionInfo_t, m_eOldState (+pad)

typedef int32_t     (*FnGetHSteamUser)();
typedef void*       (*FnAccessor)();
typedef bool        (*FnSetRichPresence)(void* self, const char* key, const char* value);
typedef const char* (*FnGetPersonaName)(void* self);
typedef const char* (*FnGetFriendPersonaName)(void* self, uint64_t id);
typedef uint64_t    (*FnGetSteamID)(void* self);
typedef void        (*FnInitRelay)(void* self);
typedef bool        (*FnSetCfgInt32)(void* self, int value, int32_t v);
typedef bool        (*FnSetCfgPtr)(void* self, int value, void* v);
typedef HListen     (*FnCreateListenP2P)(void* self, int vport, int nopt, const void* opts);
typedef HConn       (*FnConnectP2P)(void* self, const void* identity, int vport, int nopt, const void* opts);
typedef EResult     (*FnAccept)(void* self, HConn c);
typedef bool        (*FnClose)(void* self, HConn c, int reason, const char* debug, bool linger);
typedef bool        (*FnCloseListen)(void* self, HListen l);
typedef EResult     (*FnSend)(void* self, HConn c, const void* data, uint32_t n, int flags, int64_t* num);
typedef int         (*FnRecv)(void* self, HConn c, void** out, int max);
typedef EResult     (*FnRealTime)(void* self, HConn c, void* status, int nlanes, void* lanes);
typedef void        (*FnMsgRelease)(void* msg);
typedef void        (*FnIdentSetID)(void* identity, uint64_t id);
typedef void        (*FnRegisterCb)(void* cb, int id);
typedef void        (*FnUnregisterCb)(void* cb);

struct Api {
    bool resolved = false;   // GetProcAddress fait
    bool ok       = false;   // tout trouve
    char missing[96] = {};

    FnGetHSteamUser get_user = nullptr;
    FnAccessor friends_acc = nullptr, user_acc = nullptr, sockets_acc = nullptr, utils_acc = nullptr;
    FnSetRichPresence      set_rp     = nullptr;
    FnGetPersonaName       my_name    = nullptr;
    FnGetFriendPersonaName their_name = nullptr;
    FnGetSteamID           my_id      = nullptr;
    FnInitRelay            init_relay = nullptr;
    FnSetCfgInt32          cfg_int    = nullptr;
    FnSetCfgPtr            cfg_ptr    = nullptr;
    FnCreateListenP2P      listen     = nullptr;
    FnConnectP2P           connect    = nullptr;
    FnAccept               accept     = nullptr;
    FnClose                close      = nullptr;
    FnCloseListen          close_l    = nullptr;
    FnSend                 send       = nullptr;
    FnRecv                 recv       = nullptr;
    FnRealTime             realtime   = nullptr;
    FnMsgRelease           release    = nullptr;
    FnIdentSetID           ident_id   = nullptr;
    FnRegisterCb           reg_cb     = nullptr;
    FnUnregisterCb         unreg_cb   = nullptr;

    // Les interfaces, une fois Steam pret.
    void* friends = nullptr;
    void* user    = nullptr;
    void* sockets = nullptr;
    void* utils   = nullptr;
};

Api api;

// --- les rappels de Steam : un CCallbackBase fait a la main -------------------------
//
// class CCallbackBase { virtual void Run(void*) = 0; virtual void Run(void*, bool,
// SteamAPICall_t) = 0; virtual int GetCallbackSizeBytes() = 0; uint8 m_nCallbackFlags;
// int m_iCallback; } -- compilee par MSVC, comme steam_api64.dll : les surcharges
// virtuelles sont rangees a l'envers dans la vtable (Run a 3 arguments d'abord).
// Construite a la main pour ne dependre d'aucun compilateur : `this` passe en
// premier argument (rcx), comme pour une fonction libre.
struct FakeCallback {
    void**  vtbl;
    uint8_t flags;     // k_ECallbackFlagsRegistered (0x01) pose par SteamAPI_RegisterCallback
    int32_t id;
};
static_assert(sizeof(void*) == 8, "x64 seulement");

void cb_join_run(FakeCallback* self, void* param);
void cb_join_result(FakeCallback* self, void* param, bool io_fail, uint64_t call) { (void)io_fail; (void)call; cb_join_run(self, param); }
int  cb_join_size(FakeCallback*) { return kSizeJoinRequested; }

void cb_status_run(FakeCallback* self, void* param);
void cb_status_result(FakeCallback* self, void* param, bool io_fail, uint64_t call) { (void)io_fail; (void)call; cb_status_run(self, param); }
int  cb_status_size(FakeCallback*) { return kSizeConnStatus; }

void* g_join_vtbl[3]   = { (void*)&cb_join_result,   (void*)&cb_join_run,   (void*)&cb_join_size };
void* g_status_vtbl[3] = { (void*)&cb_status_result, (void*)&cb_status_run, (void*)&cb_status_size };

FakeCallback g_join_cb   = { g_join_vtbl,   0, kCbJoinRequested };
FakeCallback g_status_cb = { g_status_vtbl, 0, kCbConnStatus };

// --- l'etat ---------------------------------------------------------------------

// Un tuyau : une connexion Steam et sa connexion TCP locale.
struct Bridge {
    HConn  conn     = 0;
    SOCKET sock     = INVALID_SOCKET;
    bool   steam_up = false;     // la connexion Steam est etablie
    bool   ever_up  = false;
    std::vector<uint8_t> to_steam;   // lu en TCP, pas encore pris par Steam
    std::vector<uint8_t> to_tcp;     // recu de Steam, pas encore ecrit en TCP
};

struct State {
    bool     init_tried  = false;   // l'API a ete cherchee
    bool     ready       = false;   // interfaces obtenues, rappels enregistres
    uint64_t ready_check = 0;       // prochaine tentative (ms)
    bool     said_missing = false;

    // Deux verrous, jamais l'un dans l'autre : `cs` pour les tuyaux (le fil des
    // octets le tient pendant ses appels a Steam), `cb_cs` pour ce que les rappels
    // deposent -- un rappel de Steam ne doit jamais attendre le fil des octets.
    CRITICAL_SECTION cs;
    CRITICAL_SECTION cb_cs;
    bool     cs_ready = false;

    // Fil des octets.
    HANDLE        thread = nullptr;
    volatile LONG stop   = 0;

    // Rappels (fil du jeu) -> fil des octets (sous cb_cs).
    std::vector<HConn> changed;

    // Hote.
    bool     hosting   = false;
    HListen  listen    = 0;
    uint16_t tcp_port  = 0;
    int      rp_players = -1;       // la presence dite (nombre de joueurs)
    std::vector<Bridge> host_bridges;

    // Invite.
    bool     client_on   = false;
    uint64_t client_host = 0;
    SOCKET   client_listen = INVALID_SOCKET;
    Bridge   client;
    int      client_state = 0;      // 0 rien, 1 en cours, 2 etablie, 3 echouee
    std::string client_why;
    uint64_t client_begun = 0;

    // « Rejoindre la partie » recu (fil du jeu, sous cb_cs).
    bool        join_pending = false;
    uint64_t    join_host    = 0;
    std::string join_name;

    bool     wsa = false;
    uint32_t bridged = 0, accepted = 0;
    volatile LONG host_peers = 0;   // tuyaux relies au salon (lu sans verrou)
};

State g;

struct Lock {
    Lock()  { if (g.cs_ready) EnterCriticalSection(&g.cs); }
    ~Lock() { if (g.cs_ready) LeaveCriticalSection(&g.cs); }
};

struct CbLock {
    CbLock()  { if (g.cs_ready) EnterCriticalSection(&g.cb_cs); }
    ~CbLock() { if (g.cs_ready) LeaveCriticalSection(&g.cb_cs); }
};

// --- appels proteges -----------------------------------------------------------------
//
// Chaque appel dans la DLL de Steam passe par une fonction sans objet C++ a
// detruire, sous __try : une erreur de version doit faire une ligne au journal,
// jamais planter le jeu.

void* safe_acc(FnAccessor f) {
    __try { return f ? f() : nullptr; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}
int32_t safe_user(FnGetHSteamUser f) {
    __try { return f ? f() : 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
bool safe_set_rp(void* self, const char* k, const char* v) {
    __try { return api.set_rp(self, k, v); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool safe_name(FnGetPersonaName f, void* self, char* out, size_t cap) {
    __try {
        const char* s = f(self);
        if (!s) return false;
        strncpy_s(out, cap, s, _TRUNCATE);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool safe_friend_name(void* self, uint64_t id, char* out, size_t cap) {
    __try {
        const char* s = api.their_name(self, id);
        if (!s) return false;
        strncpy_s(out, cap, s, _TRUNCATE);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
uint64_t safe_my_id(void* self) {
    __try { return api.my_id(self); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
void safe_init_relay(void* self) {
    __try { api.init_relay(self); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}
bool safe_cfg_int(void* self, int v, int32_t x) {
    __try { return api.cfg_int(self, v, x); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool safe_cfg_ptr(void* self, int v, void* x) {
    __try { return api.cfg_ptr(self, v, x); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
HListen safe_listen(void* self) {
    __try { return api.listen(self, 0, 0, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
HConn safe_connect(void* self, const void* ident) {
    __try { return api.connect(self, ident, 0, 0, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
EResult safe_accept(void* self, HConn c) {
    __try { return api.accept(self, c); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
void safe_close(void* self, HConn c, bool linger) {
    __try { api.close(self, c, 0, nullptr, linger); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}
void safe_close_listen(void* self, HListen l) {
    __try { api.close_l(self, l); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}
EResult safe_send(void* self, HConn c, const void* p, uint32_t n) {
    __try { return api.send(self, c, p, n, kSendReliableNoNagle, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
int safe_recv(void* self, HConn c, void** out, int max) {
    __try { return api.recv(self, c, out, max); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return -1; }
}
// L'etat d'une connexion : le premier champ de SteamNetConnectionRealTimeStatus_t.
// -1 : la poignee n'est pas (ou plus) une connexion.
int safe_state(void* self, HConn c) {
    __try {
        alignas(8) uint8_t status[512] = {};
        if (api.realtime(self, c, status, 0, nullptr) != kResultOK) return -1;
        return *(const int32_t*)status;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return -1; }
}
void safe_release(void* msg) {
    __try { api.release(msg); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}
void safe_ident(void* ident, uint64_t id) {
    __try { api.ident_id(ident, id); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}
void safe_reg(void* cb, int id) {
    __try { api.reg_cb(cb, id); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}
void safe_unreg(void* cb) {
    __try { api.unreg_cb(cb); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

// --- trouver l'API ----------------------------------------------------------------

template <typename F>
bool find_fn(HMODULE m, const char* name, F& out) {
    out = (F)GetProcAddress(m, name);
    if (!out && !api.missing[0]) strncpy_s(api.missing, name, _TRUNCATE);
    return out != nullptr;
}

// La premiere des versions connues que la DLL exporte (les plus recentes
// d'abord : la DLL n'exporte que l'accesseur de la version de ses fonctions).
FnAccessor find_accessor(HMODULE m, const char* const* names, size_t n) {
    for (size_t i = 0; i < n; ++i)
        if (FnAccessor f = (FnAccessor)GetProcAddress(m, names[i])) return f;
    if (!api.missing[0]) strncpy_s(api.missing, names[0], _TRUNCATE);
    return nullptr;
}

void resolve_api() {
    if (api.resolved) return;
    api.resolved = true;
    HMODULE m = GetModuleHandleA("steam_api64.dll");
    if (!m) { strncpy_s(api.missing, "steam_api64.dll", _TRUNCATE); return; }

    // Les fonctions « flat » sans numero de version sont celles de la version de la
    // DLL, et la DLL n'exporte que l'accesseur de cette version : le premier trouve
    // est le bon.
    static const char* const kFriends[] = { "SteamAPI_SteamFriends_v018", "SteamAPI_SteamFriends_v017" };
    static const char* const kUser[]    = { "SteamAPI_SteamUser_v024", "SteamAPI_SteamUser_v023",
                                            "SteamAPI_SteamUser_v022", "SteamAPI_SteamUser_v021" };
    static const char* const kSockets[] = { "SteamAPI_SteamNetworkingSockets_SteamAPI_v013",
                                            "SteamAPI_SteamNetworkingSockets_SteamAPI_v012" };
    static const char* const kUtils[]   = { "SteamAPI_SteamNetworkingUtils_SteamAPI_v005",
                                            "SteamAPI_SteamNetworkingUtils_SteamAPI_v004" };

    bool ok = true;
    ok &= find_fn(m, "SteamAPI_GetHSteamUser", api.get_user);
    ok &= (api.friends_acc = find_accessor(m, kFriends, 2)) != nullptr;
    ok &= (api.user_acc    = find_accessor(m, kUser, 4)) != nullptr;
    ok &= (api.sockets_acc = find_accessor(m, kSockets, 2)) != nullptr;
    ok &= (api.utils_acc   = find_accessor(m, kUtils, 2)) != nullptr;
    ok &= find_fn(m, "SteamAPI_ISteamFriends_SetRichPresence", api.set_rp);
    ok &= find_fn(m, "SteamAPI_ISteamFriends_GetPersonaName", api.my_name);
    ok &= find_fn(m, "SteamAPI_ISteamFriends_GetFriendPersonaName", api.their_name);
    ok &= find_fn(m, "SteamAPI_ISteamUser_GetSteamID", api.my_id);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess", api.init_relay);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueInt32", api.cfg_int);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValuePtr", api.cfg_ptr);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P", api.listen);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_ConnectP2P", api.connect);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_AcceptConnection", api.accept);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_CloseConnection", api.close);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_CloseListenSocket", api.close_l);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_SendMessageToConnection", api.send);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnConnection", api.recv);
    ok &= find_fn(m, "SteamAPI_ISteamNetworkingSockets_GetConnectionRealTimeStatus", api.realtime);
    ok &= find_fn(m, "SteamAPI_SteamNetworkingMessage_t_Release", api.release);
    ok &= find_fn(m, "SteamAPI_SteamNetworkingIdentity_SetSteamID64", api.ident_id);
    ok &= find_fn(m, "SteamAPI_RegisterCallback", api.reg_cb);
    ok &= find_fn(m, "SteamAPI_UnregisterCallback", api.unreg_cb);
    api.ok = ok;
}

// --- le fil des octets ----------------------------------------------------------------

void close_bridge(Bridge& b, const char* why) {
    if (b.sock != INVALID_SOCKET) {
        shutdown(b.sock, SD_BOTH);
        closesocket(b.sock);
        b.sock = INVALID_SOCKET;
    }
    if (b.conn) {
        safe_close(api.sockets, b.conn, true);
        log_line("STEAM", "connexion Steam %u fermee (%s)", (unsigned)b.conn, why);
        b.conn = 0;
    }
    b.steam_up = false;
    b.to_steam.clear();
    b.to_tcp.clear();
}

void nonblocking(SOCKET s) {
    u_long on = 1;
    ioctlsocket(s, FIONBIO, &on);
    BOOL nodelay = TRUE;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
}

// Chez l'hote : la connexion TCP de cet ami vers le salon (127.0.0.1).
SOCKET connect_local(uint16_t port) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return s;
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port   = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
    if (connect(s, (sockaddr*)&a, sizeof(a)) != 0) {
        closesocket(s);
        return INVALID_SOCKET;
    }
    nonblocking(s);
    return s;
}

// Un tour des octets d'un tuyau. Faux s'il faut le fermer (`why`). `buf` : 64 Kio
// du fil des octets.
constexpr int kBufSize = 64 * 1024;
bool pump_bridge(Bridge& b, uint8_t* buf, const char** why) {
    if (!b.steam_up || b.sock == INVALID_SOCKET) return true;

    // TCP -> Steam. Pas plus d'un mega en attente : Steam garde le reste dans son
    // tampon d'envoi, et le salon attend (le TCP local se remplit). Un salon local
    // qui ferme apres un dernier message (un refus, par exemple) : ce message part
    // d'abord, puis la connexion Steam se ferme en le laissant arriver (linger).
    bool eof = false;
    while (b.to_steam.size() < (1u << 20)) {
        const int r = ::recv(b.sock, (char*)buf, kBufSize, 0);
        if (r > 0) { b.to_steam.insert(b.to_steam.end(), buf, buf + r); continue; }
        if (r == 0) { eof = true; break; }
        const int e = WSAGetLastError();
        if (e == WSAEWOULDBLOCK) break;
        eof = true;
        break;
    }
    size_t off = 0;
    while (off < b.to_steam.size()) {
        const uint32_t n = (uint32_t)std::min<size_t>(b.to_steam.size() - off, 256 * 1024);
        const EResult r = safe_send(api.sockets, b.conn, b.to_steam.data() + off, n);
        if (r == kResultOK) { off += n; continue; }
        if (r == kResultLimitExceeded) break;          // tampon de Steam plein : plus tard
        *why = "envoi Steam refuse";
        return false;
    }
    if (off) b.to_steam.erase(b.to_steam.begin(), b.to_steam.begin() + (ptrdiff_t)off);
    if (eof) { *why = "le salon local a ferme la connexion"; return false; }

    // Steam -> TCP.
    if (b.to_tcp.size() < (1u << 20)) {
        void* msgs[32] = {};
        const int n = safe_recv(api.sockets, b.conn, msgs, 32);
        if (n < 0) { *why = "connexion Steam perdue"; return false; }
        for (int i = 0; i < n; ++i) {
            if (!msgs[i]) continue;
            // SteamNetworkingMessage_t : void* m_pData ; int m_cbSize ; ...
            const uint8_t* data = *(const uint8_t* const*)msgs[i];
            const int      size = *(const int32_t*)((const uint8_t*)msgs[i] + 8);
            if (data && size > 0) b.to_tcp.insert(b.to_tcp.end(), data, data + size);
            safe_release(msgs[i]);
        }
    }
    off = 0;
    while (off < b.to_tcp.size()) {
        const int chunk = (int)std::min<size_t>(b.to_tcp.size() - off, 64 * 1024);
        const int r = ::send(b.sock, (const char*)b.to_tcp.data() + off, chunk, 0);
        if (r > 0) { off += (size_t)r; continue; }
        const int e = WSAGetLastError();
        if (r < 0 && e == WSAEWOULDBLOCK) break;
        *why = "ecriture locale impossible";
        return false;
    }
    if (off) b.to_tcp.erase(b.to_tcp.begin(), b.to_tcp.begin() + (ptrdiff_t)off);
    return true;
}

// Une connexion Steam fermee par l'autre bout : ce qu'il a envoye avant (un refus,
// la fin d'une partie) part encore vers le salon local, avant de le fermer.
void drain_to_tcp(Bridge& b) {
    if (b.sock == INVALID_SOCKET || !b.conn) return;
    for (int round = 0; round < 8; ++round) {
        void* msgs[32] = {};
        const int n = safe_recv(api.sockets, b.conn, msgs, 32);
        if (n <= 0) break;
        for (int i = 0; i < n; ++i) {
            if (!msgs[i]) continue;
            const uint8_t* data = *(const uint8_t* const*)msgs[i];
            const int      size = *(const int32_t*)((const uint8_t*)msgs[i] + 8);
            if (data && size > 0) b.to_tcp.insert(b.to_tcp.end(), data, data + size);
            safe_release(msgs[i]);
        }
    }
    if (b.to_tcp.empty()) return;
    u_long off_mode = 0;                       // bloquant, le temps de ces derniers octets
    ioctlsocket(b.sock, FIONBIO, &off_mode);
    DWORD ms = 2000;
    setsockopt(b.sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&ms, sizeof(ms));
    size_t done = 0;
    while (done < b.to_tcp.size()) {
        const int r = ::send(b.sock, (const char*)b.to_tcp.data() + done,
                             (int)std::min<size_t>(b.to_tcp.size() - done, 64 * 1024), 0);
        if (r <= 0) break;
        done += (size_t)r;
    }
    b.to_tcp.clear();
}

// Un changement d'etat d'une connexion (dit par un rappel, ou vu en la relisant).
void on_status(HConn c) {
    const int st = safe_state(api.sockets, c);

    // L'invite : sa connexion a l'hote.
    if (g.client_on && g.client.conn == c) {
        if (st == kStateConnected && !g.client.steam_up) {
            g.client.steam_up = g.client.ever_up = true;
            g.client_state = 2;
            log_line("STEAM", "connecte a l'hote par Steam");
        } else if (st == kStateClosedByPeer || st == kStateProblem || st == kStateNone || st < 0) {
            const bool was_up = g.client.ever_up;
            // Un hote qui ferme proprement (refus, salon arrete) : son dernier
            // message arrive au salon du mod, qui dira lui-meme pourquoi.
            if (st == kStateClosedByPeer) drain_to_tcp(g.client);
            if (!was_up || st != kStateClosedByPeer) {
                g.client_state = 3;
                g.client_why   = was_up ? u8"la connexion Steam avec l'hôte est coupée"
                                        : u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)";
            }
            close_bridge(g.client, was_up ? "coupee" : "impossible");
        }
        return;
    }

    // L'hote : une connexion d'un ami.
    for (Bridge& b : g.host_bridges) {
        if (b.conn != c) continue;
        if (st == kStateConnected && !b.steam_up) {
            b.steam_up = b.ever_up = true;
            b.sock = connect_local(g.tcp_port);
            if (b.sock == INVALID_SOCKET) {
                close_bridge(b, "le salon local ne repond pas");
                return;
            }
            ++g.bridged;
            log_line("STEAM", "un ami est arrive par Steam (connexion %u) : relie au salon", (unsigned)c);
        } else if (st == kStateClosedByPeer || st == kStateProblem || st == kStateNone || st < 0) {
            if (st == kStateClosedByPeer) drain_to_tcp(b);
            close_bridge(b, "l'ami est parti");
        }
        return;
    }
    if (g.hosting && g.listen && st == kStateConnecting) {
        const EResult r = safe_accept(api.sockets, c);
        if (r != kResultOK) {
            log_line("STEAM", "!! connexion Steam %u refusee par Steam (%d)", (unsigned)c, (int)r);
            safe_close(api.sockets, c, false);
            return;
        }
        Bridge b;
        b.conn = c;
        g.host_bridges.push_back(std::move(b));
        ++g.accepted;
        log_line("STEAM", "un ami se connecte par Steam (connexion %u)", (unsigned)c);
    }
}

DWORD WINAPI bridge_thread(LPVOID) {
    uint64_t last_poll = 0;
    uint8_t* buf = (uint8_t*)malloc(kBufSize);
    if (!buf) { log_line("STEAM", "!! fil Steam : memoire insuffisante"); return 0; }
    while (!InterlockedCompareExchange(&g.stop, 0, 0)) {
        std::vector<SOCKET> socks;
        std::vector<HConn> changed;
        {
            CbLock cb;
            changed.swap(g.changed);
        }
        {
            Lock lock;
            for (HConn c : changed) on_status(c);

            // Toutes les 200 ms, l'etat de chaque connexion relu (un rappel manque
            // ne doit rien bloquer).
            const uint64_t now = GetTickCount64();
            if (now - last_poll >= 200) {
                last_poll = now;
                if (g.client_on && g.client.conn) on_status(g.client.conn);
                for (size_t i = 0; i < g.host_bridges.size(); ++i)
                    if (g.host_bridges[i].conn) on_status(g.host_bridges[i].conn);
            }

            // L'invite : le salon du mod se connecte au port local.
            if (g.client_on && g.client_listen != INVALID_SOCKET && g.client.sock == INVALID_SOCKET &&
                g.client.conn) {
                SOCKET s = accept(g.client_listen, nullptr, nullptr);
                if (s != INVALID_SOCKET) {
                    nonblocking(s);
                    g.client.sock = s;
                    log_line("STEAM", "le salon du mod est relie a la connexion Steam");
                }
            }

            const char* why = nullptr;
            if (g.client_on && g.client.conn && !pump_bridge(g.client, buf, &why)) {
                if (g.client.ever_up && g.client_state != 3) {
                    g.client_state = 3;
                    g.client_why   = u8"la connexion Steam avec l'hôte est coupée";
                }
                close_bridge(g.client, why ? why : "?");
            }
            for (Bridge& b : g.host_bridges) {
                why = nullptr;
                if (b.conn && !pump_bridge(b, buf, &why)) close_bridge(b, why ? why : "?");
            }
            // Les tuyaux fermes s'en vont.
            for (size_t i = g.host_bridges.size(); i-- > 0;)
                if (!g.host_bridges[i].conn && g.host_bridges[i].sock == INVALID_SOCKET)
                    g.host_bridges.erase(g.host_bridges.begin() + (ptrdiff_t)i);

            if (g.client.sock != INVALID_SOCKET) socks.push_back(g.client.sock);
            if (g.client_listen != INVALID_SOCKET) socks.push_back(g.client_listen);
            LONG peers = 0;
            for (const Bridge& b : g.host_bridges)
                if (b.sock != INVALID_SOCKET) { socks.push_back(b.sock); ++peers; }
            InterlockedExchange(&g.host_peers, peers);
        }
        // Attendre un peu : des octets du salon local, ou 2 ms (ceux de Steam).
        if (socks.empty()) { Sleep(5); continue; }
        fd_set r;
        FD_ZERO(&r);
        for (SOCKET s : socks) if (r.fd_count < FD_SETSIZE) FD_SET(s, &r);
        timeval tv{0, 2000};
        select(0, &r, nullptr, nullptr, &tv);
    }
    free(buf);
    return 0;
}

bool ensure_thread() {
    if (g.thread) return true;
    InterlockedExchange(&g.stop, 0);
    g.thread = CreateThread(nullptr, 0, bridge_thread, nullptr, 0, nullptr);
    if (!g.thread) log_line("STEAM", "!! fil Steam impossible a lancer (%lu)", GetLastError());
    return g.thread != nullptr;
}

bool ensure_wsa() {
    if (g.wsa) return true;
    WSADATA w{};
    if (WSAStartup(MAKEWORD(2, 2), &w) != 0) return false;
    g.wsa = true;
    return true;
}

// --- les rappels (fil du jeu, pendant SteamAPI_RunCallbacks) ----------------------

void push_changed(HConn c) {
    if (!c) return;
    CbLock cb;
    for (HConn x : g.changed) if (x == c) return;
    g.changed.push_back(c);
}

// k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged : le meme rappel, par
// un pointeur de fonction (premier champ : la connexion).
void status_fn(void* info) {
    if (info) push_changed(*(const HConn*)info);
}

void cb_status_run(FakeCallback*, void* param) {
    if (param) push_changed(*(const HConn*)param);
}

void cb_join_run(FakeCallback*, void* param) {
    if (!param) return;
    uint64_t host = 0;
    char connect[257] = {};
    memcpy(&host, param, sizeof(host));
    memcpy(connect, (const uint8_t*)param + 8, 256);
    // « +mgmp_join <identifiant Steam> », pose par l'hote ; a defaut, celui sur qui
    // l'ami a clique.
    if (const char* p = strstr(connect, "mgmp_join")) {
        const uint64_t id = _strtoui64(p + 9, nullptr, 10);
        if (id) host = id;
    } else {
        log_line("STEAM", "« Rejoindre la partie » ignore : ce n'est pas un salon de Mewgenics Multi (%s)",
                 connect);
        return;
    }
    char name[128] = {};
    if (!api.friends || !safe_friend_name(api.friends, host, name, sizeof(name)) || !name[0])
        strncpy_s(name, "ton ami", _TRUNCATE);
    CbLock cb;
    g.join_pending = true;
    g.join_host    = host;
    g.join_name    = name;
    log_line("STEAM", "« Rejoindre la partie » : salon de %s (%llu)", name, (unsigned long long)host);
}

// Steam pret ? Les interfaces, le relais, les rappels -- une fois.
bool ensure_ready() {
    if (g.ready) return true;
    if (!config().net_steam) {
        static bool said = false;
        if (!said) { said = true; log_line("STEAM", "Steam coupe par mgmp.json (net.steam = false)"); }
        return false;
    }
    if (!g.cs_ready) {
        InitializeCriticalSection(&g.cs);
        InitializeCriticalSection(&g.cb_cs);
        g.cs_ready = true;
    }
    resolve_api();
    if (!api.ok) {
        if (!g.said_missing) {
            g.said_missing = true;
            log_line("STEAM", "Steam indisponible pour le mod (%s introuvable) : connexion par adresse "
                              "seulement", api.missing[0] ? api.missing : "?");
        }
        return false;
    }
    const uint64_t now = GetTickCount64();
    if (now < g.ready_check) return false;
    g.ready_check = now + 1000;
    if (!safe_user(api.get_user)) return false;      // SteamAPI pas encore initialise par le jeu
    api.friends = safe_acc(api.friends_acc);
    api.user    = safe_acc(api.user_acc);
    api.sockets = safe_acc(api.sockets_acc);
    api.utils   = safe_acc(api.utils_acc);
    if (!api.friends || !api.user || !api.sockets || !api.utils) return false;

    // Le rappel par pointeur de fonction AVANT toute connexion (Steam le prend a
    // la creation de chacune) ; un tampon d'envoi large (la save part d'un coup).
    safe_cfg_ptr(api.utils, kCfgCallbackStatus, (void*)&status_fn);
    safe_cfg_int(api.utils, kCfgSendBufferSize, 8 * 1024 * 1024);
    safe_init_relay(api.utils);
    safe_reg(&g_join_cb, kCbJoinRequested);
    safe_reg(&g_status_cb, kCbConnStatus);
    g.ready = true;
    char me[128] = {};
    safe_name(api.my_name, api.friends, me, sizeof(me));
    log_line("STEAM", "Steam pret pour le mod (%s) : « Rejoindre la partie » et invitations actives", me);
    return true;
}

void set_presence(int players) {
    if (!api.friends) return;
    if (players == g.rp_players) return;
    g.rp_players = players;
    const uint64_t me = safe_my_id(api.user);
    char connect[64];
    _snprintf_s(connect, _TRUNCATE, "+mgmp_join %llu", (unsigned long long)me);
    safe_set_rp(api.friends, "connect", connect);
    char status[96];
    _snprintf_s(status, _TRUNCATE, "Mewgenics Multi : salon ouvert (%d/4)", players < 1 ? 1 : players);
    safe_set_rp(api.friends, "status", status);
    safe_set_rp(api.friends, "steam_player_group", connect + 11);
    char size[8];
    _snprintf_s(size, _TRUNCATE, "%d", players < 1 ? 1 : players);
    safe_set_rp(api.friends, "steam_player_group_size", size);
}

} // namespace

// ---------------------------------------------------------------------------------

void steam_pump() { ensure_ready(); }

bool steam_available() { return g.ready; }

std::string steam_self_name() {
    if (!g.ready) return "";
    char me[128] = {};
    safe_name(api.my_name, api.friends, me, sizeof(me));
    return me;
}

void steam_host_open(uint16_t tcp_port, int players) {
    if (!ensure_ready()) return;
    if (g.hosting) {            // le cas de chaque image : rien a verrouiller
        set_presence(players);
        return;
    }
    {
        Lock lock;
        if (!g.hosting) {
            g.listen = safe_listen(api.sockets);
            if (!g.listen) {
                static bool said = false;
                if (!said) { said = true; log_line("STEAM", "!! salon Steam impossible a ouvrir"); }
                return;
            }
            g.hosting  = true;
            g.tcp_port = tcp_port;
            g.rp_players = -1;
            log_line("STEAM", "salon ouvert aux amis Steam (« Rejoindre la partie »)");
        }
    }
    if (!ensure_wsa() || !ensure_thread()) return;
    set_presence(players);
}

void steam_host_close() {
    if (!g.ready || !g.hosting) return;   // `hosting` ne change que sur le fil du jeu
    bool was = false;
    {
        Lock lock;
        was = g.hosting;
        if (g.hosting) {
            for (Bridge& b : g.host_bridges) close_bridge(b, "salon ferme");
            g.host_bridges.clear();
            InterlockedExchange(&g.host_peers, 0);
            if (g.listen) safe_close_listen(api.sockets, g.listen);
            g.listen  = 0;
            g.hosting = false;
        }
    }
    if (was) {
        // Seulement nos cles (une valeur vide les retire) : le jeu peut avoir les
        // siennes.
        safe_set_rp(api.friends, "connect", "");
        safe_set_rp(api.friends, "status", "");
        safe_set_rp(api.friends, "steam_player_group", "");
        safe_set_rp(api.friends, "steam_player_group_size", "");
        g.rp_players = -1;
        log_line("STEAM", "salon Steam ferme");
    }
}

int steam_host_peer_count() {
    return g.hosting ? (int)InterlockedCompareExchange(&g.host_peers, 0, 0) : 0;
}

bool steam_client_on() { return g.client_on; }

bool steam_take_join_request(uint64_t& host_id, std::string& host_name) {
    if (!g.cs_ready) return false;
    CbLock cb;
    if (!g.join_pending) return false;
    g.join_pending = false;
    host_id   = g.join_host;
    host_name = g.join_name;
    return true;
}

uint16_t steam_client_begin(uint64_t host_id) {
    if (!ensure_ready() || !host_id || !ensure_wsa() || !ensure_thread()) return 0;
    steam_client_end();
    SOCKET l = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (l == INVALID_SOCKET) return 0;
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port   = 0;
    inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
    int alen = sizeof(a);
    if (bind(l, (sockaddr*)&a, sizeof(a)) != 0 || listen(l, 1) != 0 ||
        getsockname(l, (sockaddr*)&a, &alen) != 0) {
        closesocket(l);
        return 0;
    }
    u_long on = 1;
    ioctlsocket(l, FIONBIO, &on);

    // SteamNetworkingIdentity : 136 octets ; rempli par la fonction de Steam.
    alignas(8) uint8_t ident[256] = {};
    safe_ident(ident, host_id);
    Lock lock;
    const HConn c = safe_connect(api.sockets, ident);
    if (!c) {
        closesocket(l);
        g.client_state = 3;
        g.client_why   = u8"Steam refuse la connexion";
        return 0;
    }
    g.client_on     = true;
    g.client_host   = host_id;
    g.client_listen = l;
    g.client        = Bridge{};
    g.client.conn   = c;
    g.client_state  = 1;
    g.client_why.clear();
    g.client_begun  = GetTickCount64();
    const uint16_t port = ntohs(a.sin_port);
    log_line("STEAM", "connexion Steam a l'hote %llu (connexion %u), salon local sur le port %u",
             (unsigned long long)host_id, (unsigned)c, (unsigned)port);
    return port;
}

void steam_client_end() {
    if (!g.cs_ready) return;
    Lock lock;
    if (!g.client_on) return;
    close_bridge(g.client, "salon quitte");
    if (g.client_listen != INVALID_SOCKET) { closesocket(g.client_listen); g.client_listen = INVALID_SOCKET; }
    g.client_on = false;
    if (g.client_state != 3) g.client_state = 0;
}

int steam_client_state(std::string* why) {
    if (!g.cs_ready) return 0;
    Lock lock;
    if (why) *why = g.client_why;
    return g.client_state;
}

void steam_shutdown() {
    if (!g.cs_ready) return;
    steam_host_close();
    steam_client_end();
    if (g.thread) {
        InterlockedExchange(&g.stop, 1);
        WaitForSingleObject(g.thread, 2000);
        CloseHandle(g.thread);
        g.thread = nullptr;
    }
    if (g.ready) {
        safe_unreg(&g_join_cb);
        safe_unreg(&g_status_cb);
        safe_cfg_ptr(api.utils, kCfgCallbackStatus, nullptr);
    }
    log_line_lvl(LogLevel::Trace, "STEAM", "fin : %u connexion(s) acceptee(s), %u reliee(s) au salon",
                 g.accepted, g.bridged);
}

} // namespace mgmp
