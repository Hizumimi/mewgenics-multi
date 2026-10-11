// mm_menu.cpp -- voir mm_menu.h.
//
// Les quatre regles qui rendent ces appels surs, apprises dans docs/NOTES_MGMP.md :
//
//   1. Une chaine que le JEU detruira doit avoir ete allouee par le jeu. Ce
//      module est compile avec son propre CRT (/MT) : un tampon alloue ici et
//      libere par le jeu, c'est deux tas differents et un plantage. Les chaines
//      passent donc par C_StrAssignCStr (std::string) ou par C_GameStrAlloc
//      (std::wstring), jamais par notre operator new.
//   2. Les std::function, elles, restent a nous : le jeu ne les manipule qu'a
//      travers leur table virtuelle (_Copy, _Do_call, _Delete_this), qui est
//      dans ce module. Le jeu et ce module utilisent la meme disposition MSVC
//      (64 octets, pointeur d'implementation a +56).
//   3. Les noms passes a MenuPanel::get_button / hide_button sont CONSOMMES
//      (detruits) par le jeu : ils font tous <= 15 caracteres, donc restent dans
//      le tampon interne de std::string et leur destruction ne libere rien.
//   4. Toute lecture d'un pointeur du jeu passe par mem_read : une adresse
//      fausse donne une ligne de journal, pas un plantage.
#include "mm_menu.h"

#include "mgmp_config.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_resolve.h"
#include "mgmp_session.h"

#include <winsock2.h>   // avant windows.h : types reseau d'iphlpapi
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <shellapi.h>

#include <cstdio>
#include <cstring>
#include <string>

#pragma comment(lib, "iphlpapi.lib")

