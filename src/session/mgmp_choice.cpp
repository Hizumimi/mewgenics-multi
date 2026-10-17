// mgmp_choice -- see mgmp_choice.h for why replicating the CHOICE replaces
// replicating everything the choice does.
#include "mgmp_choice.h"
#include "mgmp_follow.h"
#include "mgmp_savefile.h"   // savefile_in_coop (Mewgenics Multi)
#include "mgmp_net.h"
#include "mgmp_config.h"
#include "mgmp_tuning.h"
#include "mgmp_mem.h"
#include "mgmp_log.h"
#include "mgmp_addresses.h"
#include "mgmp_resolve.h"
#include "mm_menu.h"         // Mewgenics Multi : mm_run_peers_mask, mm_peer_name, mm_panel_set_text
#include "mm_team.h"         // Mewgenics Multi : team_owners_active, team_owner_of
#include "mgmp_catsync.h"    // Mewgenics Multi : catsync_cat_by_key (le choix du chat d'un evenement)
#include "mgmp_hooks.h"      // Mewgenics Multi : hooks_is_live

#include <windows.h>
#include <intrin.h>
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>

namespace mgmp {
namespace {

// --- the two option arrays -------------------------------------------------
//
// Both are {T* begin, T* end} with sizeof(T) == 240, and both are built in
// authored file order. Read off setupActionChoice__inner_0 (v20 = *(v1+28),
// v161 = *(v1+29), then "v20 += 240") and off the level-up button callback,
// which resolves *(LevelUpScreen+864) + 240 * index.
constexpr uintptr_t kEvt_OptBegin  = 224;
constexpr uintptr_t kEvt_OptEnd    = 232;

constexpr uintptr_t kLvl_OptBegin  = 864;
constexpr uintptr_t kLvl_OptEnd    = 872;
constexpr uintptr_t kLvl_Committed = 792;   // select_option's "already chose" guard

constexpr uintptr_t kOptStride     = 240;

// An event option entry's stat key -- "str".."lck", "coins", "quest", "none",
// "home". A std::string at entry+64, compared against exactly those literals
// all the way through setupActionChoice.
constexpr uintptr_t kEvtOpt_Stat = 64;
// A LevelUpOption's type (1 stats, 2/3 pools, 4/7 ability, 5, 6 item) and its
// authored name, both read straight off select_option's own switch.
constexpr uintptr_t kLvlOpt_Type = 0;
constexpr uintptr_t kLvlOpt_Name = 200;

// Mewgenics Multi : chacun choisit pour ses chats.
//   LevelUpScreen+0xA0 : le chat (CatData*), ecrit par LevelUpScreen::init
//     (0x14037A350) ; +0xB0 son panneau (MenuPanel, le titre dans « title ») ;
//     +0xE4 les relances restantes (LevelUpScreen::Reroll : rien a 0).
//   AbilityChooser : +0x20 sa scene (+0x4DB : en train d'etre detruite), +0x40 son
//     panneau (le texte dans « prompt »), +0x48 le choix retenu.
constexpr uintptr_t kLvl_Cat       = 0xA0;
constexpr uintptr_t kLvl_Panel     = 0xB0;
constexpr uintptr_t kLvl_Rerolls   = 0xE4;
constexpr uintptr_t kAbl_Scene     = 0x20;
constexpr uintptr_t kAbl_Panel     = 0x40;
constexpr uintptr_t kScene_Dying   = 0x4DB;
constexpr uintptr_t kCat_Id        = 0x00;
constexpr uintptr_t kCat_LevelUps  = 0xC30;   // CatData : u32 incremente par LevelUpScreen::init (0x14037A357) --
                                              // un ecran de montee de niveau de ce chat, le meme chez tous

// Mewgenics Multi : l'evenement (WorldEvent, 1.1.21239).
//   +0x40 son panneau (MenuPanel : « title », le nom de l'evenement, ecrit par
//     setupTitle 0x140919760 ; « prompt », son texte) ;
//   +0x50 le CatSelector du choix du chat (+0x88 la cle du chat montre) ;
//   +0xB0 « cat_choice » de l'evenement, lu par WorldEvent::init : 0 choose (un
//     joueur choisit le chat : setupCatChoice 0x140919AE0, puis son « ok »,
//     lambda_1 0x14093C220, ecrit +0x128), 1 random et 2 random parmi ceux qui
//     n'ont pas fait les derniers evenements (tire du flux de la simulation a
//     0x1409150F8), 3 none ;
//   +0x128 le chat de l'evenement (CatData*, nul tant qu'il n'est pas choisi ou
//     sans chat ; seuls WorldEvent::init et le « ok » du choix du chat l'ecrivent --
//     la commande set_subject ne change que l'image du clip « subject ») ;
//   +0x19F9 les options sont a l'ecran et attendent un clic (setupActionChoice
//     0x14091A7E0 l'ecrit a 1, la suite 0x14091EA80 a 0). Le choix d'une option
//     (0x14093C1C0) pose +0x100 puis relance le clip : joue a la page du titre,
//     il ferait sauter l'evenement a une etape qu'il n'a pas montree.
constexpr uintptr_t kEvt_Panel     = 0x40;
constexpr uintptr_t kEvt_Selector  = 0x50;
constexpr uintptr_t kEvt_Subject   = 0x128;
constexpr uintptr_t kEvt_Choosing  = 0x19F9;
constexpr uintptr_t kSel_Key       = 0x88;

// The shipped UI pads out to 4 buttons per screen, and a reroll appends 4 more
// each time. This bound only has to be loose enough not to reject a legitimate
// list and tight enough to refuse a wild pointer.
constexpr uint32_t kMaxOptions = 64;

// Mewgenics Multi : la file des choix (invite : ceux de l'hote, dans son ordre ;
// hote : les demandes des invites) et ses delais.
constexpr size_t   kMaxQueue      = 32;
constexpr uint64_t kBlockedDropMs = 60000;   // un choix qui ne s'applique pas sur l'ecran la, au plus
constexpr uint64_t kRequestLifeMs = 60000;   // hote : une demande d'invite sans son ecran, au plus
constexpr uint64_t kAheadWaitMs   = 5000;    // hote : une demande d'un ecran que le sien n'a pas encore, au plus
constexpr uint64_t kPendingMs     = 3000;    // invite : une demande sans reponse retient les clics suivants, au plus
constexpr uint32_t kMaxChoosers   = 8;
constexpr uint64_t kChooserLostMs = 15000;   // un AbilityChooser dont l'update ne tourne plus : oublie
constexpr uint64_t kEventSettleMs = 10000;   // le choix d'evenement d'apres attend une liste neuve, au plus
constexpr uint64_t kTitleRefreshMs = 500;    // le titre « A X de choisir » : reecrit a ce rythme (title_level)
constexpr uint64_t kNoticeMs      = 3000;    // « Ce chat est a X » dans le titre de l'evenement, autant
constexpr uint64_t kEventTextMs   = 150;     // l'ecran de l'evenement : le champ relu a ce rythme (une page changee)
constexpr uint64_t kTitleRetryMs  = 1000;    //   ... et, ni titre ni texte lisible, une nouvelle lecture au plus
constexpr uint64_t kHoldCapMs     = 300000;  // un choix qui attend la page du choix (Apply::Hold) : 5 min au plus

using fn_cap     = void (__fastcall*)(void*);
using fn_this    = void (__fastcall*)(void*);
using fn_pick    = void (__fastcall*)(void*, int32_t);
using fn_catsel  = void (__fastcall*)(void*, uint64_t);   // CatSelector::select(cle)
// AbilityChooser::init(this, Scene*, CatData*, Mode, ...) : dix arguments, tous
// passes dans un registre ou une case de pile de 8 octets -- les faire suivre
// tels quels en uint64_t est exact (les objets passes par valeur le sont par
// adresse, et c'est l'appele qui les detruit : nous ne copions rien).
using fn_ablinit = void (__fastcall*)(void*, void*, void*, uint64_t, uint64_t, uint64_t,
                                      uint64_t, uint64_t, uint64_t, uint64_t);

// Ou s'ouvre un AbilityChooser, d'apres l'adresse de retour de son init.
enum : uint8_t { kCtxOther = 0, kCtxLevelUp, kCtxEvent, kCtxItem };

struct Decision {
    uint8_t  kind      = 0;
    uint32_t index     = 0;
    uint32_t count     = 0;
    uint32_t aux       = 0;
    uint64_t node_seed = 0;
    uint64_t cat_id    = 0;
    uint32_t serial    = 0;    // l'ecran de montee de niveau (CatData+0xC30), 0 : inconnu
    char     name[48]  = {};
    uint8_t  from      = 0;    // hote : l'invite qui demande
    uint64_t since     = 0;    // son arrivee
    uint64_t blocked   = 0;    // invite : depuis quand son ecran est la sans qu'il s'applique
    uint64_t held      = 0;    // depuis quand il attend la page du choix (Apply::Hold)
    bool     warned    = false;
};

struct Chooser {
    void*    ptr    = nullptr;
    void*    cat    = nullptr;   // le CatData donne a son init (vivant tant qu'il est ouvert)
    uint64_t cat_id = 0;
    uint8_t  ctx    = kCtxOther;
    bool     picked = false;   // un choix y est passe (le jeu ignore les suivants)
    bool     titled = false;
    uint64_t seen   = 0;
};

enum class Gate { Run, Request, Refuse };
// Hold : attendre sans que le delai d'abandon coure (l'ecran est le bon, le joueur
// n'en est pas encore a la page du choix : le titre d'un evenement qu'il lit).
enum class Apply { Applied, Wait, Hold, Drop };

struct State {
    bool on        = false;
    bool is_client = false;

    // Resolved from the module base, prologue-checked, exactly like catsync's
    // call targets: a bad address here would run an unrelated function with our
    // arguments on the game's stack.
    uintptr_t base     = 0;
    bool      resolved = false;
    fn_cap    evt_commit = nullptr;   // sub_140937F30
    fn_cap    lvl_click  = nullptr;   // sub_140386810

    // Mewgenics Multi : les originaux des crochets de ce module (choice_binding).
    fn_this    o_reroll  = nullptr;
    fn_pick    o_pick    = nullptr;
    fn_ablinit o_ablinit = nullptr;
    fn_this    o_ablupd  = nullptr;
    fn_this    o_catchoice = nullptr;   // WorldEvent::setupCatChoice
    fn_cap     o_catpick   = nullptr;   // son « ok » (lambda_1, capture {_, WorldEvent*, MewDirector*})
    fn_catsel  catsel      = nullptr;   // CatSelector::select (C_CatSelectorSelect)

    void* world_event  = nullptr;   // latest WorldEvent*,    from its update
    void* level_screen = nullptr;   // latest LevelUpScreen*, from its update

    // Mewgenics Multi : la file. Chez un invite, les choix publies par l'hote,
    // dans l'ordre ou il les a faits : un choix peut arriver avant que l'ecran
    // de ce joueur n'existe (il suit l'hote dans la case), il attend ; la tete
    // seule s'applique, sur l'ecran de son genre, quand c'est le bon. Chez
    // l'hote, les demandes des invites pour leurs chats.
    std::deque<Decision> queue;

    uint64_t here_seed  = 0;
    // Mewgenics Multi (hote) : un choix a deja ete publie dans cette case.
    bool     chose_here = false;

    // NODES THIS PEER WILL NEVER ENTER, so a choice that arrives for one after
    // the fact can be refused instead of held forever. Same device, and the
    // same 16 entries, as the retired-battle_id ring in mgmp_battleid.h.
    static constexpr uint32_t kSkippedRing = 16;
    uint64_t skipped[kSkippedRing] = {};
    uint32_t skipped_next          = 0;

    // Set while we drive the game's own commit path, so the hook that exists to
    // SWALLOW a click does not swallow the click we just injected. Not
    // thread-local: every path that touches it is the game thread.
    bool injecting = false;

