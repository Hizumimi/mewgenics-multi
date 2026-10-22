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
#include "mgmp_orphan.h"    // OrphanKind (protocole 64)
#include "mgmp_follow.h"    // follow_in_node
#include "mgmp_choice.h"    // choice_made_in_node
#include "mgmp_leave.h"     // leave_will_press_quit
#include "mm_world.h"        // world_have_host : le monde de l'hote est arrive
#include "mm_team.h"
#include "mm_embark.h"         // l'equipe mixte : chacun amene ses chats
#include "mm_vote.h"           // le vote du chemin
#include "mm_rungear.h"        // les objets en aventure
#include "mm_purse.h"          // les bourses de l'aventure
#include "mm_loot.h"           // le defile du butin
#include "mm_npcgift.h"        // les dons des invites aux PNJ
#include "mm_shop.h"           // les achats en boutique avec sa bourse
#include "mm_gift.h"           // les dons entre joueurs (protocole 62)
#include "mgmp_steam.h"        // jouer ensemble par Steam (protocole 59)

#include <winsock2.h>   // avant windows.h : types reseau d'iphlpapi
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <shellapi.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

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
// PauseMenu::TryAbandonRun(PauseMenu*) (0x140296FB0) : le bouton « Abandonner
// l'aventure » du menu pause (aussi « Abandonner le combat » d'une maison
// attaquee, et « Passer le tutoriel ») ouvre la confirmation du jeu, dont le
// « oui » finit l'aventure comme une defaite (MewDirector::AbandonRun : le
// fondu « defeat » puis 0x1403B4750).
using FnTryAbandonRun = void (__fastcall*)(void* pause_menu);

FnMainMenuInit o_MainMenuInit = nullptr;
FnMeow         o_Meow         = nullptr;
FnConfirmBuild o_ConfirmBuild = nullptr;
FnHouseHud     o_HouseHud     = nullptr;
FnPipePlace    o_PipePlace    = nullptr;
FnTryAbandonRun o_TryAbandonRun = nullptr;

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
                    NightNotice, DepartWait, EmbarkWait, ArrivalWait, DepartOffer, RunOffer, JoinWait,
                    HomeOffer, Info, Vote, Loot, QuitRun };

// Les demandes posees par les clics et par le clavier, traitees au debut de
// l'image suivante. Un seul emplacement : il n'y a jamais deux clics a la fois.
enum class Action { None, OpenAuto, StartHost, OpenJoin, DoJoin, ClickPlay, StopSession, OpenBadAddress,
                    CancelNight, NightReadyNow, DepartCancel, DepartYes, DepartNo, RunJoin, RunLater,
                    GoHome, JoinCancel, ArrivalDismiss, EmbarkNoWait, VoteYes, VoteNo, LootYes, LootNo,
                    QuitRun };
Action g_action = Action::None;

// La fenetre du vote du chemin ouverte (mm_vote) : ses boutons, d'apres
// vote_popup. Garde jusqu'a l'action de ses boutons, qui vient apres sa
// fermeture.
uint32_t g_vote_layout = 0;
// De meme pour la fenetre du defile du butin (mm_loot).
uint32_t g_loot_layout = 0;

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

// La derniere confirmation du jeu lui-meme (pas l'une des notres) : ses deux
// boutons, que rien d'autre ne distingue (meme nom, « GenericButton »).
struct GamePrompt {
    void* self = nullptr;           // le composant YesNoPrompt
    void* yes  = nullptr;
    void* no   = nullptr;
};
GamePrompt g_game_prompt;
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
    for (const char* name : { "House", "Map", "MainMenu", "SaveSelectionScreen", "Battle", "Event", "Shop" })
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

