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
#include <deque>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_rungear.h ----------------------------------------------

constexpr uintptr_t kScr_Current  = 0x108;   // u64, la cle du chat montre (InventoryScreen2)
constexpr uintptr_t kScr_BoxCount = 0xAC;    // u32, le nombre de ses boites (voir mm_gear.h)...
constexpr uintptr_t kScr_Boxes    = 0xB0;    //   ... et leur tableau (InventoryItemBox*)
constexpr uintptr_t kBox_Screen   = 0x38;    // InventoryItemBox : son ecran
constexpr uintptr_t kBox_ItemId   = 0x58;    //   l'id de son objet
constexpr uintptr_t kCat_Id       = 0x00;    // CatData : son id
constexpr uintptr_t kCat_Key      = 0xC48;   //   sa cle (le registre, l'equipe de l'aventure)
constexpr uintptr_t kCat_Slots    = 0x9B0;   //   cinq emplacements d'objet...
constexpr uintptr_t kSlot_Stride  = 0x60;    //   ... de 0x60 octets (un Equipment recopie)
constexpr uint32_t  kSlots        = kItemSlots;
constexpr uintptr_t kEq_Id        = 0x00;    // Equipment : son id (propre a ce processus)
constexpr uintptr_t kEq_Name      = 0x08;    //   std::string, la cle GON
constexpr uintptr_t kEq_Text      = 0x28;    //   std::string (un livre : sa capacite)
constexpr uintptr_t kEq_A         = 0x48;
constexpr uintptr_t kEq_B         = 0x4C;
constexpr uintptr_t kEq_C         = 0x50;
constexpr uintptr_t kEq_D         = 0x54;
constexpr uintptr_t kEq_E         = 0x58;    //   u8
constexpr uintptr_t kEq_F         = 0x5C;
constexpr uintptr_t kStr_Size     = 0x10;    // std::string : taille, puis capacite
constexpr uintptr_t kStr_Cap      = 0x18;
// (bloc d'inventaire : kBucket_Head, le noeud sentinelle de sa liste, et kBucket_Count,
// son nombre d'objets en u64 -- mgmp_addresses.h)
constexpr uintptr_t kNode_Payload = 0x18;    // noeud : suivant +0, precedent +8, cle +0x10, Equipment
constexpr uintptr_t kLearn_Cat    = 0x10;    // rappel d'un livre : {vtable, boite +8, chat, objet}
constexpr uintptr_t kLearn_Item   = 0x18;    //   l'objet : un Equipment du sac (son noeud)
constexpr int32_t   kLocBackpack  = 0;       // Inventory::Loc du sac de l'aventure (le mode de l'ecran)

// --- les delais et les tailles ----------------------------------------------------------

constexpr uint8_t  kPeers         = 8;
constexpr uint32_t kMaxScreens    = 4;
constexpr uint32_t kMaxRunKeys    = 16;
constexpr size_t   kMaxQueue      = 256;
constexpr uint64_t kRevBeatMs     = 2000;    // hote : la revision, meme sans changement
constexpr uint64_t kAckWaitMs     = 20000;   // invite : un mouvement sans reponse, au plus
constexpr uint64_t kSyncRetryMs   = 20000;   // invite : redemander l'etat de l'hote
constexpr uint64_t kSameCheckMs   = 1000;    // invite : comparer les poussees gardees a son etat, au plus
constexpr uint64_t kChoiceQuietMs = 600;     // invite : pas de relecture juste apres un ecran de choix
constexpr uint32_t kStaleFrames   = 120;     // un ecran pas mis a jour depuis : verifier qu'il existe encore

// MM_RUNREV a = 1 (hote -> tous) : x = la revision, b = ces drapeaux, texte[0..7] =
// la graine de la derniere case ou l'hote est entre dans cette aventure (0 : aucune).
constexpr uint8_t  kRunRevMsg     = 1;
constexpr uint8_t  kRevSettled    = 0x01;    // il a pousse son etat d'apres cette case (ou du debut)

using FnScreenUpdate = void    (__fastcall*)(void* screen);
using FnScreenDtor   = void*   (__fastcall*)(void* screen, uint32_t flags);
using FnNodeEnd      = void    (__fastcall*)(void* map);
using FnItemAct      = void    (__fastcall*)(void** cap);
using FnLearn        = void    (__fastcall*)(void* impl, int32_t* index);
using FnEquip        = uint8_t (__fastcall*)(void* cat, void* item, int32_t loc, int32_t flag);
using FnErase        = uint32_t(__fastcall*)(void* bucket, void* item);
FnScreenUpdate o_ScreenUpdate = nullptr;
FnScreenDtor   o_ScreenDtor   = nullptr;
FnNodeEnd      o_NodeEnd      = nullptr;
FnItemAct      o_ItemAct      = nullptr;
FnLearn        o_LearnActive  = nullptr;
FnLearn        o_LearnPassive = nullptr;
FnEquip        o_CatEquip     = nullptr;
FnEquip        o_CatUnequip   = nullptr;

// Pourquoi un clic de ce joueur est retenu (et la fenetre qui le dit, apres coup).
enum class Why : uint8_t {
    None,
    Other,       // le chat n'est pas a ce joueur
    Node,        // pas sur la carte, ou une case de l'hote a suivre
    Wait,        // l'hote n'est pas encore revenu sur la carte apres la case
    Behind,      // des poussees, des mouvements ou l'etat de l'hote attendent
    Conflict,    // un mouvement refuse : un autre a pris l'objet avant
    Late,        // un mouvement arrive chez l'hote apres son entree dans une case
    Gone,        // l'objet d'une confirmation ou d'un livre ouvert n'est plus la
};

struct SlotInfo {
    bool        full   = false;
    uint64_t    id     = (uint64_t)-1;
    uint64_t    digest = 0;
    std::string name;
    ItemFields  f;                   // ses champs (le journal, quand un mouvement ne se joue pas)
};

struct BagItem {
    void*       payload = nullptr;
    uint64_t    id      = 0;
    uint64_t    digest  = 0;
    std::string name;
    ItemFields  f;
};

struct Screen {
    void*    ptr  = nullptr;
    void*    vt   = nullptr;   // sa vtable a la premiere image (InventoryScreen2)
    uint32_t seen = 0;         // la derniere image ou son update a tourne
};

// Un clic en cours (le lambda_1 d'un objet, ou le rappel d'un livre) : ce que le
// jeu y fait est note, puis envoye a la fin.
struct Scope {
    int                  depth = 0;
    void*                cat   = nullptr;
    std::vector<uint8_t> cat_before;
    std::vector<BagItem> bag_before;
    std::vector<ItemOp>  ops;
};

// Un message d'objet recu (hote : d'un invite ; invite : de l'hote).
struct InMsg {
    uint8_t  kind    = 0;    // kTeamItemOp, kTeamItemSyncReq
    uint8_t  from    = 0;
    uint32_t arrival = 0;    // follow_note_arrival (invite)
    uint32_t seq     = 0;    // kTeamItemSyncReq
    ItemOp   op;
};

struct Unacked {
    uint32_t seq  = 0;
    uint64_t at   = 0;
    uint32_t mark = 0;     // le dernier etat de l'hote applique quand il est parti
    std::string name;
};

struct State {
    uint32_t frame  = 1;
    bool     active = false;          // une fois par image (rungear_pump)
    bool     told_hooks = false;
    FnErase  erase = nullptr;

    // Les ecrans des objets ouverts chez ce joueur.
    Screen   screens[kMaxScreens];
    uint32_t n_screens   = 0;
    void*    last_screen = nullptr;   // le dernier vu : celui d'un clic

    // « ce joueur peut-il toucher au chat montre ? », une fois par image
    uint32_t touch_frame  = 0;
    void*    touch_screen = nullptr;
    uint64_t touch_key    = 0;
    bool     touch_ok     = true;
    int      touch_owner  = -1;

