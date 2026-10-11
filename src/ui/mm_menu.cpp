// mm_menu.cpp -- voir mm_menu.h.
//
// Les regles qui rendent ces appels surs, apprises dans docs/NOTES_MGMP.md :
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
//   3. Les noms passes a MenuPanel::get_button / hide_button et a la recherche
//      d'enfant SWF sont CONSOMMES (detruits) par le jeu : ils font tous <= 15
//      caracteres, donc restent dans le tampon interne de std::string et leur
//      destruction ne libere rien.
//   4. Toute lecture d'un pointeur du jeu passe par mem_read : une adresse
//      fausse donne une ligne de journal, pas un plantage.
//   5. Aucun appel au jeu depuis un clic ou depuis la procedure de fenetre : ils
//      posent une demande, traitee par mm_menu_pump au debut de l'image. Les
//      boutons, eux, sont cliques depuis Button::update (mm_on_button_update),
//      exactement la ou le jeu clique les siens.
#include "mm_menu.h"

#include "mgmp_config.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"
#include "mgmp_session.h"

#include <winsock2.h>   // avant windows.h : types reseau d'iphlpapi
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <shellapi.h>

#include <cstdarg>
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
// Popup_ConfirmationBox (0x14077D240) : `*(param_1 + 0x40) = panneau "ConfirmationBox"`.
constexpr uintptr_t kPrompt_Panel = 0x40;
// La scene de la boite : Confirm_close_call (0x14077F9C0) ecrit
// `*(*(popup + 0x20) + 0x4DB) = 1` quand elle se ferme (kScene_Destroying).
constexpr uintptr_t kPrompt_Scene = 0x20;
// MenuPanel+0x38 : le clip SWF du panneau. Le constructeur y cherche "prompt"
// (Swf_find_child) puis y ecrit le texte (Swf_set_text(champ, texte, 0, 1)).
constexpr uintptr_t kPanel_Clip = 0x38;
// Component+0x20 : la scene du composant (le constructeur de la boite lit
// `*(popup+0x20)+0x460`, sa priorite d'affichage ; Component+0x28 = Director,
// cf. docs/NOTES_MGMP.md). Le premier champ d'une scene est son Director :
// Director_get_or_create_scene (0x1409D4950) ecrit `*scene = director`.
constexpr uintptr_t kComp_Scene     = 0x20;
constexpr uintptr_t kScene_Director = 0x00;

// --- fonctions du jeu ---------------------------------------------------------

using FnAlloc       = void* (*)(size_t);
using FnStrAssign   = GameString* (*)(GameString*, const char*, size_t);
using FnWStrMove    = GameWString* (*)(GameWString*, GameWString*);
using FnConfirm     = void (*)(GameString*, std::function<void()>*, std::function<void()>*, void*);
using FnGetButton   = void* (*)(void* panel, GameString* name, bool create);
using FnFindChild   = void* (*)(void* clip, GameString* name);
using FnSetText     = void (*)(void* field, GameWString* text, bool force, bool markup);
using FnButtonClick = void (*)(void* button, bool force);

using FnMainMenuInit = void (__fastcall*)(void* self);
using FnMeow         = void (__fastcall*)(void* self);
// Popup_ConfirmationBox(self, ?, std::wstring texte (par valeur), oui, non, un_seul).
// 6 parametres d'apres le binaire ; on en relaie 8 pour ne rien perdre si
// Ghidra en a compte trop peu (lire deux emplacements de pile en trop est sans
// risque, en oublier un ne l'est pas).
using FnConfirmBuild = void (__fastcall*)(void* self, uint64_t a2, uint64_t text, uint64_t a4,
                                          uint64_t a5, uint64_t ok_only, uint64_t a7, uint64_t a8);

FnMainMenuInit o_MainMenuInit = nullptr;
FnMeow         o_Meow         = nullptr;
FnConfirmBuild o_ConfirmBuild = nullptr;

template <class F> F call_target(Call c) { return (F)addr_of_call(c); }

// --- chaines ------------------------------------------------------------------