// La confirmation du jeu relevee est-elle encore la (sa scene, pas en train
// d'etre detruite) ? Sinon on l'oublie : ses pointeurs ne valent plus rien.
bool game_prompt_alive() {
    if (!g_game_prompt.self) return false;
    void* scene = nullptr;
    uint8_t dying = 1;
    if (mem_read((char*)g_game_prompt.self + kPrompt_Scene, &scene, sizeof(scene)) && scene &&
        mem_read((char*)scene + kScene_Destroying, &dying, 1) && dying == 0)
        return true;
    g_game_prompt = GamePrompt{};
    return false;
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
    // Ce joueur lui-meme (l'hote n'a pas de HELLO de lui-meme) : son nom Steam,
    // pas « Joueur 1 » (journaux, et noms montres aux autres par l'hote).
    if (id == net_self() && mm_local_name()[0]) return display_name(mm_local_name());
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
    // Mewgenics Multi (protocole 59) : par Steam d'abord -- rien a taper.
    if (steam_available())
        where = u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
                u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n\n";
    if (g_host_ip.ip[0])
        where += fmt(u8"%s :\n%s   (%s, port %u)\n%s",
                     steam_available() ? u8"Ou par adresse" : u8"Donne cette adresse à ton ami",
                     g_host_ip.ip, g_host_ip.kind, (unsigned)g_host_port,
                     g_host_copied ? u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n" : "");
    else
        where += fmt(u8"Adresse introuvable sur ce PC (port %u).\n", (unsigned)g_host_port);

    return fmt(u8"Salon multijoueur\n\n%s\nJoueurs (%d/4) :\n%s\n"
               u8"Quand tout le monde est là, choisis ta sauvegarde.",
               where.c_str(), others + 1, players.c_str());
}

std::string join_text() {
    return fmt(u8"Rejoindre une partie\n\n"
               u8"Adresse de l'hôte :\n%s_\n\n"
               u8"Tape l'adresse que ton ami t'a donnée,\n"
               u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
               g_addr_buf.c_str(),
               steam_available() ? u8"\n\nPar Steam, c'est plus simple : dans ta liste\n"
                                   u8"d'amis, « Rejoindre la partie » sur ton ami."
                                 : "");
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
// Mewgenics Multi : « Choisir ta sauvegarde » clique avant le monde de l'hote --
// l'ecran des sauvegardes s'ouvre tout seul quand il arrive (au lieu d'un
// deuxieme clic).
bool     g_play_when_world = false;

bool client_world_wait_over() {
    if (net_role() != NetRole::Client || !session_ready()) { g_client_ready_at = 0; return false; }
    if (!g_client_ready_at) g_client_ready_at = g_frame ? g_frame : 1;
    return g_frame - g_client_ready_at > 30 * 60;
}

std::string client_lobby_text() {
    const std::string target = fmt("%s", g_join_ip.c_str());
    const NetRole  r  = net_role();
    const NetState st = net_state();

    // Mewgenics Multi (protocole 59) : par Steam, l'etat de la connexion Steam.
    std::string steam_why;
    const int steam = steam_client_on() ? steam_client_state(&steam_why) : 0;
    // Mewgenics Multi (protocole 61) : un refus d'abord. L'hote ferme la connexion
    // apres son REFUSE (net_drop_peer, 30 s plus tard ; une partie complete, tout
    // de suite) : « connexion coupee » (ou, par Steam, « connexion Steam coupee »)
    // cacherait alors la raison.
    if (r != NetRole::None && !session_ready()) {
        const char* status = session_status();
        if (status && strstr(status, "refus")) {
            // Mewgenics Multi (protocole 61) : la raison dite au joueur, pas seulement
            // « meme version » (une partie complete n'est pas une affaire de version).
            // Les raisons d'une version d'avant arrivent en anglais : reconnues aussi.
            const char* reason = strchr(status, ':');
            reason = reason ? reason + 1 : status;
            while (*reason == ' ') ++reason;
            const char* what = u8"Vérifiez que vous avez la même version du jeu et du mod.";
            if (strstr(reason, "complète") || strstr(reason, "full"))
                what = u8"La partie de l'hôte est complète (4 joueurs au plus).";
            else if (strstr(reason, "version du mod") || strstr(reason, "protocol"))
                what = u8"Vous n'avez pas la même version du mod :\nprenez tous les deux la dernière.";
            else if (strstr(reason, "version du jeu") || strstr(reason, "Mewgenics build"))
                what = u8"Vous n'avez pas la même version du jeu :\nmettez Mewgenics à jour tous les deux (Steam).";
            else if (strstr(reason, "gpak"))
                what = u8"Les fichiers du jeu ne sont pas les mêmes :\nvérifiez-les dans Steam (Propriétés, Fichiers installés).";
            else if (strstr(reason, "net.control") || strstr(reason, "net_control"))
                what = u8"Le réglage net.control de mgmp.json donne le même chat\naux deux joueurs : remettez-le sur \"auto\".";
            return fmt(u8"Salon multijoueur\n\n"
                       u8"[c:red]La connexion a été refusée.[/c]\n"
                       u8"%s\n(%s)", what, reason);
        }
    }
    if (steam == 3)
        return fmt(u8"Salon multijoueur\n\n"
                   u8"[c:red]Impossible de rejoindre %s :[/c]\n%s.\n"
                   u8"Quitte le salon, puis réessaie (ou par adresse).",
                   target.c_str(), steam_why.c_str());

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
        if (steam == 1)
            return fmt(u8"Salon multijoueur\n\nConnexion à %s par Steam...\n"
                       u8"(quelques secondes)", target.c_str());
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
                   u8"%s",
                   host.c_str(), host.c_str(), me.c_str(), host.c_str(),
                   g_play_when_world ? u8"l'écran des sauvegardes s'ouvrira tout seul."
                                     : u8"tu pourras choisir la tienne.");
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

// Mewgenics Multi (protocole 55) : les AUTRES joueurs qui defendent leur maison en
// ce moment (MM_PEER_DEFEND, defense_pump) -- les listes d'attente le disent (la
// nuit, le depart, l'arrivee dans l'aventure) plutot que « pas encore ».
uint32_t g_defend_mask       = 0;

bool peer_defending(uint8_t id) { return id < 8 && (g_defend_mask & (1u << id)) != 0; }

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
            lines += "  " + who + (ready                       ? u8" : prêt\n"
                                   : !me && peer_defending(id) ? u8" : défend sa maison\n"
                                                               : u8" : pas encore\n");
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
bool     g_dep_embark      = false;   // le vote est passe : l'equipe charge la partie (mm_embark)
bool     g_embark_wait_wanted = false; // ouvrir l'attente de l'equipe
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
// Il a quitte l'aventure de lui-meme (« Quitter l'aventure ») : on ne la lui
// repropose plus tout seul -- un clic sur un panneau de sa maison le fait.
bool     g_run_offer_quiet = false;
// « Abandonner l'aventure » dans le menu pause (PauseMenu::TryAbandonRun) : chez
// un invite dans la copie, notre fenetre « Quitter l'aventure » a la place, a
// l'image suivante (par-dessus le menu pause, toujours ouvert) ; chez l'hote
// d'une aventure ou sont des chats d'invites, la confirmation du jeu qui se
// construit pendant l'appel dit aussi ce que perdent les autres.
bool     g_quit_wanted     = false;
uint32_t g_quit_asked_at   = 0;
bool     g_abandon_note    = false;
bool     g_opening_quit    = false;   // la fenetre « Quitter l'aventure » se construit (par-dessus un combat)
// « Rentrer » choisi : il n'est plus dans l'aventure des maintenant (MM_PEER_RUN
// 0 part tout de suite, l'hote prend ses chats sans attendre son arrivee au
// menu), jusqu'a ce qu'il soit sorti de la copie.
bool     g_quitting        = false;
bool     g_halt_note_due   = false;   // il est rentre parce que sa copie du combat s'est arretee
// Mewgenics Multi (protocole 64) : ... ou parce qu'elle a fini sans le combat de
// l'hote, ou continue apres lui (OrphanKind, mgmp_orphan.h) ; 0 : elle s'est
// arretee pendant qu'il regardait.
uint8_t  g_halt_note_kind  = 0;
// ... ou parce qu'une case de l'hote n'a pas pu etre suivie (follow_refused_member).
constexpr uint8_t kHaltNoteRefused = 3;
bool     g_home_offer      = false;   // proposer de rentrer
// L'aventure de l'hote est finie (abandon, victoire, defaite, connexion coupee)
// et ce joueur n'est pas encore rentre chez lui : vrai dans la copie, et encore
// apres en etre sorti autrement (mgmp_leave le ramene au menu, ou lui-meme).
bool     g_home_due        = false;
// Mewgenics Multi (protocole 61) : ce retour vient d'un depart annule, dans la
// maison de l'hote -- le texte de la fenetre le dit (home_offer_text).
bool     g_home_no_trip    = false;
// La partie a l'ecran est une copie d'une aventure de l'hote, meme quand
// mgmp_savefile ne la compte plus (une nouvelle copie arrive pendant que
// l'ancienne est encore affichee). Faux une fois passe par le menu principal.
bool     g_copy_on_screen  = false;
int      g_peer_run_sent   = -1;      // dernier MM_PEER_RUN envoye
int      g_stable_in       = -1;      // ce joueur dans l'aventure de l'hote, tel que tenu (lockstep : sortie/retour)

// --- un message simple, un seul bouton ---
bool        g_info_wanted = false;
std::string g_info_text;
bool        g_info_yield  = false;   // kInfoYieldToFollow, pour celle qui attend
bool        g_info_live_yield = false;   // ... et pour celle qui est ouverte
std::string g_info_live_text;

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
    // « Quitter l'aventure », demande depuis le menu pause d'un combat : le menu
    // met deja le combat en pause chez ce joueur, cette fenetre-la n'y change rien.
    if (g_opening_quit) {
        bool paused = false;
        if (find_scene("Battle", &paused) && paused) return true;
    }
    if (net_role() == NetRole::Host) return false;
    if (session_has_others() && g_host_in_run) return false;     // le combat continue
    // A rentrer, ou deja en route (le trajet peut avoir a dire « appuie sur
    // Échap » par-dessus le combat).
    return g_home_due || (savefile_in_coop() && savefile_home_armed());
}

constexpr uint32_t kOfferAgain  = 30 * 60;   // 30 s avant de re-proposer
constexpr uint32_t kOfferAgainLong = 5 * 60 * 60;   // « Plus tard » a l'aventure en cours : 5 min

void show_info(const std::string& text, int flags = 0) {
    g_info_text   = text;
    g_info_wanted = true;
    g_info_yield  = (flags & kInfoYieldToFollow) != 0;
}

// Un invite que l'hote emmene dans une case, ou qui n'est pas sur la carte de
// l'aventure : une fenetre arreterait sa carte (kInfoYieldToFollow).
bool guest_follow_busy() {
    return net_role() == NetRole::Client && savefile_in_coop() &&
           (follow_pending_count() != 0 || follow_node_live() || !follow_on_map());
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
            const uint8_t a = g_dep_answer[ids[i]];
            lines += "  " + peer_name(ids[i]) + " : " +
                     (a != 1 && peer_defending(ids[i]) ? u8"défend sa maison" : dep_answer_text(a)) + "\n";
        }
    return fmt(u8"Départ en aventure\n\n"
               u8"Tu veux partir à l'aventure.\n"
               u8"Ton équipe répond :\n\n%s", lines.c_str());
}

std::string arrival_text() {
    std::string lines;
    for (uint8_t id = 0; id < 8; ++id)
        if (g_arrival_mask & (1u << id))
            lines += "  " + peer_name(id) + (peer_defending(id) ? u8" : défend sa maison\n"
                                                                : u8" : pas encore là\n");
    return fmt(u8"En attente de ton équipe\n\n%s\n"
               u8"Si tu avances sans l'attendre, tu diriges\n"
               u8"ses chats dans les combats jusqu'à son arrivée.", lines.c_str());
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
    if (team_guest_has_cats_in_run())
        return fmt(u8"Aventure en cours\n\n"
                   u8"%s est en aventure avec tes chats !\n"
                   u8"Tant que tu n'es pas là, il les dirige.\n"
                   u8"Rejoins son équipe ?", h.c_str());
    return fmt(u8"Aventure en cours\n\n"
               u8"%s est en aventure !\n"
               u8"Rejoins son équipe ?", h.c_str());
}

// « Abandonner l'aventure » dans la copie de l'hote : ce n'est pas la sienne,
// il la quitte (ses chats restent dans l'equipe de l'hote).
std::string quit_run_text() {
    const std::string h = host_display();
    if (!session_has_others() || !g_host_in_run)
        return fmt(u8"Quitter l'aventure\n\n"
                   u8"L'aventure de %s est terminée :\n"
                   u8"tu rentres dans ta maison.", h.c_str());
    if (!team_guest_has_cats_in_run())
        return fmt(u8"Quitter l'aventure\n\n"
                   u8"C'est l'aventure de %s : seul lui peut\n"
                   u8"l'abandonner. Elle continue sans toi.\n\n"
                   u8"Tu rentres dans ta maison ? Pour revenir,\n"
                   u8"clique sur un panneau de ta maison.", h.c_str());
    return fmt(u8"Quitter l'aventure\n\n"
               u8"C'est l'aventure de %s : seul lui peut\n"
               u8"l'abandonner. Elle continue sans toi, et tes\n"
               u8"chats restent dans son équipe : il les dirige,\n"
               u8"puis ils rentrent chez toi à la fin.\n\n"
               u8"Tu rentres dans ta maison ? Pour revenir,\n"
               u8"clique sur un panneau de ta maison.", h.c_str());
}

std::string join_wait_text() {
    const std::string h = host_display();
    return fmt(u8"En route pour l'aventure\n\n"
               u8"La partie de %s va se charger :\n"
               u8"tu partiras avec lui.", h.c_str());
}