    Scope    scope;
    bool     applying = false;        // nous jouons un mouvement : rien a noter
    bool     partial  = false;        // apply_op a change le jeu sans aller au bout
    uint64_t act_item = (uint64_t)-1; // l'objet du dernier clic joue (le livre d'un choix ouvert)

    // Hote.
    uint32_t rev         = 0;
    uint64_t rev_sent_at = 0;
    uint8_t  flags_sent  = 0xFF;
    uint64_t node_seed   = 0;         // la derniere case ou il est entre (0 : aucune)
    bool     settled     = false;     // son etat d'apres cette case est parti
    std::deque<InMsg> inbox;          // les messages des invites, dans l'ordre

    // Invite.
    bool     have_rev     = false;    // un MM_RUNREV recu de l'hote
    uint32_t rev_received = 0;
    uint8_t  host_flags   = 0;
    uint64_t host_seed    = 0;
    uint64_t entered_seed = 0;        // la derniere case de l'hote que ce joueur a suivie
    bool     adopted      = false;    // ... connue (suivie, ou reprise en arrivant)
    std::deque<InMsg> queue;          // les mouvements de l'hote, dans l'ordre
    uint32_t seq          = 0;        // ses mouvements a lui
    std::deque<Unacked> unacked;
    bool     sync_pending = false;    // l'etat de l'hote est demande (ou va l'etre)
    uint64_t sync_asked   = 0;
    bool     bundle_have  = false;    // ... et recu, garde jusqu'a ce qu'il se lise
    RunGearMsg bundle;
    uint32_t bundle_arrival = 0;
    uint32_t bundle_applied = 0;      // le numero d'arrivee du dernier applique
    uint64_t same_checked   = 0;      // la derniere comparaison des poussees gardees

    // Les fenetres de ce joueur : une par raison et par ecran ouvert.
    uint32_t    told = 0;
    std::string notice;

