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
#include "mgmp_catsync.h"   // republier la course quand un invite arrive
#include "mgmp_invsync.h"
#include "mgmp_runhist.h"
#include "mgmp_lockstep.h"  // lockstep_in_battle
#include "mgmp_follow.h"    // follow_inside_node
#include "mgmp_choice.h"    // choice_made_in_node
#include "mgmp_leave.h"     // leave_will_press_quit
#include "mm_world.h"        // world_have_host : le monde de l'hote est arrive
#include "mm_team.h"         // l'equipe mixte : chacun amene ses chats

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
// Component+0x18 : son entite ; Entity+0x08 : la scene de l'entite (Scene::
// CreateEntity, 0x14096B3E0, `mov [rbx+8], rdi`).
constexpr uintptr_t kComp_Entity    = 0x18;
constexpr uintptr_t kEntity_Scene   = 0x08;
constexpr uintptr_t kScene_Director = 0x00;
// Scene+0x4D8 : scene en pause. Le constructeur de la boite (0x14077D363) le met
// a 1 sur toutes les scenes de priorite plus basse que la sienne (1e6) et les
// garde dans sa liste (+0x48) pour les relancer a la fermeture ; la creation
// d'une scene (0x1409D49B9) le remet a 0. Maison en pause = une fenetre modale
// (la notre, celle du jeu, le menu pause...) est par-dessus.
constexpr uintptr_t kScene_Paused   = 0x4D8;
// House+0xB8 : le panneau de fin de journee, un Button. sub_1401E3900 y ecrit
// a chaque image le libelle (+0x1B8) puis le nom (+504) : "EndDay_Sign" sans
// chat dans l'equipe d'aventure, "Depart_Sign" avec (« À l'aventure ! », ou
// « Défendre ! » quand un boss attaque la maison).
constexpr uintptr_t kHouse_Sign     = 0xB8;

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

// sub_1401E3900(House*, ?) : rcx et rdx d'apres son seul appel (0x1401E5B4A) ;
// on relaie r8/r9 par prudence et on rend rax tel quel.
using FnHouseHud = uint64_t (__fastcall*)(void* house, void* a2, uint64_t a3, uint64_t a4);
// HousePipe::TryPlaceCat(Character*) : vrai = le tuyau prend le chat.
using FnPipePlace = bool (__fastcall*)(void* pipe, void* cat);

FnMainMenuInit o_MainMenuInit = nullptr;
FnMeow         o_Meow         = nullptr;
FnConfirmBuild o_ConfirmBuild = nullptr;
FnHouseHud     o_HouseHud     = nullptr;
FnPipePlace    o_PipePlace    = nullptr;

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

bool set_button_label_w(void* button, const std::wstring& w) {
    auto move = call_target<FnWStrMove>(C_WStrMoveAssign);
    if (!button || !move) return false;
    GameWString tmp;
    if (!make_game_wstring(tmp, w)) return false;
    // Deplacement : le jeu prend le tampon et libere l'ancien libelle. `tmp`
    // ressort vide (tampon interne), il n'y a rien a liberer de notre cote.
    move((GameWString*)((char*)button + kButton_Label), &tmp);
    return true;
}