std::string home_offer_text() {
    if (!session_has_others())
        return fmt(u8"Retour à la maison\n\n"
                   u8"La connexion avec %s est coupée.\n"
                   u8"Tu rentres dans ta maison.", host_display().c_str());
    // Mewgenics Multi : dans la maison de l'hote apres un depart annule, pas
    // d'aventure terminee.
    if (g_home_no_trip && savefile_in_coop() && !savefile_adventure_is_loaded())
        return fmt(u8"Retour à la maison\n\n"
                   u8"Le départ avec %s n'a pas eu lieu.\n"
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

void depart_cancel(const char* why);     // juste en dessous

void depart_check_go() {
    if (!g_dep_active || g_dep_go || g_dep_embark) return;   // deja decide : une seule fois
    const uint32_t others = peers_mask_others();
    for (uint8_t id = 0; id < 8; ++id)
        if ((others & (1u << id)) && g_dep_answer[id] != 1) return;
    // Mewgenics Multi (protocole 58) : la part de l'hote a pu baisser pendant le
    // vote (un 4e joueur arrive, la 4e place passe a un autre) : le jeu ferait
    // partir toute sa boite.
    {
        const int players = (int)net_peer_count();
        const int mine    = team_host_box_count();
        if (mine > team_quota_for(kHostPeer, players)) {
            log_line("MULTI", "depart : l'hote a %d chats dans sa boite, %d au plus a %d joueurs -- vote annule",
                     mine, team_quota_for(kHostPeer, players), players);
            depart_cancel("trop de chats dans la boite de l'hote");
            show_info(team_host_too_many_text(players, mine));
            return;
        }
    }
    // Depart ensemble : l'equipe charge d'abord la partie de l'hote, pour voir
    // avec lui le choix des colliers et des objets (mm_embark).
    uint32_t joiners = 0;
    for (uint8_t id = 0; id < 8; ++id)
        if ((others & (1u << id)) && g_dep_answer[id] == 1) joiners |= 1u << id;
    if (embark_begin(g_dep_seq, joiners)) {
        g_dep_embark         = true;
        g_embark_wait_wanted = true;
        log_line("MULTI", "depart : toute l'equipe vient, elle charge la partie de l'hote avant le depart");
        return;
    }
    g_dep_go = true;
    log_line("MULTI", "depart : toute l'equipe vient, l'hote part");
}

void depart_cancel(const char* why) {
    if (!g_dep_active) return;
    g_dep_active = false;
    g_dep_go     = false;
    g_dep_embark = false;
    g_dep_button = nullptr;
    embark_cancel(why);
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
    // Mewgenics Multi (protocole 58) : sa boite relue tout de suite -- un 2e chat
    // pose juste avant ce clic prend la 4e place maintenant, pas dans 500 ms.
    team_slot_refresh();
    const int mine    = team_host_box_count();
    if (mine > team_quota_for(kHostPeer, players)) {
        log_line("MULTI", "depart : l'hote a %d chats dans sa boite, %d au plus a %d joueurs",
                 mine, team_quota_for(kHostPeer, players), players);
        show_info(team_host_too_many_text(players, mine));
        return true;
    }
    // 0 est le numero des cadeaux des PNJ (kNpcGiftSeq) : jamais celui d'un vote,
    // meme apres 255 departs (relecture de la build 29).
    if (++g_dep_seq == 0) g_dep_seq = 1;
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
        if (g_dep_active && !g_dep_go) {
            log_line("MULTI", "depart : plus d'autre joueur connecte, l'hote part seul");
            g_dep_go     = true;
            g_dep_embark = false;
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
        if (g_dep_embark && embark_should_go()) {
            g_dep_embark = false;
            g_dep_go     = true;
        }
        // Les demandes de rejoindre gardees pour le retour sur la carte.
        if (g_run_req_pending && !follow_in_node()) {
            const uint32_t req = g_run_req_pending & roster;
            g_run_req_pending = 0;
            for (uint8_t id = 0; id < 8; ++id)
                if (req & (1u << id)) session_catchup_peer(id);
        }
    }

    if (!g_dep_go) return;
    if (g_live.alive) {
        if ((g_live.screen == Screen::DepartWait || g_live.screen == Screen::EmbarkWait) && !g_click_target) {
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
    lockstep_rejoin_run();                   // la reprise de l'hote va arriver : elle compte
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
    if (!g_home_due) g_home_no_trip = false;
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
        if (g_quitting && !savefile_in_coop()) g_quitting = false;   // sorti de la copie
        const int in_run =
            (savefile_in_coop() && savefile_adventure_is_loaded() && g_host_in_run && !g_quitting) ? 1 : 0;
        // Une sortie qui n'est pas voulue (pas « Quitter l'aventure ») doit tenir
        // une demi-seconde : une image a 0 en plein combat effacerait l'etat de ce
        // combat ici (lockstep_leave_run) alors qu'il continue.
        // `stable_in` est la derniere valeur tenue : g_peer_run_sent, lui, est
        // remis a -1 a chaque nouvelle de l'hote (pour la lui redire), ce qui ne
        // doit pas sauter cette attente.
        static uint32_t zero_frames = 0;
        zero_frames = in_run ? 0 : zero_frames + 1;
        const bool settled = in_run || g_quitting || g_stable_in != 1 || zero_frames >= 30;
        if (settled && in_run != g_stable_in) {
            // Les combats de l'hote : plus les siens en sortant (son etat du combat
            // en cours est oublie, il le reprendrait du debut), de nouveau en
            // arrivant. Pas en route pour la rejoindre (sauf en sortant d'une
            // aventure ou il etait) : la reprise de l'hote arrive pendant le trajet.
            if (in_run) lockstep_rejoin_run();
            else if (g_stable_in == 1 || !savefile_join_armed()) lockstep_leave_run();
            g_stable_in = in_run;
        }
        if (in_run != g_peer_run_sent && settled) {
            // Arrive : ce que l'hote a pousse pendant le trajet (chats,
            // historique) a ete garde ; on l'applique tout de suite, hors combat.
            // L'inventaire, lui, attend la premiere case suivie (mgmp_follow).
            // Mewgenics Multi (protocole 61) : pas pendant que la copie de l'hote est
            // mise de cote (le depart sans second chargement : mm_embark n'applique
            // que les poussees d'avant elle), ni sac ouvert (les boites de l'ecran
            // tiennent des objets que la poussee remplacerait) -- mm_rungear et
            // mgmp_follow les appliqueront.
            if (in_run && g_peer_run_sent != 1 && !lockstep_in_battle() && !savefile_keep_waiting() &&
                !rungear_hold_pushes()) {
                catsync_apply_pending("arrived in the host's adventure");
                runhist_apply_pending("arrived in the host's adventure");
            }
            send_simple(MM_PEER_RUN, (uint8_t)in_run);
            g_peer_run_sent = in_run;
            log_line("MULTI", "aventure : %s", in_run ? "ce joueur est dans l'aventure de l'hote"
                                                      : "ce joueur n'est pas dans l'aventure de l'hote");
        }
        // Sa copie du combat s'est arretee seule (desynchronisee alors qu'il ne
        // faisait que le regarder) : le combat des autres continue, le sien est
        // fige pour toujours. Il rentre chez lui comme par « Quitter
        // l'aventure » (ses chats restent dans l'equipe) ; un panneau de sa
        // maison le ramene, et la reprise de l'hote refait le combat.
        // Mewgenics Multi (protocole 64) : sa copie du combat et celle de l'hote ne
        // se sont pas finies ensemble (mgmp_orphan.h) -- l'hote avait fini et pas
        // lui (personne ne joue plus son combat), ou l'inverse. Meme sortie : une
        // copie qui a diverge ne se rattrape pas, la save de l'hote la remplacera.
        // Mewgenics Multi : ou une case de l'hote dont il est membre n'a pas pu
        // etre suivie ici (follow_refused_member) -- l'hote l'attendrait a la
        // barriere du combat pour toujours. Meme sortie.
        const uint8_t orphan = lockstep_orphan();
        const bool can_leave = savefile_in_coop() && !g_quitting && !g_trip.active;
        const bool halted    = lockstep_halted_watching();
        const bool refused   = can_leave && follow_refused_member();
        if ((halted || orphan || refused) && can_leave) {
            log_line("MULTI", "aventure : la copie du combat de ce joueur s'est desynchronisee (%s) -- "
                              "il quitte l'aventure, il pourra la rejoindre",
                     orphan == kOrphanHostDone ? "le combat de l'hote etait fini, pas le sien"
                     : orphan == kOrphanMineDone ? "son combat etait fini, pas celui de l'hote"
                     : halted                    ? "il regardait"
                                                 : "une case de l'hote n'a pas pu etre suivie ici");
            g_quitting        = true;
            g_run_offer_quiet = true;
            g_run_offer       = false;
            g_home_offer      = false;
            g_home_due        = false;
            g_halt_note_due   = true;
            g_halt_note_kind  = (uint8_t)(orphan ? orphan : halted ? 0 : kHaltNoteRefused);
            savefile_go_home();
            start_menu_trip("copie du combat desynchronisee");
        }
        if (g_halt_note_due && !savefile_in_coop() && !g_trip.active && mm_house()) {
            g_halt_note_due = false;
            if (g_halt_note_kind == kOrphanHostDone)
                show_info(fmt(u8"Retour à la maison\n\n"
                              u8"Le combat de %s était fini, mais ta copie\n"
                              u8"continuait : elle s'était désynchronisée,\n"
                              u8"plus personne ne jouait ce combat. Tu es\n"
                              u8"rentré ; tes chats restent dans son équipe.\n\n"
                              u8"Pour revenir dans l'aventure, clique sur\n"
                              u8"un panneau de ta maison.", host_display().c_str()));
            else if (g_halt_note_kind == kOrphanMineDone)
                show_info(fmt(u8"Retour à la maison\n\n"
                              u8"Ton combat s'est fini, mais celui de %s\n"
                              u8"continuait : ta copie s'était\n"
                              u8"désynchronisée. Tu es rentré ; tes chats\n"
                              u8"restent dans son équipe, l'IA du jeu les\n"
                              u8"joue jusqu'à la fin du combat.\n\n"
                              u8"Pour revenir dans l'aventure, clique sur\n"
                              u8"un panneau de ta maison.", host_display().c_str()));
            else if (g_halt_note_kind == kHaltNoteRefused)
                show_info(fmt(u8"Retour à la maison\n\n"
                              u8"%s est entré dans une case que ton jeu\n"
                              u8"n'a pas pu suivre : ta copie de l'aventure\n"
                              u8"n'était plus la même. Tu es rentré ; tes\n"
                              u8"chats restent dans son équipe, il les dirige.\n\n"
                              u8"Pour revenir dans l'aventure, clique sur\n"
                              u8"un panneau de ta maison.", host_display().c_str()));
            else
                show_info(fmt(u8"Retour à la maison\n\n"
                              u8"Ta copie du combat s'est désynchronisée de\n"
                              u8"celle de %s : le combat continuait sans toi,\n"
                              u8"alors tu es rentré. Tes chats restent dans\n"
                              u8"son équipe, il les dirige.\n\n"
                              u8"Pour revenir dans l'aventure, clique sur\n"
                              u8"un panneau de ta maison.", host_display().c_str()));
            g_halt_note_kind = 0;
        }
    } else {
        g_peer_run_sent = -1;
        g_stable_in     = -1;
        g_host_in_run   = false;
        g_dep_req_live  = false;
        g_dep_offer     = false;
        g_run_offer     = false;
        g_run_offer_quiet = false;
        // La connexion est coupee pendant qu'il partait : il ne la rejoint plus.
        if (savefile_join_armed() && !savefile_in_coop())
            cancel_join("connexion coupee");
    }

    // Sa propre save est en pleine aventure : en multijoueur, les combats de
    // mgmp attendent l'hote, ils ne commenceraient jamais. On le lui dit, une
    // fois par partie chargee.
    //
    // Pas pour la defense de sa maison (protocole 54) : un combat a lui seul, que
    // la session laisse au jeu. Le combat lui-meme ne compte pas comme une
    // aventure chargee (mgmp_savefile) ; avant lui, pendant le fondu qui suit le
    // clic sur « Défendre ! », la liste des chats de la course est deja remplie et
    // le boss attaque encore -- la defense est en route.
    static bool said_own_run = false;
    if (others && !savefile_in_coop() && !g_copy_on_screen && !savefile_join_armed() &&
        !g_home_due && !g_trip.active && savefile_adventure_is_loaded() && !at_main_menu() &&
        !team_house_defense_pending()) {
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

    // Il devait rentrer chez lui (depart annule pendant qu'il chargeait la
    // partie de l'hote) et s'y retrouve quand meme : on l'emmene au menu, d'ou
    // il rentre dans sa save.
    if (savefile_in_coop() && savefile_home_armed() && !g_trip.active && !at_main_menu())
        start_menu_trip("rentrer chez soi depuis la copie");

    // L'hote a recharge son aventure (ou en a commence une autre) pendant que ce
    // joueur jouait encore l'ancienne copie : on va charger la nouvelle.
    if (savefile_take_fresh_in_coop()) {
        // Mewgenics Multi : juste apres un depart ensemble, ce n'est pas l'hote
        // qui a recharge -- sa copie du depart remplace le miroir de ce joueur.
        if (embark_mirror_recent()) {
            log_line("MULTI", "aventure : la copie du depart de l'hote, ce joueur va la charger");
            show_info(fmt(u8"Départ en aventure\n\nTon jeu charge l'aventure de %s.",
                          host_display().c_str()));
        } else {
            log_line("MULTI", "aventure : nouvelle copie de l'hote, ce joueur va la charger");
            show_info(fmt(u8"%s a rechargé l'aventure :\ntu la recharges aussi.",
                          host_display().c_str()));
        }
        start_menu_trip("recharger l'aventure de l'hote");
    }

    // L'hote est en aventure et ce joueur n'y est pas (arrive en cours de route,
    // « Plus tard », sorti de la copie...) : le lui proposer -- sauf s'il l'a
    // quittee de lui-meme (un panneau de sa maison la lui repropose).
    if (others && g_host_in_run && !savefile_in_coop() && !savefile_join_armed() &&
        !g_dep_offer && !g_run_offer && !g_run_offer_quiet) {
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
        !savefile_home_armed() && !embark_guest_active()) {
        g_home_due     = true;
        g_home_no_trip = false;
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
    // « Abandonner l'aventure » dans la copie : notre fenetre a la place de celle
    // du jeu, par-dessus le menu pause encore ouvert (meme en plein combat : le
    // menu le met deja en pause). La demande passe avant tout le reste et ne vaut
    // que quelques instants (le menu ferme entre-temps : le joueur recliquera).
    if (g_quit_wanted) {
        if (!savefile_in_coop() || g_frame - g_quit_asked_at > 120 || g_trip.active) {
            g_quit_wanted = false;
        } else if (popup_parent_scene()) {
            g_opening_quit = true;
            const bool open = open_popup(Screen::QuitRun, quit_run_text(), u8"Rentrer", u8"Rester", false,
                                         Action::QuitRun, Action::None);
            g_opening_quit = false;
            if (open) {
                g_quit_wanted = false;
                log_line("MULTI", "aventure : fenetre « Quitter l'aventure » ouverte%s",
                         find_scene("Battle") ? " (en plein combat)" : "");
            }
        }
        return;
    }
    if (g_trip.active && !g_info_wanted) return;     // en route pour le menu
    // Entre deux ecrans (chargement, fondu), rien ou s'afficher : on attend, sans
    // rien ecrire au journal a chaque image. Pendant un combat, seulement chez
    // un invite dont l'hote a quitte l'aventure (battle_popups_ok).
    if (!popup_parent_scene()) return;
    if (find_scene("Battle") && !battle_popups_ok()) return;
    const bool host = net_role() == NetRole::Host;

    if (g_info_wanted && !(g_info_yield && guest_follow_busy())) {
        if (open_popup(Screen::Info, g_info_text, u8"D'accord", nullptr, true,
                       Action::None, Action::None)) {
            g_info_wanted     = false;
            g_info_live_yield = g_info_yield;
            g_info_live_text  = g_info_text;
        }
        return;
    }
    // Le vote du chemin, sur la carte (hote et invites).
    {
        VotePopup vp;
        if (vote_popup(vp)) {
            if (open_popup(Screen::Vote, vp.text, vp.yes, vp.no, vp.no == nullptr,
                           Action::VoteYes, Action::VoteNo))
                g_vote_layout = vp.layout;
            return;
        }
    }
    // Le partage du butin, a la fin d'une aventure mixte gagnee (hote et invites).
    {
        LootPopup lp;
        if (loot_popup(lp)) {
            if (open_popup(Screen::Loot, lp.text, lp.yes, lp.no, lp.no == nullptr,
                           Action::LootYes, Action::LootNo))
                g_loot_layout = lp.layout;
            return;
        }
    }
    if (host && g_dep_wait_wanted && g_dep_active && !g_dep_go && !g_dep_embark) {
        if (open_depart_wait()) g_dep_wait_wanted = false;
        return;
    }
    if (host && g_embark_wait_wanted && g_dep_embark && !g_dep_go) {
        if (open_popup(Screen::EmbarkWait, embark_wait_text(), u8"Ne pas attendre", nullptr, true,
                       Action::EmbarkNoWait, Action::EmbarkNoWait))
            g_embark_wait_wanted = false;
        return;
    }
    if (host && g_arrival_wanted) {
        if (!find_scene("Map")) return;              // sur la carte seulement
        open_popup(Screen::ArrivalWait, arrival_text(), u8"Ne pas attendre", nullptr, true,
                   Action::ArrivalDismiss, Action::ArrivalDismiss);
        return;
    }
    if (host) return;

    // Le defile du butin d'abord : rentrer pendant qu'il passe ferait manquer
    // les objets (le chargement de sa save), et il ne dure que quelques minutes.
    if (g_home_offer && g_home_due && !loot_holds_home()) {
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
            if (!g_dep_active || g_dep_go || g_dep_embark) close_silently();
            else popup_set_text(depart_wait_text());
            break;
        case Screen::EmbarkWait:
            if (!g_dep_embark || g_dep_go) close_silently();
            else popup_set_text(embark_wait_text());
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
            // Une nouvelle aventure, deja en route, ou le defile du butin arrive.
            if (!g_home_due || loot_holds_home()) close_silently();
            else popup_set_text(home_offer_text());
            break;
        case Screen::Loot: {
            // Comme le vote : d'autres boutons, on la referme et la suivante s'ouvre.
            LootPopup lp;
            if (!loot_popup(lp) || lp.layout != g_loot_layout) close_silently();
            else popup_set_text(lp.text);
            break;
        }
        case Screen::Vote: {
            // Ses boutons ne changent pas une fois ouverte : d'autres boutons (un
            // vote fait, le resultat), on la referme et la suivante s'ouvre.
            VotePopup vp;
            if (!vote_popup(vp, g_vote_layout) || vp.layout != g_vote_layout) close_silently();
            else popup_set_text(vp.text);
            break;
        }
        case Screen::Info:
            // kInfoYieldToFollow : l'hote est entre dans une case, la fenetre se
            // referme pour que la carte de ce joueur le suive ; elle reviendra.
            if (g_info_live_yield && net_role() == NetRole::Client && savefile_in_coop() &&
                follow_pending_count() != 0) {
                if (!g_info_wanted) {
                    g_info_text   = g_info_live_text;
                    g_info_wanted = true;
                    g_info_yield  = true;
                }
                g_info_live_yield = false;
                close_silently();
            }
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

// Le tuyau de la maison. Un invite en session, dans SA maison, donne ses chats
// aux PNJ comme seul : la faveur part chez l'hote, dont le monde est celui de
// tous (mm_npcgift ; sans lui, elle serait effacee au chargement suivant de sa
// save, et le chat, lui, serait parti). Ailleurs -- la maison de la copie d'une
// aventure de l'hote, ou sans les crochets des dons -- le tuyau refuse le chat,
// qui reste dans la maison.
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
        if (npcgift_guest_donates()) return o_PipePlace(pipe, cat);
        ++g_pipe_refusals;
        if (pipe && cat) pipe_unlink(pipe, cat);
        if (cat && !pipe_told(cat) && (!g_pipe_said_at || g_frame - g_pipe_said_at > kPipeGap)) {
            pipe_remember(cat);
            g_pipe_said_at = g_frame ? g_frame : 1;
            log_line("MULTI", "tuyau : envoi d'un chat aux PNJ refuse (invite en session, %s, %u refus)",
                     savefile_in_coop() ? "maison de la copie de l'hote" : "dons aux PNJ indisponibles",
                     g_pipe_refusals);
            show_info(savefile_in_coop()
                          ? fmt(u8"Ce tuyau est celui de la maison de %s :\n"
                                u8"envoie tes chats aux PNJ depuis\n"
                                u8"ta propre maison.", host_display().c_str())
                          : fmt(u8"Les PNJ suivent la partie de %s :\n"
                                u8"envoyer tes chats dans le tuyau n'est\n"
                                u8"pas possible avec cette version du jeu.",
                                host_display().c_str()));
        }
        return false;
    }
    return o_PipePlace(pipe, cat);
}

// « Abandonner l'aventure » (PauseMenu::TryAbandonRun).
void __fastcall h_TryAbandonRun(void* pause_menu) {
    // Un invite dans la copie de l'aventure de l'hote : ce n'est pas la sienne.
    // L'abandon du jeu y serait une defaite dans sa copie seulement -- ses chats
    // « pas revenus » dans sa propre save (journal du 17 octobre), pendant que
    // l'aventure de l'hote continue avec eux. On lui propose de la quitter.
    if (savefile_in_coop()) {               // un invite, ou un ancien invite dont la connexion vient de tomber
        g_quit_wanted   = true;
        g_quit_asked_at = g_frame;
        log_line("MULTI", "aventure : « Abandonner » dans la copie de l'hote -- remplace par "
                          "« Quitter l'aventure »");
        return;
    }
    // L'hote d'une aventure ou sont des chats d'invites : c'est une defaite pour
    // toute l'equipe, et sa confirmation le dit (h_ConfirmBuild, pendant l'appel).
    g_abandon_note = net_role() == NetRole::Host && session_has_others() &&
                     !team_run_guest_names().empty();
    o_TryAbandonRun(pause_menu);
    g_abandon_note = false;
}

// Une std::wstring du jeu, lue (tampon interne ou sur le tas).
bool read_game_wstring(const GameWString* s, std::wstring& out) {
    GameWString w{};
    if (!s || !mem_read(s, &w, sizeof(w)) || w.size > 4096) return false;
    out.assign(w.size, L'\0');
    if (w.cap > 7) {
        if (!w.ptr || (w.size && !mem_read(w.ptr, &out[0], w.size * sizeof(wchar_t)))) return false;
    } else {
        if (w.size > 7) return false;
        if (w.size) memcpy(&out[0], w.buf, w.size * sizeof(wchar_t));
    }
    return true;
}

// La confirmation de l'abandon chez l'hote d'une aventure a plusieurs : son texte
// (deja traduit : une std::wstring passee par valeur au constructeur, que celui-ci
// detruira) avec notre ligne en dessous.
void add_abandon_note(GameWString* text) {
    const std::string names = team_run_guest_names();
    auto move = call_target<FnWStrMove>(C_WStrMoveAssign);
    if (names.empty() || !move) return;
    std::wstring w;
    if (!read_game_wstring(text, w)) {
        log_line("MULTI", "!! abandon de l'aventure : texte de la confirmation illisible, laisse tel quel");
        return;
    }
    w += utf8_to_wide(fmt(u8"\n\nC'est une défaite pour toute l'équipe :\n"
                          u8"les chats de %s ne rentreront pas non plus.", names.c_str()).c_str());
    GameWString tmp;
    if (!make_game_wstring(tmp, w)) return;
    move(text, &tmp);
    log_line("MULTI", "abandon de l'aventure : la confirmation dit aussi que les chats de %s ne rentreront "
                      "pas", names.c_str());
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
        // La confirmation d'« Abandonner l'aventure », chez l'hote d'une aventure
        // a plusieurs (h_TryAbandonRun) : une ligne de plus.
        if (g_abandon_note && text) {
            g_abandon_note = false;          // cette boite-ci seulement
            add_abandon_note((GameWString*)text);
        }
        o_ConfirmBuild(self, a2, text, a4, a5, ok_only, a7, a8);
        // Une confirmation du jeu : ses boutons, pour le depart ensemble.
        g_game_prompt = GamePrompt{};
        void* panel = nullptr;
        if (mem_read((char*)self + kPrompt_Panel, &panel, sizeof(panel)) && panel) {
            g_game_prompt.self = self;
            g_game_prompt.yes  = panel_button(panel, "yes");
            g_game_prompt.no   = panel_button(panel, "no");
        }
        return;
    }
    Pending p = std::move(g_pending);
    g_pending = Pending{};              // une seule fenetre par demande
    // Une de nos fenetres a l'adresse d'une ancienne confirmation du jeu : ses
    // boutons ne sont pas ceux du jeu.
    if (self == g_game_prompt.self) g_game_prompt = GamePrompt{};

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

bool mm_scene_alive(const char* name) { return name && find_scene(name) != nullptr; }

bool mm_scene_running(const char* name) {
    bool paused = true;
    return name && find_scene(name, &paused) != nullptr && !paused;
}

std::string mm_scene_list() {
    std::string out;
    if (!g_director) return out;
    void** begin = nullptr;
    void** end   = nullptr;
    if (!mem_read((char*)g_director + kDirector_ScenesBegin, &begin, sizeof(begin)) ||
        !mem_read((char*)g_director + kDirector_ScenesEnd,   &end,   sizeof(end)) ||
        !begin || end < begin || end - begin > 256)
        return out;
    for (void** it = begin; it < end; ++it) {
        void* sc = nullptr;
        if (!mem_read(it, &sc, sizeof(sc)) || !sc) continue;
        uint8_t dying = 1;
        if (!mem_read((char*)sc + kScene_Destroying, &dying, 1) || dying) continue;
        char name[48] = {};
        if (!mem_read_std_string((char*)sc + kScene_Name, name, sizeof(name))) continue;
        if (!out.empty()) out += ", ";
        out += name;
    }
    return out;
}

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

void mm_show_info(const std::string& text_utf8, int flags) { show_info(text_utf8, flags); }

bool mm_info_busy() { return g_info_wanted || (g_live.alive && g_live.screen == Screen::Info); }

int mm_game_prompt_button(void* button) {
    if (!button || !game_prompt_alive()) return 0;
    if (button == g_game_prompt.yes) return 1;
    if (button == g_game_prompt.no)  return 2;
    return 0;
}

bool mm_game_prompt_open() { return game_prompt_alive(); }

bool mm_vote_popup_open() { return g_live.alive && g_live.screen == Screen::Vote; }

bool mm_loot_popup_open() { return g_live.alive && g_live.screen == Screen::Loot; }

bool mm_other_popup_open() {
    return (g_live.alive && g_live.screen != Screen::Loot) || g_info_wanted || game_prompt_alive();
}

uint32_t mm_run_peers_mask() {
    if (net_role() != NetRole::Host || !session_has_others()) return 0;
    return peer_run_mask() & peers_mask_others();
}

bool mm_guest_quitting() { return g_quitting; }

bool mm_guest_in_run() {
    return net_role() == NetRole::Client && session_has_others() && savefile_in_coop() &&
           savefile_adventure_is_loaded() && g_host_in_run;
}

std::string mm_peer_name(uint8_t id) { return peer_name(id); }

bool mm_departure_busy() {
    // Le choix des colliers (ClassChooser) s'ouvre sur la maison : elle ne s'en va
    // qu'a sa fermeture (0x1403B0550), apres le clic rejoue -- les drapeaux du vote
    // sont deja retombes. (Pas g_dep_passing : un clic rejoue abandonne le laisse
    // leve jusqu'au prochain clic du panneau.)
    if (find_scene("ClassChooser")) return true;
    if (net_role() == NetRole::Client)
        return g_dep_req_live || savefile_join_armed() || savefile_home_armed() || embark_guest_active();
    return g_dep_active || g_dep_embark || g_dep_go;
}

bool mm_panel_set_text(void* panel, const char* child, const std::string& text_utf8) {
    if (!panel || !child || strlen(child) > 15) return false;
    auto find = call_target<FnFindChild>(C_SwfFindChild);
    auto set  = call_target<FnSetText>(C_SwfSetText);
    if (!find || !set) return false;
    void* clip = nullptr;
    if (!mem_read((char*)panel + kPanel_Clip, &clip, sizeof(clip)) || !clip) return false;
    // Ce que fait MenuPanel::set_text (0x14097C6F0) : le champ du clip a ce nom,
    // puis le texte passe par valeur (le jeu le copie et libere ce tampon).
    GameString n = small_string(child);
    void* field = find(clip, &n);
    if (!field) return false;
    GameWString tmp;
    if (!make_game_wstring(tmp, utf8_to_wide(text_utf8.c_str()))) return false;
    set(field, &tmp, false, true);
    return true;
}

int mm_panel_read_text(void* panel, const char* child, std::string& out_utf8, std::string* why) {
    out_utf8.clear();
    auto fail = [&](const char* reason, int r = -1) {
        if (why) *why = reason;
        return r;
    };
    if (!panel) return fail("pas de panneau");
    if (!child || strlen(child) > 15) return fail("nom de champ invalide");
    auto find = call_target<FnFindChild>(C_SwfFindChild);
    if (!find) return fail("recherche d'un champ introuvable");
    void* clip = nullptr;
    if (!mem_read((char*)panel + kPanel_Clip, &clip, sizeof(clip)) || !clip) return fail("panneau sans clip");
    GameString n = small_string(child);
    void* field = find(clip, &n);
    if (!field) return fail("pas de champ de ce nom", 0);
    // Le texte du champ, tel que le setter l'a recopie (std::wstring a +0xB8, voir
    // C_SwfSetText) : balises comprises, il se reecrit tel quel.
    constexpr uintptr_t kField_Text = 0xB8;
    GameWString w{};
    if (!mem_read((char*)field + kField_Text, &w, sizeof(w))) return fail("champ illisible");
    if (w.size > 4096) return fail("texte trop long ou pas un texte");
    std::wstring text(w.size, L'\0');
    if (w.cap > 7) {
        if (!w.ptr || (w.size && !mem_read(w.ptr, &text[0], w.size * sizeof(wchar_t))))
            return fail("texte illisible");
    } else if (w.size) {
        if (w.size > 7) return fail("texte incoherent");
        memcpy(&text[0], w.buf, w.size * sizeof(wchar_t));
    }
    if (text.empty()) return 1;
    const int n8 = WideCharToMultiByte(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0, nullptr, nullptr);
    if (n8 <= 0) return fail("texte pas en UTF-16");
    out_utf8.assign((size_t)n8, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), (int)text.size(), &out_utf8[0], n8, nullptr, nullptr);
    return 1;
}

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
    case T_TryAbandonRun: *detour = (void*)&h_TryAbandonRun; *original = (void**)&o_TryAbandonRun; return true;
    default: return false;
    }
}

static void defense_pump();      // Mewgenics Multi : plus bas, avec MM_PEER_DEFEND

// --- Mewgenics Multi (protocole 59) : jouer ensemble par Steam ----------------------
//
// Chez l'hote, tant que son salon est ouvert : ses amis Steam voient « Rejoindre la
// partie » (mgmp_steam). Chez un ami qui clique dessus (ou accepte une invitation) :
// au menu principal, la connexion part toute seule, par Steam ; ailleurs, elle
// attend qu'il y revienne.
static bool        g_steam_join      = false;
static uint64_t    g_steam_join_id   = 0;
static std::string g_steam_join_name;
static bool        g_steam_join_said = false;
static uint32_t    g_steam_idle_from = 0;     // image depuis laquelle plus aucune session

static void steam_menu_pump() {
    steam_pump();
    if (net_role() == NetRole::Host && net_active()) {
        const uint16_t port = g_host_port ? g_host_port : config().net_port;
        const uint8_t  n    = net_peer_count();
        steam_host_open(port, n ? (int)n : 1);
    } else {
        steam_host_close();
    }

    // La connexion Steam d'un salon quitte ou tombe : fermee (3 s de grace : la
    // demande de connexion du salon ne s'applique qu'a l'image suivante).
    if (steam_client_on() && net_role() == NetRole::None && !session_request_pending()) {
        if (!g_steam_idle_from) g_steam_idle_from = g_frame;
        if (g_frame - g_steam_idle_from > 180) steam_client_end();
    } else {
        g_steam_idle_from = 0;
    }

    uint64_t id = 0;
    std::string name;
    if (steam_take_join_request(id, name)) {
        g_steam_join      = true;
        g_steam_join_id   = id;
        g_steam_join_name = display_name(name.c_str());
        g_steam_join_said = false;
    }
    if (!g_steam_join) return;
    const std::string& who = g_steam_join_name;
    if (net_role() != NetRole::None || session_request_pending()) {
        g_steam_join = false;
        log_line("MULTI", "Steam : « Rejoindre la partie » de %s ignore -- deja dans un salon", who.c_str());
        show_info(fmt(u8"Rejoindre %s\n\n"
                      u8"Tu es déjà dans un salon multijoueur :\n"
                      u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
                      u8"puis clique à nouveau sur « Rejoindre la partie ».", who.c_str()));
        return;
    }
    if (!at_main_menu()) {
        if (!g_steam_join_said) {
            g_steam_join_said = true;
            log_line("MULTI", "Steam : salon de %s rejoint au retour au menu principal", who.c_str());
            show_info(fmt(u8"Rejoindre %s\n\n"
                          u8"Ton jeu rejoindra le salon de %s\n"
                          u8"dès que tu seras au menu principal\n"
                          u8"(Échap, puis « Quitter vers le menu »).", who.c_str(), who.c_str()));
        }
        return;
    }
    // Une fenetre du menu multijoueur ouverte (« Héberger / Rejoindre », l'adresse) :
    // refermee sans son action, le salon s'ouvre a sa place ; une autre fenetre
    // (une information) : on attend qu'elle se ferme.
    if (g_live.alive) {
        if ((g_live.screen == Screen::Choice || g_live.screen == Screen::JoinEntry ||
             g_live.screen == Screen::BadAddress) && !g_click_target && !g_live.pressed) {
            g_silent = g_live.serial;
            popup_press("no");
        }
        return;
    }
    g_steam_join = false;
    const uint16_t port = steam_client_begin(g_steam_join_id);
    if (!port) {
        log_line("MULTI", "!! Steam : connexion au salon de %s impossible", who.c_str());
        show_info(fmt(u8"Rejoindre %s\n\n"
                      u8"La connexion par Steam est impossible pour l'instant.\n"
                      u8"Réessaie dans un moment, ou rejoins par adresse\n"
                      u8"(Multijoueur, puis « Rejoindre »).", who.c_str()));
        return;
    }
    g_join_ip   = who + u8" (Steam)";
    g_join_port = port;
    log_line("MULTI", "Steam : connexion au salon de %s (port local %u)", who.c_str(), (unsigned)port);
    session_request_join("127.0.0.1", port);
    open_client_lobby();
}

void mm_menu_pump() {
    ++g_frame;
    game_prompt_alive();                 // oubliee des que sa scene s'en va
    embark_frame_begin();
    embark_pump();
    steam_menu_pump();                   // Mewgenics Multi (protocole 59) : par Steam
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
                if (!g_play_when_world)
                    log_line("MULTI", "salon : le monde de l'hote n'est pas encore arrive -- l'ecran des "
                                      "sauvegardes s'ouvrira des qu'il sera la");
                g_play_when_world = true;
                g_action = Action::OpenAuto;           // le salon se rouvre l'image suivante
                break;
            }
            g_play_when_world = false;
            click_main_play();
            break;
        case Action::StopSession:
            log_line("MULTI", "fin de la session demandee par le menu");
            session_request_disconnect();
            steam_client_end();              // Mewgenics Multi (protocole 59)
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
                // La 4e place en route vers lui : la question revient dans 2 s,
                // pas dans 30 (le texte dit « dans un instant »).
                g_dep_offer_at = g_frame + (team_take_slot_wait() ? 2 * 60 : kOfferAgain);
                show_info(problem);
                break;
            }
            send_depart_answer(true);
            log_line("MULTI", "depart : ce joueur arrive");
            start_join("depart en aventure");
            embark_guest_joining(g_dep_req_seq);     // il suivra le depart de l'hote
            break;
        }
        case Action::DepartNo:
            send_depart_answer(false);
            team_guest_withdraw(g_dep_req_seq);      // ses chats envoyes plus tot ne partent plus
            embark_guest_cancel("pas encore");
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
            // Ses combats ne l'attendent plus (protocole 48) : rappel espace.
            g_run_offer_at = g_frame + kOfferAgainLong;
            break;
        case Action::QuitRun:
            log_line("MULTI", "aventure : ce joueur quitte l'aventure de l'hote (ses chats restent dans "
                              "l'equipe, l'hote les dirige)");
            g_quitting        = savefile_in_coop();
            g_run_offer_quiet = true;
            g_run_offer       = false;
            g_home_offer = false;
            g_home_due   = false;
            savefile_go_home();
            if (!savefile_home_armed())
                log_line("MULTI", "!! quitter l'aventure : la session est finie, ce joueur choisira sa "
                                  "save au menu");
            start_menu_trip("quitter l'aventure");
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
            embark_guest_cancel("annule par le joueur");
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
        case Action::EmbarkNoWait:
            embark_no_wait();
            break;
        case Action::VoteYes:
            vote_popup_button(g_vote_layout, true);
            break;
        case Action::VoteNo:
            vote_popup_button(g_vote_layout, false);
            break;
        case Action::LootYes:
            loot_popup_button(g_loot_layout, true);
            break;
        case Action::LootNo:
            loot_popup_button(g_loot_layout, false);
            break;
    }

    host_adventure_pump();
    client_adventure_pump();
    vote_pump();                    // Mewgenics Multi : le vote du chemin
    rungear_pump();                 // Mewgenics Multi : les objets en aventure
    purse_pump();                   // Mewgenics Multi : les bourses de l'aventure
    loot_pump();                    // Mewgenics Multi : le defile du butin
    npcgift_pump();                 // Mewgenics Multi : les dons des invites aux PNJ
    shop_pump();                    // Mewgenics Multi : les achats en boutique avec sa bourse
    gift_pump();                    // Mewgenics Multi : les dons entre joueurs (protocole 62)
    defense_pump();                 // Mewgenics Multi : la defense de la maison, seul
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
            // Mewgenics Multi : « Choisir ta sauvegarde » deja demande, le monde
            // de l'hote est la -- le bouton est rappuye pour lui.
            if (g_play_when_world) {
                if (net_role() != NetRole::Client || !net_active()) {
                    g_play_when_world = false;
                } else if ((world_have_host() || client_world_wait_over()) && !g_click_target &&
                           !g_live.pressed && popup_press("yes")) {
                    log_line("MULTI", "salon : le monde de l'hote est arrive -- ecran des sauvegardes");
                }
            }
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
    if (button) embark_on_button_update(button);
    if (!button || button != g_click_target) return;
    g_click_target = nullptr;
    auto click = call_target<FnButtonClick>(C_ButtonClick);
    if (click) click(button, false);   // force = 0, comme un vrai clic
}