    uint32_t sent = 0, received = 0, applied = 0, refused = 0, blocked = 0, dropped = 0, synced = 0;
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

bool is_host()  { return net_role() == NetRole::Host; }
bool is_guest() { return net_role() == NetRole::Client; }
int  self_id()  { return is_host() ? (int)kHostPeer : (int)net_self(); }

// --- lire le jeu --------------------------------------------------------------------------

std::string read_str(const void* str) {
    uint64_t len = 0, cap = 0;
    if (!str || !mem_read((const uint8_t*)str + kStr_Size, &len, sizeof(len)) ||
        !mem_read((const uint8_t*)str + kStr_Cap, &cap, sizeof(cap)) || len > 512)
        return std::string();
    const uint8_t* chars = (const uint8_t*)str;
    if (cap > 15 && (!mem_read(str, &chars, sizeof(chars)) || !chars)) return std::string();
    std::string out((size_t)len, '\0');
    if (len && !mem_read(chars, &out[0], (size_t)len)) return std::string();
    return out;
}

// Un Equipment (dans le sac ou dans un emplacement) : ses champs sauves et son id.
bool read_item(const void* eq, ItemFields& f, uint64_t* id) {
    if (!eq) return false;
    uint64_t i = 0;
    if (!mem_read((const uint8_t*)eq + kEq_Id, &i, sizeof(i))) return false;
    f.name = read_str((const uint8_t*)eq + kEq_Name);
    f.text = read_str((const uint8_t*)eq + kEq_Text);
    if (!mem_read((const uint8_t*)eq + kEq_A, &f.a, 4) || !mem_read((const uint8_t*)eq + kEq_B, &f.b, 4) ||
        !mem_read((const uint8_t*)eq + kEq_C, &f.c, 4) || !mem_read((const uint8_t*)eq + kEq_D, &f.d, 4) ||
        !mem_read((const uint8_t*)eq + kEq_E, &f.e, 1) || !mem_read((const uint8_t*)eq + kEq_F, &f.f, 4))
        return false;
    if (id) *id = i;
    return true;
}

uint64_t cat_id_of(const void* cat) {
    uint64_t id = 0;
    if (!cat || !mem_read((const uint8_t*)cat + kCat_Id, &id, sizeof(id))) return 0;
    return id;
}

uint64_t cat_key_of(const void* cat) {
    uint64_t key = 0;
    if (!cat || !mem_read((const uint8_t*)cat + kCat_Key, &key, sizeof(key))) return 0;
    return key;
}

void* slot_ptr(void* cat, uint32_t k) { return (uint8_t*)cat + kCat_Slots + k * kSlot_Stride; }

// Un emplacement est vide comme le jeu le teste : id -1 ou nom vide.
bool slot_info(void* cat, uint32_t k, SlotInfo& s) {
    s = SlotInfo{};
    if (!cat || k >= kSlots) return false;
    ItemFields f;
    uint64_t id = 0;
    if (!read_item(slot_ptr(cat, k), f, &id)) return false;
    s.id   = id;
    s.full = id != (uint64_t)-1 && !f.name.empty();
    if (s.full) {
        s.digest = item_digest(f);
        s.name   = f.name;
        s.f      = std::move(f);
    }
    return true;
}

// Les champs d'un objet, pour le journal : quand un mouvement ne se joue pas, les
// deux journaux disent ce qui differe.
std::string item_str(const ItemFields& f) {
    std::string s = fmt("a=%d b=%u c=0x%x d=%u e=%u f=%u", (int)f.a, f.b, f.c, f.d, (unsigned)f.e, f.f);
    if (!f.text.empty()) s += " texte=" + f.text.substr(0, 40);
    return s;
}

void* inventory() {
    const void* const* slot = (const void* const*)addr_of_data(D_MewDirectorPtr);
    const uint8_t* dir = nullptr;
    void* inv = nullptr;
    if (!slot || !mem_read(slot, &dir, sizeof(dir)) || !dir) return nullptr;
    if (!mem_read(dir + kDir_Inventory, &inv, sizeof(inv))) return nullptr;
    return inv;
}

void* backpack() {
    void* inv = inventory();
    return inv ? (uint8_t*)inv + kInv_Backpack : nullptr;
}

// Le sac de l'aventure, dans l'ordre de sa liste.
bool bag_items(std::vector<BagItem>& out) {
    out.clear();
    uint8_t* bucket = (uint8_t*)backpack();
    if (!bucket) return false;
    uint8_t* head = nullptr;
    uint64_t count = 0;
    if (!mem_read(bucket + kBucket_Head, &head, sizeof(head)) || !head) return false;
    if (!mem_read(bucket + kBucket_Count, &count, sizeof(count)) || count > 4096) return false;
    uint8_t* n = nullptr;
    if (!mem_read(head, &n, sizeof(n))) return false;
    for (uint32_t guard = 0; n && n != head; ++guard) {
        if (guard > count + 4) return false;           // une liste qui ne revient pas a sa tete
        BagItem b;
        b.payload = n + kNode_Payload;
        ItemFields f;
        if (read_item(b.payload, f, &b.id)) {
            b.digest = item_digest(f);
            b.name   = f.name;
            b.f      = std::move(f);
            out.push_back(std::move(b));
        }
        if (!mem_read(n, &n, sizeof(n))) return false;
    }
    return true;
}

void* find_bag(uint64_t digest, std::string* name = nullptr) {
    std::vector<BagItem> bag;
    if (!digest || !bag_items(bag)) return nullptr;
    for (const BagItem& b : bag)
        if (b.digest == digest) {
            if (name) *name = b.name;
            return b.payload;
        }
    return nullptr;
}

// Pour le journal d'un objet introuvable : les objets de ce nom au sac ici.
std::string bag_like(const std::string& name) {
    std::vector<BagItem> bag;
    if (!bag_items(bag)) return " ; sac illisible ici";
    std::string s;
    uint32_t n = 0;
    for (const BagItem& b : bag) {
        if (b.name != name) continue;
        if (++n > 3) { s += ", ..."; break; }
        s += (n == 1 ? " ; ici : " : ", ") + item_str(b.f);
    }
    return n ? s : fmt(" ; aucun %s au sac ici (%zu objet(s))", name.c_str(), bag.size());
}

std::string slot_like(const SlotInfo& si) {
    return si.full ? " ; ici : " + si.name + " " + item_str(si.f) : std::string(" ; emplacement vide ici");
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

// Le proprietaire d'un chat : celui de l'equipe mixte, l'hote a defaut.
int owner_of_cat(void* cat) {
    if (!cat) return -1;
    const int o = team_owner_of(cat_id_of(cat));
    return o < 0 ? (int)kHostPeer : o;
}

int owner_of_key(uint64_t key) { return owner_of_cat(catsync_cat_by_key(key)); }

// Le chat de l'equipe qui porte l'objet de cet id (nul : aucun, il est au sac).
void* wearer_of(uint64_t item_id) {
    if (item_id == (uint64_t)-1) return nullptr;
    std::vector<uint64_t> keys;
    if (!run_keys(keys)) return nullptr;
    for (uint64_t key : keys) {
        void* cat = catsync_cat_by_key(key);
        if (!cat) continue;
        for (uint32_t k = 0; k < kSlots; ++k) {
            uint64_t id = 0;
            if (mem_read(slot_ptr(cat, k), &id, sizeof(id)) && id == item_id) return cat;
        }
    }
    return nullptr;
}

bool box_alive(const void* box) {
    if (!box || !g.n_screens) return true;
    for (uint32_t i = 0; i < g.n_screens; ++i) {
        const uint8_t* scr = (const uint8_t*)g.screens[i].ptr;
        uint32_t n = 0;
        void* const* arr = nullptr;
        if (!mem_read(scr + kScr_BoxCount, &n, sizeof(n)) || !mem_read(scr + kScr_Boxes, &arr, sizeof(arr)) ||
            !arr || n > 4096)
            return true;                                 // illisible : le jeu decide
        std::vector<void*> boxes(n);
        if (n && !mem_read(arr, boxes.data(), n * sizeof(void*))) return true;
        for (void* b : boxes)
            if (b == box) return true;
    }
    return false;
}

// L'objet de cet id est-il encore dans l'aventure (au sac, ou porte par un chat de
// l'equipe) ? Illisible : oui (le jeu decide, comme sans nous).
bool item_exists(uint64_t item_id) {
    if (item_id == (uint64_t)-1) return false;
    if (wearer_of(item_id)) return true;
    std::vector<BagItem> bag;
    if (!bag_items(bag)) return true;
    for (const BagItem& b : bag)
        if (b.id == item_id) return true;
    return false;
}

// Cet Equipment est-il (encore) cet objet du sac (id -1 : n'importe lequel) ?
// Illisible : oui.
bool bag_holds(const void* payload, uint64_t id) {
    std::vector<BagItem> bag;
    if (!payload) return false;
    if (!bag_items(bag)) return true;
    for (const BagItem& b : bag)
        if (b.payload == payload) return id == (uint64_t)-1 || b.id == id;
    return false;
}

// Cette boite est-elle encore dans un ecran des objets ouvert ? Une boite dont
// l'objet est parti en est retiree (InventoryItemBox::update), puis detruite :
// une confirmation qui la tient encore ne doit plus la donner au jeu. Aucun
// ecran connu : on ne sait pas, oui.
bool box_alive(const void* box);

bool held_pushes() { return catsync_pending_count() != 0 || invsync_has_pending(); }

uint32_t last_state_arrival() {
    uint32_t a = catsync_last_applied_arrival();
    const uint32_t b = invsync_last_applied_arrival();
    if (b > a) a = b;
    if (g.bundle_applied > a) a = g.bundle_applied;
    return a;
}

// --- l'etat de la partie ------------------------------------------------------------------

bool compute_active() {
    if (gear_role_active()) return false;               // le choix des objets du depart (mm_gear)
    if (!savefile_adventure_is_loaded()) return false;
    bool want = false;
    if (is_host())       want = mm_run_peers_mask() != 0;
    else if (is_guest()) want = savefile_in_coop() && mm_guest_in_run();
    if (!want) return false;
    // Sans ses crochets, ce module ne sait ni quand un ecran est ouvert ni ce
    // qu'un clic deplace : mieux vaut l'ancien comportement (chacun touche a
    // tout, l'hote remet tout a chaque case). La fin d'une case (T_NodeEnd) dit
    // jusqu'ou une case dure (follow_node_live) : sans elle, « entre deux cases »
    // commencerait une seconde apres l'entree, au milieu d'un evenement.
    static const Target kNeed[] = { T_InvScreenUpdate, T_InvScreenDtor, T_InvItemAct, T_InvLearnActive,
                                    T_InvLearnPassive, T_CatEquip, T_CatUnequip, T_NodeEnd };
    for (Target t : kNeed) {
        if (!hooks_is_live(t)) {
            if (!g.told_hooks) {
                g.told_hooks = true;
                log_line("SAC", "!! crochet %s absent : objets en direct eteints, chacun touche a tous les "
                                "chats comme avant", kTargets[t].name);
            }
            return false;
        }
    }
    if (!g.erase) {
        g.erase = (FnErase)addr_of_call(C_BucketErase);
        if (!g.erase) {
            if (!g.told_hooks) {
                g.told_hooks = true;
                log_line("SAC", "!! le retrait d'un objet du sac (0x1402E24D0) ne se resout pas : objets en "
                                "direct eteints");
            }
            return false;
        }
    }
    return true;
}

// Hote : entre deux cases. Sa carte peut ne pas tourner : l'ecran des objets (comme
// toute fenetre du jeu) la met en pause -- il n'en est pas moins sur la carte. (Un
// test sur la carte qui tourne prenait l'hote, sac ouvert, pour un hote dans une
// case : aucun de ses mouvements ne partait, et ceux des invites etaient refuses.)
bool host_between() { return !follow_inside_node() && !follow_node_live(); }
// ... et sa carte a l'ecran : la poussee d'apres une case, les fenetres de ce module.
bool host_map() { return follow_on_map() && host_between(); }
// Ses mouvements partent et ceux des invites sont pris : entre deux cases, son
// etat d'apres la derniere case parti.
bool host_live() { return g.settled && host_between(); }

bool host_settled_for_me() {
    return g.have_rev && g.adopted && (g.host_flags & kRevSettled) && g.host_seed == g.entered_seed;
}

// Invite : entre deux cases, et aucune case de l'hote a suivre (sa carte en pause,
// sac ouvert, compte : il est sur la carte).
bool guest_between() { return follow_pending_count() == 0 && !follow_node_live(); }

// Invite : ce qui l'empeche de changer ses objets maintenant.
Why guest_block() {
    if (!guest_between()) return Why::Node;
    if (!host_settled_for_me()) return Why::Wait;
    if (held_pushes() || !g.queue.empty() || g.sync_pending || g.bundle_have) return Why::Behind;
    return Why::None;
}

bool guest_live() { return guest_block() == Why::None; }

// Invite : les mouvements de l'hote peuvent-ils se jouer maintenant ? (Entre deux
// cases, l'etat de l'hote a jour ; un ecran des objets ouvert ne gene pas.)
bool guest_can_apply() { return guest_between() && !held_pushes() && !g.sync_pending && !g.bundle_have; }

// Ce qui empeche CE joueur d'agir sur ce chat (Why::None : rien).
Why act_block(void* cat) {
    if (!cat) return Why::None;                          // aucun chat : le jeu ne fera rien
    if (owner_of_cat(cat) != self_id()) return Why::Other;
    if (is_guest()) return guest_block();
    return Why::None;
}

void tell(Why why, const std::string& text) {
    const uint32_t bit = 1u << (uint32_t)why;
    if (g.told & bit) return;
    g.told |= bit;
    g.notice = text;
}

const char* why_name(Why w) {
    switch (w) {
        case Why::None:     return "rien";
        case Why::Other:    return "ce chat n'est pas a ce joueur";
        case Why::Node:     return "pas sur la carte, ou une case de l'hote a suivre";
        case Why::Wait:     return "l'hote n'est pas encore revenu sur la carte";
        case Why::Behind:   return "des poussees, des mouvements ou l'etat de l'hote attendent";
        case Why::Conflict: return "un mouvement refuse";
        case Why::Late:     return "un mouvement arrive apres l'entree de l'hote dans une case";
        case Why::Gone:     return "l'objet n'est plus la (un autre joueur l'a pris)";
    }
    return "?";
}

void tell_why(Why why, int owner) {
    switch (why) {
    case Why::Other:
        tell(why, fmt(u8"Objets\n\nCe chat est à %s : seul son propriétaire\npeut changer ses objets.",
                      mm_peer_name((uint8_t)owner).c_str()));
        break;
    case Why::Node:
        if (follow_pending_count()) break;               // il va suivre l'hote : rien a lire apres coup
        tell(why, u8"Objets\n\nPendant une case (une boutique, une montée de\n"
                  u8"niveau...), seul l'hôte change les objets.\nÉquipe tes chats sur la carte.");
        break;
    case Why::Wait:
        tell(why, u8"Objets\n\nL'hôte n'est pas encore revenu sur la carte :\n"
                  u8"attends-le, ton sac se débloquera tout seul.");
        break;
    case Why::Behind:
        tell(why, u8"Objets\n\nTon sac se met à jour avec celui de l'hôte :\n"
                  u8"ferme-le, puis rouvre-le dans un instant.");
        break;
    case Why::Conflict:
        tell(why, u8"Objets\n\nUn autre joueur a pris cet objet juste avant toi :\n"
                  u8"ton changement n'a pas été pris. Ton sac revient\ncomme chez l'hôte quand tu le fermes.");
        break;
    case Why::Late:
        tell(why, u8"Objets\n\nL'hôte est entré dans une case avant que ton\n"
                  u8"dernier changement n'arrive : il est annulé.\nRefais-le sur la carte.");
        break;
    case Why::Gone:
        tell(why, u8"Objets\n\nCet objet n'est plus là : un autre joueur\nl'a pris juste avant toi.");
        break;
    case Why::None:
        break;
    }
}

// --- les ecrans des objets (les deux detours) ---------------------------------------------

Screen* find_screen(void* ptr) {
    for (uint32_t i = 0; i < g.n_screens; ++i)
        if (g.screens[i].ptr == ptr) return &g.screens[i];
    return nullptr;
}

void note_screen(void* screen) {
    g.last_screen = screen;
    if (Screen* known = find_screen(screen)) { known->seen = g.frame; return; }
    if (g.n_screens >= kMaxScreens) return;
    Screen& s = g.screens[g.n_screens++];
    s = Screen{};
    s.ptr  = screen;
    s.seen = g.frame;
    mem_read(screen, &s.vt, sizeof(s.vt));
    g.told = 0;
}

void forget_screen(void* screen) {
    for (uint32_t i = 0; i < g.n_screens; ++i) {
        if (g.screens[i].ptr != screen) continue;
        g.screens[i] = g.screens[--g.n_screens];
        g.screens[g.n_screens] = Screen{};
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

// --- envoyer ------------------------------------------------------------------------------

const char* op_name(uint8_t op) {
    switch (op) {
        case kItemEquip:   return "equipe";
        case kItemUnequip: return "rend au sac";
        case kItemLearn:   return "lit un livre";
    }
    return "?";
}

// Hote : un mouvement part chez tous les joueurs de l'aventure, sauf `except`.
void broadcast(const ItemOp& op, int except) {
    const std::vector<uint8_t> body = team_item_op_encode(op);
    const uint32_t mask = mm_run_peers_mask();
    for (uint8_t p = 1; p < kPeers; ++p) {
        if (!(mask & (1u << p)) || (int)p == except) continue;
        if (!team_send(body, p))
            log_line("SAC", "!! mouvement %u de %s non envoye a %s", op.seq, mm_peer_name(op.origin).c_str(),
                     mm_peer_name(p).c_str());
    }
}

void send_rev(uint64_t now) {
    MmMsg m{};
    m.kind = MM_RUNREV;
    m.a    = kRunRevMsg;
    m.b    = (uint8_t)(g.settled ? kRevSettled : 0);
    m.x    = g.rev;
    static_assert(sizeof(m.text) >= sizeof(g.node_seed), "MmMsg::text trop petit");
    memcpy(m.text, &g.node_seed, sizeof(g.node_seed));
    net_send_mm(m);
    g.rev_sent_at = now;
    g.flags_sent  = m.b;
}

void send_ack(uint8_t peer, uint32_t seq, uint8_t result) {
    ItemAck a;
    a.seq    = seq;
    a.result = result;
    if (!team_send(team_item_ack_encode(a), peer))
        log_line("SAC", "!! reponse a %s non envoyee", mm_peer_name(peer).c_str());
}

// Invite : l'etat de l'hote est demande (une fois ; redemande apres kSyncRetryMs).
void request_sync(const char* why) {
    const uint64_t now = GetTickCount64();
    if (g.sync_pending && now - g.sync_asked < kSyncRetryMs) return;
    if (!team_send(team_item_sync_req_encode(g.seq), kHostPeer)) {
        log_line("SAC", "!! demande de l'etat de l'hote non envoyee (%s)", why);
    } else {
        log_line("SAC", "-> l'etat de l'hote est demande (apres le mouvement %u) : %s", g.seq, why);
    }
    g.sync_pending = true;
    g.sync_asked   = now;
}

// Un mouvement fait par ce joueur, a la fin du clic qui l'a fait.
void send_local(ItemOp op) {
    op.origin = (uint8_t)self_id();
    op.seq    = ++g.seq;
    if (is_host()) {
        if (!host_live()) {
            log_line("SAC", "l'hote %s %s dans une case : sa poussee de retour sur la carte le dira",
                     op_name(op.op), op.name.c_str());
            return;
        }
        ++g.rev;
        broadcast(op, -1);
    } else {
        if (!team_send(team_item_op_encode(op), kHostPeer)) {
            log_line("SAC", "!! mouvement %u non envoye a l'hote", op.seq);
            request_sync("un mouvement n'a pas pu partir");
            return;
        }
        Unacked u;
        u.seq  = op.seq;
        u.at   = GetTickCount64();
        u.mark = last_state_arrival();
        u.name = op.name;
        g.unacked.push_back(u);
    }
    ++g.sent;
    log_line("SAC", "-> mouvement %u : %s %s (emplacement %u) chat %016llx%s%s", op.seq, op_name(op.op),
             op.name.c_str(), (unsigned)op.slot, (unsigned long long)op.cat_id,
             op.op == kItemLearn ? fmt(", %zu octets", op.cat.size()).c_str() : "",
             op.detail.empty() ? "" : (" [" + op.detail + "]").c_str());
}

// --- noter ce que fait un clic ------------------------------------------------------------

void scope_begin(void* cat) {
    if (g.scope.depth++ > 0) return;
    g.scope.cat = cat;
    g.scope.ops.clear();
    g.scope.cat_before.clear();
    if (cat) catsync_serialize_cat(cat, g.scope.cat_before);
    bag_items(g.scope.bag_before);
}

void scope_end() {
    if (--g.scope.depth > 0) return;
    std::vector<ItemOp> ops = std::move(g.scope.ops);
    void* cat = g.scope.cat;
    // Rien d'equipe ni de rendu : un livre lu, ou autre chose du jeu sur ce chat.
    // Le chat tel qu'il est apres, et l'objet sorti du sac s'il y en a un.
    if (ops.empty() && cat && !g.scope.cat_before.empty()) {
        std::vector<uint8_t> after;
        std::vector<BagItem> bag;
        if (catsync_serialize_cat(cat, after) && bag_items(bag)) {
            const BagItem* gone = nullptr;
            for (const BagItem& b : g.scope.bag_before) {
                bool still = false;
                for (const BagItem& n : bag)
                    if (n.id == b.id) { still = true; break; }
                if (!still) { gone = &b; break; }
            }
            if (after != g.scope.cat_before || gone) {
                ItemOp op;
                op.op      = kItemLearn;
                op.cat_key = cat_key_of(cat);
                op.cat_id  = cat_id_of(cat);
                op.item    = gone ? gone->digest : 0;
                op.name    = gone ? gone->name : std::string("(chat)");
                op.detail  = gone ? item_str(gone->f) : std::string();
                op.cat     = std::move(after);
                ops.push_back(std::move(op));
            }
        }
    }
    g.scope = Scope{};
    for (ItemOp& op : ops) send_local(std::move(op));
}

uint8_t __fastcall h_CatEquip(void* cat, void* item, int32_t loc, int32_t flag) {
    if (!g.scope.depth || g.applying || !cat || !item) return o_CatEquip(cat, item, loc, flag);
    SlotInfo before[kSlots];
    for (uint32_t k = 0; k < kSlots; ++k) slot_info(cat, k, before[k]);
    ItemFields f;
    uint64_t id = 0;
    const bool known = read_item(item, f, &id);          // avant : l'objet quitte le sac
    const uint8_t r = o_CatEquip(cat, item, loc, flag);
    if (r && known) {
        int slot = -1;
        for (uint32_t k = 0; k < kSlots && slot < 0; ++k) {
            SlotInfo after;
            if (slot_info(cat, k, after) && after.full && after.id == id) slot = (int)k;
        }
        if (slot < 0) {
            log_line("SAC", "!! objet equipe introuvable sur le chat apres coup (%s) -- envoye comme un livre",
                     f.name.c_str());
        } else {
            ItemOp op;
            op.op      = kItemEquip;
            op.slot    = (uint8_t)slot;
            op.cat_key = cat_key_of(cat);
            op.cat_id  = cat_id_of(cat);
            op.item    = item_digest(f);
            op.prev    = before[slot].full ? before[slot].digest : 0;
            op.name    = f.name;
            op.detail  = item_str(f);
            g.scope.ops.push_back(std::move(op));
        }
    }
    return r;
}

uint8_t __fastcall h_CatUnequip(void* cat, void* item, int32_t loc, int32_t flag) {
    if (!g.scope.depth || g.applying || !cat || !item) return o_CatUnequip(cat, item, loc, flag);
    uint64_t id = 0;
    mem_read(item, &id, sizeof(id));
    int slot = -1;
    SlotInfo si;
    for (uint32_t k = 0; k < kSlots && slot < 0; ++k)
        if (slot_info(cat, k, si) && si.full && si.id == id) slot = (int)k;
    const uint8_t r = o_CatUnequip(cat, item, loc, flag);
    if (r && slot >= 0) {
        ItemOp op;
        op.op      = kItemUnequip;
        op.slot    = (uint8_t)slot;
        op.cat_key = cat_key_of(cat);
        op.cat_id  = cat_id_of(cat);
        op.item    = si.digest;
        op.name    = si.name;
        op.detail  = item_str(si.f);
        g.scope.ops.push_back(std::move(op));
    }
    return r;
}

// Le clic d'un objet, apres les confirmations du jeu : il agit sur le chat qui
// porte l'objet, sinon sur le chat montre. A ce joueur, et il peut agir ?
void __fastcall h_ItemAct(void** cap) {
    void* box = nullptr;
    void* screen = nullptr;
    uint64_t item_id = (uint64_t)-1, key = 0;
    if (cap) mem_read(cap, &box, sizeof(box));
    if (box) {
        mem_read((const uint8_t*)box + kBox_Screen, &screen, sizeof(screen));
        mem_read((const uint8_t*)box + kBox_ItemId, &item_id, sizeof(item_id));
    }
    g.act_item = item_id;                                // le livre d'un choix qui s'ouvrirait
    if (!g.active || !cap) { o_ItemAct(cap); return; }
    // Une confirmation ouverte avant qu'un autre joueur ne prenne l'objet (un
    // livre lu) : la boite qu'elle tient n'existe plus (InventoryItemBox::update la
    // retire des que son objet manque) -- le jeu la lirait apres sa destruction.
    // D'abord la boite (son id se lit peut-etre dans une memoire rendue), puis
    // l'objet (sa boite est retiree a son update suivant seulement).
    if (box && (!box_alive(box) || !item_exists(item_id))) {
        ++g.blocked;
        log_line("SAC", "action d'objet retenue : l'objet %llx n'est plus la", (unsigned long long)item_id);
        tell_why(Why::Gone, -1);
        return;
    }
    if (screen) mem_read((const uint8_t*)screen + kScr_Current, &key, sizeof(key));
    void* target = wearer_of(item_id);
    if (!target && key) target = catsync_cat_by_key(key);
    const Why why = act_block(target);
    if (why != Why::None) {
        ++g.blocked;
        log_line("SAC", "action d'objet retenue : %s", why_name(why));
        tell_why(why, owner_of_cat(target));
        return;
    }
    scope_begin(target);
    o_ItemAct(cap);
    scope_end();
}

// Le rappel d'un livre (la capacite a remplacer choisie). -1 : annule, rien a dire.
void learn_hook(FnLearn original, void* impl, int32_t* index) {
    if (!g.active || !impl || !index) { original(impl, index); return; }
    // Le livre lu par un autre joueur pendant que ce choix etait ouvert : son
    // noeud du sac et sa boite sont detruits, et le rappel du jeu s'en servirait,
    // meme pour annuler. Rien ne se fait. (Le pointeur du rappel est compare aux
    // noeuds vivants avant toute lecture ; un noeud neuf a la meme adresse porte
    // un autre id que le livre du clic qui a ouvert ce choix.)
    void* book = nullptr;
    mem_read((const uint8_t*)impl + kLearn_Item, &book, sizeof(book));
    if (!bag_holds(book, g.act_item)) {
        ++g.blocked;
        log_line("SAC", "livre : %s retenu, le livre n'est plus dans le sac", *index == -1 ? "l'annulation" : "le choix");
        if (*index != -1) tell_why(Why::Gone, -1);
        return;
    }
    if (*index == -1) { original(impl, index); return; }
    void* cat = nullptr;
    mem_read((const uint8_t*)impl + kLearn_Cat, &cat, sizeof(cat));
    const Why why = act_block(cat);
    if (why != Why::None) {
        // Une annulation du jeu (la boite et le livre existent) : le livre reste,
        // rien n'est appris, l'ecran revient comme apres un « Retour ».
        ++g.blocked;
        log_line("SAC", "livre : remplacement retenu (%s)", why_name(why));
        tell_why(why, owner_of_cat(cat));
        int32_t cancel = -1;
        original(impl, &cancel);
        return;
    }
    scope_begin(cat);
    original(impl, index);
    scope_end();
}

void __fastcall h_LearnActive(void* impl, int32_t* index)  { learn_hook(o_LearnActive, impl, index); }
void __fastcall h_LearnPassive(void* impl, int32_t* index) { learn_hook(o_LearnPassive, impl, index); }

// --- jouer un mouvement d'un autre ----------------------------------------------------------

// Remet aux objets que la relecture d'un chat a laisses tels quels l'id qu'ils
// avaient : les boites d'un ecran des objets ouvert les retrouvent.
void restore_ids(void* cat, const SlotInfo* before) {
    for (uint32_t k = 0; k < kSlots; ++k) {
        SlotInfo after;
        if (!before[k].full || !slot_info(cat, k, after) || !after.full) continue;
        if (after.digest != before[k].digest || after.id == before[k].id) continue;
        const uint64_t id = before[k].id;
        mem_write(slot_ptr(cat, k), &id, sizeof(id));
    }
}

// kItemOk, kItemConflict (l'objet n'est plus ou il devait etre), kItemInvalid.
uint8_t apply_op(const ItemOp& op, std::string& why) {
    void* cat = catsync_cat_by_key(op.cat_key);
    if (!cat || cat_id_of(cat) != op.cat_id) { why = "chat introuvable"; return kItemInvalid; }
    switch (op.op) {
    case kItemEquip: {
        void* item = find_bag(op.item);
        if (!item) { why = "objet absent du sac" + bag_like(op.name); return kItemConflict; }
        SlotInfo si;
        if (!slot_info(cat, op.slot, si)) { why = "emplacement illisible"; return kItemInvalid; }
        if ((si.full ? si.digest : 0) != op.prev) {
            why = "l'emplacement tient autre chose" + slot_like(si);
            return kItemConflict;
        }
        g.applying = true;
        const uint8_t r = o_CatEquip(cat, item, kLocBackpack, 0);
        g.applying = false;
        if (!r) { why = "le jeu refuse"; return kItemInvalid; }
        SlotInfo after;
        if (!slot_info(cat, op.slot, after) || !after.full || after.digest != op.item) {
            why = "pas dans l'emplacement attendu";
            g.partial = true;                            // equipe quand meme, ailleurs
            return kItemInvalid;
        }
        return kItemOk;
    }
    case kItemUnequip: {
        SlotInfo si;
        if (!slot_info(cat, op.slot, si) || !si.full || si.digest != op.item) {
            why = "objet absent de l'emplacement" + slot_like(si);
            return kItemConflict;
        }
        g.applying = true;
        const uint8_t r = o_CatUnequip(cat, slot_ptr(cat, op.slot), kLocBackpack, 0);
        g.applying = false;
        if (!r) { why = "le jeu refuse"; return kItemInvalid; }
        return kItemOk;
    }
    case kItemLearn: {
        void* book = nullptr;
        if (op.item) {
            book = find_bag(op.item);
            if (!book) { why = "livre absent du sac" + bag_like(op.name); return kItemConflict; }
        }
        SlotInfo before[kSlots];
        for (uint32_t k = 0; k < kSlots; ++k) slot_info(cat, k, before[k]);
        std::vector<uint8_t> undo;
        if (!catsync_serialize_cat(cat, undo) || undo.empty()) { why = "chat illisible ici"; return kItemInvalid; }
        // Le chat d'avant, s'il ne va pas au bout.
        auto revert = [&](const char* reason) {
            g.applying = true;
            const bool back = catsync_deserialize_cat(cat, undo);
            g.applying = false;
            restore_ids(cat, before);
            if (!back) {
                g.partial = true;
                log_line("SAC", "!! le chat %016llx n'a pas pu etre remis apres un livre refuse",
                         (unsigned long long)op.cat_id);
            }
            why = reason;
        };
        g.applying = true;
        const bool ok = catsync_deserialize_cat(cat, op.cat);
        g.applying = false;
        if (!ok) { revert("chat illisible"); return kItemInvalid; }
        // Un livre ne touche pas aux objets : ceux du chat lu doivent etre ceux
        // d'ici. Sinon le chat de l'autre portait un objet qu'il n'a pas ici (un
        // mouvement refuse juste avant, pas encore su chez lui) : le relire ici le
        // doublerait.
        for (uint32_t k = 0; k < kSlots; ++k) {
            SlotInfo after;
            const bool same = slot_info(cat, k, after) && after.full == before[k].full &&
                              (!after.full || after.digest == before[k].digest);
            if (!same) { revert("les objets du chat ne sont pas les memes ici"); return kItemConflict; }
        }
        restore_ids(cat, before);
        if (book && (!g.erase || g.erase(backpack(), book) == 0)) {
            revert("livre pas retire du sac");
            return kItemInvalid;
        }
        return kItemOk;
    }
    }
    why = "mouvement inconnu";
    return kItemInvalid;
}

// --- hote -------------------------------------------------------------------------------------

void host_take_op(const InMsg& m) {
    const ItemOp& op = m.op;
    ++g.received;
    if (!host_live()) {
        ++g.refused;
        log_line("SAC", "mouvement %u de %s refuse : l'hote est dans une case", op.seq, mm_peer_name(m.from).c_str());
        send_ack(m.from, op.seq, kItemNode);
        return;
    }
    if (owner_of_key(op.cat_key) != (int)m.from) {
        ++g.refused;
        log_line("SAC", "!! mouvement %u de %s refuse : le chat %016llx n'est pas a lui", op.seq,
                 mm_peer_name(m.from).c_str(), (unsigned long long)op.cat_id);
        send_ack(m.from, op.seq, kItemInvalid);
        return;
    }
    std::string why;
    g.partial = false;
    const uint8_t r = apply_op(op, why);
    if (r != kItemOk) {
        ++g.refused;
        log_line("SAC", "mouvement %u de %s refuse (%s %s) : %s", op.seq, mm_peer_name(m.from).c_str(),
                 op_name(op.op), op.name.c_str(), why.c_str());
        send_ack(m.from, op.seq, r);
        if (g.partial) {
            // Le jeu a change quand meme : l'etat de l'hote part a tous (les autres
            // le relisent ; celui qui a ete refuse demande le sien).
            const uint32_t cats = catsync_publish("un mouvement d'objet joue en partie");
            const bool     inv  = invsync_publish("un mouvement d'objet joue en partie");
            if (cats || inv) ++g.rev;
            g.partial = false;
        }
        return;
    }
    ++g.applied;
    ++g.rev;
    broadcast(op, m.from);
    send_ack(m.from, op.seq, kItemOk);
    log_line("SAC", "<- mouvement %u de %s pris : %s %s (emplacement %u)", op.seq, mm_peer_name(m.from).c_str(),
             op_name(op.op), op.name.c_str(), (unsigned)op.slot);
}

void host_take_sync_req(const InMsg& m) {
    if (!host_live()) {
        // Sa poussee d'entree dans la case (entiere : rungear_before_node_push) le
        // remet avant la case.
        log_line("SAC", "%s demande l'etat de l'hote pendant une case : sa poussee d'entree le remettra",
                 mm_peer_name(m.from).c_str());
        return;
    }
    RunGearMsg b;
    b.seq  = m.seq;
    b.base = g.rev;
    std::vector<uint64_t> keys;
    bool ok = invsync_serialize_bucket(kInvBucketBackpack, b.backpack) &&
              invsync_serialize_bucket(kInvBucketTrash, b.trash) && run_keys(keys);
    for (size_t i = 0; ok && i < keys.size(); ++i) {
        GearCat c;
        c.key = keys[i];
        void* cat = catsync_cat_by_key(c.key);
        if (!cat || !catsync_serialize_cat(cat, c.data) || c.data.empty()) { ok = false; break; }
        b.cats.push_back(std::move(c));
    }
    if (!ok || !team_send(team_item_sync_encode(b), m.from)) {
        log_line("SAC", "!! l'etat de l'hote n'a pas pu partir chez %s", mm_peer_name(m.from).c_str());
        return;
    }
    ++g.synced;
    log_line("SAC", "-> etat de l'hote envoye a %s : sac %zu octets, %zu chat(s), revision %u",
             mm_peer_name(m.from).c_str(), b.backpack.size(), b.cats.size(), g.rev);
}

void host_pump(uint64_t now) {
    if (g.rev == 0) {
        g.rev       = 1;
        g.node_seed = 0;
        g.settled   = false;
        send_rev(now);
    }
    // De retour sur la carte (ou au debut de l'aventure) : son etat d'apres la
    // case part, une fois. Jusque-la, un invite n'equipe pas.
    if (!g.settled && host_map()) {
        g.settled = true;
        const uint32_t cats = catsync_publish(g.node_seed ? "de retour sur la carte" : "debut de l'aventure");
        const bool     inv  = invsync_publish(g.node_seed ? "de retour sur la carte" : "debut de l'aventure");
        if (cats || inv) ++g.rev;
        send_rev(now);
        log_line("SAC", "etat de l'hote %s (revision %u) : les invites peuvent changer leurs objets",
                 (cats || inv) ? "pousse" : "inchange", g.rev);
    }
    // Les messages des invites, dans l'ordre d'arrivee : les mouvements d'un
    // invite passent avant sa demande d'etat qui les suit.
    while (!g.inbox.empty()) {
        const InMsg m = std::move(g.inbox.front());
        g.inbox.pop_front();
        if (m.kind == kTeamItemOp) host_take_op(m);
        else                       host_take_sync_req(m);
    }
    const uint8_t flags = (uint8_t)(g.settled ? kRevSettled : 0);
    if (flags != g.flags_sent || now - g.rev_sent_at >= kRevBeatMs) send_rev(now);
}

// --- invite -----------------------------------------------------------------------------------

void apply_bundle() {
    // D'abord les poussees arrivees AVANT lui (un etat plus ancien), puis lui ;
    // celles arrivees apres restent gardees, plus recentes.
    catsync_apply_pending_before_arrival(g.bundle_arrival, "avant l'etat de l'hote");
    invsync_apply_pending_before_arrival(g.bundle_arrival, "avant l'etat de l'hote");
    bool ok = invsync_load_bucket(kInvBucketBackpack, g.bundle.backpack) &&
              invsync_load_bucket(kInvBucketTrash, g.bundle.trash);
    size_t cats = 0;
    for (const GearCat& c : g.bundle.cats) {
        void* cat = catsync_cat_by_key(c.key);
        if (cat && catsync_deserialize_cat(cat, c.data)) ++cats;
        else ok = false;
    }
    g.bundle_applied = g.bundle_arrival;
    g.bundle_have    = false;
    g.bundle         = RunGearMsg{};
    g.sync_pending   = false;
    ++g.synced;
    log_line("SAC", "%s l'etat de l'hote relu : sac, poubelle, %zu chat(s)",
             ok ? "<-" : "!! INCOMPLET --", cats);
}

void guest_pump(uint64_t now) {
    const bool on_map = follow_on_map() && guest_between();
    // Les poussees de l'hote gardees parce qu'un ecran des objets est ouvert :
    // identiques a son etat, elles n'ont rien a relire (au plus une fois par
    // seconde : chaque comparaison serialise des chats et le sac). Le sac ouvert
    // arrete la carte : entre deux cases suffit.
    if (held_pushes() && g.n_screens && guest_between() && now - g.same_checked >= kSameCheckMs) {
        g.same_checked = now;
        catsync_drop_identical("ecran des objets ouvert");
        invsync_drop_identical("ecran des objets ouvert");
    }
    // Sac ferme, sur la carte, pas juste apres un ecran de choix (un choix de
    // l'hote peut encore s'y appliquer) : l'etat de l'hote recu, puis les
    // poussees gardees.
    if (on_map && !g.n_screens && !choice_screen_recent(kChoiceQuietMs)) {
        if (g.bundle_have) apply_bundle();
        if (held_pushes()) {
            catsync_apply_pending("sur la carte");
            invsync_apply_pending("sur la carte");
        }
    }
    // Un mouvement de l'hote arrive avant une poussee (ou un etat) appliquee y est
    // deja : laisse.
    const uint32_t seen = last_state_arrival();
    while (!g.queue.empty() && g.queue.front().arrival < seen) {
        g.queue.pop_front();
        ++g.dropped;
    }
    // Les mouvements de l'hote, dans l'ordre, sac ouvert ou non.
    if (guest_can_apply()) {
        while (!g.queue.empty()) {
            const ItemOp& op = g.queue.front().op;
            std::string why;
            const uint8_t r = apply_op(op, why);
            if (r != kItemOk) {
                log_line("SAC", "!! mouvement de %s pas joue ici (%s %s) : %s", mm_peer_name(op.origin).c_str(),
                         op_name(op.op), op.name.c_str(), why.c_str());
                request_sync("un mouvement de l'hote ne se joue pas ici");
                break;
            }
            ++g.applied;
            log_line("SAC", "<- %s %s %s (emplacement %u)", mm_peer_name(op.origin).c_str(), op_name(op.op),
                     op.name.c_str(), (unsigned)op.slot);
            g.queue.pop_front();
        }
    }
    // Ses mouvements sans reponse.
    if (!g.unacked.empty() && now - g.unacked.front().at > kAckWaitMs) {
        log_line("SAC", "!! mouvement %u sans reponse de l'hote", g.unacked.front().seq);
        g.unacked.clear();
        request_sync("un mouvement est reste sans reponse");
    }
    // L'etat demande, pas encore la, l'hote sur la carte : redemande.
    if (g.sync_pending && !g.bundle_have && now - g.sync_asked > kSyncRetryMs && host_settled_for_me())
        request_sync("pas de reponse a la demande d'etat");
}

void reset_run() {
    g.inbox.clear();
    g.queue.clear();
    g.unacked.clear();
    g.scope        = Scope{};
    g.applying     = false;
    g.sync_pending = false;
    g.bundle_have  = false;
    g.bundle       = RunGearMsg{};
    g.rev          = 0;
    g.settled      = false;
    g.node_seed    = 0;
    g.flags_sent   = 0xFF;
}

} // namespace

// ---------------------------------------------------------------------------------------------

bool rungear_binding(Target t, void** detour, void*** original) {
    switch (t) {
        case T_InvScreenUpdate: *detour = (void*)&h_ScreenUpdate; *original = (void**)&o_ScreenUpdate; return true;
        case T_InvScreenDtor:   *detour = (void*)&h_ScreenDtor;   *original = (void**)&o_ScreenDtor;   return true;
        case T_NodeEnd:         *detour = (void*)&h_NodeEnd;      *original = (void**)&o_NodeEnd;      return true;
        case T_InvItemAct:      *detour = (void*)&h_ItemAct;      *original = (void**)&o_ItemAct;      return true;
        case T_InvLearnActive:  *detour = (void*)&h_LearnActive;  *original = (void**)&o_LearnActive;  return true;
        case T_InvLearnPassive: *detour = (void*)&h_LearnPassive; *original = (void**)&o_LearnPassive; return true;
        case T_CatEquip:        *detour = (void*)&h_CatEquip;     *original = (void**)&o_CatEquip;     return true;
        case T_CatUnequip:      *detour = (void*)&h_CatUnequip;   *original = (void**)&o_CatUnequip;   return true;
        default: return false;
    }
}

bool rungear_active() { return g.active; }

bool rungear_screen_open() { return g.n_screens != 0; }

bool rungear_hold_pushes() { return g.n_screens != 0; }

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
    else                          ok = is_host() || guest_live();
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
    const Why why = other ? Why::Other : (is_guest() ? guest_block() : Why::None);
    if (++g.blocked <= 3 || g.blocked % 20 == 0)
        log_line("SAC", "clic retenu (%s) : %s", name, why_name(why));
    tell_why(why, g.touch_owner);
    return true;
}

void rungear_pump() {
    ++g.frame;
    drop_vanished_screens();
    const bool was = g.active;
    g.active = compute_active();
    if (was && !g.active) {
        log_line("SAC", "fin des objets en aventure : %u mouvement(s) envoye(s), %u recu(s), %u joue(s), "
                        "%u refuse(s), %u clic(s) retenu(s), %u laisse(s), %u etat(s) de l'hote",
                 g.sent, g.received, g.applied, g.refused, g.blocked, g.dropped, g.synced);
        reset_run();
    } else if (!was && g.active) {
        log_line("SAC", "objets en direct : chacun n'equipe que ses chats, %s",
                 is_host() ? "l'hote joue et renvoie les mouvements des invites"
                           : "chaque mouvement part chez l'hote, les siens se jouent ici");
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
        g.rev_received = 0;
        g.have_rev = g.adopted = false;
    }
    if (!g.notice.empty() && !mm_info_busy() && !g.n_screens) {
        // Jamais au moment ou il va suivre l'hote : la fenetre arreterait sa
        // carte, et l'hote l'attendrait. Elle attend la carte d'apres la case.
        const bool show = is_host() ? host_map() : (follow_on_map() && guest_between());
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

void rungear_before_node_push() {
    if (!g.active || !is_host()) return;
    // Tout repart : ce qu'un invite aurait de different sur la carte (un
    // mouvement refuse, un mouvement de l'hote qui ne s'est pas joue chez lui)
    // est remis avant la case, meme pour un chat que l'hote n'a pas change.
    catsync_forget();
    invsync_forget();
}

void rungear_on_node_entered(uint64_t seed) {
    if (!g.active || !is_host()) return;
    const uint64_t now = GetTickCount64();
    ++g.rev;
    g.node_seed = seed;
    g.settled   = false;
    send_rev(now);
    // Ce qui attend encore ne sera pas pris : la poussee d'entree vient de partir.
    while (!g.inbox.empty()) {
        const InMsg m = std::move(g.inbox.front());
        g.inbox.pop_front();
        if (m.kind != kTeamItemOp) continue;
        ++g.refused;
        log_line("SAC", "mouvement %u de %s refuse : l'hote entre dans une case", m.op.seq,
                 mm_peer_name(m.from).c_str());
        send_ack(m.from, m.op.seq, kItemNode);
    }
}

void rungear_on_followed(uint64_t seed) {
    if (!is_guest()) return;
    g.entered_seed = seed;
    g.adopted      = true;
    // Les poussees d'avant la case viennent d'etre appliquees, entieres : elles
    // contiennent tout ce que l'hote avait. Un etat de l'hote arrive avant elles
    // est laisse (le relire les defairait) ; ses mouvements arrives avant elles,
    // guest_pump les laisse a l'image suivante, avant d'en jouer aucun.
    if (g.bundle_have && g.bundle_arrival < last_state_arrival()) {
        g.bundle_have = false;
        g.bundle      = RunGearMsg{};
    }
    if (g.sync_pending && !g.bundle_have) g.sync_pending = false;
    g.unacked.clear();
}

bool rungear_hold_follow() {
    if (!g.active || !is_guest()) return false;
    return g.n_screens != 0;
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
}

void rungear_on_team(uint8_t from, const uint8_t* data, size_t size) {
    const uint8_t kind = team_kind(data, size);
    if (kind == kTeamItemOp) {
        InMsg m;
        m.kind = kTeamItemOp;
        m.from = from;
        if (!team_item_op_decode(data, size, m.op)) {
            log_line("SAC", "!! mouvement d'objet illisible de %u -- ignore", (unsigned)from);
            return;
        }
        if (is_host()) {
            if (from == kHostPeer || from >= kPeers || m.op.origin != from) {
                log_line("SAC", "!! mouvement d'objet inattendu de %u -- ignore", (unsigned)from);
                return;
            }
            if (!g.active) {
                send_ack(from, m.op.seq, kItemNode);
                return;
            }
            if (g.inbox.size() >= kMaxQueue) g.inbox.pop_front();
            g.inbox.push_back(std::move(m));
            return;
        }
        if (from != kHostPeer || !g.active) return;      // hors de la copie : rien a jouer
        m.arrival = follow_note_arrival();
        if (g.queue.size() >= kMaxQueue) {
            log_line("SAC", "!! trop de mouvements de l'hote en attente -- l'etat de l'hote est demande");
            g.queue.clear();
            request_sync("trop de mouvements en attente");
        }
        g.queue.push_back(std::move(m));
        return;
    }
    if (kind == kTeamItemAck) {
        ItemAck a;
        if (!is_guest() || from != kHostPeer || !team_item_ack_decode(data, size, a)) {
            log_line("SAC", "!! reponse inattendue ou illisible -- ignoree");
            return;
        }
        std::string name;
        bool     known = false;
        uint32_t mark  = 0;
        while (!g.unacked.empty() && g.unacked.front().seq <= a.seq) {
            if (g.unacked.front().seq == a.seq) {
                name  = g.unacked.front().name;
                mark  = g.unacked.front().mark;
                known = true;
            }
            g.unacked.pop_front();
        }
        if (a.result == kItemOk) {
            log_line("SAC", "<- mouvement %u pris par l'hote", a.seq);
            // Un etat de l'hote arrive entre son depart et sa reponse (une poussee
            // pour un joueur qui arrive) a ete fait AVANT que l'hote ne le prenne :
            // relu ici, il l'a defait. Celui de l'hote, demande maintenant, l'a.
            if (known && (last_state_arrival() != mark || held_pushes()))
                request_sync("un etat de l'hote est arrive pendant un mouvement");
            return;
        }
        ++g.refused;
        log_line("SAC", "<- mouvement %u refuse par l'hote (%s)", a.seq,
                 a.result == kItemConflict ? "l'objet n'y etait plus"
                 : a.result == kItemNode   ? "il etait dans une case" : "invalide");
        if (a.result == kItemNode) { tell_why(Why::Late, -1); return; }
        tell_why(Why::Conflict, -1);
        request_sync(a.result == kItemConflict ? "un mouvement refuse (objet deja pris)"
                                               : "un mouvement refuse (invalide)");
        return;
    }
    if (kind == kTeamItemSyncReq) {
        InMsg m;
        m.kind = kTeamItemSyncReq;
        m.from = from;
        if (!is_host() || from == kHostPeer || from >= kPeers || !team_item_sync_req_decode(data, size, m.seq)) {
            log_line("SAC", "!! demande d'etat inattendue ou illisible -- ignoree");
            return;
        }
        if (!g.active) return;
        if (g.inbox.size() >= kMaxQueue) g.inbox.pop_front();
        g.inbox.push_back(std::move(m));
        return;
    }
    if (kind == kTeamItemSync) {
        RunGearMsg b;
        if (!is_guest() || from != kHostPeer || !team_item_sync_decode(data, size, b)) {
            log_line("SAC", "!! etat de l'hote inattendu ou illisible -- ignore");
            return;
        }
        if (!g.active) return;
        g.bundle         = std::move(b);
        g.bundle_have    = true;
        g.bundle_arrival = follow_note_arrival();
        log_line("SAC", "<- etat de l'hote recu (revision %u, %zu chat(s)) : relu sac ferme, sur la carte",
                 g.bundle.base, g.bundle.cats.size());
        return;
    }
    if (kind == kTeamRunGear || kind == kTeamRunGearAck) {
        log_line("SAC", "!! message d'objets d'une ancienne version (photo) -- ignore");
        return;
    }
}

} // namespace mgmp