    // Mewgenics Multi : les AbilityChooser vivants, leur chat et d'ou ils viennent.
    Chooser choosers[kMaxChoosers];
    void*   titled_level = nullptr;   // l'ecran de montee de niveau dont on a change le titre...
    uint64_t titled_cat  = 0;         // ... son chat et son tableau d'options (une adresse
    const void* titled_opts = nullptr; //    peut resservir pour l'ecran suivant)...
    uint64_t titled_at   = 0;         // ... et quand (reecrit de temps en temps : voir title_level)
    bool     titled_other = false;    // ... pour dire qu'un autre joueur choisit
    // Invite : sa demande sans reponse, par genre (evenement, montee de niveau,
    // relance, capacite, chat de l'evenement) -- voir request.
    struct Pending { const void* screen = nullptr; uint64_t cat = 0; uint64_t at = 0; };
    Pending pend[kChoiceKinds];

    // Le dernier choix d'evenement joue ici, et la liste ou il l'a ete.
    uint64_t evt_done_at = 0;
    uint64_t evt_done_fp = 0;
    // Les choix d'option d'evenement joues dans cette case (le meme compte chez
    // tous : l'hote les joue avant de les publier). Une demande porte celui de
    // son ecran : faite sur une etape deja decidee, elle n'est pas jouee.
    uint32_t evt_stage = 0;
    // Hote : l'etape ou il prend la main (+1 ; 0 : aucune) -- la demande du
    // proprietaire pour cette etape ne peut pas se jouer ici, personne d'autre ne
    // pourrait choisir.
    uint32_t evt_takeover = 0;
    // Le choix du chat d'un evenement (setupCatChoice, puis son « ok »), pour un
    // WorldEvent : son panneau a l'ouverture (un autre evenement a la meme adresse
    // n'est pas celui-ci), ouvert, fait, et le repli (le chat choisi ailleurs ne se
    // rejoue pas ici : ce joueur choisit le sien, comme avant, plutot qu'un ecran fige).
    struct CatPick { void* we = nullptr; void* panel = nullptr; bool open = false; bool done = false; bool local = false; };
    CatPick catpick;
    // Qui decide, ecrit sur l'ecran de l'evenement tant que ce module l'a change :
    // le champ (le titre s'il existe, sinon « prompt », le texte de l'evenement,
    // ou la ligne s'ajoute), le texte du jeu (remis apres, s'il n'a pas change
    // entre-temps), notre ligne, le texte ecrit, et jusqu'a quand un avis reste.
    void*       etitle_we     = nullptr;
    bool        etitle_on     = false;
    const char* etitle_field  = nullptr;
    std::string etitle_game;
    std::string etitle_ours;
    std::string etitle_written;
    uint64_t    etitle_at     = 0;
    uint64_t    etitle_until  = 0;   // un avis (« Ce chat est a X ») : jusque-la
    bool        etitle_other  = false;
    bool        etitle_unreadable = false;   // ni titre ni texte lisible : on n'y ecrit pas...
    uint64_t    etitle_retry_at = 0;         //   ... et on ne relit qu'a partir de la
    // Le dernier texte ecrit et le texte du jeu d'alors, gardes apres un oubli : un
    // panneau qui le montre encore (deux WorldEvent qui alternent) n'a pas notre
    // ligne pour texte du jeu.
    std::string etitle_prev_written;
    std::string etitle_prev_game;

    uint32_t sent      = 0;
    uint32_t applied   = 0;
    uint32_t swallowed = 0;
    uint32_t requested = 0;