bool mm_menu_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (hwnd) g_hwnd = hwnd;           // pour envoyer Echap (aller au menu)
    // Le defile du butin : Echap ne fait rien (la boite du jeu pourrait le prendre
    // pour son second bouton, « Passer » ou « Cupidité », ou le menu pause s'ouvrir).
    if (g_live.alive && g_live.screen == Screen::Loot && wp == VK_ESCAPE &&
        (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP))
        return true;
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
        g_home_due     = true;
        g_home_no_trip = false;
        log_line("MULTI", "aventure : celle de l'hote est finie, ce joueur doit rentrer chez lui");
    }
}

// --- Mewgenics Multi (protocoles 54 et 55) : la defense de la maison ----------------
//
// Chacun defend sa maison, seul (mm_team.h). Ce joueur dit aux autres quand son
// combat commence et finit (MM_PEER_DEFEND : a = 1 / 0, b = son numero ; l'hote
// relaie celui d'un invite aux autres) : sans cela, celui qui attend -- l'hote qui
// propose de partir, un invite pret pour la nuit -- ne saurait pas pourquoi il ne
// repond plus. La transition se lit sur le jeu (MewDirector+0x70C) : aussi pour
// une defense reprise d'une save.
//
// Protocole 55 : les arrivants l'apprennent aussi -- ce joueur redit sa defense en
// cours des que sa session est prete avec quelqu'un de nouveau, et l'hote redit
// toutes celles qu'il connait a chaque HELLO accepte. Chacun tient la liste de ceux
// qui defendent (g_defend_mask), taillee sur les presents des que leur liste est
// arrivee (pendant la presentation aussi : c'est la que l'arrivant recoit les
// rappels). Les listes d'attente le disent ; sinon une fenetre, qui attend son tour
// derriere une autre et la carte libre de l'aventure (kInfoYieldToFollow).
static bool        g_defending      = false;  // ce joueur defend sa maison (le jeu le dit)
static uint32_t    g_defend_roster  = 0;      // les presents a qui c'est deja dit
static bool        g_defense_due    = false;  // une fenetre a ce sujet attend son tour
static std::string g_defense_text;            // son texte, refait a chaque nouvelle
static std::string g_defense_shown;           // le texte confie a show_info en dernier