bool set_button_label(void* button, const char* utf8) {
    return set_button_label_w(button, utf8_to_wide(utf8));
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

enum class Screen { None, Choice, HostLobby, JoinEntry, ClientLobby, BadAddress, NightWait,
                    NightNotice, DepartWait, ArrivalWait, DepartOffer, RunOffer, JoinWait,
                    HomeOffer, Info };

// Les demandes posees par les clics et par le clavier, traitees au debut de
// l'image suivante. Un seul emplacement : il n'y a jamais deux clics a la fois.
enum class Action { None, OpenAuto, StartHost, OpenJoin, DoJoin, ClickPlay, StopSession, OpenBadAddress,
                    CancelNight, NightReadyNow, DepartCancel, DepartYes, DepartNo, RunJoin, RunLater,
                    GoHome, JoinCancel, ArrivalDismiss };
Action g_action = Action::None;

// Images depuis le lancement (mm_menu_pump), pour les delais.
uint32_t g_frame = 0;
// Le dernier panneau de la maison clique (1 « Fin de journée », 2 « À
// l'aventure ! ») et l'image du clic : Button::Click appelle l'action du
// panneau lui-meme (0x14097E9C4, std::function), donc dans la meme image.
int      g_sign_click    = 0;
uint32_t g_sign_click_at = 0;

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
//
// Une boite ne se ferme pas d'un coup : le clic lance son animation de fermeture,
// et son action n'est appelee qu'a la fin (une demi-seconde environ). Pendant ce
// temps elle est toujours « alive », et ses boutons disparaissent du panneau.
// D'ou `pressed` : un seul appui automatique, puis on attend (kPressRetry) ; la
// build 5 rappuyait a chaque image et remplissait le journal.
struct Live {
    bool         alive   = false;
    uint32_t     serial  = 0;
    Screen       screen  = Screen::None;
    void*        self    = nullptr; // le composant YesNoPrompt
    void*        panel   = nullptr; // son MenuPanel "ConfirmationBox"
    void*        prompt  = nullptr; // son champ texte "prompt"
    std::wstring shown;             // dernier texte ecrit dans ce champ
    bool         pressed = false;   // un appui automatique a ete demande...
    uint32_t     pressed_frame = 0; // ... a cette image
    bool         warned  = false;   // « bouton introuvable » deja ecrit
};
Live     g_live;
uint32_t g_serial  = 0;
uint32_t g_handled = 0;             // derniere boite dont l'action a ete prise

// Un appui qui n'a pas pris (bouton encore en animation d'ouverture) est refait
// apres ce delai ; une fermeture normale est finie bien avant.
constexpr uint32_t kPressRetry = 90;

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
void* g_director   = nullptr;       // le Director du jeu (vit tout le processus)

// La fermeture en cours n'est pas un choix du joueur : ne pas lancer l'action
// du bouton (fermeture du salon quand la sauvegarde arrive, de l'attente de la
// nuit quand elle passe).
uint32_t g_silent = 0;

// La scene vivante (pas en cours de destruction) de ce nom, ou nullptr.
// `paused` recoit son drapeau de pause (kScene_Paused).
void* find_scene(const char* wanted, bool* paused = nullptr) {
    if (!g_director) return nullptr;
    void** begin = nullptr;
    void** end   = nullptr;
    if (!mem_read((char*)g_director + kDirector_ScenesBegin, &begin, sizeof(begin)) ||
        !mem_read((char*)g_director + kDirector_ScenesEnd,   &end,   sizeof(end)) ||
        !begin || end < begin || end - begin > 256)
        return nullptr;
    for (void** it = begin; it < end; ++it) {
        void* s = nullptr;
        if (!mem_read(it, &s, sizeof(s)) || !s) continue;
        uint8_t dying = 1;
        if (!mem_read((char*)s + kScene_Destroying, &dying, 1) || dying) continue;
        char name[48] = {};
        if (!mem_read_std_string((char*)s + kScene_Name, name, sizeof(name))) continue;
        if (strcmp(name, wanted) != 0) continue;
        if (paused) {
            uint8_t p = 1;
            *paused = !mem_read((char*)s + kScene_Paused, &p, 1) || p != 0;
        }
        return s;
    }
    return nullptr;
}

// ShowConfirmation(texte, oui, non, scene) : sans scene, le jeu prend la scene
// « en cours de mise a jour » (global 0x1413C18F0), qui n'existe que pendant
// l'update des scenes. Tous les appels du jeu passent nullptr parce qu'ils sont
// faits depuis un bouton ; nous ouvrons au debut de l'image, ou ce global est
// nul (plantage de la build 3, Mewgenics.exe+0x77DD5F). On passe donc une scene
// vivante de la liste du Director : la maison si elle est la, sinon le menu
// principal ou l'ecran des sauvegardes. Le Director est retenu dans
// MainMenu::init (premier champ de toute scene) : c'est un singleton qui vit
// tout le processus.
// Le combat et l'evenement viennent en dernier : on n'y ouvre une fenetre que
// quand l'aventure de l'hote est finie (battle_popups_ok), pour ramener l'invite
// chez lui.
void* popup_parent_scene() {
    for (const char* name : { "House", "Map", "MainMenu", "SaveSelectionScreen", "Battle", "Event" })
        if (void* s = find_scene(name)) return s;
    return nullptr;
}

bool battle_popups_ok();                // section aventure, plus bas

void popup_closed(uint32_t serial) {
    if (g_live.serial != serial) return;
    g_live.alive   = false;
    g_live.self    = nullptr;
    g_live.panel   = nullptr;
    g_live.prompt  = nullptr;
    g_live.pressed = false;
    g_click_target = nullptr;
}

// L'action d'une boite, une seule fois : si un second clic passait pendant
// l'animation de fermeture, le jeu rappellerait l'action, et « Annuler » de
// l'attente de la nuit partirait apres coup.
void popup_action(uint32_t serial, Action a) {
    if (g_handled == serial) return;
    g_handled = serial;
    popup_closed(serial);
    if (g_silent == serial) { g_silent = 0; return; }
    if (a != Action::None) g_action = a;
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
    // Jamais pendant un combat : la boite met en pause toutes les scenes en
    // dessous d'elle, et un combat en pause chez un seul joueur casserait la
    // marche au pas de mgmp. La demande attend la fin du combat. Sauf chez un
    // invite dont l'hote a quitte l'aventure : ce combat-la ne reprendra pas.
    if (find_scene("Battle") && !battle_popups_ok()) return false;
    void* scene = popup_parent_scene();
    if (!scene) {
        log_line("MULTI", "!! fenetre impossible : aucune scene ou l'afficher (maison, carte, menu)");
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
    std::function<void()> yes_fn = [serial, on_yes] { popup_action(serial, on_yes); };
    std::function<void()> no_fn  = [serial, on_no]  { popup_action(serial, on_no); };

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

// Demande un clic sur l'un des deux boutons de la boite ouverte. Vrai = clic
// demande. Apres un appui, rien avant kPressRetry images : la boite est sans
// doute en train de se fermer, et ses boutons ont deja quitte le panneau.
bool popup_press(const char* name) {
    if (!g_live.alive || !live_scene_ok()) return false;
    if (g_live.pressed && g_frame - g_live.pressed_frame < kPressRetry) return false;
    void* b = panel_button(g_live.panel, name);
    if (!b) {
        if (!g_live.warned) {
            g_live.warned = true;
            log_line("MULTI", "!! bouton '%s' introuvable dans la fenetre", name);
        }
        return false;
    }
    g_click_target       = b;
    g_click_wait         = 0;
    g_click_tries        = 0;
    g_live.pressed       = true;
    g_live.pressed_frame = g_frame;
    return true;
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

// Un invite attend le monde de l'hote avant de choisir sa save (mm_world). Pas
// indefiniment : au bout de 30 secondes de session prete sans rien recu, il
// peut choisir quand meme -- sa save garde alors son monde jusqu'au chargement
// suivant.
uint32_t g_client_ready_at = 0;          // image ou la session est devenue prete

bool client_world_wait_over() {
    if (net_role() != NetRole::Client || !session_ready()) { g_client_ready_at = 0; return false; }
    if (!g_client_ready_at) g_client_ready_at = g_frame ? g_frame : 1;
    return g_frame - g_client_ready_at > 30 * 60;
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
    if (!world_have_host() && !client_world_wait_over())
        return fmt(u8"Salon multijoueur\n\n"
                   u8"Tu as rejoint la partie de %s !\n\n"
                   u8"Joueurs :\n  %s (hôte)\n  %s\n\n"
                   u8"En attente de la partie de %s :\n"
                   u8"dès qu'il aura choisi sa sauvegarde,\n"
                   u8"tu pourras choisir la tienne.",
                   host.c_str(), host.c_str(), me.c_str(), host.c_str());
    return fmt(u8"Salon multijoueur\n\n"
               u8"Tu as rejoint la partie de %s !\n\n"
               u8"Joueurs :\n  %s (hôte)\n  %s\n\n"
               u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
               u8"Les PNJ et l'histoire seront ceux de %s,\n"
               u8"et pour partir à l'aventure, tu rejoindras son équipe.",
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
    open_popup(Screen::ClientLobby, client_lobby_text(), u8"Choisir ta sauvegarde", u8"Quitter",
               false, Action::ClickPlay, Action::StopSession);
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


// --- nuit votee -------------------------------------------------------------------
//
// En session, le panneau de fin de journee de la maison (le bouton nomme
// « EndDay_Sign », Button+504) ne lance plus la nuit tout de suite : son clic est
// retenu (mm_on_button_click, appele par le detour de Button::Click), le joueur
// passe « pret » et une fenetre montre qui l'est. L'hote tient le compte et
// annonce a tous ou on en est (MM_NIGHT_STATE) ; quand tout le monde est pret
// (a = 1), chaque jeu ferme son attente et rejoue le clic retenu : le jeu fait
// alors sa fin de journee normale, chacun chez soi.
//
// Les autres le voient (build 6) : tant qu'un joueur attend, leur panneau dit
// « 1 joueur attend », et une petite fenetre le leur annonce, avec « Passer la
// nuit » pour etre pret sans chercher le panneau. Elle ne s'ouvre que quand la
// maison est a l'ecran et libre : pas en pause, pas sous une autre fenetre.
//
// Le meme panneau sert au depart en aventure : avec des chats dans l'equipe, il
// dit « À l'aventure ! » (ou « Défendre ! ») et son clic part en aventure. Ce
// clic-la n'est pas un vote de nuit et passe tel quel.

constexpr char kBtnName_EndDay[] = "EndDay_Sign";
// Le meme panneau, quand il y a des chats dans l'equipe : sub_1401E3900 lui donne
// alors ce nom-la (0x1401E41C6) et le libelle « À l'aventure ! » (ou « Défendre ! »).
constexpr char kBtnName_Depart[] = "Depart_Sign";

bool trip_active();                     // section aventure, plus bas

bool     g_night_ready       = false;   // ce joueur a clique sur le panneau
void*    g_endday_button     = nullptr; // le bouton dont le clic est retenu
bool     g_night_go          = false;   // la nuit passe : fermer l'attente, rejouer le clic
bool     g_night_passing     = false;   // le prochain clic sur le panneau est le notre
bool     g_night_wait_wanted = false;   // ouvrir l'attente au debut de l'image
uint32_t g_night_ready_mask  = 0;       // dernieres annonces de l'hote
uint32_t g_night_roster_mask = 0;
bool     g_host_ready[8]     = {};      // cote hote : qui est pret, par numero de joueur
uint32_t g_host_last_roster  = 0;

// L'annonce aux joueurs qui ne sont pas encore prets.
uint32_t g_notice_mask       = 0;       // les joueurs prets qu'on a deja annonces
bool     g_notice_wanted     = false;   // une annonce attend que la maison soit libre
uint32_t g_notice_try        = 0;       // image de la derniere tentative
uint32_t g_house_free_since  = 0;       // la maison est libre depuis cette image (0 = non)

// La maison, vue par le detour de sub_1401E3900 (une fois par image passee chez soi).
void*        g_house        = nullptr;  // le composant House
void*        g_house_sign   = nullptr;  // son panneau (House+0xB8)
bool         g_house_depart = false;    // des chats dans l'equipe : « À l'aventure ! »
uint32_t     g_house_frame  = 0;        // image ou on l'a vue pour la derniere fois
std::wstring g_sign_label;              // libelle impose au panneau ; vide = celui du jeu

bool session_has_others() {
    return session_ready() && net_active() && net_peer_count() >= 2;
}

int bit_count(uint32_t m) { int n = 0; for (; m; m &= m - 1) ++n; return n; }

uint32_t roster_mask() {
    uint8_t ids[8] = {};
    uint32_t mask = 0;
    const uint8_t n = net_peer_count();
    if (n && net_peer_ids(ids, 8))
        for (uint8_t i = 0; i < n && i < 8; ++i)
            if (ids[i] < 8) mask |= 1u << ids[i];
    return mask;
}

// Les AUTRES joueurs presents qui sont prets, d'apres la derniere annonce.
uint32_t others_ready_mask() {
    if (!session_has_others()) return 0;
    const uint8_t self = net_self();
    const uint32_t me = self < 8 ? 1u << self : 0;
    return g_night_ready_mask & g_night_roster_mask & roster_mask() & ~me;
}

void apply_night_state(bool go, uint32_t ready, uint32_t roster) {
    g_night_roster_mask = roster;
    // La nuit passe : le compte repart de zero. Sans cette remise a zero, le
    // « tout le monde est pret » de la veille restait le dernier etat connu, et
    // le lendemain chacun aurait vu « 1 joueur attend » sans raison.
    g_night_ready_mask  = go ? 0 : ready;
    if (go && g_night_ready) {
        g_night_ready = false;
        g_night_go    = true;
        log_line("MULTI", "nuit : tout le monde est pret, elle passe");
    }
}

// L'hote annonce l'etat a tous (et se l'applique). La nuit passe quand tous les
// joueurs presents, au moins deux, sont prets ; le compte repart alors de zero.
void host_broadcast_night() {
    const uint32_t roster = roster_mask();
    g_host_last_roster = roster;
    uint32_t ready = 0;
    for (uint8_t id = 0; id < 8; ++id)
        if (g_host_ready[id] && (roster & (1u << id))) ready |= 1u << id;
    const bool go = roster && ready == roster && bit_count(roster) >= 2;
    MmMsg m{};
    m.kind = MM_NIGHT_STATE;
    m.a = go ? 1 : 0;
    m.x = ready;
    m.y = roster;
    net_send_mm(m);
    apply_night_state(go, ready, roster);
    if (go) memset(g_host_ready, 0, sizeof(g_host_ready));
}

void set_night_ready(bool ready) {
    g_night_ready = ready;
    log_line("MULTI", "nuit : %s", ready ? "pret" : "annule");
    if (net_role() == NetRole::Host) {
        const uint8_t self = net_self();
        g_host_ready[self < 8 ? self : 0] = ready;
        host_broadcast_night();
    } else {
        MmMsg m{};
        m.kind = MM_NIGHT_READY;
        m.a = ready ? 1 : 0;
        net_send_mm(m);
    }
}

// La liste des joueurs, un par ligne, et combien ne sont pas encore prets
// (ce joueur compris).
std::string night_lines(int* not_ready) {
    std::string lines;
    int waiting = 0;
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (n && net_peer_ids(ids, 8)) {
        for (uint8_t i = 0; i < n && i < 8; ++i) {
            const uint8_t id = ids[i];
            const bool me = id == net_self();
            const bool ready = me ? g_night_ready : (g_night_ready_mask & (1u << id)) != 0;
            const std::string who = me ? display_name(mm_local_name()) + u8" (toi)" : peer_name(id);
            lines += "  " + who + (ready ? u8" : prêt\n" : u8" : pas encore\n");
            if (!ready) ++waiting;
        }
    }
    if (not_ready) *not_ready = waiting;
    return lines;
}

// Ce joueur est pret et attend les autres.
std::string night_text() {
    int waiting = 0;
    const std::string lines = night_lines(&waiting);
    std::string count;
    if (waiting <= 0)      count = u8"Tout le monde est prêt !";
    else if (waiting == 1) count = u8"En attente d'un joueur.";
    else                   count = fmt(u8"En attente de %d joueurs.", waiting);
    return fmt(u8"Fin de journée\n\n"
               u8"Tu es prêt à passer la nuit.\n%s\n\n%s",
               count.c_str(), lines.c_str());
}

// Un ou plusieurs autres joueurs sont prets, pas celui-ci.
std::string notice_text(bool depart) {
    const int n = bit_count(others_ready_mask());
    const std::string lines = night_lines(nullptr);
    const std::string who = n <= 1 ? std::string(u8"1 joueur attend")
                                   : fmt(u8"%d joueurs attendent", n);
    const char* tail = depart
        ? u8"\nTon panneau sert au départ en aventure :\n"
          u8"retire tes chats de l'équipe pour finir la journée."
        : "";
    return fmt(u8"Fin de journée\n\n"
               u8"%s pour passer la nuit.\n\n%s%s",
               who.c_str(), lines.c_str(), tail);
}

// Le libelle du panneau tant que d'autres attendent ce joueur.
std::wstring sign_label_wanted() {
    if (g_night_ready || savefile_in_coop()) return {};
    const int n = bit_count(others_ready_mask());
    if (n <= 0) return {};
    if (n == 1) return utf8_to_wide(u8"1 joueur attend");
    return utf8_to_wide(fmt(u8"%d joueurs attendent", n).c_str());
}

void open_night_wait() {
    open_popup(Screen::NightWait, night_text(), u8"Annuler", nullptr, true,
               Action::CancelNight, Action::CancelNight);
}

// Ce que le panneau de la maison propose, d'apres son nom (Button+504) :
// 0 pas le panneau, 1 « Fin de journée », 2 « À l'aventure ! » / « Défendre ! ».
int sign_kind(void* b) {
    char name[32] = {};
    if (!b || !mem_read_std_string((char*)b + kBtn_Name, name, sizeof(name))) return 0;
    if (strcmp(name, kBtnName_EndDay) == 0) return 1;
    if (strcmp(name, kBtnName_Depart) == 0) return 2;
    return 0;
}

bool endday_button_valid(void* b) { return sign_kind(b) == 1; }

// --- la maison -----------------------------------------------------------------

// MewDirector+0x5A8 -> +0x468 non nul et +0x478 nul : le panneau dit « Défendre ! »
// (un boss attaque la maison), meme test que sub_1401E3900 (0x1401E4282).
constexpr uintptr_t kDir_HouseBoss     = 0x5A8;
constexpr uintptr_t kBoss_Pending      = 0x468;
constexpr uintptr_t kBoss_Busy         = 0x478;

bool house_boss_defend() {
    const void** slot = (const void**)addr_of_data(D_MewDirectorPtr);
    const void* dir = nullptr;
    const void* boss = nullptr;
    const void* pending = nullptr;
    int32_t busy = 1;
    if (!slot || !mem_read(slot, &dir, sizeof(dir)) || !dir) return false;
    if (!mem_read((const char*)dir + kDir_HouseBoss, &boss, sizeof(boss)) || !boss) return false;
    if (!mem_read((const char*)boss + kBoss_Pending, &pending, sizeof(pending)) || !pending)
        return false;
    if (!mem_read((const char*)boss + kBoss_Busy, &busy, sizeof(busy))) return false;
    return busy == 0;
}

// Juste apres que le jeu a refait le panneau : on le note, et on lui donne notre
// libelle s'il en faut un. Le bouton le recopiera dans son champ SWF.
void house_after_refresh(void* house) {
    void* sign = nullptr;
    if (!mem_read((char*)house + kHouse_Sign, &sign, sizeof(sign))) return;
    const int kind = sign_kind(sign);
    if (!kind) return;
    g_house        = house;
    g_house_sign   = sign;
    g_house_frame  = g_frame;
    g_house_depart = kind == 2;
    if (!g_sign_label.empty() && kind == 1) set_button_label_w(sign, g_sign_label);
}

// Le panneau est-il cliquable pour finir la journee (connu, pas en mode depart,
// pas grise par le jeu) ?
bool sign_can_end_day() {
    void* sign = g_house_sign;
    // La scene d'abord : maison detruite = pointeur perime, qu'on ne lit pas.
    if (!find_scene("House")) return false;
    if (!endday_button_valid(sign) || g_house_depart) return false;
    int32_t state = 0;
    if (!mem_read((char*)sign + kBtn_State, &state, sizeof(state))) return false;
    return state != kBtnState_Disabled;
}

// La maison est-elle a l'ecran, sans rien par-dessus ? Scene presente, ni en
// destruction ni en pause (une fenetre modale ou le menu pause la mettent en
// pause), et refaite par le jeu a l'instant.
bool house_is_free() {
    bool paused = true;
    if (!find_scene("House", &paused) || paused) return false;
    return g_house && g_frame - g_house_frame <= 2;
}

// « Passer la nuit » depuis l'annonce : exactement comme un clic sur le panneau.
void night_ready_now() {
    if (!session_has_others() || g_night_ready) return;
    if (!sign_can_end_day()) {
        log_line("MULTI", "!! nuit : panneau de fin de journee indisponible, clique dessus toi-meme");
        return;
    }
    g_endday_button = g_house_sign;
    set_night_ready(true);
    g_night_wait_wanted = true;
}

bool g_notice_tail = false;             // l'annonce ouverte explique le mode depart

bool open_night_notice() {
    const bool can = sign_can_end_day();
    g_notice_tail = !can && g_house_depart;
    const bool ok = can
        ? open_popup(Screen::NightNotice, notice_text(false), u8"Passer la nuit", u8"Plus tard",
                     false, Action::NightReadyNow, Action::None)
        : open_popup(Screen::NightNotice, notice_text(g_notice_tail), u8"D'accord", nullptr,
                     true, Action::None, Action::None);
    if (ok)
        log_line("MULTI", "nuit : annonce affichee (%d joueur(s) pret(s))",
                 bit_count(others_ready_mask()));
    return ok;
}

// Chaque image : annoncer, faire passer la nuit quand c'est decide, suivre les departs.
void night_pump() {
    const bool others_here = session_has_others();
    // Plus personne avec qui voter (deconnexion) : la nuit passe pour ce joueur,
    // et rien de cette session ne reste dans les comptes -- sinon un ami qui
    // revient trouverait des joueurs « prets » qui ne le sont plus.
    if (!others_here) {
        if (g_night_ready) {
            log_line("MULTI", "nuit : plus d'autre joueur connecte, elle passe");
            g_night_ready = false;
            g_night_go    = true;
        }
        g_night_ready_mask  = 0;
        g_night_roster_mask = 0;
        g_host_last_roster  = 0;
        memset(g_host_ready, 0, sizeof(g_host_ready));
    }
    // Cote hote : un joueur est parti ou arrive. Le parti n'est plus pret (s'il
    // revient sous le meme numero, il repart de zero) ; si quelqu'un attend, tout
    // le monde recoit le nouvel etat, l'arrivant compris.
    if (net_role() == NetRole::Host && others_here) {
        const uint32_t roster = roster_mask();
        if (roster != g_host_last_roster) {
            for (uint8_t id = 0; id < 8; ++id)
                if (!(roster & (1u << id))) g_host_ready[id] = false;
            bool anyone = false;
            for (bool r : g_host_ready) anyone |= r;
            if (anyone) host_broadcast_night();
            else        g_host_last_roster = roster;
        }
    }

    // Les autres joueurs prets. Une annonce par nouveau joueur pret ; celui qui
    // annule puis revient est annonce de nouveau. Pret soi-meme, on a deja la
    // liste sous les yeux : rien a annoncer.
    const uint32_t others = others_ready_mask();
    g_notice_mask &= others;
    if (g_night_ready || !others) {
        g_notice_wanted = false;
        g_notice_mask   = others;
    } else if (others & ~g_notice_mask) {
        g_notice_wanted = true;
    }
    g_sign_label = sign_label_wanted();

    if (!g_night_go) return;

    // D'abord fermer l'attente (sans lancer son action « Annuler »)...
    if (g_live.alive) {
        if (g_live.screen == Screen::NightWait && !g_click_target) {
            g_silent = g_live.serial;
            popup_press("yes");
        }
        return;
    }
    // ... puis rejouer le clic sur le panneau, au moment ou le jeu clique.
    g_night_go = false;
    if (find_scene("House") && endday_button_valid(g_endday_button)) {
        g_night_passing = true;
        g_click_target  = g_endday_button;
        g_click_wait    = 0;
        g_click_tries   = 0;
    } else {
        log_line("MULTI", "!! nuit : panneau de fin de journee introuvable, clique dessus toi-meme");
        g_night_passing = true;      // le prochain clic du joueur passera sans attendre
    }
    g_endday_button = nullptr;
}

// Ouvre l'annonce quand la maison est libre depuis un moment (pas pendant un
// fondu ou la seconde ou la fenetre precedente se ferme).
void notice_pump() {
    const bool free_now = g_notice_wanted && !g_live.alive && !g_night_go && house_is_free() &&
                          !savefile_in_coop() && !savefile_join_armed() && !trip_active();
    if (!free_now) { g_house_free_since = 0; return; }
    if (!g_house_free_since) g_house_free_since = g_frame;
    if (g_frame - g_house_free_since < 45) return;
    if (g_notice_try && g_frame - g_notice_try < 120) return;
    g_notice_try = g_frame;
    if (open_night_notice()) {
        g_notice_wanted = false;
        g_notice_mask   = others_ready_mask();
    }
}

// --- aventure : chacun sa save ---------------------------------------------------
//
// Chacun joue dans sa propre save. Pour une aventure, en attendant l'equipe
// mixte, l'invite passe dans la copie de l'aventure de l'hote (mgmp_coop.sav,
// le mecanisme de mgmp) et joue avec les chats de l'hote, puis rentre chez lui.
//
//   1. L'hote clique sur « À l'aventure ! » : le clic est retenu (comme pour la
//      nuit) et chacun recoit MM_DEPART_REQ. L'invite repond « J'arrive » ou
//      « Pas encore » (re-propose 30 s plus tard). Tout le monde vient : le clic
//      est rejoue, l'hote part.
//   2. « J'arrive » : l'invite va au menu principal (Echap ouvre le menu pause,
//      le mod appuie sur « Quitter vers le menu ») et attend la save de l'hote,
//      publiee quand l'hote arrive sur la carte (mgmp_savefile). Elle se charge
//      toute seule (« Jouer », puis l'emplacement, redirige vers mgmp_coop.sav).
//      Pendant ce temps l'hote attend son equipe sur la carte (fenetre modale) :
//      les combats de mgmp ne commencent qu'avec tout le monde.
//   3. L'aventure finie (MM_HOST_RUN a = 0), l'invite rentre : menu, « Jouer »,
//      puis SON emplacement, sans redirection.
//
// Un invite qui arrive pendant une aventure recoit MM_HOST_RUN a = 1 et peut la
// rejoindre (MM_RUN_REQ : l'hote republie sa save a la case suivante).

// --- hote ---
bool     g_dep_active      = false;   // vote de depart en cours
void*    g_dep_button      = nullptr; // le panneau dont le clic est retenu
uint8_t  g_dep_answer[8]   = {};      // 0 pas de reponse, 1 « J'arrive », 2 « Pas encore »
bool     g_dep_go          = false;   // tout le monde vient : rejouer le clic
bool     g_dep_passing     = false;   // le prochain clic sur le panneau est le notre
bool     g_dep_wait_wanted = false;   // ouvrir l'attente du vote
uint8_t  g_dep_seq         = 0;       // numero du vote en cours (MM_DEPART_REQ.b) : une
                                      // reponse a un vote d'avant ne compte pas
// MM_DEPART_REQ a = 0, champ c : pourquoi le depart n'a pas lieu.
constexpr uint8_t kDepartOff_Host   = 0;  // l'hote a annule le vote
constexpr uint8_t kDepartOff_Nobody = 1;  // le vote est passe, mais aucun chat ne peut partir
int      g_host_run_sent   = -1;      // dernier MM_HOST_RUN envoye (-1 : jamais)
uint32_t g_host_run_roster = 0;       // presents au dernier envoi
bool     g_peer_run[8]     = {};      // MM_PEER_RUN recus
uint32_t g_arrival_mask    = 0;       // joueurs attendus dans l'aventure
uint32_t g_arrival_dismissed = 0;     // « Ne pas attendre » : ceux-la, plus de fenetre
uint32_t g_run_req_pending = 0;       // demandes de rejoindre a servir sur la carte
bool     g_arrival_wanted  = false;   // ouvrir l'attente des arrivants

// --- invite ---
bool     g_host_in_run     = false;   // derniere annonce de l'hote
bool     g_dep_req_live    = false;   // la demande de depart de l'hote tient toujours
uint8_t  g_dep_req_seq     = 0;       // son numero, repris dans la reponse
bool     g_dep_offer       = false;   // la proposer a ce joueur
uint32_t g_dep_offer_at    = 0;       // pas avant cette image (« Pas encore »)
bool     g_run_offer       = false;   // proposer de rejoindre l'aventure en cours
uint32_t g_run_offer_at    = 0;
bool     g_home_offer      = false;   // proposer de rentrer
// L'aventure de l'hote est finie (abandon, victoire, defaite, connexion coupee)
// et ce joueur n'est pas encore rentre chez lui : vrai dans la copie, et encore
// apres en etre sorti autrement (mgmp_leave le ramene au menu, ou lui-meme).
bool     g_home_due        = false;
// La partie a l'ecran est une copie d'une aventure de l'hote, meme quand
// mgmp_savefile ne la compte plus (une nouvelle copie arrive pendant que
// l'ancienne est encore affichee). Faux une fois passe par le menu principal.
bool     g_copy_on_screen  = false;
int      g_peer_run_sent   = -1;      // dernier MM_PEER_RUN envoye

// --- un message simple, un seul bouton ---
bool        g_info_wanted = false;
std::string g_info_text;

// --- aller au menu principal depuis la maison (ou la carte) ---
//
// Il n'y a pas d'ecran de menu pause persistant ou aller cliquer (voir
// mgmp_leave.h) : on l'ouvre avec la touche Echap, envoyee a la fenetre du jeu,
// puis on appuie sur son bouton « Quitter vers le menu » depuis Button::update,
// la ou le jeu clique les siens. Pas de nouvel Echap tant que la scene est en
// pause : le menu pause (ou une fenetre) est ouvert, il ne faut pas le refermer.
struct MenuTrip {
    bool     active   = false;
    uint32_t next_esc = 0;            // image du prochain Echap
    uint32_t key_up   = 0;            // image ou relacher la touche
    bool     key_down = false;
    int      esc_sent = 0;
    uint32_t press_ok = 0;            // pas d'autre appui sur « Quitter » avant
    bool     told     = false;        // le joueur a ete prevenu qu'il faut le faire
};
MenuTrip g_trip;
HWND     g_hwnd = nullptr;            // la fenetre du jeu (vue dans mm_menu_wndproc)
// Apres l'appui sur « Quitter vers le menu », le temps d'attendre le menu avant
// de demander au joueur de le faire lui-meme (30 s a 60 images par seconde).
constexpr uint32_t kTripPatience = 30 * 60;
const std::string kTripHelp = u8"Pour continuer, appuie sur Échap\n"
                              u8"puis sur « Quitter vers le menu ».";

bool trip_active() { return g_trip.active; }

// Une fenetre par-dessus un combat ne gene la marche au pas que si le combat
// doit continuer. Chez un invite dont l'hote a quitte l'aventure, il ne
// reprendra pas : la fenetre « Retour à la maison » peut s'y ouvrir.
// (Un invite, ou un ancien invite dont la connexion vient de tomber : il n'est
// plus « Client », mais sa copie, elle, est toujours la.)
bool battle_popups_ok() {
    if (net_role() == NetRole::Host) return false;
    if (session_has_others() && g_host_in_run) return false;     // le combat continue
    // A rentrer, ou deja en route (le trajet peut avoir a dire « appuie sur
    // Échap » par-dessus le combat).
    return g_home_due || (savefile_in_coop() && savefile_home_armed());
}

constexpr uint32_t kOfferAgain  = 30 * 60;   // 30 s avant de re-proposer

void show_info(const std::string& text) {
    g_info_text   = text;
    g_info_wanted = true;
}

bool at_main_menu() {
    return find_scene("MainMenu") || find_scene("SaveSelectionScreen");
}

void post_escape(bool down) {
    if (!g_hwnd) return;
    // lParam : repetition 1, code de balayage 0x01 (Echap) ; relachement : bits 30 et 31.
    const LPARAM lp = down ? (LPARAM)0x00010001 : (LPARAM)0xC0010001;
    PostMessageW(g_hwnd, down ? WM_KEYDOWN : WM_KEYUP, VK_ESCAPE, lp);
}

void start_menu_trip(const char* why) {
    if (at_main_menu() || g_trip.active) return;
    g_trip = MenuTrip{};
    g_trip.active   = true;
    g_trip.next_esc = g_frame + 15;   // la fenetre qui vient de se fermer d'abord
    log_line("MULTI", "aller au menu principal (%s) : Echap, puis « Quitter vers le menu »", why);
}

void trip_pump() {
    if (!g_trip.active) return;
    if (g_trip.key_down && g_frame >= g_trip.key_up) {
        post_escape(false);
        g_trip.key_down = false;
    }
    if (at_main_menu()) {
        if (g_trip.key_down) { post_escape(false); g_trip.key_down = false; }
        g_trip.active = false;
        // L'aide « appuie sur Échap » n'a plus d'objet : il y est.
        if (g_trip.told && g_info_wanted && g_info_text == kTripHelp) g_info_wanted = false;
        log_line("MULTI", "arrive au menu principal");
        return;
    }
    if (g_frame < g_trip.next_esc || g_live.alive) return;
    // « Quitter vers le menu » a ete appuye : le retour au menu est en cours,
    // et il peut etre long (sauvegarde, fenetre du jeu en arriere-plan). Plus
    // aucun Echap : arrive en retard, il tomberait sur le menu principal. Si
    // rien n'a bouge au bout d'un long moment, on le dit au joueur.
    if (g_trip.press_ok) {
        if (g_frame > g_trip.press_ok + kTripPatience && !g_trip.told) {
            g_trip.told = true;
            log_line("MULTI", "!! « Quitter vers le menu » appuye, mais toujours pas au menu");
            show_info(kTripHelp);
        }
        return;
    }
    // La scene du dessus d'abord : pendant un combat (ou un evenement), la carte
    // peut etre encore chargee en dessous, et c'est le combat que le menu pause
    // met en pause.
    bool paused = false;
    void* here = nullptr;
    for (const char* name : { "Battle", "Event", "House", "Map" })
        if ((here = find_scene(name, &paused)) != nullptr) break;
    if (here && paused) {                    // menu pause ouvert : attendre l'appui
        g_trip.next_esc = g_frame + 60;
        return;
    }
    if (g_trip.esc_sent >= 3 || !g_hwnd) {
        if (!g_trip.told) {
            g_trip.told = true;
            log_line("MULTI", "!! le menu pause ne s'ouvre pas tout seul : le joueur doit le faire");
            show_info(kTripHelp);
        }
        return;
    }
    post_escape(true);
    g_trip.key_down = true;
    g_trip.key_up   = g_frame + 3;
    ++g_trip.esc_sent;
    g_trip.next_esc = g_frame + 150;
    log_line("MULTI", "Echap envoye a la fenetre du jeu (%d/3)", g_trip.esc_sent);
}

// Depuis Button::update : le bouton « Quitter vers le menu » du menu pause.
void trip_on_button(void* button) {
    if (!g_trip.active || g_frame < g_trip.press_ok) return;
    char name[64] = {};
    if (!mem_read_std_string((char*)button + kBtn_Name, name, sizeof(name))) return;
    if (strcmp(name, kBtnName_PauseQuitToMenu) != 0) return;
    g_trip.press_ok = g_frame + 120;
    // L'hote a quitte l'aventure : mgmp_leave appuie deja sur ce bouton (dans
    // cette image ou apres son delai). Un second clic relancerait le retour au
    // menu pendant le premier.
    if (leave_will_press_quit()) {
        log_line("MULTI", "menu pause ouvert : « Quitter vers le menu » est appuye par mgmp");
        return;
    }
    log_line("MULTI", "menu pause ouvert : appui sur « Quitter vers le menu »");
    auto click = call_target<FnButtonClick>(C_ButtonClick);
    if (click) click(button, false);
}

// --- textes ---

std::string host_display() { return peer_name(0); }

const char* dep_answer_text(uint8_t a) {
    switch (a) {
        case 1:  return u8"j'arrive !";
        case 2:  return u8"pas encore";
        default: return u8"réfléchit...";
    }
}

std::string depart_wait_text() {
    std::string lines;
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (n && net_peer_ids(ids, 8))
        for (uint8_t i = 0; i < n && i < 8; ++i) {
            if (ids[i] == net_self() || ids[i] >= 8) continue;
            lines += "  " + peer_name(ids[i]) + " : " + dep_answer_text(g_dep_answer[ids[i]]) + "\n";
        }
    return fmt(u8"Départ en aventure\n\n"
               u8"Tu veux partir à l'aventure.\n"
               u8"Ton équipe répond :\n\n%s", lines.c_str());
}

std::string arrival_text() {
    std::string lines;
    for (uint8_t id = 0; id < 8; ++id)
        if (g_arrival_mask & (1u << id))
            lines += "  " + peer_name(id) + u8" : pas encore là\n";
    return fmt(u8"En attente de ton équipe\n\n%s\n"
               u8"Les combats l'attendent : attends-la\n"
               u8"avant d'avancer sur la carte.", lines.c_str());
}

std::string depart_offer_text() {
    const std::string h = host_display();
    return fmt(u8"Départ en aventure\n\n"
               u8"%s part à l'aventure !\n"
               u8"Chacun emmène ses chats et dirige les siens.\n\n"
               u8"%s", h.c_str(), team_guest_offer_line().c_str());
}

std::string run_offer_text() {
    const std::string h = host_display();
    if (team_owners_active() && !team_guest_has_cats_in_run())
        return fmt(u8"Aventure en cours\n\n"
                   u8"%s est en aventure !\n"
                   u8"Tes chats ne sont pas du voyage : tu suivras\n"
                   u8"l'aventure sans diriger de chat.", h.c_str());
    return fmt(u8"Aventure en cours\n\n"
               u8"%s est en aventure !\n"
               u8"Rejoins son équipe : ses combats t'attendent.", h.c_str());
}

std::string join_wait_text() {
    const std::string h = host_display();
    return fmt(u8"En route pour l'aventure\n\n"
               u8"L'aventure de %s se chargera\n"
               u8"dès qu'il arrivera sur la carte.", h.c_str());
}

std::string home_offer_text() {
    if (!session_has_others())
        return fmt(u8"Retour à la maison\n\n"
                   u8"La connexion avec %s est coupée.\n"
                   u8"Tu rentres dans ta maison.", host_display().c_str());
    return fmt(u8"Retour à la maison\n\n"
               u8"L'aventure de %s est terminée.\n"
               u8"Tu rentres dans ta maison.", host_display().c_str());
}

// --- hote ---

uint32_t peers_mask_others() {
    const uint8_t self = net_self();
    return roster_mask() & ~(self < 8 ? 1u << self : 0u);
}

uint32_t peer_run_mask() {
    uint32_t m = 0;
    for (uint8_t id = 0; id < 8; ++id) if (g_peer_run[id]) m |= 1u << id;
    return m;
}

void depart_check_go() {
    if (!g_dep_active || g_dep_go) return;     // deja decide : une seule fois
    const uint32_t others = peers_mask_others();
    for (uint8_t id = 0; id < 8; ++id)
        if ((others & (1u << id)) && g_dep_answer[id] != 1) return;
    g_dep_go = true;
    log_line("MULTI", "depart : toute l'equipe vient, l'hote part");
}

void depart_cancel(const char* why) {
    if (!g_dep_active) return;
    g_dep_active = false;
    g_dep_go     = false;
    g_dep_button = nullptr;
    team_vote_reset(why);
    MmMsg m{};
    m.kind = MM_DEPART_REQ;
    m.a = 0;
    m.b = g_dep_seq;
    m.c = kDepartOff_Host;
    net_send_mm(m);
    log_line("MULTI", "depart annule (%s)", why);
}

bool depart_start(void* button) {
    if (g_dep_active) return true;
    // Equipe mixte : chacun emmene ses chats, l'hote pas plus que sa part.
    const int players = (int)net_peer_count();
    const int mine    = team_host_box_count();
    if (mine > team_quota(true, players)) {
        log_line("MULTI", "depart : l'hote a %d chats dans sa boite, %d au plus a %d joueurs",
                 mine, team_quota(true, players), players);
        show_info(team_host_too_many_text(players, mine));
        return true;
    }
    ++g_dep_seq;
    team_vote_begin(g_dep_seq);
    g_dep_active = true;
    g_dep_go     = false;
    g_dep_button = button;
    memset(g_dep_answer, 0, sizeof(g_dep_answer));
    MmMsg m{};
    m.kind = MM_DEPART_REQ;
    m.a = 1;
    m.b = g_dep_seq;
    net_send_mm(m);
    g_dep_wait_wanted = true;
    log_line("MULTI", "depart : l'hote propose de partir, en attente de l'equipe");
    return true;
}

void host_adventure_pump() {
    if (net_role() != NetRole::Host) return;
    if (!session_has_others()) {
        // Plus personne : un vote en cours n'a plus d'objet, l'hote part seul.
        if (g_dep_active) {
            log_line("MULTI", "depart : plus d'autre joueur connecte, l'hote part seul");
            g_dep_go = true;
        }
        g_host_run_sent     = -1;
        memset(g_peer_run, 0, sizeof(g_peer_run));
        g_arrival_mask      = 0;
        g_arrival_dismissed = 0;
        g_arrival_wanted    = false;
        g_run_req_pending   = 0;
    } else {
        // Ou en est l'hote : annonce a chaque changement, et a chaque arrivant.
        const bool in_run = savefile_run_published();
        const uint32_t roster = roster_mask();
        if ((int)in_run != g_host_run_sent || roster != g_host_run_roster) {
            MmMsg m{};
            m.kind = MM_HOST_RUN;
            m.a = in_run ? 1 : 0;
            net_send_mm(m);
            g_host_run_sent = in_run ? 1 : 0;
            if (!in_run) {
                memset(g_peer_run, 0, sizeof(g_peer_run));
                g_arrival_dismissed = 0;
            }
            for (uint8_t id = 0; id < 8; ++id)
                if (!(roster & (1u << id))) g_peer_run[id] = false;
            // Un joueur arrive pendant un vote de depart : la demande pour lui
            // aussi (ceux qui ont deja repondu ne la revoient pas s'ils viennent).
            if (g_dep_active && roster != g_host_run_roster) {
                MmMsg r{};
                r.kind = MM_DEPART_REQ;
                r.a = 1;
                r.b = g_dep_seq;
                net_send_mm(r);
            }
            g_host_run_roster = roster;
        }
        // En aventure, ceux qui n'y sont pas encore : les combats de mgmp les
        // attendent. La fenetre le dit a l'hote, sur la carte, jusqu'a leur
        // arrivee (ou « Ne pas attendre », retenu pour ces joueurs-la).
        g_arrival_mask   = in_run ? (peers_mask_others() & ~peer_run_mask()) : 0;
        g_arrival_wanted = (g_arrival_mask & ~g_arrival_dismissed) != 0;
        if (g_dep_active) depart_check_go();
        // Les demandes de rejoindre gardees pour le retour sur la carte.
        if (g_run_req_pending && !follow_inside_node()) {
            const uint32_t req = g_run_req_pending & roster;
            g_run_req_pending = 0;
            for (uint8_t id = 0; id < 8; ++id)
                if (req & (1u << id)) session_catchup_peer(id);
        }
    }

    if (!g_dep_go) return;
    if (g_live.alive) {
        if (g_live.screen == Screen::DepartWait && !g_click_target) {
            g_silent = g_live.serial;
            popup_press("yes");
        }
        return;
    }
    g_dep_go     = false;
    g_dep_active = false;
    g_dep_wait_wanted = false;
    if (find_scene("House") && sign_kind(g_dep_button) == 2) {
        g_dep_passing  = true;
        g_click_target = g_dep_button;
        g_click_wait   = 0;
        g_click_tries  = 0;
    } else {
        log_line("MULTI", "!! depart : panneau introuvable, clique dessus toi-meme");
        g_dep_passing = true;
    }
    g_dep_button = nullptr;
}

// --- invite ---

void send_simple(MmKind kind, uint8_t a, uint8_t b = 0) {
    MmMsg m{};
    m.kind = kind;
    m.a = a;
    m.b = b;
    net_send_mm(m);
}

// La reponse de cet invite au vote de depart en cours (son numero avec).
void send_depart_answer(bool coming) { send_simple(MM_DEPART_READY, coming ? 1 : 0, g_dep_req_seq); }

void start_join(const char* why) {
    // Une ancienne copie en attente (une aventure qu'il n'avait pas rejointe)
    // serait chargee avant la bonne : on la jette, la nouvelle va arriver.
    savefile_drop_pending();
    savefile_set_join(true);
    g_dep_offer = false;
    g_run_offer = false;
    if (g_night_ready) set_night_ready(false);   // il part : plus pret pour la nuit
    start_menu_trip(why);
}

void stop_menu_trip() {
    if (!g_trip.active) return;
    if (g_trip.key_down) post_escape(false);
    g_trip = MenuTrip{};
    log_line("MULTI", "trajet vers le menu interrompu");
}

void cancel_join(const char* why) {
    if (!savefile_join_armed()) return;
    savefile_set_join(false);
    log_line("MULTI", "aventure : on ne la rejoint plus (%s)", why);
    if (savefile_in_coop()) return;
    // Encore dans une ancienne copie (une nouvelle venait d'arriver, et l'hote
    // repart aussitot) : elle ne sert plus a rien, il rentre aussi.
    if (g_copy_on_screen && !at_main_menu()) {
        savefile_go_home();                  // sans emplacement a lui : il choisira au menu
        start_menu_trip("sortir de l'ancienne copie");
        return;
    }
    // Au menu, ou « Quitter vers le menu » deja appuye : retour dans sa propre
    // save. Encore dans sa maison : il y reste, le trajet s'arrete.
    const bool leaving = at_main_menu() || (g_trip.active && g_trip.press_ok);
    if (leaving && savefile_has_own_slot()) savefile_go_home();
    if (!at_main_menu() && !(g_trip.active && g_trip.press_ok)) stop_menu_trip();
}

void client_adventure_pump() {
    if (net_role() != NetRole::Client) {
        // Plus de session (la connexion est tombee) : le retour a la maison
        // attendu reste propose tant qu'il est dans la copie, et n'a plus d'objet
        // une fois dans une maison qui n'est pas la copie.
        if (g_home_due && (net_role() == NetRole::Host ||
                           (!savefile_in_coop() && find_scene("House") &&
                            !savefile_adventure_is_loaded()))) {
            g_home_due   = false;
            g_home_offer = false;
        }
        return;
    }
    const bool others = session_has_others();

    if (savefile_in_coop())  g_copy_on_screen = true;
    else if (at_main_menu()) g_copy_on_screen = false;

    // Dire a l'hote si ce joueur est dans l'aventure.
    if (others) {
        // Seulement tant que l'hote y est : la copie d'une aventure finie n'est
        // plus son aventure (l'hote compterait sinon un arrivant fantome).
        const int in_run =
            (savefile_in_coop() && savefile_adventure_is_loaded() && g_host_in_run) ? 1 : 0;
        if (in_run != g_peer_run_sent) {
            // Arrive : ce que l'hote a pousse pendant le trajet (chats,
            // historique) a ete garde ; on l'applique tout de suite, hors combat.
            // L'inventaire, lui, attend la premiere case suivie (mgmp_follow).
            if (in_run && g_peer_run_sent != 1 && !lockstep_in_battle()) {
                catsync_apply_pending("arrived in the host's adventure");
                runhist_apply_pending("arrived in the host's adventure");
            }
            send_simple(MM_PEER_RUN, (uint8_t)in_run);
            g_peer_run_sent = in_run;
            log_line("MULTI", "aventure : %s", in_run ? "ce joueur est dans l'aventure de l'hote"
                                                      : "ce joueur n'est pas dans l'aventure de l'hote");
        }
    } else {
        g_peer_run_sent = -1;
        g_host_in_run   = false;
        g_dep_req_live  = false;
        g_dep_offer     = false;
        g_run_offer     = false;
        // La connexion est coupee pendant qu'il partait : il ne la rejoint plus.
        if (savefile_join_armed() && !savefile_in_coop())
            cancel_join("connexion coupee");
    }

    // Sa propre save est en pleine aventure : en multijoueur, les combats de
    // mgmp attendent l'hote, ils ne commenceraient jamais. On le lui dit, une
    // fois par partie chargee.
    static bool said_own_run = false;
    if (others && !savefile_in_coop() && !g_copy_on_screen && !savefile_join_armed() &&
        !g_home_due && !g_trip.active && savefile_adventure_is_loaded() && !at_main_menu()) {
        if (!said_own_run) {
            said_own_run = true;
            log_line("MULTI", "!! la save de ce joueur est en pleine aventure : elle ne peut "
                              "pas se jouer pendant une session");
            show_info(u8"Ta save est en pleine aventure.\n"
                      u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
                      u8"retourne au menu et choisis une autre save,\n"
                      u8"ou finis celle-ci en solo.");
        }
    } else if (!savefile_adventure_is_loaded()) {
        said_own_run = false;
    }

    // L'hote a recharge son aventure (ou en a commence une autre) pendant que ce
    // joueur jouait encore l'ancienne copie : on va charger la nouvelle.
    if (savefile_take_fresh_in_coop()) {
        log_line("MULTI", "aventure : nouvelle copie de l'hote, ce joueur va la charger");
        show_info(fmt(u8"%s a rechargé l'aventure :\ntu la recharges aussi.",
                      host_display().c_str()));
        start_menu_trip("recharger l'aventure de l'hote");
    }

    // L'hote est en aventure et ce joueur n'y est pas (arrive en cours de route,
    // « Plus tard », sorti de la copie...) : le lui proposer, encore et encore --
    // les combats de l'hote l'attendent.
    if (others && g_host_in_run && !savefile_in_coop() && !savefile_join_armed() &&
        !g_dep_offer && !g_run_offer) {
        g_run_offer    = true;
        g_run_offer_at = g_frame;
    }

    // L'aventure de l'hote est finie (abandon, victoire, defaite) ou la connexion
    // est coupee, et ce joueur est dans la copie : il rentre chez lui, et on le
    // lui propose tout de suite -- sans l'hote, la carte et les combats de la
    // copie ne bougent plus (la fenetre s'ouvre meme par-dessus un combat,
    // battle_popups_ok). S'il sort de la copie autrement (mgmp_leave le ramene
    // au menu quand l'hote abandonne, ou il quitte lui-meme), la fenetre
    // l'attend au menu : il n'y reste pas bloque.
    if (savefile_in_coop() && (!others || !g_host_in_run) && !g_home_due &&
        !savefile_home_armed()) {
        g_home_due = true;
        log_line("MULTI", "aventure : celle de l'hote est finie%s, ce joueur doit rentrer chez lui",
                 others ? "" : " (connexion coupee)");
    }
    if (g_home_due) {
        const char* why = nullptr;
        if (savefile_home_armed())
            why = "il est en route";
        else if (savefile_join_armed())
            why = "il rejoint une nouvelle aventure";
        else if (others && g_host_in_run && !savefile_in_coop())
            why = "l'hote est reparti, l'aventure en cours lui est proposee";
        else if (others && g_host_in_run && savefile_in_coop() && savefile_adventure_is_loaded())
            why = "la connexion est revenue, l'aventure continue";
        // Pas de « il est dans sa maison » ici : pendant la sortie de la copie,
        // la maison de la copie peut rester affichee une image apres que la
        // copie ne compte plus, et le retour serait oublie en chemin.
        if (why) {
            g_home_due = false;
            log_line("MULTI", "aventure : plus besoin de le ramener chez lui (%s)", why);
        }
    }
    g_home_offer = g_home_due && !savefile_home_armed();
}

// --- les fenetres de l'aventure, par ordre de priorite ---

bool open_depart_wait() {
    return open_popup(Screen::DepartWait, depart_wait_text(), u8"Annuler", nullptr, true,
                      Action::DepartCancel, Action::DepartCancel);
}

void adventure_popups() {
    if (g_live.alive) return;
    if (g_trip.active && !g_info_wanted) return;     // en route pour le menu
    // Entre deux ecrans (chargement, fondu), rien ou s'afficher : on attend, sans
    // rien ecrire au journal a chaque image. Pendant un combat, seulement chez
    // un invite dont l'hote a quitte l'aventure (battle_popups_ok).
    if (!popup_parent_scene()) return;
    if (find_scene("Battle") && !battle_popups_ok()) return;
    const bool host = net_role() == NetRole::Host;

    if (g_info_wanted) {
        if (open_popup(Screen::Info, g_info_text, u8"D'accord", nullptr, true,
                       Action::None, Action::None))
            g_info_wanted = false;
        return;
    }
    if (host && g_dep_wait_wanted && g_dep_active && !g_dep_go) {
        if (open_depart_wait()) g_dep_wait_wanted = false;
        return;
    }
    if (host && g_arrival_wanted) {
        if (!find_scene("Map")) return;              // sur la carte seulement
        open_popup(Screen::ArrivalWait, arrival_text(), u8"Ne pas attendre", nullptr, true,
                   Action::ArrivalDismiss, Action::ArrivalDismiss);
        return;
    }
    if (host) return;

    if (g_home_offer && g_home_due) {
        if (open_popup(Screen::HomeOffer, home_offer_text(), u8"Rentrer", nullptr, true,
                       Action::GoHome, Action::GoHome))
            log_line("MULTI", "aventure : fenetre « Retour a la maison » ouverte%s",
                     find_scene("Battle") ? " (par-dessus le combat)" : "");
        return;
    }
    if (g_dep_offer && g_dep_req_live && g_frame >= g_dep_offer_at && !savefile_in_coop() &&
        !savefile_join_armed()) {
        open_popup(Screen::DepartOffer, depart_offer_text(), u8"J'arrive", u8"Pas encore", false,
                   Action::DepartYes, Action::DepartNo);
        return;
    }
    if (g_run_offer && g_host_in_run && g_frame >= g_run_offer_at && !savefile_in_coop() &&
        !savefile_join_armed() && !g_dep_offer) {
        open_popup(Screen::RunOffer, run_offer_text(), u8"Rejoindre", u8"Plus tard", false,
                   Action::RunJoin, Action::RunLater);
        return;
    }
    if (savefile_join_armed() && !savefile_in_coop() && !savefile_client_waiting_to_load() &&
        at_main_menu() && !g_trip.active) {
        open_popup(Screen::JoinWait, join_wait_text(), u8"Annuler", nullptr, true,
                   Action::JoinCancel, Action::JoinCancel);
        return;
    }
}

// Les mises a jour des fenetres de l'aventure ouvertes (appele chaque image).
void adventure_live() {
    if (!g_live.alive || g_live.pressed) return;
    auto close_silently = [] {
        if (g_click_target) return;
        g_silent = g_live.serial;
        popup_press("yes");
    };
    switch (g_live.screen) {
        case Screen::DepartWait:
            if (!g_dep_active || g_dep_go) close_silently();
            else popup_set_text(depart_wait_text());
            break;
        case Screen::ArrivalWait:
            if (!session_has_others() || !g_arrival_mask) {
                if (session_has_others()) log_line("MULTI", "aventure : toute l'equipe est arrivee");
                close_silently();
            } else {
                popup_set_text(arrival_text());
            }
            break;
        case Screen::DepartOffer:
            if (!g_dep_req_live || savefile_in_coop()) close_silently();
            break;
        case Screen::RunOffer:
            if (!g_host_in_run || savefile_join_armed() || savefile_in_coop()) close_silently();
            break;
        case Screen::HomeOffer:
            if (!g_home_due) close_silently();       // une nouvelle aventure, ou deja en route
            else popup_set_text(home_offer_text());
            break;
        case Screen::JoinWait:
            if (savefile_client_waiting_to_load() || !savefile_join_armed() || savefile_in_coop()) {
                if (savefile_client_waiting_to_load())
                    log_line("MULTI", "aventure : la save de l'hote est la, elle se charge");
                close_silently();
            }
            break;
        default:
            break;
    }
}

// --- detours -------------------------------------------------------------------

void __fastcall h_MainMenuInit(void* self) {
    o_MainMenuInit(self);
    // Le menu principal se reconstruit : la partie d'avant (la copie de
    // l'aventure de l'hote, peut-etre) n'est plus chargee.
    savefile_left_game();

    void* panel = nullptr;
    if (!mem_read((char*)self + kMainMenu_Panel, &panel, sizeof(panel)) || !panel) {
        log_line("MULTI", "!! menu principal : panneau TitleScreen illisible, bouton non renomme");
        return;
    }
    g_main_panel = panel;
    void* scene = nullptr;
    void* director = nullptr;
    if (mem_read((char*)self + kComp_Scene, &scene, sizeof(scene)) && scene &&
        mem_read((char*)scene + kScene_Director, &director, sizeof(director)) && director) {
        g_director = director;
        if (popup_parent_scene() != scene)
            log_line("MULTI", "!! menu principal : sa scene n'est pas trouvee dans la liste du Director");
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

// La maison refait son panneau de fin de journee (a chaque image) : on passe
// juste apres le jeu. Hors session, house_after_refresh ne fait que noter le
// panneau (quelques lectures).
uint64_t __fastcall h_HouseHud(void* self, void* a2, uint64_t a3, uint64_t a4) {
    const uint64_t r = o_HouseHud(self, a2, a3, a4);
    if (self) house_after_refresh(self);
    return r;
}

// Le tuyau de la maison : un invite en session n'envoie pas encore ses chats
// aux PNJ. Le monde, PNJ compris, est celui de l'hote (mm_world) : la faveur
// gagnee ici serait effacee au chargement suivant de sa save, et le chat, lui,
// serait parti. Le tuyau refuse donc le chat, qui reste dans la maison.
//
// Le jeu demande au tuyau (Container::AddCat 0x1402E88D0, emplacement 31 de
// HousePipe) quand on y lache un chat, mais aussi a chaque image pour chaque
// chat pose dessus (HousePipe, emplacement 10, 0x1402082D0). La fenetre ne
// s'ouvre donc qu'une fois par chat et par passage a la maison, et au plus une
// fois toutes les 3 secondes.
//
// Deux chemins du jeu (un chat qui arrive dans la maison, 0x1401F97F0 ; un
// chat replace, 0x1401E7202) notent le conteneur sous le chat (chat+0xE8)
// AVANT de demander : refuse, le chat se croirait dans un tuyau qui ne le
// liste pas. On remet alors ce champ a zero -- l'etat que le jeu donne lui-meme
// a un chat lache hors de tout conteneur (0x1401FF156).
constexpr size_t   kCat_Container      = 0xE8;   // Container* (ecrit par AddCat)
constexpr size_t   kContainer_CatCount = 0x6C;   // u32
constexpr size_t   kContainer_Cats     = 0x70;   // HouseCat**
constexpr uint32_t kPipeToldMax        = 16;
constexpr uint32_t kPipeGap            = 3 * 60; // images entre deux fenetres

uint32_t g_pipe_said_at  = 0;                    // image de la derniere fenetre
uint32_t g_pipe_refusals = 0;
void*    g_pipe_told[kPipeToldMax] = {};         // chats deja prevenus (ce passage)
uint32_t g_pipe_told_n    = 0;
uint32_t g_pipe_told_next = 0;

bool pipe_told(const void* cat) {
    for (uint32_t i = 0; i < g_pipe_told_n; ++i)
        if (g_pipe_told[i] == cat) return true;
    return false;
}

void pipe_remember(void* cat) {
    g_pipe_told[g_pipe_told_next] = cat;
    g_pipe_told_next = (g_pipe_told_next + 1) % kPipeToldMax;
    if (g_pipe_told_n < kPipeToldMax) ++g_pipe_told_n;
}

// chat+0xE8 == tuyau alors que le tuyau ne le liste pas : on l'efface. S'il est
// vraiment dans la liste (deja dans le tuyau avant la session), on n'y touche pas.
void pipe_unlink(void* pipe, void* cat) {
    void* owner = nullptr;
    if (!mem_read((const char*)cat + kCat_Container, &owner, sizeof(owner)) || owner != pipe)
        return;
    uint32_t n = 0;
    void**   cats = nullptr;
    if (!mem_read((const char*)pipe + kContainer_CatCount, &n, sizeof(n)) ||
        !mem_read((const char*)pipe + kContainer_Cats, &cats, sizeof(cats)) || n > 1024)
        return;
    for (uint32_t i = 0; i < n; ++i) {
        void* c = nullptr;
        if (!mem_read(cats + i, &c, sizeof(c))) return;
        if (c == cat) return;
    }
    void* const none = nullptr;
    if (mem_write((char*)cat + kCat_Container, &none, sizeof(none)))
        log_line("MULTI", "tuyau : le chat refuse n'est plus note comme etant dans le tuyau");
}

bool __fastcall h_PipePlace(void* pipe, void* cat) {
    if (net_role() == NetRole::Client && session_has_others()) {
        ++g_pipe_refusals;
        if (pipe && cat) pipe_unlink(pipe, cat);
        if (cat && !pipe_told(cat) && (!g_pipe_said_at || g_frame - g_pipe_said_at > kPipeGap)) {
            pipe_remember(cat);
            g_pipe_said_at = g_frame ? g_frame : 1;
            log_line("MULTI", "tuyau : envoi d'un chat aux PNJ refuse (invite en session, %u refus)",
                     g_pipe_refusals);
            show_info(fmt(u8"Les PNJ suivent la partie de %s :\n"
                          u8"envoyer tes chats dans le tuyau\n"
                          u8"arrive dans une prochaine version.",
                          host_display().c_str()));
        }
        return false;
    }
    return o_PipePlace(pipe, cat);
}

// Hors de la maison, ses chats n'existent plus : on oublie qui a ete prevenu
// (de nouveaux chats peuvent reprendre les memes adresses au retour).
void pipe_pump() {
    if (g_pipe_told_n && !find_scene("House")) {
        g_pipe_told_n    = 0;
        g_pipe_told_next = 0;
    }
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

    g_live.alive   = true;
    g_live.serial  = p.serial;
    g_live.screen  = p.screen;
    g_live.self    = self;
    g_live.panel   = panel;
    g_live.prompt  = prompt;
    g_live.shown   = std::move(p.text);
    g_live.pressed = false;
    g_live.warned  = false;
}

} // namespace

// --- interface ------------------------------------------------------------------

bool mm_in_house() { return find_scene("House") != nullptr; }

void* mm_house() {
    void* scene = find_scene("House");
    if (!scene || !g_house) return nullptr;
    // Le composant retenu appartient-il a la maison vivante ? (Une maison
    // detruite puis refaite -- retour d'une aventure -- laisse un pointeur
    // perime.) Component+0x20 est sa scene (kComp_Scene) ; +0x18 son entite,
    // dont +0x08 est la scene -- le chemin que suit C_HouseTeamBox lui-meme
    // (0x1400ECD5A : mov rdi, [rcx+18h] ; mov rbx, [rdi+8]). L'un ou l'autre.
    void* owner  = nullptr;
    void* entity = nullptr;
    void* via    = nullptr;
    const bool direct = mem_read((char*)g_house + kComp_Scene, &owner, sizeof(owner)) && owner == scene;
    const bool chain  = mem_read((char*)g_house + kComp_Entity, &entity, sizeof(entity)) && entity &&
                        mem_read((char*)entity + kEntity_Scene, &via, sizeof(via)) && via == scene;
    if (!direct && !chain) return nullptr;
    return g_house;
}

void mm_show_info(const std::string& text_utf8) { show_info(text_utf8); }

bool mm_info_busy() { return g_info_wanted || (g_live.alive && g_live.screen == Screen::Info); }

std::string mm_peer_name(uint8_t id) { return peer_name(id); }

int mm_sign_click_kind() { return g_sign_click_at == g_frame ? g_sign_click : 0; }

void mm_depart_called_off() {
    if (net_role() != NetRole::Host) return;
    MmMsg m{};
    m.kind = MM_DEPART_REQ;
    m.a = 0;
    m.b = g_dep_seq;
    m.c = kDepartOff_Nobody;
    net_send_mm(m);
    log_line("MULTI", "depart : aucun chat ne peut partir, pas d'aventure -- les invites sont liberes (vote %u)",
             (unsigned)g_dep_seq);
}

bool mm_in_game() {
    for (const char* name : { "House", "Map", "Battle", "Event" })
        if (find_scene(name)) return true;
    return false;
}

bool mm_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_MainMenuInit: *detour = (void*)&h_MainMenuInit; *original = (void**)&o_MainMenuInit; return true;
    case T_MeowClick:    *detour = (void*)&h_Meow;         *original = (void**)&o_Meow;         return true;
    case T_ConfirmBuild: *detour = (void*)&h_ConfirmBuild; *original = (void**)&o_ConfirmBuild; return true;
    case T_HouseHud:     *detour = (void*)&h_HouseHud;     *original = (void**)&o_HouseHud;     return true;
    case T_PipePlaceCat: *detour = (void*)&h_PipePlace;    *original = (void**)&o_PipePlace;    return true;
    default: return false;
    }
}

void mm_menu_pump() {
    ++g_frame;
    const Action a = g_action;
    g_action = Action::None;
    switch (a) {
        case Action::None:           break;
        case Action::OpenAuto:       open_auto(); break;
        case Action::StartHost:      start_host(); break;
        case Action::OpenJoin:       prepare_join(); break;
        case Action::DoJoin:         do_join(); break;
        case Action::ClickPlay:
            // Un invite ne choisit sa save qu'une fois le monde de l'hote arrive
            // (sinon elle garderait le sien jusqu'au chargement suivant).
            if (net_role() == NetRole::Client && session_ready() && !world_have_host() &&
                !client_world_wait_over()) {
                log_line("MULTI", "salon : le monde de l'hote n'est pas encore arrive, on attend");
                g_action = Action::OpenAuto;           // le salon se rouvre l'image suivante
                break;
            }
            click_main_play();
            break;
        case Action::StopSession:
            log_line("MULTI", "fin de la session demandee par le menu");
            session_request_disconnect();
            break;
        case Action::OpenBadAddress: break;
        case Action::CancelNight:
            set_night_ready(false);
            g_endday_button = nullptr;
            break;
        case Action::NightReadyNow:
            night_ready_now();
            break;
        case Action::DepartCancel:
            depart_cancel("annule par l'hote");
            break;
        case Action::DepartYes: {
            if (!g_dep_req_live) {                   // annule pendant qu'il cliquait
                show_info(fmt(u8"%s a annulé le départ.", host_display().c_str()));
                break;
            }
            // Equipe mixte : ses chats partent chez l'hote AVANT sa reponse (le
            // meme canal les lui livre dans cet ordre) ; boite vide ou pas de
            // boite : il vient sans chat. Trop de chats dans sa boite :
            // « pas encore », et une fenetre lui dit quoi faire.
            std::string problem;
            if (!team_guest_send(g_dep_req_seq, problem)) {
                send_depart_answer(false);
                log_line("MULTI", "depart : ce joueur ne peut pas encore partir avec ses chats");
                g_dep_offer    = true;
                g_dep_offer_at = g_frame + kOfferAgain;
                show_info(problem);
                break;
            }
            send_depart_answer(true);
            log_line("MULTI", "depart : ce joueur arrive");
            start_join("depart en aventure");
            break;
        }
        case Action::DepartNo:
            send_depart_answer(false);
            team_guest_withdraw(g_dep_req_seq);      // ses chats envoyes plus tot ne partent plus
            log_line("MULTI", "depart : ce joueur n'est pas encore pret");
            g_dep_offer    = true;
            g_dep_offer_at = g_frame + kOfferAgain;
            break;
        case Action::RunJoin:
            if (!g_host_in_run) break;               // finie pendant qu'il cliquait
            // D'abord jeter la copie qui attendrait encore, PUIS demander la
            // nouvelle : start_join vide l'attente, la demande part ensuite.
            start_join("rejoindre l'aventure en cours");
            send_simple(MM_RUN_REQ, 1);
            log_line("MULTI", "aventure : ce joueur rejoint l'aventure en cours");
            break;
        case Action::RunLater:
            g_run_offer_at = g_frame + kOfferAgain;
            break;
        case Action::GoHome:
            g_home_offer = false;
            g_home_due   = false;
            savefile_go_home();
            if (!savefile_home_armed())              // plus de session : il choisira au menu
                log_line("MULTI", "!! retour a la maison : la session est finie, ce joueur "
                                  "choisira sa save au menu");
            start_menu_trip("retour chez soi");      // rien a faire s'il est deja au menu
            break;
        case Action::JoinCancel:
            if (g_dep_req_live) {                    // le vote tient : « pas encore »
                send_depart_answer(false);
                team_guest_withdraw(g_dep_req_seq);
                g_dep_offer    = true;
                g_dep_offer_at = g_frame + kOfferAgain;
            }
            cancel_join("annule par le joueur");
            break;
        case Action::ArrivalDismiss:
            g_arrival_dismissed |= g_arrival_mask;
            log_line("MULTI", "aventure : l'hote n'attend plus son equipe");
            break;
    }

    host_adventure_pump();
    client_adventure_pump();
    team_ui_pump();                 // Mewgenics Multi : fenetres de l'equipe mixte
    trip_pump();
    night_pump();
    if (g_night_wait_wanted && !g_live.alive) {
        g_night_wait_wanted = false;              // une seule tentative par clic
        if (g_night_ready && !g_night_go) open_night_wait();
    }
    adventure_popups();
    notice_pump();
    pipe_pump();

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

    // Filet de securite : la scene de la boite a ete detruite sans que son action
    // soit appelee (une transition qui emporte tout, par exemple). Sans cela la
    // boite resterait « ouverte » pour nous et plus aucune ne s'ouvrirait.
    // Deux images de suite, pour ne jamais trancher au milieu d'une fermeture.
    static int stale = 0;
    if (!live_scene_ok()) {
        if (++stale >= 2) {
            stale = 0;
            log_line("MULTI", "!! la fenetre a disparu sans passer par ses boutons");
            popup_closed(g_live.serial);
            return;
        }
    } else {
        stale = 0;
    }

    adventure_live();
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
                g_silent = g_live.serial;
                if (popup_press("yes"))
                    log_line("MULTI", "la sauvegarde de l'hote est arrivee : fermeture du salon");
            }
            break;
        case Screen::NightWait:
            // Plus pret (annule ailleurs : un depart en aventure, par exemple) :
            // l'attente se ferme sans lancer « Annuler ».
            if (!g_night_ready && !g_night_go) {
                if (!g_live.pressed && !g_click_target) {
                    g_silent = g_live.serial;
                    popup_press("yes");
                }
            } else if (!g_live.pressed) {
                popup_set_text(night_text());
            }
            break;
        case Screen::NightNotice:
            // Plus personne n'attend (il a annule, ou est parti) : l'annonce n'a
            // plus d'objet, elle se ferme sans lancer « Passer la nuit ».
            if (!session_has_others() || g_night_ready || !others_ready_mask()) {
                if (!g_click_target) {
                    g_silent = g_live.serial;
                    if (popup_press("yes"))
                        log_line("MULTI", "nuit : plus personne n'attend, annonce refermee");
                }
            } else if (!g_live.pressed) {
                popup_set_text(notice_text(g_notice_tail));
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
    if (button) trip_on_button(button);
    if (!button || button != g_click_target) return;
    g_click_target = nullptr;
    auto click = call_target<FnButtonClick>(C_ButtonClick);
    if (click) click(button, false);   // force = 0, comme un vrai clic
}

bool mm_menu_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (hwnd) g_hwnd = hwnd;           // pour envoyer Echap (aller au menu)
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

// MSG_HOSTLEFT arrive des que l'hote est sorti de son aventure ; MM_HOST_RUN 0
// suit une ou deux secondes plus tard (mgmp_savefile attend 120 images). Entre
// les deux, mgmp_leave peut deja ramener l'invite au menu (si son menu pause
// est ouvert) : la copie ne compterait plus quand MM_HOST_RUN 0 arriverait, et
// le retour a la maison serait oublie. On le note donc ici, tout de suite.
void mm_on_host_left() {
    if (net_role() != NetRole::Client) return;
    if (g_host_in_run)
        log_line("MULTI", "aventure : %s a quitte son aventure", host_display().c_str());
    g_host_in_run   = false;
    g_run_offer     = false;
    g_peer_run_sent = -1;                    // redire a l'hote ou on en est
    if (savefile_join_armed() && !savefile_in_coop())
        cancel_join("l'aventure de l'hote est finie");
    if (savefile_in_coop() && !g_home_due && !savefile_home_armed()) {
        g_home_due = true;
        log_line("MULTI", "aventure : celle de l'hote est finie, ce joueur doit rentrer chez lui");
    }
}

void mm_on_message(uint8_t from, const MmMsg& m) {
    switch (m.kind) {
        case MM_NIGHT_READY:
            if (net_role() == NetRole::Host && from < 8) {
                g_host_ready[from] = m.a != 0;
                log_line("MULTI", "nuit : %s %s", peer_name(from).c_str(),
                         m.a ? "est pret" : "annule");
                host_broadcast_night();
            }
            break;
        case MM_NIGHT_STATE:
            if (net_role() == NetRole::Client) apply_night_state(m.a != 0, m.x, m.y);
            break;
        case MM_DEPART_REQ:
            if (net_role() != NetRole::Client) break;
            if (m.a) {
                g_dep_req_live = true;
                g_dep_req_seq  = m.b;
                if (!savefile_join_armed()) {
                    g_dep_offer    = true;
                    g_dep_offer_at = 0;
                }
                // Pret pour la nuit, sa fenetre d'attente cacherait la demande :
                // l'hote veut partir, la nuit attendra.
                if (g_night_ready) set_night_ready(false);
                log_line("MULTI", "depart : %s propose de partir a l'aventure", host_display().c_str());
            } else {
                g_dep_req_live = false;
                g_dep_offer    = false;
                team_guest_withdraw(m.b);            // vote annule : ses chats restent chez lui
                const bool nobody = m.c == kDepartOff_Nobody;
                log_line("MULTI", "depart : %s", nobody ? "aucun chat ne peut partir, pas d'aventure"
                                                        : (host_display() + " a annule").c_str());
                if (savefile_join_armed() && !savefile_in_coop()) {
                    cancel_join(nobody ? "aucun chat ne peut partir" : "depart annule par l'hote");
                    show_info(nobody ? fmt(u8"Départ en aventure\n\n"
                                           u8"Aucun chat ne peut partir cette fois :\n"
                                           u8"%s ne part pas à l'aventure.", host_display().c_str())
                                     : fmt(u8"%s a annulé le départ.", host_display().c_str()));
                }
            }
            break;
        case MM_DEPART_READY:
            if (net_role() != NetRole::Host || from >= 8) break;
            if (m.b != g_dep_seq) {                 // la reponse a un vote d'avant
                log_line("MULTI", "depart : reponse de %s a un vote d'avant (%u, celui-ci %u) -- ignoree",
                         peer_name(from).c_str(), (unsigned)m.b, (unsigned)g_dep_seq);
                break;
            }
            if (!m.a) team_forget_peer(from);       // ses chats ne partent plus
            if (g_dep_active) {
                g_dep_answer[from] = m.a ? 1 : 2;
                log_line("MULTI", "depart : %s %s", peer_name(from).c_str(),
                         m.a ? "arrive" : "n'est pas encore pret");
                depart_check_go();
            }
            break;
        case MM_HOST_RUN: {
            if (net_role() != NetRole::Client) break;
            const bool was = g_host_in_run;
            g_host_in_run  = m.a != 0;
            // L'hote renvoie cet etat a chaque arrivee ou depart d'un joueur, et
            // apres avoir recharge sa partie : redire ou on en est, a chaque fois.
            g_peer_run_sent = -1;
            if (g_host_in_run == was) break;     // seuls les changements comptent
            log_line("MULTI", "aventure : %s %s", host_display().c_str(),
                     g_host_in_run ? "est en aventure" : "n'est plus en aventure");
            if (g_host_in_run) {
                g_dep_req_live = false;           // le depart a eu lieu
                g_dep_offer    = false;
                if (!savefile_in_coop() && !savefile_join_armed()) {
                    g_run_offer    = true;
                    g_run_offer_at = 0;
                }
            } else {
                g_run_offer = false;
                if (savefile_join_armed() && !savefile_in_coop())
                    cancel_join("l'aventure de l'hote est finie");
            }
            break;
        }
        case MM_PEER_RUN:
            if (net_role() == NetRole::Host && from < 8) {
                const bool was = g_peer_run[from];
                g_peer_run[from] = m.a != 0;
                if (g_peer_run[from] == was) break;
                log_line("MULTI", "aventure : %s %s", peer_name(from).c_str(),
                         m.a ? "est arrive dans l'aventure" : "n'est plus dans l'aventure");
                if (m.a) g_arrival_dismissed &= ~(1u << from);
            }
            break;
        case MM_RUN_REQ:
            if (net_role() == NetRole::Host && from < 8) {
                log_line("MULTI", "aventure : %s veut rejoindre l'aventure en cours",
                         peer_name(from).c_str());
                g_arrival_dismissed &= ~(1u << from);
                // Dans une case ou l'hote a deja fait un choix (evenement, montee
                // de niveau), la save envoyee serait d'avant ce choix, qui ne lui
                // serait jamais renvoye : on attend le retour sur la carte.
                if (follow_inside_node() && choice_made_in_node()) {
                    g_run_req_pending |= 1u << from;
                    log_line("MULTI", "aventure : il la recevra au retour sur la carte");
                } else {
                    session_catchup_peer(from);
                }
            }
            break;
        default:
            log_line("MULTI", "message MM inconnu (%u) de %u", (unsigned)m.kind, (unsigned)from);
            break;
    }
}

bool mm_on_button_click(void* button) {
    const int kind = sign_kind(button);
    if (!kind) return false;
    // Le panneau clique, pour l'action qu'il declenche pendant ce meme clic
    // (mm_sign_click_kind) : retenu ou pas, c'est ce clic-ci.
    g_sign_click    = kind;
    g_sign_click_at = g_frame;
    if (kind == 1 && g_night_passing) {          // le clic rejoue par night_pump
        g_night_passing = false;
        log_line("MULTI", "nuit : clic sur le panneau rejoue, la journee se termine");
        return false;
    }
    if (kind == 2 && g_dep_passing) {            // le depart, rejoue par l'hote
        g_dep_passing = false;
        log_line("MULTI", "depart : clic sur le panneau rejoue, l'aventure commence");
        return false;
    }
    if (!session_has_others()) return false;     // seul : comme d'habitude
    // Dans la copie de l'aventure de l'hote, la maison n'est pas la sienne : ni
    // nuit ni depart ici, on lui propose de rentrer.
    if (net_role() == NetRole::Client && savefile_in_coop()) {
        if (!g_home_due) log_line("MULTI", "aventure : clic sur le panneau de la copie, retour propose");
        g_home_due = true;
        return true;
    }
    if (kind == 2) {
        // « Défendre ! » : la maison de ce joueur est attaquee. Un combat ou
        // l'autre joueur n'est pas attendrait sa presence pour toujours (mgmp
        // synchronise tous les combats d'une session) : pas encore en multi.
        if (house_boss_defend()) {
            log_line("MULTI", "panneau « Defendre » en session : refuse pour l'instant");
            show_info(u8"Défendre la maison n'est pas encore prévu\n"
                      u8"en multijoueur : quitte la session pour la défendre,\n"
                      u8"puis reconnecte-toi.");
            return true;
        }
        // « À l'aventure ! » : l'hote propose le depart, l'invite ne part pas seul.
        if (net_role() == NetRole::Host) return depart_start(button);
        // L'hote propose deja un depart, ou il est deja parti : apres un « Pas
        // encore » ou un « Plus tard », le clic du joueur le lui repropose tout
        // de suite plutot que d'attendre la relance.
        if (!savefile_join_armed() && g_dep_req_live) {
            log_line("MULTI", "depart : clic d'un invite sur le panneau, le depart de l'hote lui est repropose");
            g_dep_offer    = true;
            g_dep_offer_at = g_frame;
            return true;
        }
        if (!savefile_join_armed() && g_host_in_run) {
            log_line("MULTI", "aventure : clic d'un invite sur le panneau, l'aventure en cours lui est reproposee");
            g_run_offer    = true;
            g_run_offer_at = g_frame;
            return true;
        }
        log_line("MULTI", "depart demande par un invite : c'est l'hote qui lance l'aventure");
        show_info(fmt(u8"C'est %s qui lance l'aventure :\n"
                      u8"demande-lui de cliquer sur « À l'aventure ! ».",
                      host_display().c_str()));
        return true;
    }
    if (g_night_ready || g_live.alive) return true;
    g_endday_button = button;
    set_night_ready(true);
    g_night_wait_wanted = true;                  // l'attente s'ouvre au debut de l'image
    return true;
}

void mm_on_peer_hello(uint8_t peer, const char* name_utf8) {
    if (peer >= 8) return;
    copy_utf8(g_peer_names[peer], sizeof(g_peer_names[peer]), name_utf8 ? name_utf8 : "");
}

} // namespace mgmp