namespace mgmp {
namespace {

// --- dispositions MSVC x64, identiques dans le jeu et dans ce module ---------

struct GameString {                 // std::string
    union { char buf[16]; char* ptr; };
    size_t size;
    size_t cap;                     // 15 = tampon interne
};
struct GameWString {                // std::wstring
    union { wchar_t buf[8]; wchar_t* ptr; };
    size_t size;
    size_t cap;                     // 7 = tampon interne
};
static_assert(sizeof(GameString) == 32, "std::string MSVC = 32 octets");
static_assert(sizeof(GameWString) == 32, "std::wstring MSVC = 32 octets");
static_assert(sizeof(std::function<void()>) == 64, "std::function MSVC = 64 octets");

// --- offsets lus dans le binaire (build 1.1.21239) ---------------------------

// MainMenu::init : `*(param_1 + 0x38) = FUN_140069ed0(param_1, "TitleScreen")`.
constexpr uintptr_t kMainMenu_Panel = 0x38;
// Le texte affiche par un bouton, un std::wstring. Le menu Pause l'ecrit juste
// apres register_button pour « Concede Fight » / « Skip Tutorial », et la boite
// de confirmation l'ecrit pour son bouton « OK » : meme moment que nous.
constexpr uintptr_t kButton_Label = 0x1B8;
// Popup_ConfirmationBox : `*(param_1 + 0x40) = panneau "ConfirmationBox"`.
constexpr uintptr_t kPrompt_Panel = 0x40;

// --- fonctions du jeu ---------------------------------------------------------

using FnAlloc      = void* (*)(size_t);
using FnStrAssign  = GameString* (*)(GameString*, const char*, size_t);
using FnWStrMove   = GameWString* (*)(GameWString*, GameWString*);
using FnConfirm    = void (*)(GameString*, std::function<void()>*, std::function<void()>*, void*);
using FnGetButton  = void* (*)(void* panel, GameString* name, bool create);
using FnHideButton = void (*)(void* panel, GameString* name);

using FnMainMenuInit  = void (__fastcall*)(void* self);
using FnMeow          = void (__fastcall*)(void* self);
// 6 parametres d'apres le binaire ; on en relaie 8 pour ne rien perdre si
// Ghidra en a compte trop peu (lire deux emplacements de pile en trop est sans
// risque, en oublier un ne l'est pas).
using FnConfirmBuild  = void (__fastcall*)(void* self, uint64_t a2, uint64_t a3, uint64_t a4,
                                           uint64_t a5, uint64_t ok_only, uint64_t a7, uint64_t a8);

FnMainMenuInit o_MainMenuInit = nullptr;
FnMeow         o_Meow         = nullptr;
FnConfirmBuild o_ConfirmBuild = nullptr;

template <class F> F call_target(Call c) { return (F)addr_of_call(c); }

// La fenetre en cours de construction, quand c'est l'une des notres. Ecrit
// juste avant ShowConfirmation, consomme par le detour du constructeur qui
// s'execute de facon synchrone a l'interieur.
struct Pending {
    bool        active  = false;
    bool        ok_only = false;
    std::string yes_label;          // UTF-8, vide = libelle du jeu
    std::string no_label;
};
Pending g_pending;

// --- chaines ------------------------------------------------------------------

std::wstring utf8_to_wide(const char* s) {
    if (!s || !*s) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (n <= 1) return {};
    std::wstring w((size_t)n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    return w;
}

// Une std::string courte (<= 15), sans allocation : rien a liberer par personne.
GameString small_string(const char* s) {
    GameString g{};
    size_t n = strlen(s);
    if (n > 15) n = 15;
    memcpy(g.buf, s, n);
    g.buf[n] = 0;
    g.size = n;
    g.cap = 15;
    return g;
}

// Une std::wstring dont le tampon vient du tas du JEU, avec la capacite que
// MSVC aurait choisie (n | 7), pour que la liberation par le jeu retombe juste.
bool make_game_wstring(GameWString& out, const std::wstring& w) {
    out = GameWString{};
    const size_t n = w.size();
    if (n <= 7) {
        memcpy(out.buf, w.data(), n * sizeof(wchar_t));
        out.buf[n] = 0;
        out.size = n;
        out.cap = 7;
        return true;
    }
    auto alloc = call_target<FnAlloc>(C_GameStrAlloc);
    if (!alloc) return false;
    const size_t cap = n | 7;
    auto* p = (wchar_t*)alloc((cap + 1) * sizeof(wchar_t));
    if (!p) return false;
    memcpy(p, w.data(), n * sizeof(wchar_t));
    p[n] = 0;
    out.ptr = p;
    out.size = n;
    out.cap = cap;
    return true;
}

bool set_button_label(void* button, const char* utf8) {
    auto move = call_target<FnWStrMove>(C_WStrMoveAssign);
    if (!button || !move) return false;
    GameWString tmp;
    if (!make_game_wstring(tmp, utf8_to_wide(utf8))) return false;
    // Deplacement : le jeu prend le tampon et libere l'ancien libelle. `tmp`
    // ressort vide (tampon interne), il n'y a rien a liberer de notre cote.
    move((GameWString*)((char*)button + kButton_Label), &tmp);
    return true;
}

void* panel_button(void* panel, const char* name) {
    auto get = call_target<FnGetButton>(C_PanelGetButton);
    if (!panel || !get) return nullptr;
    GameString n = small_string(name);
    return get(panel, &n, false);
}

// --- reseau : l'adresse a donner a son ami -------------------------------------

struct LocalIp {
    char        ip[32] = "";
    const char* kind   = "";
};

// Prefere l'adresse d'un reseau virtuel (Radmin VPN en 26.x, Tailscale en
// 100.64/10, Hamachi en 25.x), sinon celle du reseau local.
LocalIp best_local_ip() {
    LocalIp best;
    int best_rank = 99;
    ULONG size = 16 * 1024;
    for (int tries = 0; tries < 3; ++tries) {
        std::string buf(size, '\0');
        auto* aa = (IP_ADAPTER_ADDRESSES*)&buf[0];
        ULONG r = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                                       GAA_FLAG_SKIP_DNS_SERVER, nullptr, aa, &size);
        if (r == ERROR_BUFFER_OVERFLOW) continue;
        if (r != NO_ERROR) break;
        for (auto* a = aa; a; a = a->Next) {
            if (a->OperStatus != IfOperStatusUp) continue;
            for (auto* u = a->FirstUnicastAddress; u; u = u->Next) {
                auto* sin = (sockaddr_in*)u->Address.lpSockaddr;
                if (!sin || sin->sin_family != AF_INET) continue;
                const uint8_t* b = (const uint8_t*)&sin->sin_addr;
                int rank;
                const char* kind;
                if (b[0] == 26)                              { rank = 0; kind = "Radmin VPN"; }
                else if (b[0] == 100 && (b[1] & 0xC0) == 64) { rank = 1; kind = "Tailscale"; }
                else if (b[0] == 25)                         { rank = 2; kind = "Hamachi"; }
                else if (b[0] == 192 && b[1] == 168)         { rank = 3; kind = "reseau local"; }
                else if (b[0] == 10)                         { rank = 3; kind = "reseau local"; }
                else if (b[0] == 172 && (b[1] & 0xF0) == 16) { rank = 3; kind = "reseau local"; }
                else if (b[0] == 127 || (b[0] == 169 && b[1] == 254)) continue;
                else                                         { rank = 4; kind = "Internet"; }
                if (rank < best_rank) {
                    best_rank = rank;
                    snprintf(best.ip, sizeof(best.ip), "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
                    best.kind = kind;
                }
            }
        }
        break;
    }
    return best;
}

bool copy_to_clipboard(const char* utf8) {
    std::wstring w = utf8_to_wide(utf8);
    if (!OpenClipboard(nullptr)) return false;
    bool ok = false;
    EmptyClipboard();
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, (w.size() + 1) * sizeof(wchar_t));
    if (h) {
        void* p = GlobalLock(h);
        memcpy(p, w.c_str(), (w.size() + 1) * sizeof(wchar_t));
        GlobalUnlock(h);
        ok = SetClipboardData(CF_UNICODETEXT, h) != nullptr;
        if (!ok) GlobalFree(h);
    }
    CloseClipboard();
    return ok;
}

void open_mod_folder() {
    wchar_t path[MAX_PATH] = {};
    HMODULE self = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)&open_mod_folder, &self) &&
        GetModuleFileNameW(self, path, MAX_PATH)) {
        if (wchar_t* slash = wcsrchr(path, L'\\')) *slash = 0;
        ShellExecuteW(nullptr, L"open", path, nullptr, nullptr, SW_SHOWNORMAL);
    }
}