static void send_defend(uint8_t who, bool on, uint8_t flags = 0) {
    MmMsg m{};
    m.kind = MM_PEER_DEFEND;
    m.a    = on ? 1 : 0;
    m.b    = who;
    m.c    = flags;
    net_send_mm(m);
}

// « X », « X et Y », « X, Y et Z ».
static std::string defender_names(uint32_t mask) {
    std::vector<std::string> n;
    for (uint8_t id = 0; id < 8; ++id)
        if (mask & (1u << id)) n.push_back(peer_name(id));
    std::string s;
    for (size_t i = 0; i < n.size(); ++i)
        s += (i == 0 ? "" : i + 1 == n.size() ? " et " : ", ") + n[i];
    return s;
}

// Le texte de la fenetre apres la nouvelle sur `who` : l'etat de tous, a jour (une
// fenetre pas encore vue est remplacee par celle-ci, rien ne se perd).
static std::string defense_text(uint8_t who, bool on) {
    const uint32_t others = g_defend_mask & ~(1u << who);
    if (on) {
        if (!others)
            return fmt(u8"%s défend sa maison : un combat qu'il joue\n"
                       u8"seul, avec ses chats. Il revient ensuite.", peer_name(who).c_str());
        return fmt(u8"%s défendent leur maison : chacun joue\n"
                   u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
                   defender_names(g_defend_mask).c_str());
    }
    if (!others) return fmt(u8"%s a fini de défendre sa maison.", peer_name(who).c_str());
    const bool one = (others & (others - 1)) == 0;
    return fmt(u8"%s a fini de défendre sa maison.\n%s %s encore.", peer_name(who).c_str(),
               defender_names(others).c_str(), one ? u8"défend la sienne" : u8"défendent la leur");
}