    CRITICAL_SECTION cs;
    bool cs_ready = false;
};

State g;

struct Guard {
    Guard()  { if (g.cs_ready) EnterCriticalSection(&g.cs); }
    ~Guard() { if (g.cs_ready) LeaveCriticalSection(&g.cs); }
};

const char* kind_name(uint8_t k) {
    switch (k) {
        case kChoiceLevelUp:  return "level-up";
        case kChoiceReroll:   return "reroll";
        case kChoiceAbility:  return "ability";
        case kChoiceEventCat: return "event cat";
        default:              return "event";
    }
}

// Le genre d'ecran ou un choix s'applique (la relance : la montee de niveau ; le
// chat d'un evenement : l'evenement).
uint8_t screen_kind_of(uint8_t kind) {
    if (kind == kChoiceReroll) return kChoiceLevelUp;
    if (kind == kChoiceEventCat) return kChoiceEvent;
    return kind;
}

// Both called with the Guard held.
void remember_skipped(uint64_t seed) {
    if (!seed) return;
    for (uint32_t i = 0; i < State::kSkippedRing; ++i)
        if (g.skipped[i] == seed) return;          // already known, keep it once
    g.skipped[g.skipped_next] = seed;
    g.skipped_next = (g.skipped_next + 1) % State::kSkippedRing;
}

bool was_skipped(uint64_t seed) {
    if (!seed) return false;
    for (uint32_t i = 0; i < State::kSkippedRing; ++i)
        if (g.skipped[i] == seed) return true;
    return false;
}

// --- reading a std::string out of the game ---------------------------------
//
// MSVC's std::string is {union{char buf[16]; char* ptr;}, size, capacity}, so
// the characters are inline when capacity <= 15 and behind a pointer otherwise.
// Copy at most cap-1 and always terminate: a garbled length must not walk.
bool read_game_string(const void* str, char* out, uint32_t cap) {
    if (!out || cap == 0) return false;
    out[0] = '\0';
    if (!str) return false;
    uint64_t len = 0, capacity = 0;
    if (!mem_read((const uint8_t*)str + 16, &len, sizeof(len))) return false;
    if (!mem_read((const uint8_t*)str + 24, &capacity, sizeof(capacity))) return false;
    if (len > 4096) return false;                  // not a string we recognise
    const uint8_t* chars = (const uint8_t*)str;
    if (capacity > 15) {
        void* p = nullptr;
        if (!mem_read((const uint8_t*)str, &p, sizeof(p)) || !p) return false;
        chars = (const uint8_t*)p;
    }
    uint32_t n = (uint32_t)len;
    if (n > cap - 1) n = cap - 1;
    if (n && !mem_read(chars, out, n)) { out[0] = '\0'; return false; }
    out[n] = '\0';
    return true;
}

// --- the option arrays -----------------------------------------------------

bool read_options(const void* screen, uintptr_t begin_off, uintptr_t end_off,
                  uint8_t*& begin, uint32_t& count) {
    begin = nullptr; count = 0;
    if (!screen) return false;
    uint8_t* b = nullptr; uint8_t* e = nullptr;
    if (!mem_read((const uint8_t*)screen + begin_off, &b, sizeof(b))) return false;
    if (!mem_read((const uint8_t*)screen + end_off,   &e, sizeof(e))) return false;
    if (!b || e < b) return false;
    const uintptr_t bytes = (uintptr_t)(e - b);
    if (bytes % kOptStride) return false;           // not the array we think it is
    const uintptr_t n = bytes / kOptStride;
    if (n == 0 || n > kMaxOptions) return false;
    begin = b; count = (uint32_t)n;
    return true;
}

void option_name(uint8_t kind, const uint8_t* entry, char* out, uint32_t cap) {
    const uintptr_t off = (kind == kChoiceEvent) ? kEvtOpt_Stat : kLvlOpt_Name;
    if (!read_game_string(entry + off, out, cap)) snprintf(out, cap, "?");
}

uint32_t option_type(const uint8_t* entry) {
    uint32_t t = 0;
    mem_read(entry + kLvlOpt_Type, &t, sizeof(t));
    return t;
}

// --- Mewgenics Multi : a qui est la decision ---------------------------------

uint8_t self_peer() { return g.is_client ? net_self() : (uint8_t)kHostPeer; }

uint64_t cat_id_of(const void* cat) {
    uint64_t id = 0;
    if (!cat || !mem_read((const uint8_t*)cat + kCat_Id, &id, sizeof(id))) return 0;
    return id;
}

uint64_t level_cat_id(const void* screen) {
    void* cat = nullptr;
    if (!screen || !mem_read((const uint8_t*)screen + kLvl_Cat, &cat, sizeof(cat))) return 0;
    return cat_id_of(cat);
}

// Le numero de l'ecran de montee de niveau ou en est ce chat (CatData+0xC30).
uint32_t cat_levelups(const void* cat) {
    uint32_t n = 0;
    if (!cat || !mem_read((const uint8_t*)cat + kCat_LevelUps, &n, sizeof(n))) return 0;
    return n;
}

uint32_t level_serial(const void* screen) {
    void* cat = nullptr;
    if (!screen || !mem_read((const uint8_t*)screen + kLvl_Cat, &cat, sizeof(cat))) return 0;
    return cat_levelups(cat);
}

bool level_committed(const void* screen) {
    uint8_t committed = 0;
    return screen && mem_read((const uint8_t*)screen + kLvl_Committed, &committed, 1) && committed;
}

int32_t level_rerolls_left(const void* screen) {
    int32_t left = 0;
    if (!screen || !mem_read((const uint8_t*)screen + kLvl_Rerolls, &left, sizeof(left))) return 0;
    return left;
}

// --- Mewgenics Multi : l'evenement --------------------------------------------

void* event_subject(const void* we) {
    void* cat = nullptr;
    if (!we || !mem_read((const uint8_t*)we + kEvt_Subject, &cat, sizeof(cat))) return nullptr;
    return cat;
}

uint64_t event_subject_id(const void* we) { return cat_id_of(event_subject(we)); }

// Ses options sont a l'ecran et attendent un clic.
bool event_choosing(const void* we) {
    uint8_t on = 0;
    return we && mem_read((const uint8_t*)we + kEvt_Choosing, &on, 1) && on;
}

void* event_panel(const void* we) {
    void* panel = nullptr;
    if (!we || !mem_read((const uint8_t*)we + kEvt_Panel, &panel, sizeof(panel))) return nullptr;
    return panel;
}

// Le choix du chat suivi est celui de cet evenement (meme adresse, meme panneau).
// (Ni le mode « cat_choice » ni le chat pose ne disent s'il est ouvert : le clip
// d'un evenement appelle setupCatChoice quand il veut, et different_from_last_x_cats
// fait tirer un chat a WorldEvent::init meme pour un evenement « choose ».)
bool catpick_is(const void* we) { return we && g.catpick.we == we && g.catpick.panel == event_panel(we); }

void catpick_forget() { g.catpick = State::CatPick{}; }

// setupCatChoice a tourne sur lui (ou le bouton « ok » de son choix a ete clique).
void catpick_mark_open(void* we) {
    if (!catpick_is(we)) {
        g.catpick       = State::CatPick{};
        g.catpick.we    = we;
        g.catpick.panel = event_panel(we);
    }
    g.catpick.open = true;
    g.catpick.done = false;
}

// Un « ok » a tourne : le chat est pose.
void catpick_closed(const void* we) {
    if (!catpick_is(we)) return;
    g.catpick.open = false;
    g.catpick.done = true;
}

bool catpick_open_on(const void* we) { return catpick_is(we) && g.catpick.open; }
bool catpick_done_on(const void* we) { return catpick_is(we) && g.catpick.done; }

// Les offsets bruts de WorldEvent et le rejeu du chat ne valent que pour le build
// vise, et demandent les deux crochets du choix du chat et CatSelector::select.
bool catpick_replicated() {
    return g.catsel && hooks_is_live(T_EventCatChoice) && hooks_is_live(T_EventCatPick) && resolve_is_pinned_build();
}

// La cle du chat que montre le CatSelector de l'evenement (0 : illisible).
uint64_t catpick_key(const void* we) {
    void* sel = nullptr;
    uint64_t key = 0;
    if (!we || !mem_read((const uint8_t*)we + kEvt_Selector, &sel, sizeof(sel)) || !sel) return 0;
    if (!mem_read((const uint8_t*)sel + kSel_Key, &key, sizeof(key))) return 0;
    return key;
}

const void* director() {
    const void* const* slot = (const void* const*)addr_of_data(D_MewDirectorPtr);
    const void* dir = nullptr;
    if (!slot || !mem_read(slot, &dir, sizeof(dir))) return nullptr;
    return dir;
}

// La cle d'un chat de l'equipe de l'aventure d'apres son id (MewDirector+0x5C0, les
// cles que parcourt le CatSelector) ; 0 : pas dans l'equipe.
uint64_t run_cat_key(uint64_t cat_id) {
    const uint8_t* dir = (const uint8_t*)director();
    uint32_t n = 0;
    const uint64_t* keys = nullptr;
    if (!cat_id || !dir || !mem_read(dir + kDir_CatIdCount, &n, sizeof(n)) ||
        !mem_read(dir + kDir_CatIdData, &keys, sizeof(keys)) || !keys || n == 0 || n > 16)
        return 0;
    for (uint32_t i = 0; i < n; ++i) {
        uint64_t key = 0;
        if (!mem_read(keys + i, &key, sizeof(key)) || !key) continue;
        if (cat_id_of(catsync_cat_by_key(key)) == cat_id) return key;
    }
    return 0;
}

// Le proprietaire d'un chat dans l'equipe mixte ; -1 sans equipe mixte ou pour un
// chat qu'elle ne connait pas (une recrue de l'aventure) : l'hote decide.
int owner_of(uint64_t cat_id) {
    if (!cat_id || !team_owners_active()) return -1;
    return team_owner_of(cat_id);
}

// Qui decide pour ce chat : son proprietaire s'il est dans l'aventure, l'hote
// sinon. Chez un invite, la presence n'est connue que pour lui-meme : un autre
// proprietaire est suppose la (s'il est parti, l'hote decide, et c'est son choix
// publie qui arrivera).
int decider_of(uint64_t cat_id) {
    const int o = owner_of(cat_id);
    if (o < 0 || o == (int)kHostPeer) return (int)kHostPeer;
    if (!g.is_client) {
        const uint32_t mask = mm_run_peers_mask();
        if (o >= 32 || !(mask & (1u << o))) return (int)kHostPeer;
    }
    return o;
}

// Ce que fait ici un clic de ce joueur sur une decision au sujet de ce chat.
// Sans equipe mixte : comme avant, l'hote decide de tout.
Gate gate_for(uint64_t cat_id, int* decider_out) {
    const int d = team_owners_active() ? decider_of(cat_id) : (int)kHostPeer;
    if (decider_out) *decider_out = d;
    if (d == (int)self_peer()) return g.is_client ? Gate::Request : Gate::Run;
    return Gate::Refuse;
}

// Un nom affichable dans un champ du jeu : [ et ] seraient des balises.
std::string display(std::string s) {
    for (char& c : s)
        if (c == '[') c = '(';
        else if (c == ']') c = ')';
        else if (c == '\n' || c == '\r') c = ' ';
    return s;
}

std::string decider_name(int decider) {
    return display(mm_peer_name((uint8_t)decider));
}

// Hote : il a pris la main sur l'etape ou en est l'evenement (voir host_apply).
bool host_took_over() { return !g.is_client && g.evt_takeover == g.evt_stage + 1; }

// --- injecting -------------------------------------------------------------
//
// Both are the game's own commit path with our index substituted, rather than a
// reimplementation of what the commit does. That matters: sub_140937F30 also
// sets MewDirector+1812 and calls sub_14091AA00, and the level-up callback also
// copy-constructs the LevelUpOption through sub_14037BBB0. Neither is something
// worth reproducing by hand.

bool inject_event(uint32_t index) {
    uint8_t* begin = nullptr; uint32_t count = 0;
    if (!read_options(g.world_event, kEvt_OptBegin, kEvt_OptEnd, begin, count)) return false;
    if (index >= count) return false;

    // sub_140937F30 reads only a1+8 and a1+16 and never touches the vftable
    // slot, so a bare three-qword block is a complete capture for it.
    void* cap[3] = { nullptr, g.world_event, begin + (uintptr_t)index * kOptStride };
    if (!g.evt_commit) return false;
    // Calling the spliced address re-enters our own hook, which sees `injecting`
    // and answers "run the original" -- so the game's commit runs exactly once
    // and we do not need the trampoline exposed outside mgmp_hooks.cpp.
    g.injecting = true;
    g.evt_commit(cap);
    g.injecting = false;
    return true;
}

bool inject_level(void* screen, uint32_t index) {
    uint8_t* begin = nullptr; uint32_t count = 0;
    if (!read_options(screen, kLvl_OptBegin, kLvl_OptEnd, begin, count)) return false;
    if (index >= count) return false;

    // sub_140386810's capture is {vftable, LevelUpScreen*, int index}; it reads
    // the index as a 32-bit signed at +16 and does the +240*index itself.
    struct Cap { void* vt; void* screen; int32_t index; int32_t pad; };
    Cap cap = { nullptr, screen, (int32_t)index, 0 };
    if (!g.lvl_click) return false;
    g.injecting = true;
    g.lvl_click(&cap);
    g.injecting = false;
    return true;
}

// Mewgenics Multi : le chat d'un evenement, par le chemin du jeu -- le CatSelector
// montre ce chat (CatSelector::select), puis son « ok » (le lambda de setupCatChoice
// ne lit que capture+8, le WorldEvent, et capture+0x10, le MewDirector : il pose
// WorldEvent+0x128, note le chat parmi ceux des derniers evenements, puis relance
// le clip).
bool inject_catpick(void* we, uint64_t key) {
    void* sel = nullptr;
    void* dir = (void*)director();
    if (!we || !key || !dir || !g.catsel || !g.o_catpick) return false;
    if (!mem_read((const uint8_t*)we + kEvt_Selector, &sel, sizeof(sel)) || !sel) return false;
    g.catsel(sel, key);
    if (catpick_key(we) != key) return false;
    void* cap[3] = { nullptr, we, dir };
    g.injecting = true;
    g.o_catpick(cap);              // l'original : notre crochet ne le voit pas
    g.injecting = false;
    catpick_closed(we);
    return true;
}

// --- Mewgenics Multi : envoyer ------------------------------------------------

ChoiceMsg to_msg(const Decision& d, uint8_t flags) {
    ChoiceMsg m{};
    m.kind      = d.kind;
    m.index     = d.index;
    m.count     = d.count;
    m.aux       = d.aux;
    m.node_seed = follow_here_seed();
    m.cat_id    = d.cat_id;
    m.serial    = d.serial;
    m.flags     = flags;
    memcpy(m.name, d.name, sizeof(m.name));
    m.name[sizeof(m.name) - 1] = '\0';
    return m;
}

// Hote : le choix part chez tous les invites.
bool publish(const Decision& d) {
    const ChoiceMsg m = to_msg(d, 0);
    if (!net_send_choice(m)) {
        log_line("CHOICE", "!! %s choice %u could not be sent", kind_name(d.kind), d.index);
        return false;
    }
    ++g.sent;
    g.chose_here = true;   // Mewgenics Multi : voir choice_made_in_node
    log_line("CHOICE", "-> %s option %u/%u (type %u '%s') cat %016llx", kind_name(d.kind), d.index,
             d.count, d.aux, d.name, (unsigned long long)d.cat_id);
    return true;
}

State::Pending& pend_of(uint8_t kind) { return g.pend[kind < kChoiceKinds ? kind : kChoiceEvent]; }

// Invite : une demande de ce genre pour cet ecran attend encore sa reponse.
bool pending(uint8_t kind, const void* screen, uint64_t cat, uint64_t now) {
    const State::Pending& p = pend_of(kind);
    return p.screen == screen && p.cat == cat && now - p.at < kPendingMs;
}

// Invite : la reponse a sa demande est arrivee (un choix de ce genre pour ce
// chat, joue ou laisse ; le chat d'un evenement : quel qu'il soit, le choix du
// chat est fait).
void answered(const Decision& d) {
    State::Pending& p = pend_of(d.kind);
    if (p.cat == d.cat_id || d.kind == kChoiceEventCat) p = State::Pending{};
}

// Invite proprietaire : la demande part chez l'hote. Tant qu'elle attend sa
// reponse (kPendingMs au plus), un autre clic du meme genre sur le meme ecran est
// retenu, et une relance l'est derriere une option demandee : arrivee chez l'hote
// apres que l'option a engage son ecran, elle attendrait le prochain ecran de ce
// chat et s'y jouerait. (Une option derriere une relance part : l'hote les joue
// dans l'ordre, l'option d'avant la relance garde sa place.)
void request(const Decision& d, const void* screen) {
    const uint64_t now = GetTickCount64();
    const State::Pending& cp = pend_of(kChoiceEventCat);
    const bool cat_waiting = d.kind == kChoiceEventCat && cp.screen == screen && now - cp.at < kPendingMs;
    if (pending(d.kind, screen, d.cat_id, now) || cat_waiting ||
        (d.kind == kChoiceReroll && pending(kChoiceLevelUp, screen, d.cat_id, now))) {
        log_line_lvl(LogLevel::Trace, "CHOICE", "a %s click waits for the answer to this player's request",
                     kind_name(d.kind));
        return;
    }
    const ChoiceMsg m = to_msg(d, kChoiceFlagRequest);
    if (!net_send_choice(m)) {
        log_line("CHOICE", "!! the request for %s option %u could not reach the host",
                 kind_name(d.kind), d.index);
        return;
    }
    State::Pending& p = pend_of(d.kind);
    p.screen = screen;
    p.cat    = d.cat_id;
    p.at     = now;
    ++g.requested;
    log_line("CHOICE", "-> asking the host for %s option %u/%u ('%s') on this player's cat %016llx",
             kind_name(d.kind), d.index, d.count, d.name, (unsigned long long)d.cat_id);
}

void refused_click(const char* what, uint64_t cat_id, int decider) {
    ++g.swallowed;
    log_line("CHOICE", "suppressed a local %s click -- cat %016llx is decided by %s", what,
             (unsigned long long)cat_id, mm_peer_name((uint8_t)decider).c_str());
}

// --- Mewgenics Multi : les AbilityChooser ------------------------------------

Chooser* chooser_find(const void* p) {
    for (uint32_t i = 0; i < kMaxChoosers; ++i)
        if (g.choosers[i].ptr == p && p) return &g.choosers[i];
    return nullptr;
}

Chooser* chooser_add(void* p) {
    if (Chooser* c = chooser_find(p)) { *c = Chooser{}; c->ptr = p; return c; }
    uint32_t slot = 0;
    for (uint32_t i = 0; i < kMaxChoosers; ++i) {
        if (!g.choosers[i].ptr) { slot = i; break; }
        if (g.choosers[i].seen < g.choosers[slot].seen) slot = i;
    }
    g.choosers[slot] = Chooser{};
    g.choosers[slot].ptr = p;
    return &g.choosers[slot];
}

void choosers_expire(uint64_t now) {
    for (uint32_t i = 0; i < kMaxChoosers; ++i)
        if (g.choosers[i].ptr && now - g.choosers[i].seen > kChooserLostMs) g.choosers[i] = Chooser{};
}

bool chooser_dying(const void* chooser) {
    void* scene = nullptr;
    uint8_t dying = 1;
    if (!mem_read((const uint8_t*)chooser + kAbl_Scene, &scene, sizeof(scene)) || !scene) return true;
    if (!mem_read((const uint8_t*)scene + kScene_Dying, &dying, 1)) return true;
    return dying != 0;
}

// D'ou vient un AbilityChooser : la fonction qui a appele son init. Les plages
// sont celles de ce build (signatures epinglees) ; un autre build : « autre »,
// qui reste local (le pire est l'ancien comportement, jamais un ecran fige).
uint8_t chooser_context(uintptr_t ret) {
    if (!g.base || ret < g.base || !resolve_is_pinned_build()) return kCtxOther;
    const uintptr_t rva = ret - g.base;
    static const struct { uint32_t b, e; uint8_t ctx; } kFrom[] = {
        { 0x386610, 0x3868F2, kCtxLevelUp },   // select_option : une capacite (lambda_1)
        { 0x386900, 0x386BE2, kCtxLevelUp },   //   un passif (lambda_2)
        { 0x386BF0, 0x386ED2, kCtxLevelUp },   //   l'autre passif (lambda_3)
        { 0x939CD0, 0x939FC1, kCtxEvent   },   // evenement : WORLDEVENT_ENDBUTTON_REPLACEABILITY
        { 0x939FD0, 0x93A2C1, kCtxEvent   },
        { 0x350550, 0x3507FC, kCtxItem    },   // un livre de l'ecran des objets
        { 0x350800, 0x350AAC, kCtxItem    },
        { 0x350AB0, 0x350D5C, kCtxItem    },
    };
    for (const auto& r : kFrom)
        if (rva >= r.b && rva < r.e) return r.ctx;
    return kCtxOther;
}

bool chooser_replicated(const Chooser* c) {
    return c && (c->ctx == kCtxLevelUp || c->ctx == kCtxEvent);
}

// Hote : un AbilityChooser de montee de niveau de ce chat est ouvert et attend son
// choix. Le sien s'ouvre toujours avant celui de l'invite (l'hote joue l'option
// avant de la publier) : une demande qui n'en trouve pas vient d'un choix deja
// fait. (Celui d'un evenement, chacun l'ouvre de son cote : sa demande attend.)
bool levelup_chooser_waiting(uint64_t cat_id) {
    // (Pas de delai ici : l'hote en pause n'a plus d'update depuis longtemps, son
    // choix n'en attend pas moins. Un AbilityChooser perdu est oublie par
    // choosers_expire, au prochain update d'un autre.)
    for (uint32_t i = 0; i < kMaxChoosers; ++i) {
        const Chooser& c = g.choosers[i];
        if (c.ptr && c.cat_id == cat_id && c.ctx == kCtxLevelUp && !c.picked) return true;
    }
    return false;
}

// L'empreinte d'une liste d'options d'evenement : son tableau, sa taille, ses noms.
uint64_t event_fp(const uint8_t* begin, uint32_t count) {
    uint64_t h = 1469598103934665603ull;
    auto mix = [&h](uint64_t v) { for (int i = 0; i < 8; ++i) { h ^= (v >> (8 * i)) & 0xFF; h *= 1099511628211ull; } };
    mix((uint64_t)(uintptr_t)begin);
    mix(count);
    for (uint32_t i = 0; i < count; ++i) {
        char nm[48] = {};
        option_name(kChoiceEvent, begin + (uintptr_t)i * kOptStride, nm, sizeof(nm));
        for (const char* p = nm; *p; ++p) mix((uint8_t)*p);
        mix(0xFF);
    }
    return h;
}

// --- Mewgenics Multi : appliquer un choix -------------------------------------
//
// Commun a l'invite (un choix publie par l'hote) et a l'hote (la demande d'un
// invite pour son chat). `screen` est l'ecran du genre du choix, vivant : chaque
// appel vient de son update.
Apply try_apply(Decision& d, void* screen) {
    switch (d.kind) {
    case kChoiceEvent: {
        uint8_t* begin = nullptr; uint32_t count = 0;
        if (!read_options(screen, kEvt_OptBegin, kEvt_OptEnd, begin, count)) {
            if (!d.warned) {
                d.warned = true;
                log_line("CHOICE", "holding an event choice %u -- this peer's option list is not built yet", d.index);
            }
            return Apply::Wait;
        }
        if (!g.is_client) {
            // Mewgenics Multi (hote) : la demande d'un invite. Faite sur une etape
            // deja decidee (le compte des choix de la case a avance depuis), elle
            // n'est jamais jouee sur la suivante -- laissee tout de suite, sans
            // attendre des options qui ne reviendront peut-etre pas ; le chat de
            // l'evenement doit etre celui de la demande (set_subject peut l'avoir change).
            if (d.serial != g.evt_stage) {
                log_line("CHOICE", "!! an event request made after %u choice(s) arrived after %u -- refusing, %s",
                         d.serial, g.evt_stage, d.serial < g.evt_stage ? "that step was already decided"
                                                                       : "this peer is behind it");
                return Apply::Drop;
            }
            if (event_subject_id(screen) != d.cat_id) {
                log_line("CHOICE", "!! an event request for cat %016llx arrived on an event about cat %016llx -- refusing",
                         (unsigned long long)d.cat_id, (unsigned long long)event_subject_id(screen));
                return Apply::Drop;
            }
        }
        // Mewgenics Multi : les options a l'ecran. La liste existe des WorldEvent::init,
        // mais a la page du titre (ou du choix du chat) le choix ferait sauter le clip
        // a la suite sans passer par elles : il attend que le joueur y arrive. Un
        // choix joue remet +0x19F9 a zero tout de suite (0x14091EA80) : celui d'apres
        // attend aussi que setupActionChoice ait montre l'etape suivante.
        const bool gate = resolve_is_pinned_build();     // l'offset ne vaut que pour ce build
        if (gate && !event_choosing(screen)) {
            if (!d.warned) {
                d.warned = true;
                log_line("CHOICE", "holding an event choice %u -- this peer's event is not showing its options yet",
                         d.index);
            }
            return Apply::Hold;
        }
        // Sans cet offset (un autre build) : juste apres un choix, la liste est
        // peut-etre encore celle de l'etape choisie -- le suivant attend qu'elle change
        // (kEventSettleMs au plus).
        const uint64_t fp  = event_fp(begin, count);
        const uint64_t now = GetTickCount64();
        if (!gate && g.evt_done_at && fp == g.evt_done_fp) {
            if (now - g.evt_done_at < kEventSettleMs) return Apply::Wait;
            if (!d.warned) {
                d.warned = true;
                log_line("CHOICE", "!! the event's option list did not change in %u s after the last choice --"
                                   " applying choice %u on it anyway", (unsigned)(kEventSettleMs / 1000), d.index);
            }
        }
        // Count first: if the two lists are different lengths the index means
        // nothing, and saying so beats picking the wrong option.
        if (d.count && d.count != count) {
            log_line("CHOICE", "!! the event choice was made on %u option(s), this peer built %u -- refusing to"
                               " choose; the two screens are not the same screen", d.count, count);
            return Apply::Drop;
        }
        if (d.index >= count) {
            log_line("CHOICE", "!! event option %u chosen but this peer has only %u -- refusing", d.index, count);
            return Apply::Drop;
        }
        // Then the name: reported and obeyed on a guest (the host DID press that
        // button) ; chez l'hote, une demande dont l'option n'est pas la sienne est
        // refusee.
        char mine[48] = {};
        option_name(kChoiceEvent, begin + (uintptr_t)d.index * kOptStride, mine, sizeof(mine));
        if (d.name[0] && strcmp(mine, d.name) != 0) {
            log_line("CHOICE", "!! event option %u is '%s' here but '%s' where it was chosen%s", d.index, mine, d.name,
                     g.is_client ? " -- taking it anyway, the lists are the same length"
                                 : " -- refusing the player's request");
            if (!g.is_client) return Apply::Drop;
        }
        g.world_event = screen;
        if (!inject_event(d.index)) {
            log_line("CHOICE", "!! failed to apply the event choice %u", d.index);
            return Apply::Drop;
        }
        g.evt_done_at = now;
        g.evt_done_fp = fp;
        ++g.evt_stage;
        log_line("CHOICE", "applied an event choice: option %u/%u ('%s')", d.index, count, mine);
        d.count = count;
        return Apply::Applied;
    }
    case kChoiceEventCat: {
        // Mewgenics Multi : le chat d'un evenement. Son choix doit etre ouvert ici
        // (setupCatChoice a tourne, le chat n'est pas encore pose).
        if (!catpick_replicated()) {
            log_line("CHOICE", "!! the event's cat was chosen elsewhere, but this peer cannot replay it (cat selector,"
                               " cat-choice hooks or game build)");
            return Apply::Drop;
        }
        if (!catpick_open_on(screen)) {
            if (catpick_done_on(screen)) {
                log_line("CHOICE", "dropped the choice of cat %016llx -- this event's cat is already chosen",
                         (unsigned long long)d.cat_id);
                return Apply::Drop;
            }
            if (!d.warned) {
                d.warned = true;
                log_line("CHOICE", "holding the choice of cat %016llx for the event -- its cat choice is not open here yet",
                         (unsigned long long)d.cat_id);
            }
            return Apply::Hold;
        }
        // Chez un invite, un chat qui ne se rejoue pas (pas dans son equipe, le jeu
        // ne le pose pas) : il choisit le sien, comme avant -- jamais un ecran fige.
        const uint64_t key = run_cat_key(d.cat_id);
        if (!key) {
            log_line("CHOICE", "!! cat %016llx is not in this peer's team -- the event's cat is not chosen here%s",
                     (unsigned long long)d.cat_id, g.is_client ? " ; this player picks one locally" : "");
            if (g.is_client) g.catpick.local = true;
            return Apply::Drop;
        }
        g.world_event = screen;
        if (!inject_catpick(screen, key) || event_subject_id(screen) != d.cat_id) {
            log_line("CHOICE", "!! failed to send cat %016llx into the event%s", (unsigned long long)d.cat_id,
                     g.is_client && catpick_open_on(screen) ? " ; this player picks one locally" : "");
            if (g.is_client) g.catpick.local = true;
            return Apply::Drop;
        }
        log_line("CHOICE", "applied the event's cat: %016llx ('%s')", (unsigned long long)d.cat_id, d.name);
        return Apply::Applied;
    }
    case kChoiceLevelUp:
    case kChoiceReroll: {
        if (d.cat_id && level_cat_id(screen) != d.cat_id) return Apply::Wait;   // pas encore son ecran
        // Chez l'hote, une demande faite sur un ecran plus ancien de ce chat (deja
        // decide : CatData+0xC30 a grandi depuis) n'est jamais jouee sur celui-ci.
        // (Plus recent : le sien n'est pas encore la, elle l'attend.)
        if (!g.is_client && d.serial) {
            const uint32_t here = level_serial(screen);
            if (here && d.serial < here) {
                log_line("CHOICE", "!! a %s request made on level-up screen #%u of cat %016llx arrived on screen #%u"
                                   " -- refusing, that screen was already decided", kind_name(d.kind), d.serial,
                         (unsigned long long)d.cat_id, here);
                return Apply::Drop;
            }
            // Plus recent : son ecran vient (quelques images) ; au-dela, une
            // divergence -- elle se joue comme avant plutot que d'attendre toujours.
            if (here && d.serial > here && GetTickCount64() - d.since < kAheadWaitMs) return Apply::Wait;
        }
        // select_option no-ops once LevelUpScreen+792 is set, and a reroll on a
        // committed screen would land under an open AbilityChooser: the choice
        // waits for the screen to come back (a cancelled chooser resets it).
        if (level_committed(screen)) return Apply::Wait;
        uint8_t* begin = nullptr; uint32_t count = 0;
        if (!read_options(screen, kLvl_OptBegin, kLvl_OptEnd, begin, count)) {
            if (!d.warned) {
                d.warned = true;
                log_line("CHOICE", "holding a %s choice -- the option list is not built yet", kind_name(d.kind));
            }
            return Apply::Wait;
        }
        if (d.count && d.count != count) {
            // Chez l'hote, l'option demandee par un invite juste apres la relance
            // qu'il avait demandee : la relance ajoute ses options a la fin, celles
            // d'avant gardent leur place -- le nom et le type, verifies plus bas,
            // disent si c'est bien elle.
            const bool before_reroll = !g.is_client && d.kind == kChoiceLevelUp && d.count < count &&
                                       d.index < d.count;
            if (!before_reroll) {
                log_line("CHOICE", "!! the %s choice was made on %u level-up option(s), this peer has %u --"
                                   " refusing; the two screens are not the same screen",
                         kind_name(d.kind), d.count, count);
                return Apply::Drop;
            }
        }
        if (d.kind == kChoiceReroll) {
            // L'empreinte : la relance a-t-elle ete demandee sur CET ecran ? Chez
            // l'hote, une demande faite sur l'ecran d'avant du meme chat (deja
            // decide) est laissee ; chez un invite, le choix publie est suivi.
            char first[48] = {};
            option_name(kChoiceLevelUp, begin, first, sizeof(first));
            if (d.name[0] && strcmp(d.name, "reroll") != 0 && strcmp(first, d.name) != 0) {
                log_line("CHOICE", "!! a reroll made on the screen whose first option is '%s' arrived on one whose"
                                   " first option is '%s'%s", d.name, first,
                         g.is_client ? " -- taking it anyway" : " -- refusing, that screen was already decided");
                if (!g.is_client) return Apply::Drop;
            }
            if (level_rerolls_left(screen) <= 0) {
                log_line("CHOICE", "!! a reroll arrived for a level-up screen with no reroll left -- refusing");
                return Apply::Drop;
            }
            if (!g.o_reroll) return Apply::Drop;
            g.o_reroll(screen);     // l'original : notre crochet ne le voit pas
            log_line("CHOICE", "applied a level-up reroll (%u option(s) before it)", count);
            return Apply::Applied;
        }
        if (d.index >= count) {
            log_line("CHOICE", "!! level-up option %u chosen but this peer has only %u -- refusing",
                     d.index, count);
            return Apply::Drop;
        }
        const uint8_t* entry = begin + (uintptr_t)d.index * kOptStride;
        char mine[48] = {};
        option_name(kChoiceLevelUp, entry, mine, sizeof(mine));
        const uint32_t mine_type = option_type(entry);
        const bool same = (!d.name[0] || strcmp(mine, d.name) == 0) && (!d.aux || mine_type == d.aux);
        if (!same) {
            log_line("CHOICE", "!! level-up option %u is '%s' (type %u) here but '%s' (type %u) where it"
                               " was chosen%s", d.index, mine, mine_type, d.name, d.aux,
                     g.is_client ? " -- taking it anyway, the lists are the same length"
                                 : " -- refusing the player's request");
            if (!g.is_client) return Apply::Drop;
        }
        g.level_screen = screen;
        if (!inject_level(screen, d.index)) {
            log_line("CHOICE", "!! failed to apply level-up choice %u", d.index);
            return Apply::Drop;
        }
        log_line("CHOICE", "applied level-up choice: option %u/%u ('%s')", d.index, count, mine);
        d.count = count;            // l'hote le publie avec SA liste (voir before_reroll)
        return Apply::Applied;
    }
    case kChoiceAbility: {
        Chooser* c = chooser_find(screen);
        if (!c || !chooser_replicated(c)) return Apply::Wait;
        if (d.cat_id && c->cat_id != d.cat_id) return Apply::Wait;
        if (!g.is_client) {
            // Chez l'hote : la demande est-elle pour CE choix ? Un evenement et une
            // montee de niveau ne se melangent pas, et un choix de montee de niveau
            // d'un ecran plus ancien de ce chat est deja decide.
            if (d.aux && d.aux != c->ctx) {
                log_line("CHOICE", "!! an ability request made on another kind of ability choice -- refusing");
                return Apply::Drop;
            }
            const uint32_t here = c->ctx == kCtxLevelUp ? cat_levelups(c->cat) : 0;
            if (d.serial && here && d.serial < here) {
                log_line("CHOICE", "!! an ability request made on level-up screen #%u arrived on screen #%u -- refusing,"
                                   " that one was already decided", d.serial, here);
                return Apply::Drop;
            }
        }
        if (c->picked || chooser_dying(screen)) return Apply::Wait;   // celui d'apres viendra
        if (!g.o_pick) return Apply::Drop;
        c->picked = true;
        g.o_pick(screen, (int32_t)d.index);   // l'original : notre crochet ne le voit pas
        log_line("CHOICE", "applied an ability choice: %d", (int32_t)d.index);
        return Apply::Applied;
    }
    default:
        return Apply::Drop;
    }
}

// Invite : la tete de la file, si elle est du genre de cet ecran. Plusieurs dans
// la meme image si elles s'enchainent (une relance puis le choix).
void guest_apply(uint8_t screen_kind, void* screen) {
    const uint64_t now = GetTickCount64();
    while (!g.queue.empty()) {
        Decision& d = g.queue.front();

        // NODE IDENTITY FIRST, BEFORE THE COUNT AND BEFORE THE NAME. A mismatch
        // HOLDS, it does not drop: the host publishes the instant it clicks,
        // routinely before this peer has entered that node at all. Deciding a
        // choice is stale is choice_on_node_skipped's job.
        if (d.node_seed && g.here_seed && d.node_seed != g.here_seed) {
            if (!d.warned) {
                d.warned = true;
                log_line("CHOICE", "holding the host's %s choice %u -- it was made on node %016llx and this"
                                   " peer is still in %016llx", kind_name(d.kind), d.index,
                         (unsigned long long)d.node_seed, (unsigned long long)g.here_seed);
            }
            return;
        }
        // Un choix pour un autre genre d'ecran attend le sien. Mais s'il retient un
        // choix pour CET ecran, plus loin dans la file, son delai court : un choix
        // dont l'ecran ne vient jamais chez ce joueur retiendrait sinon tous ceux
        // d'apres, pour toujours. (S'il ne retient rien -- l'ecran d'un evenement
        // fini qu'on lit encore --, il attend sans compter.)
        if (screen_kind_of(d.kind) != screen_kind) {
            bool in_the_way = false;
            for (size_t i = 1; i < g.queue.size() && !in_the_way; ++i) {
                const Decision& e = g.queue[i];
                in_the_way = screen_kind_of(e.kind) == screen_kind &&
                             (!e.node_seed || !g.here_seed || e.node_seed == g.here_seed);
            }
            if (!in_the_way) return;
        }
        const Apply r = screen_kind_of(d.kind) == screen_kind ? try_apply(d, screen) : Apply::Wait;
        Apply rr = r;
        if (r == Apply::Hold) {
            // Son ecran est la, le joueur n'y est pas encore (il lit le titre) : le
            // delai d'abandon ne court pas -- 5 min au plus (un ecran qui n'y vient
            // jamais retiendrait sinon tous les choix d'apres).
            if (!d.held) d.held = now;
            if (now - d.held < kHoldCapMs) { d.blocked = 0; return; }
            rr = Apply::Wait;
        }
        if (rr == Apply::Wait) {
            if (!d.blocked) d.blocked = now;
            if (now - d.blocked > kBlockedDropMs) {
                log_line("CHOICE", "!! the host's %s choice %u never fit this peer's screen in %u s"
                                   " (cat %016llx) -- dropped; the two runs may differ here",
                         kind_name(d.kind), d.index, (unsigned)(kBlockedDropMs / 1000),
                         (unsigned long long)d.cat_id);
                if (d.kind == kChoiceEvent) ++g.evt_stage;   // l'hote en est a l'etape d'apres
                answered(d);
                g.queue.pop_front();
                continue;
            }
            return;
        }
        const bool event = screen_kind_of(d.kind) == kChoiceEvent;
        if (r == Apply::Applied) ++g.applied;
        else if (d.kind == kChoiceEvent) ++g.evt_stage;     // laisse : l'hote, lui, l'a joue
        answered(d);
        g.queue.pop_front();
        // Un evenement batit son etape suivante apres coup : le choix d'apres
        // attend au moins l'image suivante (et sa liste neuve, voir try_apply).
        if (event && r == Apply::Applied) return;
    }
}

// Hote : une option prise engage l'ecran de ce chat, une capacite prise ferme son
// AbilityChooser -- les autres demandes pour eux, faites avant que l'invite ne le
// voie, n'ont plus d'ecran : elles attendraient le suivant de ce chat (deux
// montees de niveau de suite) et s'y joueraient. (Une relance prise ne change que
// la liste : l'option demandee derriere elle reste.)
void drop_siblings(const Decision& done) {
    if (done.kind != kChoiceLevelUp && done.kind != kChoiceAbility) return;
    for (auto it = g.queue.begin(); it != g.queue.end();) {
        const bool same = it->cat_id == done.cat_id &&
                          (done.kind == kChoiceAbility ? it->kind == kChoiceAbility
                                                       : screen_kind_of(it->kind) == kChoiceLevelUp);
        if (!same) { ++it; continue; }
        log_line("CHOICE", "dropped %s's %s request %u -- made on the screen that was just decided",
                 mm_peer_name(it->from).c_str(), kind_name(it->kind), it->index);
        it = g.queue.erase(it);
    }
}

// Hote : la plus ancienne demande d'invite pour ce chat et cet ecran.
void host_apply(uint8_t screen_kind, void* screen) {
    const uint64_t now = GetTickCount64();
    uint64_t cat = 0;
    if (screen_kind == kChoiceAbility)     cat = chooser_find(screen) ? chooser_find(screen)->cat_id : 0;
    else if (screen_kind == kChoiceEvent)  cat = event_subject_id(screen);   // 0 : pas encore pose
    else                                   cat = level_cat_id(screen);
    for (auto it = g.queue.begin(); it != g.queue.end();) {
        Decision& d = *it;
        const bool stale = (d.node_seed && g.here_seed && d.node_seed != g.here_seed) ||
                           now - d.since > kRequestLifeMs;
        if (stale) {
            log_line("CHOICE", "dropped %s's %s request %u -- %s", mm_peer_name(d.from).c_str(),
                     kind_name(d.kind), d.index,
                     now - d.since > kRequestLifeMs ? "its screen never came" : "made on another node");
            it = g.queue.erase(it);
            continue;
        }
        if (screen_kind_of(d.kind) != screen_kind) { ++it; continue; }
        if (d.kind == kChoiceEvent && cat && d.cat_id != cat) {
            // L'evenement parle d'un autre chat ici : leurs choix du chat different.
            log_line("CHOICE", "dropped %s's event request %u -- it was made for cat %016llx, this event is about"
                               " cat %016llx", mm_peer_name(d.from).c_str(), d.index, (unsigned long long)d.cat_id,
                     (unsigned long long)cat);
            it = g.queue.erase(it);
            continue;
        }
        // (Le chat d'un evenement : n'importe lequel des siens, tant que son choix est ouvert.)
        if (d.kind != kChoiceEventCat && (!cat || d.cat_id != cat)) { ++it; continue; }
        // Toujours le sien ? (Il a pu partir : l'hote decide alors.)
        if (decider_of(d.cat_id) != (int)d.from) {
            log_line("CHOICE", "dropped %s's %s request -- that player no longer decides for cat %016llx",
                     mm_peer_name(d.from).c_str(), kind_name(d.kind), (unsigned long long)d.cat_id);
            it = g.queue.erase(it);
            continue;
        }
        const Apply r = try_apply(d, screen);
        if (r == Apply::Hold) {
            // L'hote n'en est pas encore a la page du choix (il lit le titre) : la
            // demande reste vivante (5 min au plus), son ordre est garde.
            if (!d.held) d.held = now;
            if (now - d.held < kHoldCapMs) d.since = now;
            return;
        }
        if (r == Apply::Wait) return;           // l'ordre de ce chat est garde
        if (r == Apply::Drop && d.kind == kChoiceEvent && d.serial >= g.evt_stage &&
            event_subject_id(screen) == d.cat_id) {
            // La demande du proprietaire pour CETTE etape ne se joue pas ici (les deux
            // listes different) : l'hote, qui retient ses propres clics, prend la main
            // pour elle -- sinon personne ne pourrait plus choisir.
            g.evt_takeover = g.evt_stage + 1;
            log_line("CHOICE", "!! %s's choice for this step of the event cannot be played here -- the host takes it over",
                     mm_peer_name(d.from).c_str());
        }
        if (r == Apply::Applied) {
            ++g.applied;
            const Decision done = d;
            g.queue.erase(it);
            publish(done);
            drop_siblings(done);
        } else {
            g.queue.erase(it);
        }
        return;                                  // l'ecran a change : la suite a la prochaine image
    }
}

// Le titre (montee de niveau) ou le texte (choix d'une capacite) dit qui choisit,
// sur l'ecran d'un autre joueur.
void title_level(void* screen) {
    if (!team_owners_active()) return;
    const uint64_t cat = level_cat_id(screen);
    const void* opts = nullptr;
    mem_read((const uint8_t*)screen + kLvl_OptBegin, &opts, sizeof(opts));
    if (!cat) return;
    // Le meme ecran : pas a chaque image. Mais l'ecran d'apres peut reprendre
    // l'adresse de celui-ci, et son tableau d'options aussi (deux montees de
    // niveau du meme chat de suite) : son titre neuf serait laisse -- d'ou une
    // reecriture tous les kTitleRefreshMs.
    const uint64_t now = GetTickCount64();
    const bool same = g.titled_level == screen && g.titled_cat == cat && g.titled_opts == opts;
    if (same && now - g.titled_at < kTitleRefreshMs) return;
    int decider = 0;
    const bool other = gate_for(cat, &decider) == Gate::Refuse;
    // Ce joueur decide : rien a dire -- sauf s'il vient de prendre la main sur cet
    // ecran (son proprietaire a quitte l'aventure), ou « A X de choisir » reste.
    if (!other && !(same && g.titled_other)) return;
    void* panel = nullptr;
    if (!mem_read((const uint8_t*)screen + kLvl_Panel, &panel, sizeof(panel)) || !panel) return;
    g.titled_level = screen;
    g.titled_cat   = cat;
    g.titled_opts  = opts;
    g.titled_at    = now;
    g.titled_other = other;
    char buf[160];
    if (other) snprintf(buf, sizeof(buf), u8"À %s de choisir", decider_name(decider).c_str());
    else       snprintf(buf, sizeof(buf), u8"À toi de choisir");
    if (!mm_panel_set_text(panel, "title", buf) && !same)
        log_line("CHOICE", "!! could not write who decides on the level-up screen (no 'title' field)");
}

void title_chooser(void* chooser, Chooser* c) {
    if (!team_owners_active() || c->titled || !chooser_replicated(c)) return;
    int decider = 0;
    if (gate_for(c->cat_id, &decider) != Gate::Refuse) return;
    void* panel = nullptr;
    if (!mem_read((const uint8_t*)chooser + kAbl_Panel, &panel, sizeof(panel)) || !panel) return;
    c->titled = true;
    char buf[200];
    snprintf(buf, sizeof(buf), u8"À %s de choisir la capacité à remplacer.", decider_name(decider).c_str());
    if (!mm_panel_set_text(panel, "prompt", buf))
        log_line("CHOICE", "!! could not write who decides on the ability chooser (no 'prompt' field)");
}

// Qui decide, sur l'ecran de l'evenement. WorldEvent::setupTitle ecrit « title »
// par MenuPanel::set_text, qui n'ecrit rien quand le clip n'a pas de champ de ce
// nom -- en build 23 ce titre ne se lisait pas chez l'hote (sans doute pas de tel
// champ), et qui decide n'etait pas ecrit. Alors, quand le titre ne se lit pas,
// la ligne va sous le texte de l'evenement, « prompt », que le jeu remplit a
// chaque page (setupTitle : l'introduction, setupActionChoice : la question,
// setupResult : le resultat).
// Le texte du jeu est garde la premiere fois que ce module y ecrit, et remis
// quand il n'a plus rien a dire -- seulement si le champ montre encore notre
// texte : le jeu qui l'a remplace (le resultat du choix) garde le sien. Un autre
// WorldEvent : rien a remettre (son panneau est neuf).
// Le champ « montre notre texte » s'il est ce qui y a ete relu juste apres notre
// ecriture, ou s'il contient notre ligne : le setter du jeu pourrait reformater
// ce qu'il garde (la relecture du champ, +0xB8, n'a encore jamais ete vue en
// jeu), et un texte qui contient notre ligne n'est jamais pris pour celui du jeu
// (sinon la ligne s'ajouterait encore a chaque reecriture). Rien n'est reecrit
// tant que le champ montre notre texte.

void etitle_forget() {
    g.etitle_we = nullptr;
    g.etitle_on = false;
    g.etitle_field = nullptr;
    g.etitle_game.clear();
    g.etitle_ours.clear();
    g.etitle_written.clear();
    g.etitle_at = g.etitle_until = 0;
    g.etitle_other = false;
    g.etitle_unreadable = false;
    g.etitle_retry_at = 0;
}

bool etitle_appends() { return g.etitle_field && strcmp(g.etitle_field, "prompt") == 0; }

std::string etitle_full(const std::string& line) {
    if (!etitle_appends() || g.etitle_game.empty()) return line;
    return g.etitle_game + "\n\n" + line;
}

bool etitle_shows_ours(const std::string& cur) {
    if (!g.etitle_written.empty() && cur == g.etitle_written) return true;
    return !g.etitle_ours.empty() && cur.find(g.etitle_ours) != std::string::npos;
}

void etitle_write(void* we, const std::string& text, uint64_t now) {
    void* panel = event_panel(we);
    if (!panel || text.empty()) return;
    if (!g.etitle_on) {
        // Le texte du jeu d'abord, pour le remettre : illisible, on n'y touche pas
        // (notre texte resterait). Un titre vide n'est pas affiche : le texte.
        if (g.etitle_unreadable && now < g.etitle_retry_at) return;
        std::string game, why_title, why_prompt;
        const int t = mm_panel_read_text(panel, "title", game, &why_title);
        if (t == 1 && !game.empty()) {
            g.etitle_field = "title";
        } else if (mm_panel_read_text(panel, "prompt", game, &why_prompt) == 1) {
            g.etitle_field = "prompt";
        } else {
            if (!g.etitle_unreadable)
                log_line("CHOICE", "!! who decides cannot be written on the event screen -- title: %s, prompt: %s",
                         t == 1 ? "vide" : why_title.c_str(), why_prompt.c_str());
            g.etitle_unreadable = true;
            g.etitle_retry_at   = now + kTitleRetryMs;
            return;
        }
        if (!g.etitle_prev_written.empty() && game == g.etitle_prev_written) game = g.etitle_prev_game;
        g.etitle_game = game;
        g.etitle_ours.clear();
        g.etitle_written.clear();
        g.etitle_at = 0;
        g.etitle_on = true;
    }
    if (text == g.etitle_ours && now - g.etitle_at < kEventTextMs) return;
    g.etitle_at = now;
    if (!g.etitle_written.empty()) {
        std::string cur;
        if (mm_panel_read_text(panel, g.etitle_field, cur) != 1) return;   // ne se lit plus : rien
        if (etitle_shows_ours(cur)) {
            if (text == g.etitle_ours) return;           // deja la
        } else {
            g.etitle_game = cur;                         // le jeu l'a remplace (une autre page) : le sien
        }
    }
    const std::string full = etitle_full(text);
    const bool first = text != g.etitle_ours;
    g.etitle_ours = text;
    if (!mm_panel_set_text(panel, g.etitle_field, full)) {
        g.etitle_written = full;
        if (first) log_line("CHOICE", "!! could not write who decides on the event screen ('%s')", g.etitle_field);
        return;
    }
    // Ce que le champ garde vraiment.
    std::string back;
    g.etitle_written      = mm_panel_read_text(panel, g.etitle_field, back) == 1 ? back : full;
    g.etitle_prev_written = g.etitle_written;
    g.etitle_prev_game    = g.etitle_game;
}

void etitle_restore(void* we) {
    g.etitle_until = 0;
    if (!g.etitle_on) return;
    void* panel = event_panel(we);
    std::string cur;
    if (panel && mm_panel_read_text(panel, g.etitle_field, cur) == 1 && etitle_shows_ours(cur))
        mm_panel_set_text(panel, g.etitle_field, g.etitle_game);
    g.etitle_prev_written.clear();
    g.etitle_prev_game.clear();
    g.etitle_on = false;
    g.etitle_field = nullptr;
    g.etitle_ours.clear();
    g.etitle_written.clear();
    g.etitle_other = false;
}

// Un avis court dans le titre (le « ok » d'un chat qui n'est pas a ce joueur).
void etitle_notice(void* we, const std::string& text) {
    if (g.etitle_we != we) { etitle_forget(); g.etitle_we = we; }
    const uint64_t now = GetTickCount64();
    g.etitle_at = 0;                                   // reecrit tout de suite
    etitle_write(we, text, now);
    g.etitle_until = now + kNoticeMs;
}

// Chez qui ne decide pas, les options de l'evenement a l'ecran : « A X de choisir ».
// « A toi de choisir » quand ce joueur prend la main (le proprietaire est parti).
void title_event(void* we) {
    const uint64_t now = GetTickCount64();
    if (g.etitle_we != we) { etitle_forget(); g.etitle_we = we; }
    if (g.etitle_until) {
        if (now < g.etitle_until) return;              // l'avis reste
        etitle_restore(we);
    }
    std::string want;
    bool other = false;
    if (event_choosing(we)) {
        int decider = 0;
        other = gate_for(event_subject_id(we), &decider) == Gate::Refuse && !host_took_over();
        if (other) {
            char buf[160];
            snprintf(buf, sizeof(buf), u8"À %s de choisir", decider_name(decider).c_str());
            want = buf;
        } else if (g.etitle_on && (g.etitle_other || g.etitle_ours == u8"À toi de choisir")) {
            // Ce joueur a pris la main : « A toi de choisir » tant que les options
            // sont la (et remis si le jeu change le texte de la page).
            want = u8"À toi de choisir";
        }
    }
    if (want.empty()) {
        // Plus d'options a l'ecran (le resultat), ou ce joueur decide : le texte du
        // jeu revient.
        if (g.etitle_on) etitle_restore(we);
        return;
    }
    etitle_write(we, want, now);
    g.etitle_other = other;
}

// Mewgenics Multi : la derniere image ou un ecran de choix a tourne (sans le
// verrou : un horodatage, lu par mm_rungear sur le meme thread).
volatile uint64_t s_screen_tick = 0;

// --- Mewgenics Multi : les crochets de ce module (choice_binding) -------------

bool on_level_reroll(void* screen) {
    if (!g.on || !screen) return true;
    if (g.is_client && !savefile_in_coop()) return true;   // sa propre save
    Guard guard;
    g.level_screen = screen;
    if (level_rerolls_left(screen) <= 0) return true;      // le jeu ne fait rien
    // Un choix deja engage sur cet ecran (une capacite a remplacer ouverte
    // par-dessus, ou l'ecran qui se ferme) : chez les autres, la relance
    // attendrait un ecran libre qui ne revient pas -- personne ne relance.
    if (level_committed(screen)) {
        log_line("CHOICE", "ignored a reroll on a level-up screen whose choice is already made");
        return false;
    }
    uint8_t* begin = nullptr; uint32_t count = 0;
    read_options(screen, kLvl_OptBegin, kLvl_OptEnd, begin, count);
    Decision d;
    d.kind   = kChoiceReroll;
    d.count  = count;
    d.cat_id = level_cat_id(screen);
    d.serial = level_serial(screen);
    // Son empreinte : la premiere option de l'ecran (une relance ajoute les siennes
    // a la fin, elle ne change pas tant que l'ecran est le meme).
    if (begin && count) option_name(kChoiceLevelUp, begin, d.name, sizeof(d.name));
    else                snprintf(d.name, sizeof(d.name), "reroll");
    int decider = 0;
    switch (gate_for(d.cat_id, &decider)) {
        case Gate::Refuse:  refused_click("reroll", d.cat_id, decider); return false;
        case Gate::Request: request(d, screen); return false;
        case Gate::Run:     publish(d); return true;
    }
    return true;
}

void __fastcall h_LevelReroll(void* screen) {
    if (on_level_reroll(screen) && g.o_reroll) g.o_reroll(screen);
}

void __fastcall h_AbilityInit(void* self, void* scene, void* cat, uint64_t mode, uint64_t a5,
                              uint64_t a6, uint64_t a7, uint64_t a8, uint64_t a9, uint64_t a10) {
    const uintptr_t ret = (uintptr_t)_ReturnAddress();
    if (self && g.on) {
        Guard guard;
        Chooser* c = chooser_add(self);
        c->cat    = cat;
        c->cat_id = cat_id_of(cat);
        c->ctx    = chooser_context(ret);
        c->seen   = GetTickCount64();
        log_line_lvl(LogLevel::Trace, "CHOICE", "ability chooser opened (%s) for cat %016llx",
                     c->ctx == kCtxLevelUp ? "level-up" : c->ctx == kCtxEvent ? "event"
                     : c->ctx == kCtxItem ? "item" : "other", (unsigned long long)c->cat_id);
    }
    g.o_ablinit(self, scene, cat, mode, a5, a6, a7, a8, a9, a10);
}

bool on_ability_pick(void* chooser, int32_t index) {
    if (!g.on || !chooser) return true;
    if (g.is_client && !savefile_in_coop()) return true;   // sa propre save
    Guard guard;
    Chooser* c = chooser_find(chooser);
    if (!chooser_replicated(c)) return true;               // un livre d'objet : le sien, local
    if (c->picked) return true;                            // le jeu ignore les suivants
    Decision d;
    d.kind   = kChoiceAbility;
    d.index  = (uint32_t)index;
    d.cat_id = c->cat_id;
    d.aux    = c->ctx;                                     // montee de niveau ou evenement
    d.serial = c->ctx == kCtxLevelUp ? cat_levelups(c->cat) : 0;
    snprintf(d.name, sizeof(d.name), "%d", index);
    int decider = 0;
    switch (gate_for(d.cat_id, &decider)) {
        case Gate::Refuse:  refused_click("ability", d.cat_id, decider); return false;
        case Gate::Request: request(d, chooser); return false;
        case Gate::Run:
            publish(d);
            c->picked = true;
            return true;
    }
    return true;
}

void __fastcall h_AbilityPick(void* chooser, int32_t index) {
    if (on_ability_pick(chooser, index) && g.o_pick) g.o_pick(chooser, index);
}

void __fastcall h_AbilityUpdate(void* chooser) {
    // Le choix qui attend, AVANT l'update du jeu : l'ecran est vivant ici (c'est
    // lui qu'on met a jour) ; apres, sa fermeture a pu commencer.
    if (chooser && g.on && !(g.is_client && !savefile_in_coop())) {
        Guard guard;
        const uint64_t now = GetTickCount64();
        s_screen_tick = now;
        // Celui-ci d'abord : son update n'a peut-etre pas tourne depuis longtemps
        // (le menu de pause, une fenetre par-dessus), il n'en est pas moins vivant.
        Chooser* c = chooser_find(chooser);
        if (c) c->seen = now;
        choosers_expire(now);
        if (c) {
            if (chooser_replicated(c)) {
                if (g.is_client) guest_apply(kChoiceAbility, chooser);
                else             host_apply(kChoiceAbility, chooser);
                title_chooser(chooser, c);
            }
        }
    }
    g.o_ablupd(chooser);
}

// WorldEvent::setupCatChoice (appele par le clip de l'evenement, par son nom) :
// d'abord le jeu (le CatSelector, le « ok »), puis le choix du chat est ouvert --
// note meme hors session : une session qui commence sur cet ecran le connait.
void __fastcall h_EventCatChoice(void* we) {
    g.o_catchoice(we);
    if (!we) return;
    Guard guard;
    catpick_mark_open(we);
    if (g.on) log_line_lvl(LogLevel::Trace, "CHOICE", "the event's cat choice is open");
}

// Ce que fait le « ok » du choix du chat chez ce joueur.
enum class Pick { Local, Publish, Swallow };

// Chacun n'envoie que ses chats ; un invite le demande a l'hote, qui le joue et
// publie le chat que le jeu a pose (le premier pris l'emporte).
Pick on_event_cat_pick(void* cap, void** we_out) {
    void* we = nullptr;
    if (cap) mem_read((const uint8_t*)cap + 8, &we, sizeof(we));
    *we_out = we;
    if (!g.on || !cap || !we || g.injecting) return Pick::Local;
    if (g.is_client && !savefile_in_coop()) return Pick::Local;   // sa propre save
    if (!catpick_replicated()) return Pick::Local;                 // impossible a rejouer : chacun le sien, comme avant
    Guard guard;
    g.world_event = we;
    // Le bouton n'existe que pendant le choix du chat : un clic dit qu'il est ouvert
    // (setupCatChoice a pu tourner avant la session).
    if (!catpick_is(we)) catpick_mark_open(we);
    if (!g.catpick.open) {
        // Deja choisi (un double clic, ou le chat d'un autre joueur vient d'arriver).
        log_line("CHOICE", "ignored a cat choice on an event whose cat is already chosen");
        return Pick::Swallow;
    }
    if (g.catpick.local) {
        log_line("CHOICE", "the event's cat is picked locally -- the one chosen elsewhere could not be replayed here");
        return Pick::Local;
    }
    const uint64_t cat_id = cat_id_of(catsync_cat_by_key(catpick_key(we)));
    if (!cat_id) {
        // Un chat que ce joueur ne sait pas nommer. L'hote le joue et publie celui
        // que le jeu aura pose ; un invite le prend chez lui, plutot qu'un ecran fige.
        log_line("CHOICE", "!! the event's cat selector shows a cat this peer cannot name -- %s",
                 g.is_client ? "picked locally" : "played here, then the cat it sets is published");
        return g.is_client ? Pick::Local : Pick::Publish;
    }
    Decision d;
    d.kind   = kChoiceEventCat;
    d.cat_id = cat_id;
    snprintf(d.name, sizeof(d.name), "%016llx", (unsigned long long)cat_id);
    int decider = 0;
    switch (gate_for(cat_id, &decider)) {
    case Gate::Refuse: {
        refused_click("event cat", cat_id, decider);
        char buf[160];
        snprintf(buf, sizeof(buf), u8"Ce chat est à %s", decider_name(decider).c_str());
        etitle_notice(we, buf);
        return Pick::Swallow;
    }
    case Gate::Request:
        request(d, we);
        return Pick::Swallow;
    case Gate::Run:
        return Pick::Publish;
    }
    return Pick::Local;
}

void __fastcall h_EventCatPick(void* cap) {
    void* we = nullptr;
    const Pick p = on_event_cat_pick(cap, &we);
    if (p == Pick::Swallow) return;
    g.o_catpick(cap);
    if (!we) return;
    Guard guard;
    catpick_closed(we);
    if (p != Pick::Publish || !g.on) return;
    // Le chat que le jeu a pose (pas celui que montrait le CatSelector avant).
    Decision d;
    d.kind   = kChoiceEventCat;
    d.cat_id = event_subject_id(we);
    if (!d.cat_id) {
        log_line("CHOICE", "!! the event's ok set no cat this peer can name -- the others are not told");
        return;
    }
    snprintf(d.name, sizeof(d.name), "%016llx", (unsigned long long)d.cat_id);
    publish(d);
}

} // namespace

// ---------------------------------------------------------------------------

bool choice_binding(Target t, void** detour, void*** original) {
    switch (t) {
        case T_LevelReroll:   *detour = (void*)&h_LevelReroll;   *original = (void**)&g.o_reroll;  return true;
        case T_AbilityInit:   *detour = (void*)&h_AbilityInit;   *original = (void**)&g.o_ablinit; return true;
        case T_AbilityPick:   *detour = (void*)&h_AbilityPick;   *original = (void**)&g.o_pick;    return true;
        case T_AbilityUpdate: *detour = (void*)&h_AbilityUpdate; *original = (void**)&g.o_ablupd;  return true;
        case T_EventCatChoice: *detour = (void*)&h_EventCatChoice; *original = (void**)&g.o_catchoice; return true;
        case T_EventCatPick:   *detour = (void*)&h_EventCatPick;   *original = (void**)&g.o_catpick;   return true;
        default: return false;
    }
}

void choice_set_base(uintptr_t base) {
    g.base = base;
    g.resolved = false;
    g.evt_commit = nullptr;
    g.lvl_click  = nullptr;
    g.catsel     = nullptr;

    // An armed client that cannot inject is worse than a desync: it swallows
    // clicks it has no way to replace and the screen is permanently dead with
    // nothing to time out. So both call targets must resolve or the whole
    // feature disarms.
    const uintptr_t evt = addr_of(T_EventChoice);
    const uintptr_t lvl = addr_of_call(C_LevelUpClick);
    if (!evt || !lvl) {
        log_line("CHOICE", "!! %s did not resolve by signature"
                           " -- choice replication is OFF",
                 !evt ? "WorldEvent option commit" : "LevelUpScreen button click");
        return;
    }
    g.evt_commit = (fn_cap)evt;
    g.lvl_click  = (fn_cap)lvl;
    g.resolved = true;
    // Mewgenics Multi : montrer un chat dans le CatSelector d'un evenement, pour
    // rejouer le chat choisi ailleurs. Sans lui, chacun choisit le sien, comme avant.
    g.catsel = (fn_catsel)addr_of_call(C_CatSelectorSelect);
    if (!g.catsel)
        log_line("CHOICE", "!! CatSelector::select did not resolve -- each peer keeps choosing an event's cat on its own");
}

void choice_init() {
    if (!g.cs_ready) { InitializeCriticalSection(&g.cs); g.cs_ready = true; }
    g.on          = tune::kChoice;
    g.is_client   = (net_role() == NetRole::Client);
    g.world_event = nullptr;
    g.level_screen = nullptr;
    g.queue.clear();
    g.injecting = false;
    g.evt_stage = 0;
    g.evt_takeover = 0;
    catpick_forget();
    etitle_forget();
    if (!g.on) { log_line("CHOICE", "choice replication disabled by net_choice = 0"); return; }
    if (g.is_client && !g.resolved) {
        // Refusing is the safe direction. An armed client SWALLOWS every local
        // click on these screens, so one that cannot inject the host's choice
        // would not desync -- it would sit on a dead screen forever with no
        // way to press anything.
        g.on = false;
        log_line("CHOICE", "!! call targets unresolved -- choice replication is OFF."
                           " This peer keeps its own event and level-up clicks; expect"
                           " the two runs to diverge on the first event.");
        return;
    }
    log_line_lvl(LogLevel::Trace, "CHOICE", "armed -- %s",
             g.is_client ? "event options, level-ups and ability choices of this player's own cats (and the"
                           " event cats it sends) are asked of the host, the others' are taken from it"
                         : "publishing every choice; those about a guest's cat (event options, level-ups,"
                           " ability choices, the event cats it sends) come from that guest");
}

void choice_shutdown() {
    if (!g.on) return;
    log_line_lvl(LogLevel::Trace, "CHOICE",
             "done: %u sent, %u applied, %u requested, %u local click(s) suppressed",
             g.sent, g.applied, g.requested, g.swallowed);
    g.on = false;
    g.queue.clear();
    if (g.cs_ready) { DeleteCriticalSection(&g.cs); g.cs_ready = false; }
}

// --- the world event -------------------------------------------------------

bool choice_on_event_commit(void* cap) {
    if (!g.on || !cap) return true;
    if (g.injecting) return true;           // this is our own injected click

    Guard guard;

    // Mewgenics Multi : un invite dans SA save joue ses propres choix, et on
    // ne retient rien de ses ecrans.
    if (g.is_client && !savefile_in_coop()) return true;

    void* we    = nullptr;
    void* entry = nullptr;
    if (!mem_read((const uint8_t*)cap + 8,  &we,    sizeof(we)))    return true;
    if (!mem_read((const uint8_t*)cap + 16, &entry, sizeof(entry))) return true;
    g.world_event = we;

    uint8_t* begin = nullptr; uint32_t count = 0;
    const bool have = read_options(we, kEvt_OptBegin, kEvt_OptEnd, begin, count);
    const uintptr_t delta = have ? (uintptr_t)((uint8_t*)entry - begin) : 0;
    const bool in_list = have && delta % kOptStride == 0 && delta / kOptStride < count;

    // Mewgenics Multi : le proprietaire du chat de l'evenement decide (l'hote pour
    // un evenement sans chat, une recrue, ou sans equipe mixte).
    Decision d;
    d.kind   = kChoiceEvent;
    d.index  = in_list ? (uint32_t)(delta / kOptStride) : 0;
    d.count  = count;
    d.cat_id = event_subject_id(we);
    d.serial = g.evt_stage;
    if (in_list) option_name(kChoiceEvent, (const uint8_t*)entry, d.name, sizeof(d.name));

    int decider = 0;
    Gate gate = gate_for(d.cat_id, &decider);
    if (gate == Gate::Refuse && host_took_over()) gate = Gate::Run;   // la demande du proprietaire ne se joue pas ici
    switch (gate) {
    case Gate::Refuse:
        refused_click("event", d.cat_id, decider);
        title_event(we);
        return false;                        // do NOT run the original
    case Gate::Request:
        if (!in_list) {
            log_line("CHOICE", "!! the chosen event option is not an element of this event's array -- not asked");
            return false;
        }
        request(d, we);
        return false;
    case Gate::Run:
        break;
    }

    // Host: name the index, publish it, then let the click through.
    if (!have) {
        log_line("CHOICE", "!! could not read this event's option array -- the peer will"
                           " not be told which option was picked");
        ++g.evt_stage;
        return true;
    }
    if (!in_list) {
        log_line("CHOICE", "!! the chosen option is not an element of this event's array"
                           " -- not publishing");
        ++g.evt_stage;
        return true;
    }
    publish(d);
    // Mewgenics Multi : une demande d'invite pour l'etape suivante attendra la liste neuve.
    g.evt_done_at = GetTickCount64();
    g.evt_done_fp = event_fp(begin, count);
    ++g.evt_stage;
    return true;
}

bool choice_screen_recent(uint64_t within_ms) {
    const uint64_t t = s_screen_tick;
    return t && GetTickCount64() - t < within_ms;
}

void choice_on_event_update(void* world_event) {
    if (world_event) s_screen_tick = GetTickCount64();
    if (!g.on || !world_event) return;
    if (g.is_client && !savefile_in_coop()) return;   // Mewgenics Multi : sa propre save
    Guard guard;
    g.world_event = world_event;
    // Mewgenics Multi : les options a l'ecran, le choix du chat est passe.
    if (catpick_open_on(world_event) && resolve_is_pinned_build() && event_choosing(world_event))
        catpick_closed(world_event);
    if (g.is_client) guest_apply(kChoiceEvent, world_event);
    else             host_apply(kChoiceEvent, world_event);   // Mewgenics Multi : les demandes des invites
    title_event(world_event);
}

// --- the level-up screen ---------------------------------------------------

bool choice_on_level_select(void* screen, void* opt) {
    if (!g.on || !screen || !opt) return true;
    if (g.injecting) return true;

    if (g.is_client && !savefile_in_coop()) return true;   // Mewgenics Multi : sa propre save
    // Mewgenics Multi : un ecran deja engage (un double clic) -- select_option ne
    // fera rien. Publie ou demande, ce clic attendrait chez les autres le
    // prochain ecran de ce chat, et s'y appliquerait.
    if (level_committed(screen)) return true;
    Guard guard;
    g.level_screen = screen;

    // The option arrives as a COPY (the button callback copy-constructs it
    // through sub_14037BBB0), so the index cannot come from pointer arithmetic
    // the way the event's does. Match on content instead: type plus authored
    // name, which is the same identity the wire carries.
    uint8_t* begin = nullptr; uint32_t count = 0;
    const bool have = read_options(screen, kLvl_OptBegin, kLvl_OptEnd, begin, count);

    Decision d;
    d.kind   = kChoiceLevelUp;
    d.count  = count;
    d.cat_id = level_cat_id(screen);
    d.serial = level_serial(screen);
    option_name(kChoiceLevelUp, (const uint8_t*)opt, d.name, sizeof(d.name));
    d.aux    = option_type((const uint8_t*)opt);

    uint32_t found = UINT32_MAX;
    for (uint32_t i = 0; have && i < count; ++i) {
        const uint8_t* e = begin + (uintptr_t)i * kOptStride;
        if (option_type(e) != d.aux) continue;
        char nm[48] = {};
        option_name(kChoiceLevelUp, e, nm, sizeof(nm));
        if (strcmp(nm, d.name) == 0) { found = i; break; }
    }
    d.index = found;

    int decider = 0;
    switch (gate_for(d.cat_id, &decider)) {
    case Gate::Refuse:
        refused_click("level-up", d.cat_id, decider);
        return false;
    case Gate::Request:
        if (found == UINT32_MAX) {
            log_line("CHOICE", "!! the chosen level-up option (type %u '%s') matches none of the %u on"
                               " offer -- not asked", d.aux, d.name, count);
            return false;
        }
        request(d, screen);
        return false;
    case Gate::Run:
        if (!have) {
            log_line("CHOICE", "!! could not read the level-up option array -- the peer will"
                               " not be told which option was picked");
            return true;
        }
        if (found == UINT32_MAX) {
            log_line("CHOICE", "!! the chosen level-up option (type %u '%s') matches none of"
                               " the %u on offer -- not publishing", d.aux, d.name, count);
            return true;
        }
        publish(d);
        return true;
    }
    return true;
}

void choice_on_level_update(void* level_screen) {
    if (level_screen) s_screen_tick = GetTickCount64();
    if (!g.on || !level_screen) return;
    if (g.is_client && !savefile_in_coop()) return;   // Mewgenics Multi : sa propre save
    Guard guard;
    g.level_screen = level_screen;
    if (g.is_client) guest_apply(kChoiceLevelUp, level_screen);
    else             host_apply(kChoiceLevelUp, level_screen);
    title_level(level_screen);
}

// --- receiving -------------------------------------------------------------

void choice_on_message(uint8_t from, const ChoiceMsg& m) {
    if (!g.on) return;
    Guard guard;

    if (m.kind >= kChoiceKinds) {
        log_line("CHOICE", "!! a choice of unknown kind %u from peer %u -- ignored", (unsigned)m.kind, (unsigned)from);
        return;
    }
    Decision d;
    d.kind      = m.kind;
    d.index     = m.index;
    d.count     = m.count;
    d.aux       = m.aux;
    d.node_seed = m.node_seed;
    d.cat_id    = m.cat_id;
    d.serial    = m.serial;
    d.from      = from;
    d.since     = GetTickCount64();
    memcpy(d.name, m.name, sizeof(d.name));
    d.name[sizeof(d.name) - 1] = '\0';

    if (!g.is_client) {
        // Mewgenics Multi : la demande d'un invite pour un chat a lui. Sinon, le
        // message va de l'hote aux invites : un hote qui en recoit un parle a un
        // pair qui se croit hote aussi.
        if (!(m.flags & kChoiceFlagRequest)) {
            log_line("CHOICE", "!! received a %s choice while hosting -- both peers believe"
                               " they own the run", kind_name(m.kind));
            return;
        }
        if (!d.cat_id) {
            log_line("CHOICE", "!! %s asked for a %s choice without a cat -- those are the host's",
                     mm_peer_name(from).c_str(), kind_name(d.kind));
            return;
        }
        if (owner_of(d.cat_id) != (int)from) {
            log_line("CHOICE", "!! %s asked for a %s choice on cat %016llx, which is not theirs -- refused",
                     mm_peer_name(from).c_str(), kind_name(d.kind), (unsigned long long)d.cat_id);
            return;
        }
        if (d.kind == kChoiceAbility && d.aux == kCtxLevelUp && !levelup_chooser_waiting(d.cat_id)) {
            // Faite avant que l'invite ne voie le choix precedent pris : elle
            // attendrait le prochain AbilityChooser de ce chat et s'y jouerait.
            log_line("CHOICE", "dropped %s's ability request %d -- no ability choice of cat %016llx is waiting here"
                               " (already decided)", mm_peer_name(from).c_str(), (int32_t)d.index,
                     (unsigned long long)d.cat_id);
            return;
        }
        if (g.queue.size() >= kMaxQueue) {
            log_line("CHOICE", "!! too many requests waiting -- the oldest is dropped");
            g.queue.pop_front();
        }
        log_line("CHOICE", "<- %s asks for %s option %u/%u ('%s') on cat %016llx", mm_peer_name(from).c_str(),
                 kind_name(d.kind), d.index, d.count, d.name, (unsigned long long)d.cat_id);
        g.queue.push_back(d);
        return;
    }
    if (m.flags & kChoiceFlagRequest) return;     // une demande n'est jamais pour un invite

    // Mewgenics Multi : hors de la copie de l'aventure de l'hote, un choix de
    // l'hote ne concerne pas ce joueur -- sauf s'il est en train de la rejoindre :
    // alors il est garde, et applique une fois dans la copie (pas tout de suite).
    const bool outside = !savefile_in_coop();
    if (outside && !savefile_join_armed()) return;

    // A choice for a node mgmp_follow already gave up on. Refusing it here is
    // the other half of choice_on_node_skipped: that one catches the choice
    // that was already held when the node was discarded, this one catches the
    // choice that arrives afterwards.
    if (was_skipped(m.node_seed)) {
        log_line("CHOICE", "!! a %s choice arrived for node %016llx, which this peer"
                           " passed over without entering -- refusing it. The host"
                           " resolved that node and this peer did not.",
                 kind_name(d.kind), (unsigned long long)m.node_seed);
        return;
    }

    if (g.queue.size() >= kMaxQueue) {
        log_line("CHOICE", "!! %u choices already waiting -- the oldest (%s %u) is dropped",
                 (unsigned)g.queue.size(), kind_name(g.queue.front().kind), g.queue.front().index);
        g.queue.pop_front();
    }
    g.queue.push_back(d);
    log_line("CHOICE", "<- %s option %u/%u ('%s') on node %016llx, cat %016llx", kind_name(d.kind),
             d.index, d.count, d.name, (unsigned long long)m.node_seed, (unsigned long long)d.cat_id);
    // Appliquee par l'update de son ecran (au plus une image) : jamais dans un
    // pointeur retenu qui pourrait ne plus etre un ecran.
}

bool choice_made_in_node() { return g.chose_here; }

void choice_forget_screens() {
    Guard guard;
    g.world_event  = nullptr;
    g.level_screen = nullptr;
    g.titled_level = nullptr;
    catpick_forget();
    etitle_forget();
    for (uint32_t i = 0; i < kMaxChoosers; ++i) g.choosers[i] = Chooser{};
}

void choice_forget() {
    Guard guard;
    g.world_event  = nullptr;
    g.level_screen = nullptr;
    g.titled_level = nullptr;
    catpick_forget();
    etitle_forget();
    g.here_seed    = 0;
    if (!g.queue.empty())
        log_line("CHOICE", "dropped %u held choice(s) -- the host's save that just arrived already has"
                           " them", (unsigned)g.queue.size());
    g.queue.clear();
    for (uint32_t i = 0; i < kMaxChoosers; ++i) g.choosers[i] = Chooser{};
}

void choice_on_node_entered(uint64_t node_seed) {
    if (!g.on) return;
    Guard guard;
    const uint64_t left = g.here_seed;
    g.here_seed  = node_seed;
    g.chose_here = false;
    for (State::Pending& p : g.pend) p = State::Pending{};
    g.evt_done_at = 0;                    // un autre evenement : sa liste peut ressembler a l'ancienne
    if (left != node_seed) {
        // Mewgenics Multi : une autre case, un autre evenement -- ses choix se
        // comptent de zero, son choix du chat et son titre sont les siens.
        g.evt_stage    = 0;
        g.evt_takeover = 0;
        catpick_forget();
        etitle_forget();
    }

    // Mewgenics Multi (invite) : un choix de la case qu'il vient de quitter ne
    // trouvera plus son ecran ; en tete de file, il retiendrait tous ceux d'apres
    // (la file s'applique dans l'ordre, et un choix d'une autre case attend).
    if (g.is_client && left && left != node_seed) {
        for (auto it = g.queue.begin(); it != g.queue.end();) {
            if (it->node_seed != left) { ++it; continue; }
            log_line("CHOICE", "!! dropped the host's %s choice %u for node %016llx, which this peer has left"
                               " without its screen", kind_name(it->kind), it->index, (unsigned long long)left);
            it = g.queue.erase(it);
        }
    }

    // The cached screens belong to the node just left. Clearing them is not
    // tidiness: a stale one is both a wrong screen and a possible dead object.
    // The update ticks re-cache a live pointer on their very next frame.
    g.world_event  = nullptr;
    g.level_screen = nullptr;
    g.titled_level = nullptr;

    // A choice already waiting for THIS node becomes due the moment we arrive.
    // Re-arm the "holding" line so the next one is reported afresh.
    for (Decision& d : g.queue)
        if (d.node_seed == node_seed) { d.warned = false; d.blocked = 0; }
    // Hote : les demandes d'une autre case ne valent plus.
    if (!g.is_client) {
        for (auto it = g.queue.begin(); it != g.queue.end();) {
            if (it->node_seed && it->node_seed != node_seed) it = g.queue.erase(it);
            else ++it;
        }
    }
}

void choice_on_node_skipped(uint64_t node_seed) {
    if (!g.on || !node_seed) return;
    Guard guard;
    remember_skipped(node_seed);

    for (auto it = g.queue.begin(); it != g.queue.end();) {
        if (it->node_seed != node_seed) { ++it; continue; }
        log_line("CHOICE", "!! node %016llx was passed over without this peer"
                           " entering it, and a %s choice for it (option %u '%s') was"
                           " still held -- DROPPING it rather than letting it surface"
                           " on a later node. The host resolved that node and this"
                           " peer did not: the two runs have diverged.",
                 (unsigned long long)node_seed, kind_name(it->kind), it->index, it->name);
        it = g.queue.erase(it);
    }
}

} // namespace mgmp