// --- detours -------------------------------------------------------------------

void __fastcall h_MainMenuInit(void* self) {
    o_MainMenuInit(self);

    void* panel = nullptr;
    if (!mem_read((char*)self + kMainMenu_Panel, &panel, sizeof(panel)) || !panel) {
        log_line("MULTI", "!! menu principal : panneau TitleScreen illisible, bouton non renomme");
        return;
    }
    void* meow = panel_button(panel, "btn_feedback");
    if (!meow) {
        log_line("MULTI", "!! menu principal : bouton btn_feedback introuvable");
        return;
    }
    if (set_button_label(meow, u8"Multijoueur"))
        log_line("MULTI", "menu principal : « Miauler » devient « Multijoueur »");
    else
        log_line("MULTI", "!! menu principal : impossible de changer le texte du bouton");
}

// Le bouton ne miaule plus : il ouvre la fenetre Multijoueur.
void __fastcall h_Meow(void* /*self*/) {
    mm_open_multiplayer_menu();
}

void __fastcall h_ConfirmBuild(void* self, uint64_t a2, uint64_t a3, uint64_t a4,
                               uint64_t a5, uint64_t ok_only, uint64_t a7, uint64_t a8) {
    if (!g_pending.active) {
        o_ConfirmBuild(self, a2, a3, a4, a5, ok_only, a7, a8);
        return;
    }
    Pending p = g_pending;
    g_pending.active = false;           // une seule fenetre par demande

    // Un seul bouton : le jeu a deja ce mode (il cache « no » et renomme
    // « yes » en « OK »), il ne s'en sert simplement nulle part.
    o_ConfirmBuild(self, a2, a3, a4, a5, p.ok_only ? 1 : ok_only, a7, a8);

    void* panel = nullptr;
    if (!mem_read((char*)self + kPrompt_Panel, &panel, sizeof(panel)) || !panel) return;
    if (!p.yes_label.empty()) set_button_label(panel_button(panel, "yes"), p.yes_label.c_str());
    if (!p.no_label.empty() && !p.ok_only)
        set_button_label(panel_button(panel, "no"), p.no_label.c_str());
}

} // namespace

bool mm_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_MainMenuInit: *detour = (void*)&h_MainMenuInit; *original = (void**)&o_MainMenuInit; return true;
    case T_MeowClick:    *detour = (void*)&h_Meow;         *original = (void**)&o_Meow;         return true;
    case T_ConfirmBuild: *detour = (void*)&h_ConfirmBuild; *original = (void**)&o_ConfirmBuild; return true;
    default: return false;
    }
}

bool mm_show_popup(const char* texte, const char* oui, const char* non, bool un_seul,
                   std::function<void()> on_oui, std::function<void()> on_non) {
    auto assign  = call_target<FnStrAssign>(C_StrAssignCStr);
    auto confirm = call_target<FnConfirm>(C_ShowConfirmation);
    if (!assign || !confirm || !texte) {
        log_line("MULTI", "!! fenetre impossible : fonctions du jeu introuvables");
        return false;
    }

    // Texte alloue par le jeu : ShowConfirmation le detruit avec son propre tas.
    GameString text = small_string("");
    assign(&text, texte, strlen(texte));

    // Des actions jamais vides : le jeu appelle _Do_call sans verifier.
    std::function<void()> yes = on_oui ? std::move(on_oui) : std::function<void()>([] {});
    std::function<void()> no  = on_non ? std::move(on_non) : std::function<void()>([] {});

    g_pending.active    = true;
    g_pending.ok_only   = un_seul;
    g_pending.yes_label = oui ? oui : "";
    g_pending.no_label  = non ? non : "";

    // Le jeu copie les deux actions puis vide nos objets (_Delete_this et
    // pointeur d'implementation remis a zero) : leurs destructeurs ici ne font
    // donc plus rien.
    confirm(&text, &yes, &no, nullptr);

    if (g_pending.active) {
        g_pending.active = false;
        log_line("MULTI", "!! la boite de confirmation n'a pas ete construite tout de suite");
    }
    return true;
}