// Une liste d'attente est a l'ecran : elle dit qui defend sa maison, a jour.
static bool defense_listed_on_screen() {
    return g_live.alive && (g_live.screen == Screen::NightWait || g_live.screen == Screen::DepartWait ||
                            g_live.screen == Screen::ArrivalWait);
}

// Chaque image : la fenetre « X défend sa maison » / « X a fini » prend son tour.
// Jamais a la place d'une autre fenetre en attente (show_info n'en garde qu'une) :
// elle attend. Une liste d'attente a l'ecran la rend inutile, et retire celle qui
// attendait encore.
static void defense_note_pump() {
    const bool ours_waiting = g_info_wanted && !g_defense_shown.empty() && g_info_text == g_defense_shown;
    if (defense_listed_on_screen()) {
        if (ours_waiting) g_info_wanted = false;
        g_defense_due = false;
        return;
    }
    if (!g_defense_due) return;
    if (g_info_wanted && !ours_waiting) return;
    g_defense_due   = false;
    g_defense_shown = g_defense_text;
    show_info(g_defense_shown, kInfoYieldToFollow);
}

static void defense_pump() {
    // Au menu, il n'y a pas de defense : celle qu'on a quittee par « Quitter vers le
    // menu » laisse son drapeau pose (le jeu ne le remet a zero qu'a la fin d'une
    // aventure), et la save chargee ensuite -- une autre, ou la copie de l'aventure
    // de l'hote -- se croirait encore en defense (mm_team.h). La reprise d'une
    // defense sauvee le repose elle-meme. (Le chargement d'une partie le fait
    // aussi : h_MewDirInit.)
    if (at_main_menu() && !find_scene("Battle") && !find_scene("House"))
        team_house_defense_forget_stale();

    // Ceux qui sont partis ne defendent plus rien ici. La liste des presents vaut
    // des qu'elle est arrivee, avant la fin de la presentation.
    g_defend_mask &= net_active() ? roster_mask() : 0;

    const bool now     = team_house_defense_running();
    const bool changed = now != g_defending;
    if (changed) {
        g_defending = now;
        log_line("MULTI", "defense de la maison : %s", now ? "le combat commence -- ce joueur le joue seul, "
                                                             "la session n'y entre pas"
                                                           : "le combat est fini");
    }
    defense_note_pump();

    // Personne a qui le dire (pas de session prete) : tous seront nouveaux ensuite.
    if (!session_has_others()) {
        g_defend_roster = 0;
        return;
    }
    // Un arrivant (ou la session qui devient prete) : ce joueur lui redit sa defense
    // en cours. L'hote redit celles qu'il connait a chaque HELLO (mm_on_peer_hello).
    const uint32_t roster    = roster_mask();
    const bool     newcomers = (roster & ~g_defend_roster) != 0;
    g_defend_roster = roster;
    if (changed || (newcomers && g_defending)) send_defend(net_self(), g_defending);
}