std::wstring utf8_to_wide(const char* s) {
    if (!s || !*s) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (n <= 1) return {};
    std::wstring w((size_t)n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    return w;
}

// Texte de secours, si le detour du constructeur de la boite n'est pas en place :
// le jeu lirait alors notre texte octet par octet, et les accents en UTF-8
// deviendraient des signes illisibles. On les retire plutot.
std::string ascii_fallback(const std::string& utf8) {
    std::wstring w = utf8_to_wide(utf8.c_str());
    std::string out;
    out.reserve(w.size());
    for (wchar_t c : w) {
        if (c < 0x80) { out += (char)c; continue; }
        switch (c) {
            case L'à': case L'â': case L'ä': out += 'a'; break;
            case L'é': case L'è': case L'ê': case L'ë': out += 'e'; break;
            case L'î': case L'ï': out += 'i'; break;
            case L'ô': case L'ö': out += 'o'; break;
            case L'ù': case L'û': case L'ü': out += 'u'; break;
            case L'ç': out += 'c'; break;
            case L'À': case L'Â': out += 'A'; break;
            case L'É': case L'È': case L'Ê': out += 'E'; break;
            case L'Ç': out += 'C'; break;
            case L'«': case L'»': out += '"'; break;
            default: out += '?'; break;
        }
    }
    return out;
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

// Un nom affichable : [ et ] seraient pris pour des balises de couleur.
std::string display_name(const char* s) {
    std::string out = s ? s : "";
    for (char& c : out)
        if (c == '[') c = '(';
        else if (c == ']') c = ')';
        else if (c == '\n' || c == '\r') c = ' ';
    return out;
}

// Coupe une chaine UTF-8 a `max` octets sans casser un caractere.
void copy_utf8(char* dst, size_t cap, const char* src) {
    size_t n = src ? strlen(src) : 0;
    if (n >= cap) {
        n = cap - 1;
        while (n > 0 && ((unsigned char)src[n] & 0xC0) == 0x80) --n;
    }
    if (n) memcpy(dst, src, n);
    dst[n] = 0;
}

// --- fichiers a cote du mod -------------------------------------------------------

std::wstring mod_dir() {
    wchar_t path[MAX_PATH] = {};
    HMODULE self = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)&mod_dir, &self) &&
        GetModuleFileNameW(self, path, MAX_PATH)) {
        if (wchar_t* slash = wcsrchr(path, L'\\')) *slash = 0;
        return path;
    }
    return L".";
}

constexpr wchar_t kLastHostFile[] = L"dernier_hote.txt";

std::string read_last_host() {
    std::wstring p = mod_dir() + L"\\" + kLastHostFile;
    FILE* f = nullptr;
    if (_wfopen_s(&f, p.c_str(), L"rb") != 0 || !f) return {};
    char buf[80] = {};
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    std::string s(buf, n);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ')) s.pop_back();
    return s;
}