void mm_open_multiplayer_menu() {
    const NetRole  role  = net_role();
    const NetState state = net_state();
    const int      peers = net_peer_count();
    const bool     ready = session_ready();
    char text[1024];

    log_line("MULTI", "fenetre Multijoueur : role=%d etat=%d joueurs=%d pret=%d",
             (int)role, (int)state, peers, (int)ready);

    if (role == NetRole::None) {
        snprintf(text, sizeof(text),
                 u8"Multijoueur\n\n"
                 u8"Le multijoueur est désactivé.\n"
                 u8"Dans mgmp.json, mets «&nbsp;role&nbsp;» sur «&nbsp;host&nbsp;» pour héberger "
                 u8"ou sur «&nbsp;client&nbsp;» pour rejoindre, puis relance le jeu.");
        mm_show_popup(text, u8"Ouvrir le dossier", u8"Fermer", false, [] { open_mod_folder(); });
        return;
    }

    if (role == NetRole::Host) {
        const uint16_t port = config().net_port;
        if (state == NetState::Failed) {
            snprintf(text, sizeof(text),
                     u8"Multijoueur (hôte)\n\n[c:red]Impossible d'ouvrir le port %u.[/c]\n%s",
                     port, net_error());
            mm_show_popup(text, u8"Fermer", nullptr, true);
            return;
        }
        if (ready && peers >= 2) {
            snprintf(text, sizeof(text),
                     u8"Multijoueur (hôte)\n\n"
                     u8"Ton ami est connecté&nbsp;!\n"
                     u8"Choisis ta sauvegarde avec «&nbsp;Jouer&nbsp;» : il te rejoindra automatiquement.");
            mm_show_popup(text, u8"D'accord", nullptr, true);
            return;
        }
        const LocalIp ip = best_local_ip();
        if (ip.ip[0]) {
            snprintf(text, sizeof(text),
                     u8"Multijoueur (hôte)\n\n"
                     u8"En attente de ton ami...\n"
                     u8"Ton adresse (%s) : %s, port %u",
                     ip.kind, ip.ip, port);
            std::string addr = ip.ip;
            mm_show_popup(text, u8"Copier l'adresse", u8"Fermer", false, [addr] {
                if (copy_to_clipboard(addr.c_str()))
                    log_line("MULTI", "adresse %s copiee dans le presse-papiers", addr.c_str());
            });
        } else {
            snprintf(text, sizeof(text),
                     u8"Multijoueur (hôte)\n\nEn attente de ton ami...\nPort %u", port);
            mm_show_popup(text, u8"Fermer", nullptr, true);
        }
        return;
    }

    // Invite
    const char* addr = config().net_addr;
    const uint16_t port = config().net_port;
    if (ready) {
        snprintf(text, sizeof(text),
                 u8"Multijoueur (invité)\n\n"
                 u8"Connecté à l'hôte (%s).\n"
                 u8"Attends qu'il lance sa partie : le jeu te la chargera tout seul.", addr);
        mm_show_popup(text, u8"D'accord", nullptr, true);
        return;
    }
    if (state == NetState::Connecting || state == NetState::Connected) {
        snprintf(text, sizeof(text), u8"Multijoueur (invité)\n\nConnexion à %s en cours...", addr);
        mm_show_popup(text, u8"Fermer", nullptr, true);
        return;
    }
    snprintf(text, sizeof(text),
             u8"Multijoueur (invité)\n\n"
             u8"[c:red]Impossible de joindre l'hôte (%s).[/c]\n"
             u8"Vérifie qu'il a lancé le jeu et que vous êtes sur le même réseau.", addr);
    std::string a = addr;
    mm_show_popup(text, u8"Réessayer", u8"Fermer", false, [a, port] {
        session_request_join(a.c_str(), port);
        log_line("MULTI", "nouvelle tentative de connexion a %s:%u", a.c_str(), port);
    });
}

} // namespace mgmp