// Hote : un joueur vient d'etre accepte (HELLO). Son numero repart de zero (un
// numero repris ne defend rien : tous l'oublient, sans fenetre), et il apprend les
// defenses en cours -- la sienne propre, il la redira lui-meme.
static void defense_on_hello(uint8_t peer) {
    if (net_role() != NetRole::Host || peer >= 8) return;
    const uint32_t bit = 1u << peer;
    if (g_defend_mask & bit) {
        g_defend_mask &= ~bit;
        log_line("MULTI", "defense de la maison : le numero %u repart de zero (un joueur arrive)", (unsigned)peer + 1);
    }
    send_defend(peer, false, kDefendSilent);
    if (g_defending) send_defend(net_self(), true);
    for (uint8_t id = 0; id < 8; ++id)
        if (g_defend_mask & (1u << id)) send_defend(id, true);
}

static void on_peer_defend(uint8_t from, const MmMsg& m) {
    // L'hote : de l'invite lui-meme (son numero de connexion fait foi), relaye aux
    // autres. Un invite : de l'hote, qui dit pour qui.
    const bool    host = net_role() == NetRole::Host;
    const uint8_t who  = host ? from : m.b;
    if (who >= 8 || who == net_self()) return;
    const bool on     = m.a != 0;
    const bool silent = !host && !on && (m.c & kDefendSilent);
    if (host) {
        MmMsg relay = m;
        relay.b = who;
        relay.c = 0;
        net_send_mm(relay);
    }
    const uint32_t bit = 1u << who;
    if (((g_defend_mask & bit) != 0) == on) return;    // un rappel pour un arrivant : rien de neuf ici
    g_defend_mask = on ? (g_defend_mask | bit) : (g_defend_mask & ~bit);
    if (silent) {
        log_line("MULTI", "defense de la maison : le numero %u repart de zero (un joueur arrive)", (unsigned)who + 1);
        return;
    }
    log_line("MULTI", "defense de la maison : %s %s", peer_name(who).c_str(),
             on ? "defend sa maison (un combat a lui seul)" : "a fini de defendre sa maison");
    g_defense_text = defense_text(who, on);
    g_defense_due  = true;
    defense_note_pump();
}