void write_last_host(const std::string& s) {
    std::wstring p = mod_dir() + L"\\" + kLastHostFile;
    FILE* f = nullptr;
    if (_wfopen_s(&f, p.c_str(), L"wb") != 0 || !f) return;
    fwrite(s.data(), 1, s.size(), f);
    fclose(f);
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
                else if (b[0] == 192 && b[1] == 168)         { rank = 3; kind = u8"réseau local"; }
                else if (b[0] == 10)                         { rank = 3; kind = u8"réseau local"; }
                else if (b[0] == 172 && (b[1] & 0xF0) == 16) { rank = 3; kind = u8"réseau local"; }
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

// Le presse-papiers, reduit aux caracteres d'une adresse (chiffres, '.', ':').
// La virgule devient un point : le pave numerique francais en donne une.
std::string clipboard_address_chars() {
    std::string out;
    if (!OpenClipboard(nullptr)) return out;
    if (HANDLE h = GetClipboardData(CF_UNICODETEXT)) {
        if (const wchar_t* p = (const wchar_t*)GlobalLock(h)) {
            for (size_t i = 0; p[i] && out.size() < 40; ++i) {
                wchar_t c = p[i];
                if (c == L',') c = L'.';
                if ((c >= L'0' && c <= L'9') || c == L'.' || c == L':') out += (char)c;
            }
            GlobalUnlock(h);
        }
    }
    CloseClipboard();
    return out;
}

// "26.12.34.56" ou "26.12.34.56:27600". Le port par defaut est celui de mgmp.json.
bool parse_address(const std::string& text, std::string& ip, uint16_t& port) {
    std::string t;
    for (char c : text) if (c != ' ') t += c;
    if (t.empty()) return false;
    port = config().net_port;
    const size_t colon = t.find(':');
    ip = t.substr(0, colon);
    if (colon != std::string::npos) {
        const std::string p = t.substr(colon + 1);
        if (!p.empty()) {
            unsigned long v = 0;
            for (char c : p) {
                if (c < '0' || c > '9') return false;
                v = v * 10 + (unsigned long)(c - '0');
                if (v > 65535) return false;
            }
            if (v == 0) return false;
            port = (uint16_t)v;
        }
    }
    in_addr a{};
    return inet_pton(AF_INET, ip.c_str(), &a) == 1;
}

// --- noms des joueurs -------------------------------------------------------------

char g_local_name[64] = "";
bool g_local_name_tried = false;
char g_peer_names[8][64] = {};

// --- la fenetre ouverte -------------------------------------------------------------

enum class Screen { None, Choice, HostLobby, JoinEntry, ClientLobby, BadAddress };

// Les demandes posees par les clics et par le clavier, traitees au debut de
// l'image suivante. Un seul emplacement : il n'y a jamais deux clics a la fois.
enum class Action { None, OpenAuto, StartHost, OpenJoin, DoJoin, ClickPlay, StopSession, OpenBadAddress };
Action g_action = Action::None;

// La fenetre en cours de construction, quand c'est l'une des notres. Ecrite
// juste avant ShowConfirmation, consommee par le detour du constructeur qui
// s'execute de facon synchrone a l'interieur.
struct Pending {
    bool         active  = false;
    bool         ok_only = false;
    uint32_t     serial  = 0;
    Screen       screen  = Screen::None;
    std::wstring text;              // le vrai texte, en Unicode
    std::string  yes_label;         // UTF-8, vide = libelle du jeu
    std::string  no_label;
};
Pending g_pending;

// La fenetre affichee. `alive` passe a faux des que l'un de ses boutons a ete
// clique : ses actions ne s'executent qu'une fois la boite refermee
// (Confirm_close_call), au moment ou sa scene est marquee pour destruction.
struct Live {
    bool         alive  = false;
    uint32_t     serial = 0;
    Screen       screen = Screen::None;
    void*        self   = nullptr;  // le composant YesNoPrompt
    void*        panel  = nullptr;  // son MenuPanel "ConfirmationBox"
    void*        prompt = nullptr;  // son champ texte "prompt"
    std::wstring shown;             // dernier texte ecrit dans ce champ
};
Live     g_live;
uint32_t g_serial = 0;

// Un bouton a cliquer au prochain passage dans Button::update.
void* g_click_target = nullptr;
int   g_click_wait   = 0;           // images depuis la demande (relance si rien)
int   g_click_tries  = 0;

// La saisie de l'adresse (ecran JoinEntry).
std::string g_addr_buf;
bool        g_buf_dirty  = false;
bool        g_key_enter  = false;
bool        g_key_escape = false;

// Ce qu'il faut afficher dans les salons.
LocalIp     g_host_ip;
bool        g_host_copied = false;
uint16_t    g_host_port   = 0;
std::string g_join_ip;
uint16_t    g_join_port   = 0;

void* g_main_panel = nullptr;       // panneau TitleScreen du menu principal
void* g_main_scene = nullptr;       // scene du menu principal (parent de nos boites)

// ShowConfirmation(texte, oui, non, scene) : sans scene, le jeu prend la scene
// « en cours de mise a jour » (global 0x1413C18F0), qui n'existe que pendant
// l'update des scenes. Tous les appels du jeu passent nullptr parce qu'ils sont
// faits depuis un bouton ; nous ouvrons au debut de l'image, ou ce global est
// nul (plantage de la build 3, Mewgenics.exe+0x77DD5F). On passe donc la scene
// du menu principal, apres avoir verifie qu'elle est toujours dans la liste du
// Director et pas en cours de destruction.
void* live_main_scene() {
    if (!g_main_scene) return nullptr;
    void* director = nullptr;
    if (!mem_read((char*)g_main_scene + kScene_Director, &director, sizeof(director)) || !director)
        return nullptr;
    void** begin = nullptr;
    void** end   = nullptr;
    if (!mem_read((char*)director + kDirector_ScenesBegin, &begin, sizeof(begin)) ||
        !mem_read((char*)director + kDirector_ScenesEnd,   &end,   sizeof(end)) ||
        !begin || end < begin || end - begin > 256)
        return nullptr;
    for (void** it = begin; it < end; ++it) {
        void* s = nullptr;
        if (!mem_read(it, &s, sizeof(s)) || s != g_main_scene) continue;
        uint8_t dying = 1;
        if (!mem_read((char*)s + kScene_Destroying, &dying, 1) || dying) return nullptr;
        return s;
    }
    return nullptr;
}

void popup_closed(uint32_t serial) {
    if (g_live.serial != serial) return;
    g_live.alive  = false;
    g_live.self   = nullptr;
    g_live.panel  = nullptr;
    g_live.prompt = nullptr;
    g_click_target = nullptr;
}

// Toujours depuis mm_menu_pump : la boite precedente doit etre fermee.
bool open_popup(Screen screen, const std::string& text_utf8, const char* yes, const char* no,
                bool ok_only, Action on_yes, Action on_no) {
    auto confirm = call_target<FnConfirm>(C_ShowConfirmation);
    if (!confirm) {
        log_line("MULTI", "!! fenetre impossible : ShowConfirmation introuvable");
        return false;
    }
    if (g_live.alive) {
        log_line("MULTI", "!! une fenetre est deja ouverte, demande ignoree");
        return false;
    }
    void* scene = live_main_scene();
    if (!scene) {
        log_line("MULTI", "!! fenetre impossible : scene du menu principal introuvable");
        return false;
    }
    const uint32_t serial = ++g_serial;

    // Le texte que voit ShowConfirmation (une cle de traduction : inconnue, elle
    // est affichee telle quelle). Le detour du constructeur le remplace par la
    // version Unicode ; celle-ci ne sert que s'il n'est pas en place.
    GameString text = small_string("");
    const std::string fallback = ascii_fallback(text_utf8);
    if (auto assign = call_target<FnStrAssign>(C_StrAssignCStr))
        assign(&text, fallback.c_str(), fallback.size());
    else
        text = small_string("Multijoueur");

    // Des actions jamais vides : le jeu appelle _Do_call sans verifier. Elles ne
    // touchent pas au jeu (regle 5) : elles posent la demande suivante.
    std::function<void()> yes_fn = [serial, on_yes] {
        popup_closed(serial);
        if (on_yes != Action::None) g_action = on_yes;
    };
    std::function<void()> no_fn = [serial, on_no] {
        popup_closed(serial);
        if (on_no != Action::None) g_action = on_no;
    };

    g_pending.active    = true;
    g_pending.ok_only   = ok_only;
    g_pending.serial    = serial;
    g_pending.screen    = screen;
    g_pending.text      = utf8_to_wide(text_utf8.c_str());
    g_pending.yes_label = yes ? yes : "";
    g_pending.no_label  = no ? no : "";

    // Le jeu copie les deux actions puis vide nos objets (_Delete_this et
    // pointeur d'implementation remis a zero) : leurs destructeurs ici ne font
    // donc plus rien.
    confirm(&text, &yes_fn, &no_fn, scene);

    if (g_pending.active) {
        g_pending.active = false;
        log_line("MULTI", "!! la boite de confirmation n'a pas ete construite tout de suite");
        return false;
    }
    return true;
}

// La scene de la boite est-elle encore la, et pas en train d'etre detruite ?
bool live_scene_ok() {
    if (!g_live.alive || !g_live.self) return false;
    void* scene = nullptr;
    if (!mem_read((char*)g_live.self + kPrompt_Scene, &scene, sizeof(scene)) || !scene)
        return false;
    uint8_t dying = 0;
    if (!mem_read((char*)scene + kScene_Destroying, &dying, 1)) return false;
    return dying == 0;
}

// Met a jour le texte de la boite ouverte, s'il a change.
void popup_set_text(const std::string& text_utf8) {
    if (!g_live.alive || !g_live.prompt) return;
    std::wstring w = utf8_to_wide(text_utf8.c_str());
    if (w == g_live.shown) return;
    if (!live_scene_ok()) {
        log_line("MULTI", "!! la fenetre a disparu sans passer par ses boutons");
        popup_closed(g_live.serial);
        return;
    }
    auto set = call_target<FnSetText>(C_SwfSetText);
    if (!set) return;
    GameWString tmp;
    if (!make_game_wstring(tmp, w)) return;
    // Passe par valeur : le jeu copie le texte puis libere ce tampon (regle 1).
    set(g_live.prompt, &tmp, false, true);
    g_live.shown = std::move(w);
}

// Demande un clic sur l'un des deux boutons de la boite ouverte.
void popup_press(const char* name) {
    if (!g_live.alive || !live_scene_ok()) return;
    void* b = panel_button(g_live.panel, name);
    if (!b) {
        log_line("MULTI", "!! bouton '%s' introuvable dans la fenetre", name);
        return;
    }
    g_click_target = b;
    g_click_wait   = 0;
    g_click_tries  = 0;
}

// --- textes des ecrans ----------------------------------------------------------

std::string fmt(const char* f, ...) {
    char buf[1600];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

std::string peer_name(uint8_t id) {
    if (id < 8 && g_peer_names[id][0]) return display_name(g_peer_names[id]);
    return fmt("Joueur %u", (unsigned)id + 1);
}

std::string choice_text() {
    return u8"Multijoueur\n\n"
           u8"Héberge une partie pour que tes amis te rejoignent,\n"
           u8"ou rejoins la partie d'un ami.";
}

std::string host_lobby_text() {
    const std::string me = display_name(mm_local_name());
    const NetState st = net_state();
    if (net_role() != NetRole::Host) {
        if (st == NetState::Failed)
            return fmt(u8"Salon multijoueur\n\n"
                       u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
                       u8"Un autre programme l'utilise peut-être.\n"
                       u8"Arrête le salon, puis réessaie.", (unsigned)g_host_port);
        return u8"Salon multijoueur\n\nOuverture du salon...";
    }

    std::string players = fmt(u8"  %s (hôte)\n", me.c_str());
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    int others = 0;
    if (n && net_peer_ids(ids, 8)) {
        for (uint8_t i = 0; i < n && i < 8; ++i) {
            if (ids[i] == net_self()) continue;
            players += "  " + peer_name(ids[i]) + "\n";
            ++others;
        }
    }
    if (!others) players += u8"  En attente d'un ami...\n";

    std::string where;
    if (g_host_ip.ip[0])
        where = fmt(u8"Donne cette adresse à ton ami :\n%s   (%s, port %u)\n%s",
                    g_host_ip.ip, g_host_ip.kind, (unsigned)g_host_port,
                    g_host_copied ? u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n" : "");
    else
        where = fmt(u8"Adresse introuvable sur ce PC (port %u).\n", (unsigned)g_host_port);

    return fmt(u8"Salon multijoueur\n\n%s\nJoueurs (%d/4) :\n%s\n"
               u8"Quand tout le monde est là, choisis ta sauvegarde.",
               where.c_str(), others + 1, players.c_str());
}

std::string join_text() {
    return fmt(u8"Rejoindre une partie\n\n"
               u8"Adresse de l'hôte :\n%s_\n\n"
               u8"Tape l'adresse que ton ami t'a donnée,\n"
               u8"ou colle-la avec Ctrl+V. Entrée pour valider.",
               g_addr_buf.c_str());
}

std::string bad_address_text() {
    return fmt(u8"Rejoindre une partie\n\n"
               u8"[c:red]Adresse invalide : %s[/c]\n\n"
               u8"Exemple : 26.123.45.67\n"
               u8"ou, avec un autre port : 26.123.45.67:27600",
               g_addr_buf.empty() ? u8"(vide)" : g_addr_buf.c_str());
}

std::string client_lobby_text() {
    const std::string target = fmt("%s", g_join_ip.c_str());
    const NetRole  r  = net_role();
    const NetState st = net_state();

    if (r == NetRole::None) {
        if (session_request_pending())
            return fmt(u8"Salon multijoueur\n\nConnexion à %s...", target.c_str());
        if (st == NetState::Failed)
            return fmt(u8"Salon multijoueur\n\n"
                       u8"[c:red]Impossible de joindre %s.[/c]\n"
                       u8"Vérifie l'adresse, et que ton ami a bien\n"
                       u8"cliqué sur « Héberger ».", target.c_str());
        return u8"Salon multijoueur\n\nDéconnecté.";
    }
    if (st == NetState::Connecting)
        return fmt(u8"Salon multijoueur\n\nConnexion à %s...", target.c_str());
    if (st == NetState::Closed || st == NetState::Failed)
        return u8"Salon multijoueur\n\n[c:red]La connexion avec l'hôte est coupée.[/c]\n"
               u8"Quitte le salon, puis rejoins-le à nouveau.";
    if (!session_ready()) {
        const char* status = session_status();
        if (status && strstr(status, "refus"))
            return fmt(u8"Salon multijoueur\n\n"
                       u8"[c:red]L'hôte a refusé la connexion.[/c]\n"
                       u8"Vérifiez que vous avez la même version du jeu et du mod.\n(%s)", status);
        return u8"Salon multijoueur\n\nConnecté, présentation à l'hôte...";
    }

    const std::string host = peer_name(0);
    const std::string me   = display_name(mm_local_name());
    return fmt(u8"Salon multijoueur\n\n"
               u8"Tu as rejoint la partie de %s !\n\n"
               u8"Joueurs :\n  %s (hôte)\n  %s\n\n"
               u8"%s choisit la sauvegarde :\n"
               u8"elle se chargera toute seule chez toi.",
               host.c_str(), host.c_str(), me.c_str(), host.c_str());
}

// --- ouverture des ecrans --------------------------------------------------------

void open_choice() {
    open_popup(Screen::Choice, choice_text(), u8"Héberger", u8"Rejoindre", false,
               Action::StartHost, Action::OpenJoin);
}

void open_host_lobby() {
    // Salon lance par mgmp.json (role "host") plutot que par le menu : l'adresse
    // n'a pas encore ete cherchee.
    if (!g_host_port) g_host_port = config().net_port;
    if (!g_host_ip.ip[0]) g_host_ip = best_local_ip();
    open_popup(Screen::HostLobby, host_lobby_text(), u8"Choisir la sauvegarde", u8"Arrêter",
               false, Action::ClickPlay, Action::StopSession);
}

void open_client_lobby() {
    // Connexion lancee par mgmp.json (role "client") plutot que par le menu.
    if (g_join_ip.empty()) g_join_ip = config().net_addr;
    open_popup(Screen::ClientLobby, client_lobby_text(), u8"D'accord", u8"Quitter", false,
               Action::None, Action::StopSession);
}

void open_join() {
    open_popup(Screen::JoinEntry, join_text(), u8"Rejoindre", u8"Annuler", false,
               Action::DoJoin, Action::None);
}

void start_host() {
    g_host_port = config().net_port;
    session_request_host(g_host_port);
    g_host_ip = best_local_ip();
    g_host_copied = g_host_ip.ip[0] && copy_to_clipboard(g_host_ip.ip);
    log_line("MULTI", "salon ouvert par le menu : %s (%s), port %u%s", g_host_ip.ip,
             g_host_ip.kind, (unsigned)g_host_port, g_host_copied ? ", adresse copiee" : "");
    open_host_lobby();
}

void prepare_join() {
    // Le presse-papiers d'abord : on vient sans doute de copier l'adresse que
    // l'ami a envoyee. Sinon la derniere adresse utilisee.
    std::string ip;
    uint16_t port = 0;
    const std::string clip = clipboard_address_chars();
    if (parse_address(clip, ip, port))  g_addr_buf = clip;
    else if (g_addr_buf.empty())        g_addr_buf = read_last_host();
    g_buf_dirty = false;
    open_join();
}

void do_join() {
    std::string ip;
    uint16_t port = 0;
    if (!parse_address(g_addr_buf, ip, port)) {
        log_line("MULTI", "adresse invalide : '%s'", g_addr_buf.c_str());
        open_popup(Screen::BadAddress, bad_address_text(), u8"D'accord", nullptr, true,
                   Action::OpenJoin, Action::OpenJoin);
        return;
    }
    g_join_ip   = ip;
    g_join_port = port;
    write_last_host(g_addr_buf);
    log_line("MULTI", "connexion demandee par le menu : %s:%u", ip.c_str(), (unsigned)port);
    session_request_join(ip.c_str(), port);
    open_client_lobby();
}

void click_main_play() {
    void* b = panel_button(g_main_panel, "btn_play");
    char name[64] = {};
    if (!b || !mem_read_std_string((const uint8_t*)b + kBtn_Name, name, sizeof(name)) ||
        strcmp(name, kBtnName_MainMenuPlay) != 0) {
        log_line("MULTI", "!! bouton Jouer introuvable : clique sur « Jouer » toi-meme");
        return;
    }
    g_click_target = b;
    g_click_wait   = 0;
    g_click_tries  = 0;
    log_line("MULTI", "ouverture de l'ecran des sauvegardes (clic sur Jouer)");
}

// Ou mene le bouton Multijoueur, selon l'etat de la session.
void open_auto() {
    const NetRole r = net_role();
    if (r == NetRole::Host && net_active())   { open_host_lobby();   return; }
    if (r == NetRole::Client && net_active()) { open_client_lobby(); return; }
    open_choice();
}

// --- detours -------------------------------------------------------------------

void __fastcall h_MainMenuInit(void* self) {
    o_MainMenuInit(self);

    void* panel = nullptr;
    if (!mem_read((char*)self + kMainMenu_Panel, &panel, sizeof(panel)) || !panel) {
        log_line("MULTI", "!! menu principal : panneau TitleScreen illisible, bouton non renomme");
        return;
    }
    g_main_panel = panel;
    void* scene = nullptr;
    if (mem_read((char*)self + kComp_Scene, &scene, sizeof(scene)) && scene) {
        g_main_scene = scene;
        if (!live_main_scene())
            log_line("MULTI", "!! menu principal : scene %p absente de la liste du Director", scene);
    } else {
        log_line("MULTI", "!! menu principal : scene illisible, les fenetres ne s'ouvriront pas");
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

// Le bouton ne miaule plus : il demande la fenetre Multijoueur, qui s'ouvre au
// debut de l'image suivante (voir mm_menu_pump).
void __fastcall h_Meow(void* /*self*/) {
    if (g_live.alive) return;
    g_action = Action::OpenAuto;
    log_line("MULTI", "bouton Multijoueur clique");
}

void __fastcall h_ConfirmBuild(void* self, uint64_t a2, uint64_t text, uint64_t a4,
                               uint64_t a5, uint64_t ok_only, uint64_t a7, uint64_t a8) {
    if (!g_pending.active) {
        o_ConfirmBuild(self, a2, text, a4, a5, ok_only, a7, a8);
        return;
    }
    Pending p = std::move(g_pending);
    g_pending = Pending{};              // une seule fenetre par demande

    // Le texte arrive deja converti en std::wstring par la recherche de
    // traduction, qui lit notre texte octet par octet : les accents en UTF-8 y
    // sont casses. On le remplace par la vraie version Unicode. Passe par
    // valeur, ce std::wstring est detruit par le constructeur : notre tampon
    // vient donc du tas du jeu (regle 1), et l'ancien est libere par le jeu.
    if (text && !p.text.empty()) {
        auto move = call_target<FnWStrMove>(C_WStrMoveAssign);
        GameWString tmp;
        if (move && make_game_wstring(tmp, p.text))
            move((GameWString*)text, &tmp);
    }

    // Un seul bouton : le jeu a deja ce mode (il cache « no » et renomme
    // « yes » en « OK »), il ne s'en sert simplement nulle part.
    o_ConfirmBuild(self, a2, text, a4, a5, p.ok_only ? 1 : ok_only, a7, a8);

    void* panel = nullptr;
    if (!mem_read((char*)self + kPrompt_Panel, &panel, sizeof(panel)) || !panel) {
        log_line("MULTI", "!! construction de la boite : panneau illisible");
        return;
    }
    if (!p.yes_label.empty()) set_button_label(panel_button(panel, "yes"), p.yes_label.c_str());
    if (!p.no_label.empty() && !p.ok_only)
        set_button_label(panel_button(panel, "no"), p.no_label.c_str());

    // Le champ texte, pour les mises a jour en direct.
    void* prompt = nullptr;
    void* clip   = nullptr;
    auto find = call_target<FnFindChild>(C_SwfFindChild);
    if (find && mem_read((char*)panel + kPanel_Clip, &clip, sizeof(clip)) && clip) {
        GameString n = small_string("prompt");
        prompt = find(clip, &n);
    }
    if (!prompt) log_line("MULTI", "!! champ texte de la boite introuvable : pas de mise a jour en direct");

    g_live.alive  = true;
    g_live.serial = p.serial;
    g_live.screen = p.screen;
    g_live.self   = self;
    g_live.panel  = panel;
    g_live.prompt = prompt;
    g_live.shown  = std::move(p.text);
}

} // namespace

// --- interface ------------------------------------------------------------------

bool mm_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_MainMenuInit: *detour = (void*)&h_MainMenuInit; *original = (void**)&o_MainMenuInit; return true;
    case T_MeowClick:    *detour = (void*)&h_Meow;         *original = (void**)&o_Meow;         return true;
    case T_ConfirmBuild: *detour = (void*)&h_ConfirmBuild; *original = (void**)&o_ConfirmBuild; return true;
    default: return false;
    }
}

void mm_menu_pump() {
    const Action a = g_action;
    g_action = Action::None;
    switch (a) {
        case Action::None:           break;
        case Action::OpenAuto:       open_auto(); break;
        case Action::StartHost:      start_host(); break;
        case Action::OpenJoin:       prepare_join(); break;
        case Action::DoJoin:         do_join(); break;
        case Action::ClickPlay:      click_main_play(); break;
        case Action::StopSession:
            log_line("MULTI", "fin de la session demandee par le menu");
            session_request_disconnect();
            break;
        case Action::OpenBadAddress: break;
    }

    // Un clic demande qui n'a pas pris (bouton encore en animation, par exemple) :
    // on le redemande, quelques fois.
    if (g_click_target && ++g_click_wait > 30) {
        g_click_wait = 0;
        if (++g_click_tries > 3) {
            log_line("MULTI", "!! le clic n'a pas pris, abandon");
            g_click_target = nullptr;
        }
    }

    if (!g_live.alive) return;
    switch (g_live.screen) {
        case Screen::HostLobby:
            popup_set_text(host_lobby_text());
            break;
        case Screen::ClientLobby:
            popup_set_text(client_lobby_text());
            // La sauvegarde de l'hote est arrivee : le jeu doit pouvoir appuyer
            // sur « Jouer » pour nous (mgmp_savefile), ce que la boite, modale,
            // empeche. On la referme.
            if (savefile_client_waiting_to_load() && !g_click_target) {
                log_line("MULTI", "la sauvegarde de l'hote est arrivee : fermeture du salon");
                popup_press("yes");
            }
            break;
        case Screen::JoinEntry:
            if (g_key_enter)  { g_key_enter = false;  popup_press("yes"); }
            if (g_key_escape) { g_key_escape = false; popup_press("no"); }
            if (g_buf_dirty)  { g_buf_dirty = false;  popup_set_text(join_text()); }
            break;
        default:
            break;
    }
}

void mm_on_button_update(void* button) {
    if (!button || button != g_click_target) return;
    g_click_target = nullptr;
    auto click = call_target<FnButtonClick>(C_ButtonClick);
    if (click) click(button, false);   // force = 0, comme un vrai clic
}

bool mm_menu_wndproc(HWND /*hwnd*/, UINT msg, WPARAM wp, LPARAM lp) {
    if (!g_live.alive || g_live.screen != Screen::JoinEntry) return false;

    switch (msg) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            const UINT vk   = (UINT)wp;
            const bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if (vk == VK_RETURN) { g_key_enter = true;  return true; }
            if (vk == VK_ESCAPE) { g_key_escape = true; return true; }
            if (vk == VK_BACK) {
                if (!g_addr_buf.empty()) { g_addr_buf.pop_back(); g_buf_dirty = true; }
                return true;
            }
            if (vk == VK_DELETE) { g_addr_buf.clear(); g_buf_dirty = true; return true; }
            if (ctrl && vk == 'V') {
                const std::string clip = clipboard_address_chars();
                if (!clip.empty()) { g_addr_buf = clip; g_buf_dirty = true; }
                return true;
            }
            // Le caractere selon la disposition du clavier (AZERTY : les chiffres
            // du haut demandent Maj). Drapeau 4 : sans toucher a l'etat du clavier
            // (touches mortes) de la fenetre.
            BYTE keys[256] = {};
            if (GetKeyboardState(keys)) {
                wchar_t out[4] = {};
                const UINT scan = (UINT)((lp >> 16) & 0xFF);
                const int n = ToUnicodeEx(vk, scan, keys, out, 4, 4, GetKeyboardLayout(0));
                if (n == 1) {
                    wchar_t c = out[0];
                    if (c == L',') c = L'.';
                    const bool ok = (c >= L'0' && c <= L'9') || c == L'.' || c == L':';
                    if (ok && g_addr_buf.size() < 40) { g_addr_buf += (char)c; g_buf_dirty = true; }
                }
            }
            return true;                // le jeu ne voit pas ces touches
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_DEADCHAR:
            return true;
        default:
            return false;
    }
}

const char* mm_local_name() {
    if (!g_local_name_tried) {
        g_local_name_tried = true;
        // L'API « plate » de Steam : le jeu lie ISteamFriends018.
        if (HMODULE steam = GetModuleHandleA("steam_api64.dll")) {
            using FnFriends = void* (*)();
            using FnPersona = const char* (*)(void*);
            FnFriends friends = nullptr;
            for (const char* v : { "SteamAPI_SteamFriends_v018", "SteamAPI_SteamFriends_v017" }) {
                friends = (FnFriends)GetProcAddress(steam, v);
                if (friends) break;
            }
            auto persona = (FnPersona)GetProcAddress(steam, "SteamAPI_ISteamFriends_GetPersonaName");
            if (friends && persona) {
                if (void* f = friends()) {
                    const char* n = persona(f);
                    if (n && *n) copy_utf8(g_local_name, sizeof(g_local_name), n);
                }
            }
        }
        if (!g_local_name[0]) copy_utf8(g_local_name, sizeof(g_local_name), "Joueur");
        log_line("MULTI", "nom du joueur : %s", g_local_name);
    }
    return g_local_name;
}

void mm_on_peer_hello(uint8_t peer, const char* name_utf8) {
    if (peer >= 8) return;
    copy_utf8(g_peer_names[peer], sizeof(g_peer_names[peer]), name_utf8 ? name_utf8 : "");
}

} // namespace mgmp