void mm_on_message(uint8_t from, const MmMsg& m) {
    switch (m.kind) {
        case MM_PEER_DEFEND:
            on_peer_defend(from, m);
            break;
        case MM_SLOT:
            team_on_slot(from, m);           // Mewgenics Multi (protocole 58) : la 4e place
            break;
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
                // Deja dans la maison de l'hote (depart ensemble) : il rentre chez lui.
                // En route vers elle : il n'y entrera pas.
                const bool in_copy = embark_guest_active();
                embark_guest_cancel(nobody ? "personne ne part" : "depart annule par l'hote");
                if (in_copy) {
                    g_home_due     = true;
                    g_home_no_trip = true;
                    if (nobody)
                        show_info(fmt(u8"Départ en aventure\n\n"
                                      u8"Aucun chat ne peut partir cette fois :\n"
                                      u8"%s ne part pas à l'aventure.", host_display().c_str()));
                }
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
        case MM_EMBARK:
            embark_on_mm(from, m);
            break;
        case MM_VOTE:
            vote_on_mm(from, m);
            break;
        case MM_RUNREV:
            rungear_on_mm(from, m);
            break;
        case MM_PURSE:
            purse_on_mm(from, m);
            break;
        case MM_GIFT:
            gift_on_mm(from, m);             // Mewgenics Multi (protocole 62) : qui peut recevoir un don
            break;
        case MM_BATTLE_END:
            lockstep_on_battle_end(from, m); // Mewgenics Multi (protocole 64) : l'hote a fini un combat
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
                g_run_offer       = false;
                g_run_offer_quiet = false;           // la prochaine aventure se propose
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
                // follow_in_node : un evenement fait tourner la carte par moments
                // (follow_inside_node seul la croyait alors finie).
                // Mewgenics Multi : de meme une case finie dont l'hote n'est pas
                // encore sorti (ses montees de niveau, avant le premier choix) :
                // servie, l'arrivant rejouerait tout le combat.
                if (follow_in_node() && (choice_made_in_node() || follow_node_ended())) {
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
    // Le depart ensemble : les clics de l'hote dans le choix des colliers et
    // des objets partent chez l'equipe ; chez un invite qui regarde, les siens
    // sont retenus.
    if (embark_on_button_click(button)) return true;
    // Les objets en aventure : seul le proprietaire d'un chat change les siens.
    if (rungear_on_click(button)) return true;
    // Les boutiques : l'hote ne sort pas sans le confirmer si un invite y achete.
    if (shop_on_click(button)) return true;
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
        if (embark_guest_active()) return true;  // il attend le depart de l'hote, dans sa maison
        if (!g_home_due) {
            log_line("MULTI", "aventure : clic sur le panneau de la copie, retour propose");
            g_home_no_trip = false;
        }
        g_home_due = true;
        return true;
    }
    // Hote : le defile du butin n'est pas fini -- ni nuit ni depart avant (les
    // objets partis chez les invites quittent encore son rangement).
    if (net_role() == NetRole::Host && loot_host_busy()) {
        log_line("MULTI", "panneau : le partage du butin n'est pas fini, clic retenu");
        show_info(u8"Le partage du butin n'est pas fini :\n"
                  u8"attends la fin du défilé des objets.");
        return true;
    }
    if (kind == 2) {
        // « Défendre ! » (protocole 54) : la maison de CE joueur est attaquee (le
        // boss de la maison est a chacun). Il la defend comme seul, avec les chats
        // de sa boite : le depart du jeu passe tel quel (h_ButchDepart, chez
        // l'hote), la session n'entre pas dans ce combat (mgmp_lockstep :
        // team_house_defense_running), et les autres le savent (MM_PEER_DEFEND,
        // defense_pump).
        if (house_boss_defend()) {
            // En route pour l'aventure de l'hote (le trajet vers le menu, ou sa partie
            // qui va se charger) : le trajet quitterait ce combat a peine commence.
            // Sa maison attendra son retour (l'attaque reste dans sa save).
            if (savefile_join_armed() || g_trip.active) {
                log_line("MULTI", "panneau « Defendre » : ce joueur part rejoindre l'aventure de l'hote -- "
                                  "clic retenu, la defense attendra son retour");
                show_info(fmt(u8"Tu pars rejoindre l'aventure de %s :\n"
                              u8"tu défendras ta maison à ton retour.", host_display().c_str()));
                return true;
            }
            // Pret pour la nuit : il ne l'est plus, il part se battre (les autres
            // l'attendront, leur liste dit qu'il defend sa maison).
            if (g_night_ready) set_night_ready(false);
            log_line("MULTI", "panneau « Defendre » en session : ce joueur defend sa maison, seul");
            return false;
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
            g_run_offer       = true;
            g_run_offer_at    = g_frame;
            g_run_offer_quiet = false;
            return true;
        }
        log_line("MULTI", "depart demande par un invite : c'est l'hote qui lance l'aventure");
        show_info(fmt(u8"C'est %s qui lance l'aventure :\n"
                      u8"demande-lui de cliquer sur « À l'aventure ! ».",
                      host_display().c_str()));
        return true;
    }
    // Un invite chez lui pendant que l'hote est en aventure : la nuit ne peut
    // pas passer sans l'hote, le panneau lui repropose plutot l'aventure.
    if (net_role() == NetRole::Client && !savefile_join_armed() && g_host_in_run && !g_live.alive) {
        log_line("MULTI", "aventure : clic d'un invite sur « Fin de journée » pendant l'aventure de l'hote -- "
                          "elle lui est reproposee");
        g_run_offer       = true;
        g_run_offer_at    = g_frame;
        g_run_offer_quiet = false;
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
    defense_on_hello(peer);              // Mewgenics Multi : les defenses en cours (protocole 55)
}

} // namespace mgmp
