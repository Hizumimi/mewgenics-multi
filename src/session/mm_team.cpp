// mm_team -- Mewgenics Multi : l'equipe mixte. Voir mm_team.h.

#include "mm_team.h"
#include "mm_team_merge.h"
#include "mm_embark.h"
#include "mm_gear.h"
#include "mm_loot.h"
#include "mm_purse_core.h"
#include "mm_recruit_core.h"
#include "mm_rungear.h"
#include "mm_world.h"

#include "mgmp_addresses.h"
#include "mgmp_catsync.h"
#include "mgmp_invsync.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"
#include "mm_menu.h"

#include "sqlite3.h"

#include <windows.h>
#include <algorithm>
#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239) -------------------------------------------------------

// MewDirector+0x4A8 : la SQLSaveFile de la save chargee (MewSaveFile, a +0x38,
// la tient a +0x470). Les fins d'aventure y ouvrent leur transaction
// (0x1403B545E : add rcx, 4A8h ; call BEGIN), Registry::Save y ecrit ses chats.
constexpr uintptr_t kDir_SqlCtx    = 0x4A8;
// MewDirector+0x580 : le jour de la save (propriete current_day, lue en int64
// a 0x1403A6DDA ; ne bouge qu'a la maison). L'age d'un chat se compte avec lui
// (0x1400D31BE).
constexpr uintptr_t kDir_Day       = 0x580;
// MewDirector+0x5A8 : la progression ; un boss attaque la maison quand
// +0x468 != 0 et +0x478 == 0 (meme test que 0x1400AE3E1 : le depart devient
// alors la defense de la maison, que le multijoueur ne sait pas jouer).
constexpr uintptr_t kDir_Progress  = 0x5A8;
constexpr uintptr_t kBoss_Pending  = 0x468;
constexpr uintptr_t kBoss_Busy     = 0x478;
// ButchBox+0x100 : quatre emplacements, chacun un chat de la maison ou nul ;
// la cle de save d'un chat de la maison est a +0x80 (0x1400ADC52).
constexpr uintptr_t kBox_Slots     = 0x100;
constexpr uintptr_t kHouseCat_Key  = 0x80;
// CatData : l'id 64 bits a +0x00 (le premier champ serialise) ; deux liens
// {cle d'un autre chat, force} a +0xBC8/+0xBD0 et +0xBD8/+0xBE0 (-1 : aucun,
// valeur du constructeur 0x14005D859 ; lus par la maison, 0x1400D29E0) ; ses
// drapeaux u64 ; ses jours de naissance et de mort (-1 : vivant), comptes dans
// le calendrier de SA save (0x1400D31B1).
constexpr uintptr_t kCat_Id        = 0x00;
constexpr uintptr_t kCat_LinkA     = 0xBC8;
constexpr uintptr_t kCat_LinkB     = 0xBD8;
constexpr uintptr_t kCat_Flags     = 0xBF8;
constexpr uintptr_t kCat_Born      = 0xC38;
constexpr uintptr_t kCat_Died      = 0xC40;
constexpr uint64_t  kNoLink        = ~0ull;
// Le registre des chats (MewDirector+0x598) : son pedigree a +0x38, celui que
// Registry::NewCat complete (Pedigree::add, 0x1400D71EC) et que Registry::Load
// relit du fichier « pedigree » de la save (0x1400D8DC6).
constexpr uintptr_t kRegistry_Pedigree = 0x38;
constexpr int       kBoxSlots      = 4;
constexpr int       kTeamSize      = 4;
// Les Variant de SQLSaveFile (0x1400D879C, 0x140A0E8E0) : 1 = blob, 5 = entier.
constexpr uint32_t  kVarBlob       = 1;
constexpr uint32_t  kVarInt        = 5;
constexpr int64_t   kNoDay         = INT64_MIN;
constexpr size_t    kMaxDeparts    = 8;   // departs d'un invite en attente de leur retour
constexpr size_t    kMaxPayouts    = 32;  // bourses d'un invite en attente (mm_purse)

using FnButchDepart = bool (__fastcall*)(void* box);
using FnReturnCat   = void (__fastcall*)(void* house, uint64_t key);
using FnRunCleared  = void (__fastcall*)(void* director);
using FnDepartGo    = void (__fastcall*)(void* capture);
using FnSqlUpsert   = void (__fastcall*)(void* sql, void* table, void* key, void* value);
using FnTeamBox     = void* (__fastcall*)(void* house);

// Un std::string de MSVC, petit : 15 caracteres au plus, dans le tampon. Le
// detruire (_Tidy_deallocate) ne libere rien, ce qui le rend sur a confier a
// une fonction du jeu qui consomme sa chaine.
struct GameStr {
    char     buf[16];
    uint64_t size;
    uint64_t cap;
};
static_assert(sizeof(GameStr) == 32, "std::string de MSVC");

// Lu par movaps : aligne sur 16 octets, ou le jeu plante.
struct alignas(16) Variant {
    uint32_t type;
    uint32_t pad;
    uint64_t a;
    uint64_t b;
};

// La capture du depart : la boite, puis la liste {capacite, nombre, cles}.
struct DepartCapture {
    void*     box;
    uint32_t  cap;
    uint32_t  count;
    uint64_t* keys;
};
static_assert(offsetof(DepartCapture, cap) == 8 && offsetof(DepartCapture, keys) == 16,
              "capture de ButchBox::depart");

FnButchDepart o_ButchDepart = nullptr;
FnReturnCat   o_ReturnCat   = nullptr;
FnRunCleared  o_RunCleared  = nullptr;

// --- appels proteges (pas d'objet C++ dans ces fonctions : __try) ---------------

bool safe_team_box(FnTeamBox f, void* house, void** out) {
    __try { *out = f(house); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_upsert(FnSqlUpsert f, void* sql, GameStr* table, Variant* key, Variant* value) {
    __try { f(sql, table, key, value); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

using FnPedigreeAddRaw = void (__fastcall*)(void* pedigree, uint64_t key, uint64_t pa, uint64_t pb);
bool safe_pedigree_add(FnPedigreeAddRaw f, void* pedigree, uint64_t key) {
    __try { f(pedigree, key, ~0ull, ~0ull); return true; } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_depart_go(FnDepartGo f, DepartCapture* c) {
    __try { f(c); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// --- etat ------------------------------------------------------------------------

// Les champs d'un chat qui dependent de SA save : ses jours et ses liens.
struct CatDays {
    int64_t  born = 0;
    int64_t  died = -1;
    uint64_t la   = kNoLink;
    uint64_t lb   = kNoLink;
};

struct PendingTeam {                 // hote : ce qu'un invite amene, pour ce vote
    uint8_t                  peer = 0;
    int64_t                  day  = kNoDay;
    std::vector<TeamCat>     cats;
    std::vector<std::string> names;
};

struct RunCat {                      // hote : un chat d'invite dans l'aventure en cours
    uint64_t    host_key    = 0;     // sa cle dans la save de l'hote
    uint64_t    own_key     = 0;     // sa cle chez son proprietaire
    uint64_t    cat_id      = 0;
    uint8_t     first_owner = 0;     // le numero de son joueur au depart
    uint8_t     owner       = 0;     // ... et la derniere fois qu'on l'a vu
    std::string owner_name;          // son joueur, par son nom
    std::string name;
    CatDays     orig;                // ses jours et liens chez lui
    int64_t     shift = 0;           // jour de l'hote - jour de l'invite
};

struct HostCat {                     // hote : un de ses propres chats dans l'aventure
    uint64_t key = 0;
    uint64_t id  = 0;
};

struct DepartCat {                   // invite : un chat parti avec l'hote
    uint64_t             own_key = 0;
    uint64_t             id      = 0;
    std::string          name;
    std::vector<uint8_t> raw;        // tel qu'au depart
    CatDays              orig;
};

struct GuestDepart {                 // invite : un depart avec ses chats
    uint8_t                seq    = 0;
    std::string            save;     // la save d'ou ils sont partis ("" : inconnue)
    int64_t                day    = kNoDay;
    std::vector<DepartCat> cats;
    bool                   owners_seen  = false;   // la liste de son depart est arrivee
    bool                   departed     = false;   // ... et ses chats y sont
    bool                   withdrawn    = false;   // il a repondu « pas encore » ensuite
    bool                   home_loaded  = false;   // sa save s'est chargee sans l'etat final
    bool                   have_results = false;
    bool                   from_host    = false;
    std::vector<TeamCat>   baseline;   // ses lignes au premier chargement sans l'etat final
    std::vector<TeamCat>   results;
    // Son rangement (mm_gear) : a « J'arrive », tel que le jeu l'avait en memoire
    // (ce qu'il equipera au depart) et tel que le fichier l'avait (pas de ligne :
    // disk_found faux) ; puis ce qu'il en reste, photo appliquee par l'hote.
    bool                   storage_ok   = false;
    std::vector<uint8_t>   storage_mem;
    std::vector<uint8_t>   storage_disk;
    bool                   storage_disk_found = false;
    bool                   storage_post_set   = false;
    std::vector<uint8_t>   storage_post;
};

struct Payout {                      // invite : ce qu'il rapporte d'une aventure gagnee (mm_purse)
    uint8_t     seq   = 0;           // le vote du depart
    std::string save;                // sa save ("" : inconnue -- la prochaine qui se charge)
    int32_t     coins = 0;
    int32_t     food  = 0;
};

struct LootItem {                    // invite : un objet gagne au defile du butin (mm_loot)
    uint8_t     seq  = 0;            // le vote du depart
    uint64_t    key  = 0;            // le defile et l'objet (une seule fois)
    std::string save;                // sa save ("" : inconnue -- la prochaine qui se charge)
    ItemFields  item;
    std::string name;                // pour la fenetre du retour
};
constexpr size_t kMaxLoot = 256;

struct Recruit {                     // hote : un chat qui a rejoint l'equipe en route (mm_recruit_core)
    uint64_t    host_key = 0;
    uint64_t    cat_id   = 0;
    std::string name;
    uint8_t     owner    = kHostPeer;    // son attributaire, par son numero...
    std::string owner_name;              // ... et par son nom (il peut revenir sous un autre numero)
    uint8_t     first_owner = kHostPeer; // son numero quand la recrue lui a ete donnee
};

struct NewCat {                      // invite : une recrue qui rentre chez lui, a ajouter a sa save
    uint8_t              seq      = 0;   // le vote du depart de l'aventure
    std::string          save;           // sa save ("" : inconnue -- la prochaine qui se charge)
    int64_t              host_day = INT64_MIN;
    int64_t              born     = 0;   // ses jours chez l'hote, recales sur sa save (cat_shift_days)
    int64_t              died     = -1;
    std::vector<uint8_t> data;           // serialise, non compresse
    std::string          name;
};

struct PedigreeDue {                 // une recrue ajoutee a une save : le jeu l'inscrit a son pedigree
    std::string save;                // la save ou elle a ete ajoutee (rien ailleurs : une autre save
                                     // peut avoir un autre chat sous la meme cle)
    uint64_t    key  = 0;
    bool        fix_days = false;    // ses jours n'ont pas pu etre recales dans la ligne (cat_shift_days) :
    int64_t     born = 0;            // ... ils le sont en memoire, une fois la save chargee
    int64_t     died = -1;
};
constexpr size_t kMaxNewCats = 32;

// Le temps d'une session.
struct State {
    bool                     on       = false;
    bool                     host     = false;
    uint8_t                  vote_seq = 0;
    std::vector<PendingTeam> pending;
    uint32_t                 roster   = 0;
    // Hote, depart ensemble (mm_embark) : l'equipe est preparee des le vote,
    // le depart du jeu viendra quand les invites auront charge la partie.
    bool                     prepared         = false;
    uint8_t                  prepared_seq     = 0;
    bool                     prepared_brought = false;   // des invites amenaient des chats
};

// Ce qui survit a une session : une aventure continue, des chats sont a rendre.
struct Keep {
    std::vector<TeamOwner>   owners;         // la liste en cours
    std::vector<RunCat>      run;            // hote
    std::vector<HostCat>     host_cats;      // hote
    std::string              run_save;       // hote : la save de cette aventure
    uint8_t                  run_seq = 0;    // hote : le vote de son depart
    uint32_t                 run_gen = 0;    // hote : change a chaque equipe mixte (mm_purse)
    std::vector<GuestDepart> departs;        // invite, du plus ancien au plus recent
    std::vector<uint64_t>    came_home;      // fin d'aventure : chats d'invites que le jeu ramenait
                                             // a la maison (T_HouseReturnCat), cles de la save chargee
    std::vector<std::string> infos;          // fenetres pour ce joueur
    std::vector<std::string> home_infos;     // invite : le retour, une fois chez lui
    std::vector<Payout>      payouts;        // invite : ses bourses, a ajouter a sa save (mm_purse)
    std::vector<LootItem>    loot;           // invite : son butin, a ajouter au rangement de sa save (mm_loot)
    std::vector<Recruit>     recruits;       // hote : les recrues de l'aventure en cours, et a qui elles sont
    std::vector<uint64_t>    team_seen;      // hote : les cles deja vues dans l'equipe de cette aventure
    uint64_t                 catless_since[8] = {};   // hote : sans chat vivant depuis (ms) ; 0 : il en a
    void*                    return_house = nullptr; // hote : la maison ou le jeu ramenait l'equipe
    std::vector<uint64_t>    withheld;       // hote, fin d'aventure : recrues d'invites gardees hors de sa
                                             // maison (h_ReturnCat) -- envoyees, ou rendues a sa maison
    std::vector<std::string> map_infos;      // fenetres a montrer sur la carte (une recrue)
    std::vector<NewCat>      newcats;        // invite : ses recrues, a ajouter a sa save
    std::vector<PedigreeDue> pedigree_due;   // recrues ajoutees a une save, a inscrire au pedigree du jeu
    bool                     backed_up = false;   // invite : copie de securite faite pour ce chargement
    uint32_t departs_made = 0, refused = 0, returned = 0, applied = 0;
    uint32_t recruits_made = 0, recruits_sent = 0, recruits_added = 0;
};

State g;
Keep  k;

// --- petits outils ------------------------------------------------------------------

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

std::string join_names(const std::vector<std::string>& v) {
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += (i + 1 == v.size()) ? " et " : ", ";
        s += v[i];
    }
    return s;
}

const char* plural(size_t n) { return n > 1 ? "s" : ""; }

void say(const std::string& text) { k.infos.push_back(text); }

void* director() {
    const void** slot = (const void**)addr_of_data(D_MewDirectorPtr);
    void* d = nullptr;
    if (!slot || !mem_read(slot, &d, sizeof(d))) return nullptr;
    return d;
}

int64_t save_day(void* dir) {
    int64_t day = kNoDay;
    if (!dir || !mem_read((const char*)dir + kDir_Day, &day, sizeof(day))) return kNoDay;
    return day;
}

bool peer_present(uint8_t peer) {
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (!n || !net_peer_ids(ids, 8)) return false;
    for (uint8_t i = 0; i < n && i < 8; ++i)
        if (ids[i] == peer) return true;
    return false;
}

uint32_t roster_mask() {
    uint8_t ids[8] = {};
    uint32_t mask = 0;
    const uint8_t n = net_peer_count();
    if (n && net_peer_ids(ids, 8))
        for (uint8_t i = 0; i < n && i < 8; ++i)
            if (ids[i] < 32) mask |= 1u << ids[i];
    return mask;
}

bool in_session() { return g.on && net_active() && net_peer_count() >= 2; }

bool send_team(const std::vector<uint8_t>& body, int peer) {
    SaveFileMsg m{};
    m.size = (uint32_t)body.size();
    m.data = const_cast<uint8_t*>(body.data());
    m.hash = savefile_hash(body.data(), m.size);
    strncpy_s(m.name, "equipe", _TRUNCATE);
    return peer < 0 ? net_send_team(m) : net_send_team_to((uint8_t)peer, m);
}

void send_owners(uint8_t seq, const char* why) {
    if (!in_session() || !g.host) return;
    if (send_team(team_owners_encode(seq, k.owners), -1))
        log_line("TEAM", "-> proprietaires envoyes : %zu chat(s), vote %u (%s)", k.owners.size(),
                 (unsigned)seq, why);
    else
        log_line("TEAM", "!! envoi des proprietaires impossible (%s)", why);
}

// --- un chat du jeu -------------------------------------------------------------------

uint64_t cat_id_of(void* cat) {
    uint64_t id = 0;
    if (!cat || !mem_read((const char*)cat + kCat_Id, &id, sizeof(id))) return 0;
    return id;
}

uint64_t cat_id_at(uint64_t key) { return cat_id_of(catsync_cat_by_key(key)); }

bool read_days(void* cat, CatDays& d) {
    return cat && mem_read((const char*)cat + kCat_Born, &d.born, sizeof(d.born)) &&
           mem_read((const char*)cat + kCat_Died, &d.died, sizeof(d.died)) &&
           mem_read((const char*)cat + kCat_LinkA, &d.la, sizeof(d.la)) &&
           mem_read((const char*)cat + kCat_LinkB, &d.lb, sizeof(d.lb));
}

bool write_days(void* cat, const CatDays& d) {
    return cat && mem_write((char*)cat + kCat_Born, &d.born, sizeof(d.born)) &&
           mem_write((char*)cat + kCat_Died, &d.died, sizeof(d.died)) &&
           mem_write((char*)cat + kCat_LinkA, &d.la, sizeof(d.la)) &&
           mem_write((char*)cat + kCat_LinkB, &d.lb, sizeof(d.lb));
}

// Le jeu l'a-t-il ramene a la maison, a cette fin d'aventure ? Il appelle
// House::AddReturningCat (T_HouseReturnCat) pour chaque chat de l'equipe qui
// rentre, juste avant ClearRunState (T_RunCleared) : jamais pour un mort, et
// pour personne quand l'aventure est abandonnee ou perdue (test de
// MewDirector+0x70C, 0x1403B4F03 : seule une defense de la maison ramene son
// equipe). C'est sa regle qui decide, pour les chats des invites comme pour
// ceux de l'hote.
bool came_home(uint64_t key) {
    return std::find(k.came_home.begin(), k.came_home.end(), key) != k.came_home.end();
}

// L'etat final d'un chat d'invite, comme chez lui : ses jours et liens remis,
// sa mort (s'il est mort en route) comptee dans son calendrier ; rentre si le
// jeu le ramenait a la maison, sinon perdu (kFateGone).
bool make_result(void* cat, uint64_t host_key, uint64_t own_key, const CatDays& orig, int64_t shift,
                 TeamCat& out, CatInfo& info) {
    CatDays cur;
    if (!read_days(cat, cur)) return false;
    CatDays back = orig;
    back.died = cur.died == -1 ? -1 : cur.died - shift;
    if (!write_days(cat, back)) return false;
    out = TeamCat{};
    out.own_key  = own_key;
    out.host_key = host_key;
    if (!catsync_serialize_cat(cat, out.data) || !cat_info(out.data, info)) return false;
    out.fate = ((info.flags & kCatDead) || came_home(host_key)) ? kFateHome : kFateGone;
    return true;
}

// --- la boite de depart --------------------------------------------------------------

bool box_keys(void* box, std::vector<uint64_t>& keys) {
    keys.clear();
    void* slots = nullptr;
    if (!box || !mem_read((const char*)box + kBox_Slots, &slots, sizeof(slots)) || !slots) return false;
    for (int i = 0; i < kBoxSlots; ++i) {
        void* hc = nullptr;
        if (!mem_read((const char*)slots + i * sizeof(void*), &hc, sizeof(hc))) return false;
        if (!hc) continue;
        uint64_t key = 0;
        if (!mem_read((const char*)hc + kHouseCat_Key, &key, sizeof(key)) || !key) return false;
        keys.push_back(key);
    }
    return true;
}

enum BoxState { kBoxOk = 0, kNoHouse = -1, kNoBox = -2 };

// La boite de depart de la maison a l'ecran. Le composant 0x1AC n'existe
// qu'une fois la boite debloquee (C_HouseTeamBox rend nul sinon).
void* house_box(BoxState& state) {
    void* house = mm_house();
    if (!house) { state = kNoHouse; return nullptr; }
    auto f = (FnTeamBox)addr_of_call(C_HouseTeamBox);
    void* box = nullptr;
    if (!f || !safe_team_box(f, house, &box) || !box) { state = kNoBox; return nullptr; }
    state = kBoxOk;
    return box;
}

struct BoxCat {
    TeamCat cat;
    CatInfo info;
    CatDays days;
};

// Les chats de la boite de ce joueur : leur nombre, ou kNoHouse / kNoBox ;
// `all_read` faux si un chat n'a pas pu etre lu.
int read_box(std::vector<BoxCat>& out, bool& all_read) {
    out.clear();
    all_read = true;
    BoxState state = kBoxOk;
    std::vector<uint64_t> keys;
    void* box = house_box(state);
    if (!box) return state;
    if (!box_keys(box, keys)) return kNoBox;
    for (uint64_t key : keys) {
        BoxCat bc;
        void* cat = catsync_cat_by_key(key);
        if (!cat || !catsync_serialize_cat(cat, bc.cat.data) || !cat_info(bc.cat.data, bc.info) ||
            !read_days(cat, bc.days)) {
            log_line("TEAM", "!! chat %llu de la boite de depart illisible", (unsigned long long)key);
            all_read = false;
            continue;
        }
        bc.cat.own_key = key;
        out.push_back(std::move(bc));
    }
    return (int)keys.size();
}

std::string too_many_text(int have, int quota) {
    return fmt(u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
               u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
               have, quota, have - quota);
}

// --- hote : les proprietaires -------------------------------------------------------

// Le joueur qui dirige ces chats, s'il est la : son numero du depart s'il porte
// toujours son nom, sinon le seul autre joueur de ce nom -- a condition qu'il
// ne soit pas deja celui d'un autre groupe du meme nom. -1 sinon.
int peer_for(const RunCat& rc) {
    if (rc.owner != kHostPeer && peer_present(rc.owner) && mm_peer_name(rc.owner) == rc.owner_name)
        return rc.owner;
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (!n || !net_peer_ids(ids, 8)) return -1;
    int found = -1;
    for (uint8_t i = 0; i < n && i < 8; ++i) {
        const uint8_t id = ids[i];
        if (id == kHostPeer || mm_peer_name(id) != rc.owner_name) continue;
        bool taken = false;
        for (const RunCat& o : k.run)
            if (o.owner_name == rc.owner_name && o.first_owner != rc.first_owner && o.owner == id &&
                peer_present(o.owner))
                taken = true;
        if (taken) continue;
        if (found >= 0) return -1;           // deux candidats : on ne devine pas
        found = id;
    }
    return found;
}

// Le joueur a qui est cette recrue, s'il est la : son numero s'il porte toujours
// son nom, sinon le seul autre joueur de ce nom ; l'hote pour les siennes ; -1
// s'il est parti.
int recruit_peer(const Recruit& r) {
    if (r.owner == kHostPeer) return kHostPeer;
    if (peer_present(r.owner) && mm_peer_name(r.owner) == r.owner_name) return r.owner;
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (!n || !net_peer_ids(ids, 8)) return -1;
    int found = -1;
    for (uint8_t i = 0; i < n && i < 8; ++i) {
        const uint8_t id = ids[i];
        if (id == kHostPeer || mm_peer_name(id) != r.owner_name) continue;
        // Un autre joueur du meme nom, qui dirige deja ses propres chats (un
        // autre numero au depart) : ce n'est pas lui qui revient (comme peer_for).
        bool taken = false;
        for (const RunCat& o : k.run)
            if (o.owner_name == r.owner_name && o.first_owner != r.first_owner && o.owner == id &&
                peer_present(o.owner))
                taken = true;
        if (taken) continue;
        if (found >= 0) return -1;           // deux candidats : on ne devine pas
        found = id;
    }
    return found;
}

// La liste des proprietaires, refaite d'apres les joueurs presents. Vrai si
// elle a change.
bool rebuild_owners() {
    std::vector<TeamOwner> fresh;
    for (const HostCat& hc : k.host_cats) fresh.push_back(TeamOwner{ hc.id, hc.key, kHostPeer });
    for (RunCat& rc : k.run) {
        const int peer = peer_for(rc);
        if (peer >= 0) rc.owner = (uint8_t)peer;
        fresh.push_back(TeamOwner{ rc.cat_id, rc.host_key, peer >= 0 ? (uint8_t)peer : kHostPeer });
    }
    // Les recrues (mm_recruit_core) : a leur attributaire tant qu'il est la.
    bool guest_recruit = false;
    for (Recruit& r : k.recruits) {
        const int peer = recruit_peer(r);
        if (peer >= 0) r.owner = (uint8_t)peer;
        guest_recruit |= peer >= 0 && peer != (int)kHostPeer;
        fresh.push_back(TeamOwner{ r.cat_id, r.host_key, peer >= 0 ? (uint8_t)peer : kHostPeer });
    }
    // Aventure sans chat d'invite et sans recrue a un invite : pas de liste, le
    // partage des combats reste celui de mgmp (par place).
    if (k.run.empty() && k.host_cats.empty() && !guest_recruit) fresh.clear();
    bool same = fresh.size() == k.owners.size();
    for (size_t i = 0; same && i < fresh.size(); ++i)
        same = fresh[i].cat_id == k.owners[i].cat_id && fresh[i].host_key == k.owners[i].host_key &&
               fresh[i].peer == k.owners[i].peer;
    k.owners = std::move(fresh);
    return !same;
}

// Hote : une nouvelle aventure (ou plus d'aventure du tout) -- les recrues de la
// precedente ne comptent plus, ni sa photo de l'equipe : la premiere photo de la
// suivante sera son depart. Sans cela, les chats d'une aventure sans chat
// d'invite ni recrue restaient « vus », et toute l'equipe de la suivante passait
// pour des recrues (relecture du 17 octobre) -- chaque depart en session, la fin
// de chaque aventure et chaque chargement d'une save repartent donc d'ici.
void reset_recruits() {
    k.recruits.clear();
    k.team_seen.clear();
    for (uint64_t& t : k.catless_since) t = 0;
    k.return_house = nullptr;
    k.withheld.clear();
}

void forget_run(const char* why) {
    const bool had = !k.owners.empty() || !k.run.empty() || !k.host_cats.empty() || !k.recruits.empty();
    k.owners.clear();
    k.run.clear();
    k.host_cats.clear();
    k.run_save.clear();
    reset_recruits();
    if (had) {
        log_line("TEAM", "plus d'equipe mixte en cours (%s)", why);
        send_owners(k.run_seq, why);
    }
}

// --- hote : les recrues (mm_recruit_core) ------------------------------------------------

// L'equipe de l'aventure : ses cles (MewDirector+0x5C0, que parcourt le CatSelector).
bool team_keys(std::vector<uint64_t>& out) {
    out.clear();
    const uint8_t*  dir  = (const uint8_t*)director();
    uint32_t        n    = 0;
    const uint64_t* keys = nullptr;
    if (!dir || !mem_read(dir + kDir_CatIdCount, &n, sizeof(n)) ||
        !mem_read(dir + kDir_CatIdData, &keys, sizeof(keys)) || n > 16 || (n && !keys))
        return false;
    out.resize(n);
    return n == 0 || mem_read(keys, out.data(), n * sizeof(uint64_t));
}

bool cat_alive_at(uint64_t key) {
    void*    cat   = catsync_cat_by_key(key);
    uint64_t flags = 0;
    return cat && mem_read((const char*)cat + kCat_Flags, &flags, sizeof(flags)) && !(flags & kCatDead);
}

// A qui est ce chat de l'equipe : un invite present pour les siens et ses
// recrues, l'hote pour tous les autres.
int key_owner(uint64_t key) {
    for (const RunCat& rc : k.run)
        if (rc.host_key == key) {
            const int p = peer_for(rc);
            return p >= 0 ? p : kHostPeer;
        }
    for (const Recruit& r : k.recruits)
        if (r.host_key == key) {
            const int p = recruit_peer(r);
            return p >= 0 ? p : kHostPeer;
        }
    return kHostPeer;
}

std::string no_markup(std::string s) {
    for (char& c : s)
        if (c == '[') c = '(';
        else if (c == ']') c = ')';
    return s;
}

// Le nom d'un chat, tel que le jeu le serialise (sans [ ] : balises).
std::string cat_display_name(void* cat) {
    std::vector<uint8_t> raw;
    CatInfo info;
    if (cat && catsync_serialize_cat(cat, raw) && cat_info(raw, info) && !info.name.empty())
        return no_markup(info.name);
    return u8"Un nouveau chat";
}

// Le tirage au sort de l'hote, hors du flux du jeu (il ne doit rien lui prendre).
uint32_t recruit_roll(uint32_t n, void*) {
    static uint64_t s = 0;
    if (!s) s = GetTickCount64() ^ 0x9E3779B97F4A7C15ull;
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return n ? (uint32_t)(s % n) : 0;
}

// La fenetre de chacun quand une recrue arrive.
std::string recruit_text(const std::string& cat, int owner) {
    const uint8_t self = net_self();
    if (owner == (int)self && self != kHostPeer)
        return fmt(u8"Nouveau chat dans l'équipe\n\n"
                   u8"%s rejoint l'équipe : ce chat est à toi !\n"
                   u8"Tu le diriges, et il rentrera chez toi\n"
                   u8"à la fin de l'aventure.", cat.c_str());
    if (owner == (int)self)
        return fmt(u8"Nouveau chat dans l'équipe\n\n"
                   u8"%s rejoint l'équipe : ce chat est à toi.", cat.c_str());
    return fmt(u8"Nouveau chat dans l'équipe\n\n"
               u8"%s rejoint l'équipe : ce chat est à %s.", cat.c_str(),
               no_markup(mm_peer_name((uint8_t)owner)).c_str());
}

// Hote, a chaque image d'une aventure en session : un chat de l'equipe que
// l'aventure n'avait pas a son depart est une recrue (un evenement, un
// suiveur qui prend une place libre...). Elle va a un spectateur -- celui qui
// est sans chat vivant depuis le plus longtemps --, sinon au joueur qui a le
// moins de chats vivants (mm_recruit_core, egalites au sort). Chacun le sait par
// la liste des proprietaires (le partage des combats, les choix) et par une
// fenetre ; a la fin, elle rentre chez son attributaire (send_new_cats).
void recruits_tick() {
    if (!g.on || !g.host || !in_session() || !savefile_run_published()) return;
    std::vector<uint64_t> keys;
    if (!team_keys(keys) || keys.empty()) return;
    // La premiere photo d'une aventure : l'equipe de son depart (reset_recruits
    // l'efface a chaque depart, a chaque fin d'aventure et a chaque chargement).
    if (k.team_seen.empty()) {
        k.team_seen = keys;
        return;
    }
    const uint64_t now    = GetTickCount64();
    const uint8_t  in_run = (uint8_t)(mm_run_peers_mask() & 0xFFu);
    // Les chats vivants de chacun, sans ceux qui arrivent a cette image : une
    // recrue pas encore attribuee n'est a personne (key_owner la donnerait a
    // l'hote, qui ne serait alors jamais spectateur).
    int living[8] = {};
    for (uint64_t key : keys) {
        if (std::find(k.team_seen.begin(), k.team_seen.end(), key) == k.team_seen.end()) continue;
        const int o = key_owner(key);
        if (o >= 0 && o < 8 && cat_alive_at(key)) ++living[o];
    }
    auto playing = [&](uint8_t p) { return p == kHostPeer || ((in_run >> p) & 1u) != 0; };
    for (uint8_t p = 0; p < 8; ++p) {
        if (!playing(p) || living[p] > 0) k.catless_since[p] = 0;
        else if (!k.catless_since[p]) k.catless_since[p] = now;
    }
    bool added = false;
    for (uint64_t key : keys) {
        if (std::find(k.team_seen.begin(), k.team_seen.end(), key) != k.team_seen.end()) continue;
        k.team_seen.push_back(key);
        void* cat = catsync_cat_by_key(key);
        const uint64_t id = cat_id_of(cat);
        if (!id) continue;
        std::vector<RecruitPlayer> players;
        for (uint8_t p = 0; p < 8; ++p)
            if (playing(p)) players.push_back(RecruitPlayer{ p, living[p], k.catless_since[p] });
        const int pick = recruit_owner(players, recruit_roll, nullptr);
        Recruit r;
        r.host_key   = key;
        r.cat_id     = id;
        r.name       = cat_display_name(cat);
        r.owner      = pick >= 0 && pick < 8 ? (uint8_t)pick : kHostPeer;
        r.owner_name = mm_peer_name(r.owner);
        r.first_owner = r.owner;
        ++living[r.owner];
        k.catless_since[r.owner] = 0;
        k.recruits.push_back(r);
        ++k.recruits_made;
        added = true;
        log_line("TEAM", "recrue : %s (cle %llu) rejoint l'equipe -- elle est a %s (%s)", r.name.c_str(),
                 (unsigned long long)key, r.owner_name.c_str(),
                 players.size() < 2 ? "seul joueur de l'aventure" : "spectateur, sinon le moins de chats");
        RecruitNote note;
        note.seq      = k.run_seq;
        note.cat_id   = id;
        note.host_key = key;
        note.owner    = r.owner;
        note.name     = r.name;
        if (!send_team(team_recruit_encode(note), -1))
            log_line("TEAM", "!! recrue : la fenetre des invites n'a pas pu partir");
        k.map_infos.push_back(recruit_text(r.name, r.owner));
    }
    if (added && rebuild_owners()) send_owners(k.run_seq, "une recrue");
}

// --- hote : le depart -------------------------------------------------------------------

bool house_defense_pending() {
    void* d = director();
    void* prog = nullptr;
    void* pending = nullptr;
    int32_t busy = 1;
    if (!d || !mem_read((const char*)d + kDir_Progress, &prog, sizeof(prog)) || !prog) return false;
    if (!mem_read((const char*)prog + kBoss_Pending, &pending, sizeof(pending)) || !pending) return false;
    if (!mem_read((const char*)prog + kBoss_Busy, &busy, sizeof(busy))) return false;
    return busy == 0;
}

// Une ligne de la table cats, par la connexion du jeu (comme Registry::Save).
bool write_cat_row(void* dir, uint64_t key, const std::vector<uint8_t>& blob) {
    auto f = (FnSqlUpsert)addr_of_call(C_SqlUpsert);
    if (!f || !dir || blob.empty()) return false;
    GameStr table{};
    memcpy(table.buf, "cats", 5);
    table.size = 4;
    table.cap  = 15;
    Variant kv{};
    kv.type = kVarInt;
    kv.a    = key;
    Variant bv{};
    bv.type = kVarBlob;
    bv.a    = (uint64_t)(uintptr_t)blob.data();
    bv.b    = (uint64_t)blob.size();
    return safe_upsert(f, (char*)dir + kDir_SqlCtx, &table, &kv, &bv);
}

// Le compteur du jeu passe a `last` s'il est en dessous : son prochain chat
// prendra la cle d'apres.
bool bump_counter(uint64_t last) {
    void* p = (void*)addr_of_data(D_CatKeyCounter);
    uint64_t v = 0;
    if (!p || !mem_read(p, &v, sizeof(v))) return false;
    return v >= last || mem_write(p, &last, sizeof(last));
}

// Pas d'equipe mixte pour ce depart : ni aventure ni chats d'invites en memoire.
void drop_mixed() {
    k.owners.clear();
    k.run.clear();
    k.host_cats.clear();
    k.run_save.clear();
    g.pending.clear();
}

// Les chats des invites ne partent pas : h_ButchDepart dit ensuite a chacun la
// suite (l'hote part avec les siens, ou personne ne part). `written` : des chats
// deja ecrits (inoffensif, ils reprendront leur ligne au prochain depart).
void refuse_mixed(const std::string& why, size_t written = 0) {
    log_line("TEAM", "!! equipe mixte impossible : %s%s", why.c_str(),
             written ? " (les chats deja ecrits restent dans la save, marques : sans effet)" : "");
    ++k.refused;
    drop_mixed();
}

bool has_pending_cats() {
    for (const PendingTeam& pt : g.pending)
        if (!pt.cats.empty()) return true;
    return false;
}

// Prepare l'equipe mixte : les chats des invites ecrits dans la save de l'hote
// (marques), relus par le jeu, recales sur son calendrier ; l'aventure et ses
// proprietaires en memoire. Faux si aucun ne peut venir (refuse_mixed l'a dit
// au journal) : l'hote partira avec les siens.
bool prepare_mixed(void* box) {
    std::vector<uint64_t> hkeys;
    if (!box_keys(box, hkeys)) {
        refuse_mixed("boite de depart de l'hote illisible");
        return false;
    }
    if (house_defense_pending()) {
        refuse_mixed("un boss attaque la maison");
        return false;
    }
    void* dir = director();
    if (!dir || !addr_of_call(C_SqlUpsert) || !addr_of_call(C_DepartGo) ||
        !addr_of_data(D_CatKeyCounter)) {
        refuse_mixed("fonctions du jeu introuvables (signatures)");
        return false;
    }

    // Les chats des invites, par numero de joueur : chacun sa part au plus, tant
    // qu'il reste de la place. (L'hote peut n'en avoir aucun : il a vide sa
    // boite pendant le vote. L'equipe est alors celle des invites.) Un chat du
    // meme id qu'un chat de l'hote (un clone de l'un chez l'autre) reste chez
    // lui : le meme chat deux fois dans l'equipe, le jeu ne le prevoit pas, et
    // ses proprietaires se confondraient.
    std::vector<uint64_t> host_ids;
    for (uint64_t hk : hkeys)
        if (const uint64_t id = cat_id_at(hk)) host_ids.push_back(id);
    std::sort(g.pending.begin(), g.pending.end(),
              [](const PendingTeam& a, const PendingTeam& b) { return a.peer < b.peer; });
    const int quota = team_quota(false, (int)net_peer_count());
    std::vector<TeamCat>     cats;
    std::vector<size_t>      from;          // index dans g.pending
    std::vector<std::string> left;
    int room = kTeamSize - (int)hkeys.size();
    for (size_t p = 0; p < g.pending.size(); ++p) {
        const PendingTeam& pt = g.pending[p];
        if (!peer_present(pt.peer)) {
            log_line("TEAM", "le joueur %u est parti : ses chats ne viennent pas", (unsigned)pt.peer);
            continue;
        }
        int taken = 0;
        for (size_t i = 0; i < pt.cats.size(); ++i) {
            CatInfo ci;
            if (cat_info(pt.cats[i].data, ci) &&
                std::find(host_ids.begin(), host_ids.end(), ci.id) != host_ids.end()) {
                log_line("TEAM", "!! un chat reste a la maison : %s est deja dans l'equipe de l'hote "
                                 "(meme id : un clone)", pt.names[i].c_str());
                continue;
            }
            if (room <= 0 || taken >= quota) { left.push_back(pt.names[i]); continue; }
            cats.push_back(pt.cats[i]);
            from.push_back(p);
            ++taken;
            --room;
        }
    }
    if (!left.empty())
        log_line("TEAM", "!! equipe pleine : %s reste(nt) a la maison", join_names(left).c_str());

    // Leurs cles : une ligne deja la pour un chat deja venu, sinon apres la
    // table de l'hote et apres le compteur du jeu. Un chat refuse reste chez lui.
    uint64_t counter = 0;
    if (!mem_read((void*)addr_of_data(D_CatKeyCounter), &counter, sizeof(counter))) {
        refuse_mixed("compteur des cles du jeu illisible");
        return false;
    }
    std::vector<KeyPlan> plan;
    if (!cats.empty()) {
        sqlite3* snap = world_open_host_save_now();
        if (!snap) {
            refuse_mixed("la save de l'hote ne se lit pas");
            return false;
        }
        std::string why;
        const bool planned = team_plan_keys(snap, cats, counter + 1, plan, why);
        sqlite3_close(snap);
        if (!planned) {
            refuse_mixed(why);
            return false;
        }
        for (size_t i = plan.size(); i-- > 0;)
            if (plan[i].refused) {
                log_line("TEAM", "!! un chat reste a la maison : %s", plan[i].why.c_str());
                plan.erase(plan.begin() + (ptrdiff_t)i);
                cats.erase(cats.begin() + (ptrdiff_t)i);
                from.erase(from.begin() + (ptrdiff_t)i);
            }
    }
    if (cats.empty()) {
        refuse_mixed("aucun chat d'invite ne peut partir");
        return false;
    }
    if (hkeys.empty())
        log_line("TEAM", "la boite de l'hote est vide : l'equipe sera celle des invites");
    uint64_t last_new = 0;
    for (const KeyPlan& kp : plan) {
        if (kp.reuse) continue;
        if (catsync_cat_by_key(kp.key)) {
            refuse_mixed(fmt("la cle %llu est deja prise", (unsigned long long)kp.key));
            return false;
        }
        last_new = std::max(last_new, kp.key);
    }
    // Le compteur du jeu d'abord : quoi qu'il arrive ensuite, son prochain chat
    // ne prendra jamais une de ces cles (au pire, des numeros sautes).
    if (last_new && !bump_counter(last_new)) {
        refuse_mixed("compteur des cles du jeu illisible");
        return false;
    }

    // Ecrits marques, par la connexion du jeu ; relus par le jeu (un chat deja
    // en memoire, d'une aventure d'avant, est rafraichi) ; puis recales sur le
    // calendrier de l'hote, sans liens chez lui.
    const int64_t host_day = save_day(dir);
    std::vector<RunCat> run;
    for (size_t i = 0; i < cats.size(); ++i) {
        const PendingTeam& pt = g.pending[from[i]];
        CatInfo info;
        std::vector<uint8_t> raw = cats[i].data;
        if (!cat_info(raw, info) || !cat_set_flags(raw, info.flags | kCatGuest) ||
            !write_cat_row(dir, plan[i].key, cat_blob_pack(raw))) {
            refuse_mixed(fmt("%s n'a pas pu etre ecrit dans la save", info.name.c_str()), i);
            return false;
        }
        void* cat = catsync_cat_by_key(plan[i].key);
        if (cat && plan[i].reuse && !catsync_deserialize_cat(cat, raw)) cat = nullptr;
        RunCat rc;
        if (!cat || cat_id_of(cat) != info.id || !read_days(cat, rc.orig)) {
            refuse_mixed(fmt("le jeu ne relit pas %s", info.name.c_str()), i + 1);
            return false;
        }
        rc.shift = (host_day != kNoDay && pt.day != kNoDay) ? host_day - pt.day : 0;
        CatDays here = rc.orig;
        here.born += rc.shift;
        if (here.died != -1) here.died += rc.shift;
        here.la = here.lb = kNoLink;
        if (!write_days(cat, here)) {
            refuse_mixed(fmt("%s ne se recale pas sur le jour de l'hote", info.name.c_str()), i + 1);
            return false;
        }
        // Et la ligne aussi : la save peut partir telle quelle chez les invites
        // (depart ensemble), leur copie doit avoir le chat comme l'hote l'a en
        // memoire -- recale, sans liens. (Echec : la copie aura l'ancienne
        // ligne, sans effet sur l'aventure de l'hote.)
        std::vector<uint8_t> now;
        if (!catsync_serialize_cat(cat, now) || !write_cat_row(dir, plan[i].key, cat_blob_pack(now)))
            log_line("TEAM", "!! %s : ligne recalee non ecrite (la copie des invites aura l'ancienne)",
                     info.name.c_str());
        rc.host_key    = plan[i].key;
        rc.own_key     = cats[i].own_key;
        rc.cat_id      = info.id;
        rc.first_owner = pt.peer;
        rc.owner       = pt.peer;
        rc.owner_name  = mm_peer_name(pt.peer);
        rc.name        = info.name;
        run.push_back(std::move(rc));
    }

    // L'aventure et ses proprietaires, avant le depart du jeu (qui change de scene).
    k.came_home.clear();
    char save[64] = {};
    k.run      = std::move(run);
    k.run_save = savefile_host_save_name(save, sizeof(save)) ? save : "";
    k.run_seq  = g.vote_seq;
    ++k.run_gen;                             // les bourses de cette aventure repartent de zero
    k.host_cats.clear();
    for (uint64_t hk : hkeys)
        if (const uint64_t id = cat_id_at(hk)) k.host_cats.push_back(HostCat{ hk, id });
    rebuild_owners();
    std::vector<std::string> names;
    for (size_t i = 0; i < k.run.size(); ++i)
        names.push_back(fmt("%s (%s, cle %llu%s, %+lld jour(s))", k.run[i].name.c_str(),
                            k.run[i].owner_name.c_str(), (unsigned long long)k.run[i].host_key,
                            plan[i].reuse ? ", reprise" : "", (long long)k.run[i].shift));

    g.pending.clear();
    log_line("TEAM", "equipe mixte prete : %zu chat(s) de l'hote + %s", hkeys.size(),
             join_names(names).c_str());
    return true;
}

// Le depart du jeu, directement (C_DepartGo, comme la fin de ButchBox::depart) :
// le vote valait confirmation, pas de « moins de 4 chats ? » -- cette boite de
// ButchBox::depart ne fait partir qu'a son « oui » (0x1400AFDB4), et
// ButchBox::depart rend vrai des qu'elle est ouverte (0x1400AE111).
void depart_keys(void* box, std::vector<uint64_t> keys) {
    DepartCapture capture{ box, (uint32_t)keys.size(), (uint32_t)keys.size(), keys.data() };
    if (!safe_depart_go((FnDepartGo)addr_of_call(C_DepartGo), &capture))
        log_line("TEAM", "!! le depart du jeu a echoue en route -- l'aventure est peut-etre incomplete");
}

// Le depart en equipe mixte : les chats de la boite de l'hote (lus maintenant :
// il a pu la changer pendant l'attente), puis ceux des invites prepares. Les
// cles, dans l'ordre ou le jeu les recoit.
std::vector<uint64_t> depart_mixed(void* box, const std::vector<uint64_t>& hkeys) {
    std::vector<uint64_t> all = hkeys;
    for (const RunCat& rc : k.run) all.push_back(rc.host_key);
    k.host_cats.clear();
    for (uint64_t hk : hkeys)
        if (const uint64_t id = cat_id_at(hk)) k.host_cats.push_back(HostCat{ hk, id });
    rebuild_owners();
    ++k.departs_made;
    log_line("TEAM", "equipe mixte : %zu chat(s) de l'hote + %zu d'invite(s) -- depart", hkeys.size(),
             k.run.size());
    depart_keys(box, all);
    send_owners(k.run_seq, "depart");
    return all;
}

// Depart ensemble : l'equipe, pour que chaque invite fasse le meme depart.
void send_embark(uint8_t seq, const std::vector<uint64_t>& keys) {
    if (keys.empty()) return;
    if (send_team(team_embark_encode(seq, keys), -1))
        log_line("TEAM", "-> equipe du depart envoyee aux invites (%zu chat(s), vote %u)", keys.size(),
                 (unsigned)seq);
    else
        log_line("TEAM", "!! envoi de l'equipe du depart impossible");
}

// --- hote : le retour ---------------------------------------------------------------------

// Un chat d'invite ? Avec une aventure en memoire, c'est elle qui decide (cle
// et id) ; un chat marque qui n'en est pas est un clone (CloneCat 0x1400D7530
// copie l'id et les drapeaux) : il devient un chat de l'hote, sans la marque.
// Sans memoire (l'hote a redemarre en pleine aventure) : la marque.
bool is_guest_cat(uint64_t key) {
    void* cat = catsync_cat_by_key(key);
    const uint64_t id = cat_id_of(cat);
    uint64_t flags = 0;
    const bool marked = cat && mem_read((const char*)cat + kCat_Flags, &flags, sizeof(flags)) &&
                        (flags & kCatGuest) != 0;
    if (k.run.empty()) return marked;
    for (const RunCat& rc : k.run)
        if (rc.host_key == key && id == rc.cat_id) return true;
    if (marked) {
        flags &= ~kCatGuest;
        if (mem_write((char*)cat + kCat_Flags, &flags, sizeof(flags)))
            log_line("TEAM", "chat %llu : marque d'invite sur un chat de l'hote (un clone ?) -- "
                             "effacee, il reste chez l'hote", (unsigned long long)key);
    }
    return false;
}

// Hote : la recrue a cette cle, si elle est a un invite encore la.
const Recruit* guest_recruit_at(uint64_t key) {
    for (const Recruit& r : k.recruits) {
        if (r.host_key != key || r.owner == kHostPeer || cat_id_at(key) != r.cat_id) continue;
        const int p = recruit_peer(r);
        return p >= 0 && p != (int)kHostPeer ? &r : nullptr;
    }
    return nullptr;
}

// Hote, a la fin de l'aventure : les recrues que le jeu ramenait a la maison et
// qui sont a un invite encore la partent chez lui (kTeamNewCats), sans liens (ce
// seraient des chats de l'hote), avec leurs jours tels quels et le jour de
// l'hote (l'invite les recale sur sa save) ; elles restent dans la save de
// l'hote marquees kCatGuest, hors de sa maison, comme les chats d'invites. Un
// envoi qui echoue : elles rentrent chez l'hote (o_ReturnCat, avant la fin de
// course du jeu, comme les autres).
void send_new_cats(void* dir) {
    // Une recrue gardee hors de la maison de l'hote (h_ReturnCat) qui ne part pas
    // finalement -- son joueur est parti entre-temps, elle est illisible, l'envoi
    // echoue -- rentre chez l'hote, comme le jeu le voulait : rien ne se perd.
    auto give_back = [](uint64_t key, const char* why) {
        if (k.return_house) {
            o_ReturnCat(k.return_house, key);
            log_line("TEAM", "recrue (cle %llu) : %s -- elle rentre chez l'hote", (unsigned long long)key, why);
        } else {
            log_line("TEAM", "!! recrue (cle %llu) : %s, et la maison de l'hote est inconnue", (unsigned long long)key,
                     why);
        }
    };
    std::vector<int> owners;
    for (uint64_t key : k.withheld) {
        const Recruit* r = guest_recruit_at(key);
        if (!r) { give_back(key, "son joueur n'est plus la"); continue; }
        const int p = recruit_peer(*r);
        if (std::find(owners.begin(), owners.end(), p) == owners.end()) owners.push_back(p);
    }
    for (int p : owners) {
        NewCatsMsg msg;
        msg.seq      = k.run_seq;
        msg.host_day = save_day(dir);
        std::vector<std::string> names;
        std::vector<uint64_t>    keys;
        std::vector<CatDays>     was_days;   // pour tout remettre si l'envoi echoue
        std::vector<uint64_t>    was_flags;
        for (uint64_t key : k.withheld) {
            const Recruit* r = guest_recruit_at(key);
            if (!r || recruit_peer(*r) != p) continue;
            void* cat = catsync_cat_by_key(key);
            CatDays days;
            uint64_t flags = 0;
            if (!cat) {
                give_back(key, "introuvable a la fin de l'aventure");
                continue;
            }
            if (!read_days(cat, days) || !mem_read((const char*)cat + kCat_Flags, &flags, sizeof(flags))) {
                give_back(key, "illisible");
                continue;
            }
            if (days.died != -1 || (flags & kCatDead)) {
                give_back(key, "morte");             // le jeu la ramenait quand meme : il decide
                continue;
            }
            CatDays bare = days;
            bare.la = kNoLink;
            bare.lb = kNoLink;
            const uint64_t marked = flags | kCatGuest;
            NewCatEntry e;
            e.host_key = key;
            e.born     = days.born;
            e.died     = days.died;
            if (!write_days(cat, bare) || !mem_write((char*)cat + kCat_Flags, &marked, sizeof(marked)) ||
                !catsync_serialize_cat(cat, e.data)) {
                write_days(cat, days);
                mem_write((char*)cat + kCat_Flags, &flags, sizeof(flags));
                give_back(key, "copie impossible");
                continue;
            }
            msg.cats.push_back(std::move(e));
            names.push_back(r->name);
            keys.push_back(key);
            was_days.push_back(days);
            was_flags.push_back(flags);
        }
        if (msg.cats.empty()) continue;
        const std::string who = no_markup(mm_peer_name((uint8_t)p));
        if (send_team(team_newcats_encode(msg), p)) {
            k.recruits_sent += (uint32_t)msg.cats.size();
            log_line("TEAM", "-> recrue(s) %s envoyee(s) chez %s (jour de l'hote %lld)", join_names(names).c_str(),
                     who.c_str(), (long long)msg.host_day);
            say(fmt(u8"Fin de l'aventure\n\n%s %s chez %s.", join_names(names).c_str(),
                    names.size() > 1 ? u8"rentrent" : u8"rentre", who.c_str()));
            continue;
        }
        // Pas envoye : chez l'hote, comme si elles etaient les siennes -- leurs
        // liens et leurs drapeaux d'avant remis.
        for (size_t i = 0; i < keys.size(); ++i) {
            if (void* cat = catsync_cat_by_key(keys[i])) {
                write_days(cat, was_days[i]);
                mem_write((char*)cat + kCat_Flags, &was_flags[i], sizeof(was_flags[i]));
            }
            give_back(keys[i], "envoi impossible");
        }
        log_line("TEAM", "!! les recrues de %s n'ont pas pu partir (%s) -- elles restent chez l'hote", who.c_str(),
                 join_names(names).c_str());
    }
    k.withheld.clear();
}

void send_results() {
    // Un message par joueur : son nom ET son numero au depart (deux joueurs
    // peuvent porter le meme nom).
    std::vector<size_t> heads;
    for (size_t i = 0; i < k.run.size(); ++i) {
        bool seen = false;
        for (size_t h : heads)
            if (k.run[h].owner_name == k.run[i].owner_name && k.run[h].first_owner == k.run[i].first_owner)
                seen = true;
        if (!seen) heads.push_back(i);
    }
    for (size_t h : heads) {
        const RunCat&            head = k.run[h];
        TeamMsg                  msg;
        std::vector<std::string> back, lost, gone;
        msg.kind = kTeamResult;
        msg.seq  = k.run_seq;
        for (const RunCat& rc : k.run) {
            if (rc.owner_name != head.owner_name || rc.first_owner != head.first_owner) continue;
            void* cat = catsync_cat_by_key(rc.host_key);
            TeamCat t;
            CatInfo info;
            if (!cat || cat_id_of(cat) != rc.cat_id ||
                !make_result(cat, rc.host_key, rc.own_key, rc.orig, rc.shift, t, info)) {
                log_line("TEAM", "!! %s (cle %llu) introuvable a la fin de l'aventure -- il garde chez %s "
                                 "son etat d'avant le depart", rc.name.c_str(),
                         (unsigned long long)rc.host_key, rc.owner_name.c_str());
                continue;
            }
            ((info.flags & kCatDead) ? lost : t.fate == kFateGone ? gone : back).push_back(rc.name);
            msg.cats.push_back(std::move(t));
        }
        if (msg.cats.empty()) continue;
        const int peer = in_session() ? peer_for(head) : -1;
        if (peer >= 0 && send_team(team_encode(msg), peer)) {
            ++k.returned;
            log_line("TEAM", "-> etat final des chats de %s envoye : rentre(s) %s, mort(s) %s, pas revenu(s) %s",
                     head.owner_name.c_str(), back.empty() ? "aucun" : join_names(back).c_str(),
                     lost.empty() ? "aucun" : join_names(lost).c_str(),
                     gone.empty() ? "aucun" : join_names(gone).c_str());
        } else {
            log_line("TEAM", "!! %s n'est plus la : ses chats gardent chez lui leur etat d'avant le depart "
                             "(ou celui de sa copie de l'aventure)", head.owner_name.c_str());
        }
    }
}

// --- invite : ses departs ------------------------------------------------------------------

GuestDepart* departure(uint8_t seq) {
    for (size_t i = k.departs.size(); i-- > 0;)
        if (k.departs[i].seq == seq) return &k.departs[i];
    return nullptr;
}

// Les chats du depart `newest` sont partis : un chat ne rentre que de son
// dernier depart. S'il etait aussi dans un depart plus ancien de la meme save,
// encore en attente (son etat final arrive en retard, pas encore recopie), il
// n'en fait plus partie -- l'aventure d'apres est partie de l'etat de la save,
// et c'est son etat final a elle qui compte. Sinon, le plus ancien serait
// recopie d'abord (mort, il quitterait la maison), puis le plus recent
// l'ecraserait (vivant, mais hors de la maison).
void supersede_older(size_t newest) {
    if (newest >= k.departs.size()) return;
    const std::string      save = k.departs[newest].save;
    std::vector<DepartCat> cats = k.departs[newest].cats;
    std::vector<std::string> names;
    for (size_t i = newest; i-- > 0;) {                // du plus recent au plus ancien
        GuestDepart& o = k.departs[i];
        if (!o.save.empty() && !save.empty() && _stricmp(o.save.c_str(), save.c_str()) != 0) continue;
        for (size_t c = o.cats.size(); c-- > 0;) {
            bool again = false;
            for (const DepartCat& n : cats)
                if (n.own_key == o.cats[c].own_key && n.id == o.cats[c].id) again = true;
            if (!again) continue;
            const uint64_t key = o.cats[c].own_key;
            names.push_back(o.cats[c].name);
            o.cats.erase(o.cats.begin() + (ptrdiff_t)c);
            auto same_key = [key](const TeamCat& t) { return t.own_key == key; };
            o.results.erase(std::remove_if(o.results.begin(), o.results.end(), same_key), o.results.end());
            o.baseline.erase(std::remove_if(o.baseline.begin(), o.baseline.end(), same_key), o.baseline.end());
        }
        if (o.results.empty()) o.have_results = false;
        // Plus aucun chat a rendre : ce depart est regle. (Les indices au-dessus
        // de i bougent ; on ne les relit plus.)
        if (o.cats.empty()) k.departs.erase(k.departs.begin() + (ptrdiff_t)i);
    }
    if (!names.empty())
        log_line("TEAM", "%s : deja dans un depart plus ancien, pas encore rentre(s) -- seul ce depart-ci "
                         "les ramenera", join_names(names).c_str());
}

// L'invite : l'etat final de ses chats, tel que SA copie de l'aventure le voit
// (elle finit en meme temps que celle de l'hote). L'etat envoye par l'hote,
// s'il est deja la, l'emporte.
void copy_results(void* dir) {
    if (!savefile_in_coop()) return;
    GuestDepart* d = nullptr;
    for (size_t i = k.departs.size(); i-- > 0 && !d;)
        if (k.departs[i].departed) d = &k.departs[i];
    if (!d || d->have_results) return;
    const int64_t day   = save_day(dir);
    const int64_t shift = (day != kNoDay && d->day != kNoDay) ? day - d->day : 0;
    std::vector<TeamCat>     res;
    std::vector<std::string> names;
    for (const DepartCat& dc : d->cats) {
        uint64_t key = 0;
        for (const TeamOwner& o : k.owners)
            if (o.cat_id == dc.id) key = o.host_key;
        void* cat = key ? catsync_cat_by_key(key) : nullptr;
        TeamCat t;
        CatInfo info;
        if (!cat || cat_id_of(cat) != dc.id || !make_result(cat, key, dc.own_key, dc.orig, shift, t, info))
            continue;
        names.push_back(dc.name);
        res.push_back(std::move(t));
    }
    if (res.empty()) return;
    d->results      = std::move(res);
    d->have_results = true;
    d->from_host    = false;
    log_line("TEAM", "etat final de tes chats vu dans ta copie de l'aventure : %s -- recopie dans ta save "
                     "a son prochain chargement", join_names(names).c_str());
}

// --- detours ---------------------------------------------------------------------------------

bool __fastcall h_ButchDepart(void* box) {
    // Le panneau de la maison appelle cette fonction pour ses deux usages :
    // « À l'aventure ! » (des chats dans la boite) et « Fin de journée » (boite
    // vide : elle echoue, et le jeu finit alors la journee, 0x1401F8EDF). Seul
    // le depart nous regarde -- la fin de journee passe telle quelle, comme le
    // depart d'un joueur seul. Plus d'aventure mixte en cours dans ces cas-la
    // (une d'avant, jamais terminee ici, ne doit rien decider pour la suite).
    const int sign = mm_sign_click_kind();
    if (!g.on || !g.host || !in_session() || sign != 2) {
        g.pending.clear();
        g.prepared = false;
        forget_run(sign == 1 ? "fin de journee" : "depart sans autre joueur");
        return o_ButchDepart(box);
    }
    // En session, l'hote ne part qu'apres le vote de depart (mm_menu) : c'est
    // son clic, rejoue, qui arrive ici. L'equipe a ete preparee au vote (depart
    // ensemble : les invites ont charge sa partie et le suivent), ou se prepare
    // maintenant. Une nouvelle aventure : ses recrues repartent de zero, et
    // elle porte ce vote (la note d'une recrue, les proprietaires et les
    // recrues qui rentrent le donnent aux invites), meme sans chat d'invite.
    reset_recruits();
    k.run_seq = g.vote_seq;
    const bool prepared = g.prepared && g.prepared_seq == g.vote_seq;
    g.prepared = false;
    const bool brought = prepared ? g.prepared_brought : has_pending_cats();
    bool mixed = prepared ? !k.run.empty() : (brought && prepare_mixed(box));
    std::vector<uint64_t> keys;
    const bool read = box_keys(box, keys);
    if (mixed && !read) {
        refuse_mixed("boite de depart de l'hote illisible");
        mixed = false;
    }
    if (mixed) {
        const std::vector<uint64_t> all = depart_mixed(box, keys);
        if (prepared) {
            send_embark(g.vote_seq, all);
            embark_host_departed(g.vote_seq);
        }
        return true;
    }
    // Pas d'equipe mixte : les chats des invites restent chez eux.
    drop_mixed();
    // Et la boite de l'hote est vide (videe pendant le vote) : surtout pas le
    // depart du jeu, qui finirait la journee (0x1401F8EDF) hors du vote de la
    // nuit. Personne ne part, et les invites qui attendaient l'aventure en sont
    // liberes.
    if (read && keys.empty()) {
        // Une nuit ou personne n'a mis de chat : la journee passe, c'est tout.
        // L'alerte n'est que pour des chats d'invite qui n'ont pas pu venir.
        if (brought)
            log_line("TEAM", "!! boite de l'hote vide et les chats des invites n'ont pas pu "
                             "venir : personne ne part");
        else
            log_line("TEAM", "boite de l'hote vide et aucun chat d'invite : pas de depart");
        mm_depart_called_off();
        say(brought ? u8"Départ en aventure\n\n"
                      u8"Les chats des autres joueurs n'ont pas pu partir\n"
                      u8"et ta boîte de départ est vide : pas d'aventure\n"
                      u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)"
                    : u8"Départ en aventure\n\n"
                      u8"Ta boîte de départ est vide et personne n'amène\n"
                      u8"de chat : pas d'aventure cette fois.");
        return true;
    }
    // L'hote part avec les siens ; chacun le sait (une liste vide : le partage
    // reste celui de mgmp, par place dans l'equipe).
    send_owners(g.vote_seq, "depart sans les chats des invites");
    if (brought)
        say(u8"Équipe mixte\n\n"
            u8"Les chats des autres joueurs n'ont pas pu\n"
            u8"rejoindre l'équipe : tu pars avec les tiens.\n"
            u8"(Le journal du mod, mgmp.log, dit pourquoi.)");
    // Depart ensemble : les invites attendent dans la maison de l'hote. Le
    // depart du jeu directement, avec les chats de la boite dans l'ordre de ses
    // emplacements (celui de ButchBox::depart, 0x1400ADC21..), puis l'equipe
    // pour eux.
    if (prepared && read) {
        log_line("TEAM", "depart avec les %zu chat(s) de l'hote (depart ensemble)", keys.size());
        depart_keys(box, keys);
        send_embark(g.vote_seq, keys);
        embark_host_departed(g.vote_seq);
        return true;
    }
    return o_ButchDepart(box);
}

void __fastcall h_ReturnCat(void* house, uint64_t key) {
    if (is_guest_cat(key)) {
        // Le jeu le ramene a la maison : il rentre, mais chez son joueur.
        k.came_home.push_back(key);
        log_line("TEAM", "chat %llu : chat d'invite, il rentre chez son joueur et pas chez l'hote",
                 (unsigned long long)key);
        return;
    }
    // Une recrue d'un invite encore la : elle rentre chez lui (send_new_cats,
    // qui la rend a la maison de l'hote si elle ne peut pas partir).
    if (const Recruit* r = guest_recruit_at(key)) {
        k.came_home.push_back(key);
        if (std::find(k.withheld.begin(), k.withheld.end(), key) == k.withheld.end()) k.withheld.push_back(key);
        k.return_house = house;
        log_line("TEAM", "recrue %s (cle %llu) : elle rentre chez %s et pas chez l'hote", r->name.c_str(),
                 (unsigned long long)key, r->owner_name.c_str());
        return;
    }
    o_ReturnCat(house, key);
}

void __fastcall h_RunCleared(void* dir) {
    // Avant l'original : les listes de la course sont encore la, et les chats
    // d'invites, nettoyes par la fin de course, sont dans le registre du jeu.
    if (!k.run.empty() || !k.recruits.empty()) {
        send_results();
        send_new_cats(dir);
        forget_run("fin d'aventure");
    } else {
        copy_results(dir);                   // chez un invite, dans sa copie
        if (!g.host) k.owners.clear();       // l'hote enverra la sienne, vide
    }
    // Toujours : une aventure sans chat d'invite ni recrue a aussi sa photo de
    // l'equipe, qui ne doit pas servir a la suivante.
    reset_recruits();
    k.came_home.clear();
    o_RunCleared(dir);
}

// --- invite : retour -----------------------------------------------------------------------------

std::string home_text(const TeamApplySummary& sum) {
    std::string s = u8"Retour de l'aventure\n";
    if (!sum.back.empty())
        s += fmt(u8"\n%s %s de retour à la maison.", join_names(sum.back).c_str(),
                 sum.back.size() > 1 ? u8"sont" : u8"est");
    if (!sum.lost.empty())
        s += fmt(u8"\n%s %s péri pendant l'aventure.", join_names(sum.lost).c_str(),
                 sum.lost.size() > 1 ? u8"ont" : u8"a");
    if (!sum.gone.empty())
        s += fmt(u8"\n%s ne %s pas de l'aventure.", join_names(sum.gone).c_str(),
                 sum.gone.size() > 1 ? u8"reviennent" : u8"revient");
    if (!sum.changed.empty())
        s += fmt(u8"\n%s %s changé depuis le départ :\n%s retour d'aventure n'a pas été recopié.",
                 join_names(sum.changed).c_str(), sum.changed.size() > 1 ? u8"ont" : u8"a",
                 sum.changed.size() > 1 ? u8"leur" : u8"son");
    if (sum.storage == kStorageChanged)
        s += u8"\nTon rangement a changé depuis le départ :\nles objets équipés au départ n'en ont pas été retirés.";
    else if (sum.storage == kStorageCatsKept)
        s += u8"\nTon rangement n'a pas été modifié : les objets\néquipés au départ y sont encore.";
    return s;
}

bool own_save_path(const char* name, wchar_t* path, size_t n) {
    wchar_t dir[MAX_PATH];
    if (!savefile_save_dir(dir, MAX_PATH)) return false;
    wchar_t wname[128] = {};
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 128);
    _snwprintf_s(path, n, _TRUNCATE, L"%s\\%s", dir, wname);
    return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

sqlite3* open_own(const char* name, bool write) {
    wchar_t path[MAX_PATH];
    if (!own_save_path(name, path, MAX_PATH)) return nullptr;
    char upath[MAX_PATH * 3] = {};
    WideCharToMultiByte(CP_UTF8, 0, path, -1, upath, (int)sizeof(upath), nullptr, nullptr);
    sqlite3* db = nullptr;
    if (sqlite3_open_v2(upath, &db, write ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY, nullptr) !=
        SQLITE_OK) {
        log_line("TEAM", "!! %s ne s'ouvre pas (%s)", name, db ? sqlite3_errmsg(db) : "?");
        sqlite3_close(db);
        return nullptr;
    }
    sqlite3_busy_timeout(db, 500);
    return db;
}

// La copie de securite de sa save, avant la premiere recopie de ce chargement
// (deux dans la meme seconde porteraient le meme nom : la seconde, deja
// modifiee, remplacerait celle d'avant toute recopie).
bool back_up_own(const char* name) {
    if (k.backed_up) return true;
    wchar_t path[MAX_PATH];
    wchar_t dir[MAX_PATH];
    if (!own_save_path(name, path, MAX_PATH) || !savefile_save_dir(dir, MAX_PATH)) return false;
    wchar_t bdir[MAX_PATH];
    _snwprintf_s(bdir, _TRUNCATE, L"%s\\mgmp_backups", dir);
    CreateDirectoryW(bdir, nullptr);
    wchar_t wname[128] = {};
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 128);
    SYSTEMTIME t{};
    GetLocalTime(&t);
    wchar_t dst[MAX_PATH];
    _snwprintf_s(dst, _TRUNCATE, L"%s\\%s.avant-retour-chats.%04u%02u%02u-%02u%02u%02u.bak",
                 bdir, wname, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    if (!CopyFileW(path, dst, FALSE)) {
        log_line("TEAM", "!! copie de securite de %s impossible (win32 %lu)", name, GetLastError());
        return false;
    }
    log_line("TEAM", "copie de securite : mgmp_backups\\%ls", wcsrchr(dst, L'\\') + 1);
    k.backed_up = true;
    return true;
}

// Ses lignes telles que la save les a au premier chargement sans l'etat
// final : ce que « inchange depuis le depart » voudra dire.
void take_baseline(const char* name, GuestDepart& d) {
    sqlite3* db = open_own(name, false);
    if (!db) return;
    d.baseline.clear();
    for (const DepartCat& dc : d.cats) {
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT data FROM cats WHERE key = ?1", -1, &st, nullptr) != SQLITE_OK) break;
        sqlite3_bind_int64(st, 1, (sqlite3_int64)dc.own_key);
        if (sqlite3_step(st) == SQLITE_ROW) {
            const uint8_t* p = (const uint8_t*)sqlite3_column_blob(st, 0);
            const int      n = sqlite3_column_bytes(st, 0);
            TeamCat t;
            t.own_key = dc.own_key;
            if (p && n > 0 && cat_blob_unpack(p, (size_t)n, t.data)) d.baseline.push_back(std::move(t));
        }
        sqlite3_finalize(st);
    }
    sqlite3_close(db);
}

// 1 recopie (ou rien a recopier), -1 erreur : on reessaiera au prochain chargement.
int apply_results(const char* name, GuestDepart& d) {
    if (!back_up_own(name)) return -1;
    sqlite3* db = open_own(name, true);
    if (!db) return -1;
    std::vector<TeamCat> departed;
    if (d.home_loaded) {
        departed = d.baseline;
        for (const DepartCat& dc : d.cats) {
            bool have = false;
            for (const TeamCat& b : departed)
                if (b.own_key == dc.own_key) have = true;
            if (!have) {                     // pas de ligne relevee : les octets du depart
                TeamCat t;
                t.own_key = dc.own_key;
                t.data    = dc.raw;
                departed.push_back(std::move(t));
            }
        }
    }
    // Le rangement, si l'hote a applique une photo de ses chats equipes.
    TeamStorageChange storage;
    const bool with_storage = d.storage_ok && d.storage_post_set;
    if (with_storage) {
        for (const DepartCat& dc : d.cats) storage.cats.push_back(dc.own_key);
        storage.pre_disk       = d.storage_disk;
        storage.pre_disk_found = d.storage_disk_found;
        storage.pre_mem        = d.storage_mem;
        storage.post           = d.storage_post;
    }
    TeamApplySummary sum;
    const bool ok = team_apply_results(db, d.results, d.home_loaded ? &departed : nullptr, sum,
                                       with_storage ? &storage : nullptr);
    sqlite3_close(db);
    if (!ok) {
        log_line("TEAM", "!! l'etat final des chats n'a pas pu etre recopie dans %s -- rien n'est change", name);
        return -1;
    }
    log_line("TEAM", "retour dans %s (etat %s%s) : rentre(s) %s, mort(s) %s, pas revenu(s) %s, change(s) "
                     "depuis le depart %s%s", name, d.from_host ? "de l'hote" : "de ta copie",
             d.home_loaded ? ", en retard" : "",
             sum.back.empty() ? "aucun" : join_names(sum.back).c_str(),
             sum.lost.empty() ? "aucun" : join_names(sum.lost).c_str(),
             sum.gone.empty() ? "aucun" : join_names(sum.gone).c_str(),
             sum.changed.empty() ? "aucun" : join_names(sum.changed).c_str(),
             sum.skipped ? " (des chats ignores, voir plus haut)" : "");
    if (with_storage)
        log_line("TEAM", "rangement de %s : %s", name,
                 sum.storage == kStorageWritten  ? "celui d'apres le choix des objets (recopie)" :
                 sum.storage == kStorageSame     ? "deja celui d'apres le choix des objets" :
                 sum.storage == kStorageChanged  ? "!! il a change depuis le depart -- laisse tel quel" :
                 sum.storage == kStorageCatsKept ? "!! des chats ne sont pas recopies -- laisse tel quel" :
                                                   "rien a faire");
    if (!sum.back.empty() || !sum.lost.empty() || !sum.gone.empty() || !sum.changed.empty()) {
        ++k.applied;
        k.home_infos.push_back(home_text(sum));
        return 1;
    }
    // Aucun de ses chats dans cette save : si l'on ne savait pas de quelle save
    // ils venaient, ce n'est sans doute pas la bonne -- on garde l'etat final.
    return d.save.empty() ? -1 : 1;
}

bool pending_store(const char* save);   // plus bas : le fichier « <save>.mgmp-attente »

// Ses bourses (mm_purse) pour cette save : ajoutees a l'or et a la nourriture de
// sa maison, en une transaction, apres la copie de securite de ce chargement.
// La ligne s'ajoute a la fenetre du retour de ce chargement (ses chats), ou en
// fait une. Pas ajoutees (erreur) : on reessaiera au prochain chargement.
void apply_payouts(const char* name, size_t home_before) {
    auto mine = [name](const Payout& p) { return p.save.empty() || _stricmp(p.save.c_str(), name) == 0; };
    int64_t coins = 0, food = 0;
    size_t  n = 0;
    for (const Payout& p : k.payouts)
        if (mine(p)) { coins += p.coins; food += p.food; ++n; }
    if (!n) return;
    if (!back_up_own(name)) return;
    sqlite3* db = open_own(name, true);
    if (!db) return;
    TeamPurseApply r;
    const bool ok = team_apply_purse(db, coins, food, &r);
    sqlite3_close(db);
    if (!ok) {
        log_line("TEAM", "!! ta bourse n'a pas pu etre ajoutee a %s -- on reessaiera au prochain chargement", name);
        return;
    }
    k.payouts.erase(std::remove_if(k.payouts.begin(), k.payouts.end(), mine), k.payouts.end());
    pending_store(name);
    log_line("TEAM", "bourse ajoutee a %s : or %lld -> %lld, nourriture %lld -> %lld (%zu aventure(s))", name,
             (long long)r.gold_before, (long long)r.gold_after, (long long)r.food_before, (long long)r.food_after, n);
    const std::string line = u8"Tu rapportes " + purse_amount_text(coins, food) + u8" dans ta maison.";
    if (k.home_infos.size() > home_before) k.home_infos.back() += "\n" + line;
    else k.home_infos.push_back(u8"Retour de l'aventure\n\n" + line);
}

// Ce qui attend d'etre ajoute a une save (ses bourses, son butin), garde dans un
// fichier a cote d'elle : « <save>.mgmp-attente ». Ecrit des qu'une bourse ou un
// objet arrive, reecrit (ou efface) apres chaque ajout a la save ; relu juste
// avant qu'elle se charge. Rien n'est perdu si le joueur ferme le jeu avant.
bool pending_path(const char* save, wchar_t* path, size_t n) {
    wchar_t dir[MAX_PATH];
    if (!save || !save[0] || !savefile_save_dir(dir, MAX_PATH)) return false;
    wchar_t wname[128] = {};
    MultiByteToWideChar(CP_UTF8, 0, save, -1, wname, 128);
    _snwprintf_s(path, n, _TRUNCATE, L"%s\\%s.mgmp-attente", dir, wname);
    return true;
}

bool same_save(const std::string& a, const char* b) { return b && _stricmp(a.c_str(), b) == 0; }

// Ecrit le fichier de cette save d'apres k.loot et k.payouts (efface s'il n'y a
// plus rien). Faux si l'ecriture a rate.
bool pending_store(const char* save) {
    wchar_t path[MAX_PATH];
    if (!pending_path(save, path, MAX_PATH)) return false;
    std::vector<PendingLoot>   loot;
    std::vector<PendingPayout> pay;
    for (const LootItem& l : k.loot)
        if (same_save(l.save, save)) {
            PendingLoot e;
            e.seq  = l.seq;
            e.key  = l.key;
            e.item = l.item;
            e.name = l.name;
            loot.push_back(std::move(e));
        }
    for (const Payout& p : k.payouts)
        if (same_save(p.save, save)) {
            PendingPayout e;
            e.seq   = p.seq;
            e.coins = p.coins;
            e.food  = p.food;
            pay.push_back(e);
        }
    std::vector<PendingNewCat> cats;
    for (const NewCat& c : k.newcats)
        if (same_save(c.save, save)) {
            PendingNewCat e;
            e.seq      = c.seq;
            e.host_day = c.host_day;
            e.born     = c.born;
            e.died     = c.died;
            e.cat      = c.data;
            cats.push_back(std::move(e));
        }
    if (loot.empty() && pay.empty() && cats.empty()) {
        if (!DeleteFileW(path) && GetLastError() != ERROR_FILE_NOT_FOUND)
            log_line("TEAM", "!! %s.mgmp-attente ne s'efface pas (win32 %lu)", save, GetLastError());
        return true;
    }
    const std::vector<uint8_t> body = team_pending_encode(loot, pay, &cats);
    wchar_t tmp[MAX_PATH];
    _snwprintf_s(tmp, _TRUNCATE, L"%s.tmp", path);
    HANDLE h = CreateFileW(tmp, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        log_line("TEAM", "!! %s.mgmp-attente : ecriture impossible (win32 %lu)", save, GetLastError());
        return false;
    }
    DWORD done = 0;
    const bool wrote = WriteFile(h, body.data(), (DWORD)body.size(), &done, nullptr) && done == body.size() &&
                       FlushFileBuffers(h);
    CloseHandle(h);
    if (!wrote || !MoveFileExW(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        log_line("TEAM", "!! %s.mgmp-attente : ecriture impossible (win32 %lu)", save, GetLastError());
        DeleteFileW(tmp);
        return false;
    }
    return true;
}

// Relit le fichier de cette save : ce qui n'est pas deja en memoire s'y ajoute.
void pending_load(const char* save) {
    wchar_t path[MAX_PATH];
    if (!pending_path(save, path, MAX_PATH)) return;
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;               // rien en attente
    LARGE_INTEGER size{};
    std::vector<uint8_t> body;
    if (GetFileSizeEx(h, &size) && size.QuadPart > 0 && size.QuadPart < (16 << 20)) {
        body.resize((size_t)size.QuadPart);
        DWORD got = 0;
        if (!ReadFile(h, body.data(), (DWORD)body.size(), &got, nullptr) || got != body.size()) body.clear();
    }
    CloseHandle(h);
    std::vector<PendingLoot>   loot;
    std::vector<PendingPayout> pay;
    std::vector<PendingNewCat> cats;
    if (body.empty() || !team_pending_decode(body.data(), body.size(), loot, pay, &cats)) {
        log_line("TEAM", "!! %s.mgmp-attente illisible -- laisse tel quel", save);
        return;
    }
    size_t added = 0;
    for (const PendingLoot& e : loot) {
        bool have = false;
        for (const LootItem& l : k.loot)
            if (l.key == e.key && l.seq == e.seq && same_save(l.save, save)) have = true;
        if (have || k.loot.size() >= kMaxLoot) continue;
        LootItem l;
        l.seq  = e.seq;
        l.key  = e.key;
        l.save = save;
        l.item = e.item;
        l.name = e.name;
        k.loot.push_back(std::move(l));
        ++added;
    }
    for (const PendingPayout& e : pay) {
        bool have = false;
        for (const Payout& p : k.payouts)
            if (p.seq == e.seq && same_save(p.save, save)) have = true;
        if (have || k.payouts.size() >= kMaxPayouts) continue;
        Payout p;
        p.seq   = e.seq;
        p.save  = save;
        p.coins = e.coins;
        p.food  = e.food;
        k.payouts.push_back(p);
        ++added;
    }
    for (PendingNewCat& e : cats) {
        CatInfo info;
        if (!cat_info(e.cat, info)) continue;
        bool have = false;
        for (const NewCat& c : k.newcats) {
            CatInfo h2;
            have = have || (c.seq == e.seq && same_save(c.save, save) && cat_info(c.data, h2) && h2.id == info.id);
        }
        if (have || k.newcats.size() >= kMaxNewCats) continue;
        NewCat c;
        c.seq      = e.seq;
        c.save     = save;
        c.host_day = e.host_day;
        c.born     = e.born;
        c.died     = e.died;
        c.data     = std::move(e.cat);
        c.name     = no_markup(info.name);
        k.newcats.push_back(std::move(c));
        ++added;
    }
    if (added)
        log_line("TEAM", "%s.mgmp-attente : %zu objet(s) de butin, %zu bourse(s) et %zu recrue(s) en attente pour "
                         "cette save (%zu repris)", save, loot.size(), pay.size(), cats.size(), added);
}

// Son butin (mm_loot) pour cette save : ajoute au rangement, en une transaction,
// apres la copie de securite de ce chargement -- et apres le retour de ses
// chats : tant que l'etat final du depart de la meme aventure attend encore
// (pas arrive, ou une erreur juste avant), le rangement peut devenir celui de la
// photo de l'hote (mm_gear) ; ajoute avant, il ferait croire a un rangement
// change depuis le depart. Ce butin-la attend le chargement suivant.
void apply_loot(const char* name, size_t home_before) {
    auto waits = [name](const LootItem& l) {
        for (const GuestDepart& d : k.departs)
            if (d.seq == l.seq && (d.save.empty() || _stricmp(d.save.c_str(), name) == 0)) return true;
        return false;
    };
    auto mine = [name, &waits](const LootItem& l) {
        return (l.save.empty() || _stricmp(l.save.c_str(), name) == 0) && !waits(l);
    };
    std::vector<ItemFields>  items;
    std::vector<std::string> names;
    size_t held = 0;
    for (const LootItem& l : k.loot)
        if (mine(l)) {
            items.push_back(l.item);
            names.push_back(l.name);
        } else if ((l.save.empty() || _stricmp(l.save.c_str(), name) == 0)) {
            ++held;
        }
    if (held)
        log_line("TEAM", "butin : %zu objet(s) attendent que le retour de tes chats de la meme aventure soit recopie "
                         "dans %s", held, name);
    if (items.empty()) return;
    if (!back_up_own(name)) return;
    sqlite3* db = open_own(name, true);
    if (!db) return;
    uint32_t before = 0, after = 0;
    const int r = team_apply_loot(db, items, &before, &after);
    sqlite3_close(db);
    if (r != 1) {
        log_line("TEAM", "!! ton butin (%zu objet(s)) n'a pas pu etre ajoute au rangement de %s (%s) -- on reessaiera "
                         "au prochain chargement", items.size(), name, r == 0 ? "rangement illisible" : "erreur sqlite");
        return;
    }
    k.loot.erase(std::remove_if(k.loot.begin(), k.loot.end(), mine), k.loot.end());
    pending_store(name);
    log_line("TEAM", "butin ajoute au rangement de %s : %zu objet(s), %u -> %u", name, items.size(), before, after);
    const std::string line = u8"Ton butin est dans ton rangement : " + join_names(names) + ".";
    if (k.home_infos.size() > home_before) k.home_infos.back() += "\n" + line;
    else k.home_infos.push_back(u8"Retour de l'aventure\n\n" + line);
}

// Ses recrues (kTeamNewCats) pour cette save : ajoutees en une transaction,
// apres la copie de securite de ce chargement (et apres le retour de ses chats
// de la meme aventure, comme le butin). Leurs jours sont recales sur le
// calendrier de cette save (cat_shift_days : le jour de l'hote a la fin, celui
// de la save maintenant) ; une fois la save chargee, le jeu les inscrit a son
// pedigree (pedigree_pump).
void apply_new_cats(const char* name, size_t home_before) {
    auto waits = [name](const NewCat& c) {
        for (const GuestDepart& d : k.departs)
            if (d.seq == c.seq && (d.save.empty() || _stricmp(d.save.c_str(), name) == 0)) return true;
        return false;
    };
    auto mine = [name, &waits](const NewCat& c) {
        return (c.save.empty() || _stricmp(c.save.c_str(), name) == 0) && !waits(c);
    };
    bool any = false;
    for (const NewCat& c : k.newcats) any = any || mine(c);
    if (!any) return;
    if (!back_up_own(name)) return;
    sqlite3* db = open_own(name, true);
    if (!db) return;
    // Le jour de cette save (propriete current_day, celle que le jeu lit).
    int64_t day = kNoDay;
    {
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT data FROM properties WHERE key = 'current_day'", -1, &st, nullptr) ==
            SQLITE_OK) {
            if (sqlite3_step(st) == SQLITE_ROW && sqlite3_column_type(st, 0) == SQLITE_INTEGER)
                day = (int64_t)sqlite3_column_int64(st, 0);
            sqlite3_finalize(st);
        }
    }
    std::vector<TeamCat>     cats;
    std::vector<std::string> unshifted;
    std::vector<PedigreeDue> due;            // dans l'ordre de `cats`
    for (const NewCat& c : k.newcats) {
        if (!mine(c)) continue;
        TeamCat t;
        t.data = c.data;
        PedigreeDue pd;
        pd.save = name;
        const bool can_shift = day != kNoDay && c.host_day != kNoDay;
        if (!can_shift || !cat_shift_days(t.data, c.born, c.died, c.host_day - day)) {
            unshifted.push_back(c.name);
            // Le recalage se fera en memoire (pedigree_pump), si le jour est connu.
            if (can_shift) {
                pd.fix_days = true;
                pd.born     = c.born - (c.host_day - day);
                pd.died     = c.died == -1 ? -1 : c.died - (c.host_day - day);
            }
        }
        cats.push_back(std::move(t));
        due.push_back(pd);
    }
    TeamNewCats res;
    const bool ok = team_add_new_cats(db, cats, res);
    sqlite3_close(db);
    if (!ok) {
        log_line("TEAM", "!! tes recrues n'ont pas pu etre ajoutees a %s -- on reessaiera au prochain chargement", name);
        return;
    }
    for (size_t i = 0; i < res.keys.size() && i < due.size(); ++i)
        if (res.keys[i]) {
            due[i].key = res.keys[i];
            k.pedigree_due.push_back(due[i]);
        }
    k.newcats.erase(std::remove_if(k.newcats.begin(), k.newcats.end(), mine), k.newcats.end());
    pending_store(name);
    k.recruits_added += (uint32_t)res.added.size();
    log_line("TEAM", "recrue(s) ajoutee(s) a %s : %s%s%s%s", name,
             res.added.empty() ? "aucune" : join_names(res.added).c_str(),
             res.already.empty() ? "" : (" ; deja la : " + join_names(res.already)).c_str(),
             unshifted.empty() ? "" : (" ; jours laisses tels quels : " + join_names(unshifted)).c_str(),
             res.unplaced ? " ; pas de maison ou les placer" : "");
    if (res.added.empty()) return;
    const std::string line = fmt(u8"%s, %s pendant l'aventure, %s dans ta maison.",
                                 join_names(res.added).c_str(),
                                 res.added.size() > 1 ? u8"recrutés" : u8"recruté",
                                 res.added.size() > 1 ? u8"arrivent" : u8"arrive");
    if (k.home_infos.size() > home_before) k.home_infos.back() += "\n" + line;
    else k.home_infos.push_back(u8"Retour de l'aventure\n\n" + line);
}

// Chez soi, la save ou on les a ajoutees chargee (invite, ou ancien invite qui
// heberge maintenant) : les recrues entrent au pedigree du jeu
// (Pedigree::add(registre+0x38, cle, -1, -1), ce que fait Registry::NewCat a la
// naissance d'un chat ; sans parents), et leurs jours sont recales en memoire si
// la ligne n'a pas pu l'etre. Une seule fois, et jamais dans une autre save : la
// meme cle y est peut-etre un autre chat (relecture du 17 octobre).
std::string loaded_own_save() {
    char name[64] = {};
    if (savefile_in_coop()) return std::string();
    if (savefile_own_save_name(name, sizeof(name)) || savefile_host_save_name(name, sizeof(name))) return name;
    return std::string();
}

void pedigree_pump() {
    if (k.pedigree_due.empty() || savefile_in_coop() || !mm_house()) return;
    const std::string loaded = loaded_own_save();
    if (loaded.empty()) return;
    bool any = false;
    for (const PedigreeDue& d : k.pedigree_due) any = any || _stricmp(d.save.c_str(), loaded.c_str()) == 0;
    if (!any) return;
    auto add = (FnPedigreeAddRaw)addr_of_call(C_PedigreeAdd);
    const uint8_t* dir = (const uint8_t*)director();
    void* registry = nullptr;
    const bool ready = add && dir && mem_read(dir + kDir_CatRegistry, &registry, sizeof(registry)) && registry;
    if (!ready)
        log_line("TEAM", "!! pedigree : registre ou fonction introuvable -- les recrues de %s n'y sont pas inscrites",
                 loaded.c_str());
    size_t done = 0, fixed = 0, total = 0;
    for (const PedigreeDue& d : k.pedigree_due) {
        if (_stricmp(d.save.c_str(), loaded.c_str()) != 0) continue;
        ++total;
        void* cat = catsync_cat_by_key(d.key);
        if (!cat) continue;
        if (ready && safe_pedigree_add(add, (char*)registry + kRegistry_Pedigree, d.key)) ++done;
        CatDays days;
        if (d.fix_days && read_days(cat, days)) {
            days.born = d.born;
            if (days.died != -1 || d.died != -1) days.died = d.died;
            if (write_days(cat, days)) ++fixed;
        }
    }
    log_line("TEAM", "pedigree : %zu recrue(s) inscrite(s) sur %zu dans %s%s", done, total, loaded.c_str(),
             fixed ? fmt(" (%zu recalee(s) sur ton calendrier)", fixed).c_str() : "");
    k.pedigree_due.erase(std::remove_if(k.pedigree_due.begin(), k.pedigree_due.end(),
                                        [&](const PedigreeDue& d) {
                                            return _stricmp(d.save.c_str(), loaded.c_str()) == 0;
                                        }),
                         k.pedigree_due.end());
}

// --- messages --------------------------------------------------------------------------------------

void on_depart(uint8_t from, const SaveFileMsg& m) {
    TeamMsg msg;
    if (net_role() != NetRole::Host || from == kHostPeer) {
        log_line("TEAM", "!! chats de depart recus de %u alors que ce joueur n'heberge pas -- ignores",
                 (unsigned)from);
        return;
    }
    if (!team_decode(m.data, m.size, msg) || msg.kind != kTeamDepart || msg.cats.size() > (size_t)kTeamSize) {
        log_line("TEAM", "!! chats de depart de %s illisibles (%u octets) -- ignores",
                 mm_peer_name(from).c_str(), m.size);
        return;
    }
    if (msg.seq != g.vote_seq) {
        log_line("TEAM", "chats de %s pour un vote d'avant (%u, celui-ci %u) -- ignores",
                 mm_peer_name(from).c_str(), (unsigned)msg.seq, (unsigned)g.vote_seq);
        return;
    }
    PendingTeam pt;
    pt.peer = from;
    pt.day  = msg.day;
    for (const TeamCat& c : msg.cats) {
        CatInfo info;
        if (!cat_info(c.data, info)) {
            log_line("TEAM", "!! un chat de %s n'a pas le format attendu -- ses chats sont ignores",
                     mm_peer_name(from).c_str());
            return;
        }
        pt.names.push_back(info.name);
    }
    pt.cats = std::move(msg.cats);
    const std::string what = pt.names.empty() ? std::string("vient sans chat")
                                              : "amene " + join_names(pt.names);
    log_line("TEAM", "<- %s %s", mm_peer_name(from).c_str(), what.c_str());
    g.pending.erase(std::remove_if(g.pending.begin(), g.pending.end(),
                                   [&](const PendingTeam& p) { return p.peer == from; }),
                    g.pending.end());
    g.pending.push_back(std::move(pt));
}

void on_owners(uint8_t from, const SaveFileMsg& m) {
    std::vector<TeamOwner> owners;
    uint8_t seq = 0;
    if (net_role() != NetRole::Client || from != kHostPeer ||
        !team_owners_decode(m.data, m.size, seq, owners)) {
        log_line("TEAM", "!! liste des proprietaires inattendue ou illisible -- ignoree");
        return;
    }
    k.owners = std::move(owners);
    const uint8_t self = net_self();
    size_t mine = 0;
    for (const TeamOwner& o : k.owners)
        if (o.peer == self) ++mine;
    log_line("TEAM", "<- proprietaires : %zu chat(s), dont %zu a ce joueur (vote %u)", k.owners.size(),
             mine, (unsigned)seq);
    // La premiere liste du vote d'un de ses departs : ses chats y sont-ils ?
    GuestDepart* d = departure(seq);
    if (!d || d->owners_seen) return;
    d->owners_seen = true;
    std::vector<std::string> missing;
    std::vector<DepartCat>   kept;
    for (const DepartCat& dc : d->cats) {
        bool in = false;
        for (const TeamOwner& o : k.owners)
            if (o.cat_id == dc.id) in = true;
        if (in) kept.push_back(dc);
        else    missing.push_back(dc.name);
    }
    const bool withdrawn = d->withdrawn;
    d->departed = !kept.empty();
    if (kept.empty()) {
        // Aucun de ses chats n'est parti : rien a attendre de ce depart.
        k.departs.erase(k.departs.begin() + (d - k.departs.data()));
        if (!withdrawn)
            say(fmt(u8"Équipe mixte\n\n"
                    u8"%s %s pas pu rejoindre l'équipe :\n%s",
                    join_names(missing).c_str(), missing.size() > 1 ? u8"n'ont" : u8"n'a",
                    k.owners.empty() ? u8"pour cette aventure, tu diriges une partie\ndes chats de l'équipe."
                                     : u8"tu suis l'aventure sans diriger de chat."));
        return;
    }
    if (!missing.empty()) {
        d->cats = std::move(kept);
        if (!withdrawn)
            say(fmt(u8"Équipe mixte\n\n"
                    u8"%s %s pas pu partir et %s à la maison.",
                    join_names(missing).c_str(), missing.size() > 1 ? u8"n'ont" : u8"n'a",
                    missing.size() > 1 ? u8"restent" : u8"reste"));
    }
    supersede_older((size_t)(d - k.departs.data()));
}

// Invite : l'hote part, avec cette equipe (depart ensemble).
void on_embark(uint8_t from, const SaveFileMsg& m) {
    std::vector<uint64_t> keys;
    uint8_t seq = 0;
    if (net_role() != NetRole::Client || from != kHostPeer ||
        !team_embark_decode(m.data, m.size, seq, keys)) {
        log_line("TEAM", "!! equipe du depart inattendue ou illisible -- ignoree");
        return;
    }
    embark_on_go(seq, keys);
}

void on_result(uint8_t from, const SaveFileMsg& m) {
    TeamMsg msg;
    if (net_role() != NetRole::Client || from != kHostPeer || !team_decode(m.data, m.size, msg) ||
        msg.kind != kTeamResult || msg.cats.empty()) {
        log_line("TEAM", "!! etat final des chats inattendu ou illisible -- ignore");
        return;
    }
    GuestDepart* d = departure(msg.seq);
    if (!d) {
        log_line("TEAM", "!! etat final des chats d'un depart deja regle (vote %u) -- ignore",
                 (unsigned)msg.seq);
        return;
    }
    // Seulement les chats que ce depart doit encore ramener (un depart plus
    // recent a pu en reprendre : supersede_older).
    msg.cats.erase(std::remove_if(msg.cats.begin(), msg.cats.end(),
                                  [d](const TeamCat& c) {
                                      for (const DepartCat& dc : d->cats)
                                          if (dc.own_key == c.own_key) return false;
                                      return true;
                                  }),
                   msg.cats.end());
    if (msg.cats.empty()) {
        log_line("TEAM", "etat final des chats du vote %u : ils sont tous repartis depuis -- ignore",
                 (unsigned)msg.seq);
        return;
    }
    std::vector<std::string> back, lost, gone;
    for (const TeamCat& c : msg.cats) {
        CatInfo info;
        if (!cat_info(c.data, info)) continue;
        ((info.flags & kCatDead) ? lost : c.fate == kFateGone ? gone : back).push_back(info.name);
    }
    log_line("TEAM", "<- etat final de tes chats (hote) : rentre(s) %s, mort(s) %s, pas revenu(s) %s -- recopie "
                     "dans %s a son prochain chargement%s", back.empty() ? "aucun" : join_names(back).c_str(),
             lost.empty() ? "aucun" : join_names(lost).c_str(), gone.empty() ? "aucun" : join_names(gone).c_str(),
             d->save.empty() ? "ta save" : d->save.c_str(),
             d->home_loaded ? " (en retard : seuls les chats inchanges depuis seront recopies)" : "");
    d->results      = std::move(msg.cats);
    d->have_results = true;
    d->from_host    = true;
    d->departed     = true;
    // Deja chez lui : sa save est ouverte par le jeu, on ne peut rien y ecrire.
    if (d->home_loaded && mm_house() && !savefile_in_coop())
        say(u8"Retour de l'aventure\n\n"
            u8"Tes chats sont rentrés de l'aventure, mais ta save\n"
            u8"est déjà ouverte : retourne au menu principal\n"
            u8"et recharge-la pour les retrouver.");
}

} // namespace

// Une recrue rejoint l'equipe (hote -> tous) : la fenetre, sur la carte.
void on_recruit(uint8_t from, const SaveFileMsg& m) {
    RecruitNote n;
    if (net_role() != NetRole::Client || from != kHostPeer || !team_recruit_decode(m.data, m.size, n)) {
        log_line("TEAM", "!! recrue : message inattendu ou illisible -- ignore");
        return;
    }
    const std::string cat = n.name.empty() ? std::string(u8"Un nouveau chat") : no_markup(n.name);
    log_line("TEAM", "<- recrue : %s rejoint l'equipe -- elle est a %s%s", cat.c_str(),
             mm_peer_name(n.owner).c_str(), n.owner == net_self() ? " (ce joueur)" : "");
    // Dans l'aventure : la fenetre sur la carte. Hors de l'aventure (chez lui),
    // seulement si la recrue est a lui, et tout de suite -- sa carte suivante
    // serait une autre aventure.
    if (mm_guest_in_run()) {
        k.map_infos.push_back(recruit_text(cat, n.owner));
    } else if (n.owner == net_self()) {
        k.infos.push_back(fmt(u8"Nouveau chat dans l'équipe\n\n"
                              u8"%s rejoint l'équipe de %s : ce chat est\n"
                              u8"à toi ! Tant que tu n'es pas là, %s le dirige,\n"
                              u8"et il rentrera chez toi à la fin de l'aventure.", cat.c_str(),
                              no_markup(mm_peer_name(kHostPeer)).c_str(),
                              no_markup(mm_peer_name(kHostPeer)).c_str()));
    }
}

// Les recrues qui rentrent chez lui (hote -> cet invite, a la fin) : gardees
// sur le disque avec sa save, ajoutees a son prochain chargement.
void on_newcats(uint8_t from, const SaveFileMsg& m) {
    NewCatsMsg msg;
    if (net_role() != NetRole::Client || from != kHostPeer || !team_newcats_decode(m.data, m.size, msg) ||
        msg.cats.empty()) {
        log_line("TEAM", "!! recrues : message inattendu ou illisible -- ignore");
        return;
    }
    std::string save;
    if (const GuestDepart* d = departure(msg.seq)) save = d->save;
    if (save.empty()) {
        char own[64] = {};
        if (savefile_own_save_name(own, sizeof(own))) save = own;
    }
    std::vector<std::string> names;
    for (NewCatEntry& e : msg.cats) {
        CatInfo info;
        if (!cat_info(e.data, info)) continue;
        bool dup = false;
        for (const NewCat& c : k.newcats) {
            CatInfo have;
            dup = dup || (c.seq == msg.seq && cat_info(c.data, have) && have.id == info.id);
        }
        if (dup) continue;
        if (k.newcats.size() >= kMaxNewCats) {
            log_line("TEAM", "!! trop de recrues en attente : %s n'est pas gardee", info.name.c_str());
            break;
        }
        NewCat c;
        c.seq      = msg.seq;
        c.save     = save;
        c.host_day = msg.host_day;
        c.born     = e.born;
        c.died     = e.died;
        c.data     = std::move(e.data);
        c.name     = no_markup(info.name);
        names.push_back(c.name);
        k.newcats.push_back(std::move(c));
    }
    if (names.empty()) return;
    if (!save.empty() && !pending_store(save.c_str()))
        log_line("TEAM", "!! recrues : pas gardees sur le disque -- elles attendent en memoire (ne ferme pas le jeu "
                         "avant de recharger ta save)");
    log_line("TEAM", "<- recrue(s) qui rentrent chez toi : %s -- ajoutee(s) a %s a son prochain chargement",
             join_names(names).c_str(), save.empty() ? "ta save" : save.c_str());
    // Deja chez lui : sa save est ouverte par le jeu, on ne peut rien y ecrire.
    if (mm_house() && !savefile_in_coop())
        say(fmt(u8"Nouveaux chats\n\n"
                u8"%s %s chez toi, mais ta save est déjà\n"
                u8"ouverte : retourne au menu principal et\n"
                u8"recharge-la pour %s.", join_names(names).c_str(),
                names.size() > 1 ? u8"arrivent" : u8"arrive", names.size() > 1 ? u8"les trouver" : u8"le trouver"));
}

// ---------------------------------------------------------------------------------------------------

void team_init() {
    g = State{};
    g.on     = true;
    g.host   = net_role() == NetRole::Host;
    g.roster = roster_mask();
    if (!g.host) {                           // un invite ne connait que l'aventure de cette session
        k.owners.clear();
        k.run.clear();
        k.host_cats.clear();
        k.run_save.clear();
        reset_recruits();
    }
    log_line("TEAM", g.host
        ? "arme -- equipe mixte : les chats des invites rejoindront l'equipe de l'hote au depart"
        : "arme -- equipe mixte : les chats de ta boite de depart partiront avec toi");
}

void team_shutdown() {
    if (!g.on) return;
    log_line("TEAM", "fin : %u depart(s) en equipe mixte, %u refuse(s), %u retour(s) envoye(s), "
                     "%u recopie(s) chez soi", k.departs_made, k.refused, k.returned, k.applied);
    if (!g.host) k.owners.clear();
    g = State{};
}

void team_pump() {
    if (!g.on || !g.host) return;
    recruits_tick();
    const uint32_t roster = roster_mask();
    if (roster == g.roster) return;
    g.roster = roster;
    if (!g.pending.empty())
        g.pending.erase(std::remove_if(g.pending.begin(), g.pending.end(),
                                       [](const PendingTeam& p) { return !peer_present(p.peer); }),
                        g.pending.end());
    if ((!k.run.empty() || !k.recruits.empty()) && rebuild_owners())
        send_owners(k.run_seq, "un joueur est arrive ou parti");
}

void team_ui_pump() {
    pedigree_pump();
    if (mm_info_busy()) return;
    if (!k.infos.empty()) {
        mm_show_info(k.infos.front());
        k.infos.erase(k.infos.begin());
        return;
    }
    // Une recrue : sur la carte, pas par-dessus l'evenement qui l'amene.
    if (!k.map_infos.empty()) {
        if (mm_scene_running("Map") && !mm_scene_alive("Event")) {
            mm_show_info(k.map_infos.front(), kInfoYieldToFollow);
            k.map_infos.erase(k.map_infos.begin());
            return;
        }
        if (!mm_in_game()) k.map_infos.clear();   // retour au menu : trop tard
    }
    // Le retour d'aventure, une fois chez lui -- session ou pas (elle a pu
    // tomber pendant qu'il rentrait).
    if (!k.home_infos.empty() && net_role() != NetRole::Host && mm_house() && !savefile_in_coop()) {
        mm_show_info(k.home_infos.front());
        k.home_infos.erase(k.home_infos.begin());
    }
}

void team_catchup(uint8_t peer) {
    if (!g.on || !g.host || (k.run.empty() && k.owners.empty())) return;
    // Le nom de l'arrivant est deja connu (HELLO) : il retrouve ses chats.
    if ((!k.run.empty() || !k.recruits.empty()) && rebuild_owners()) {
        g.roster = roster_mask();
        send_owners(k.run_seq, "un joueur revient");
        return;
    }
    if (send_team(team_owners_encode(k.run_seq, k.owners), peer))
        log_line("TEAM", "-> proprietaires de l'aventure en cours envoyes au joueur %u", (unsigned)peer);
}

void team_on_message(uint8_t from, const SaveFileMsg& m) {
    if (!g.on) return;
    if (!m.data || savefile_hash(m.data, m.size) != m.hash) {
        log_line("TEAM", "!! message d'equipe abime (%u octets) -- ignore", m.size);
        return;
    }
    switch (team_kind(m.data, m.size)) {
        case kTeamDepart: on_depart(from, m); break;
        case kTeamOwners: on_owners(from, m); break;
        case kTeamResult: on_result(from, m); break;
        case kTeamEmbark: on_embark(from, m); break;
        case kTeamGear:
        case kTeamGearAck: gear_on_team(from, m.data, m.size); break;
        case kTeamRunGear:
        case kTeamRunGearAck:
        case kTeamItemOp:
        case kTeamItemAck:
        case kTeamItemSync:
        case kTeamItemSyncReq: rungear_on_team(from, m.data, m.size); break;
        case kTeamLoot: loot_on_team(from, m.data, m.size); break;
        case kTeamRecruit: on_recruit(from, m); break;
        case kTeamNewCats: on_newcats(from, m); break;
        default:
            log_line("TEAM", "!! message d'equipe inconnu (%u octets) -- ignore", m.size);
            break;
    }
}

bool team_send(const std::vector<uint8_t>& body, int peer) {
    if (!g.on || body.empty()) return false;
    return send_team(body, peer);
}

int team_quota(bool host, int players) {
    if (players <= 1) return kTeamSize;
    if (players == 2) return 2;
    if (players == 3) return host ? 2 : 1;   // la place libre est a l'hote
    return 1;
}

int team_host_box_count() {
    BoxState state = kBoxOk;
    std::vector<uint64_t> keys;
    void* box = house_box(state);
    if (!box || !box_keys(box, keys)) return -1;
    return (int)keys.size();
}

std::string team_host_too_many_text(int players, int have) {
    const int q = team_quota(true, players);
    return fmt(u8"Équipe mixte\n\n"
               u8"À %d joueurs, tu emmènes %d chat%s au maximum :\n"
               u8"retire %d chat%s de ta boîte de départ.\n"
               u8"Les autres joueurs compléteront l'équipe avec les leurs.",
               players, q, plural((size_t)q), have - q, plural((size_t)(have - q)));
}

void team_vote_begin(uint8_t seq) {
    g.vote_seq = seq;
    g.pending.clear();
    g.prepared = false;
}

void team_vote_reset(const char* why) {
    if (!g.pending.empty())
        log_line("TEAM", "chats des invites oublies (%s)", why ? why : "?");
    g.pending.clear();
    g.prepared = false;
}

bool team_embark_prepare(uint8_t seq) {
    if (!g.on || !g.host || !in_session()) return false;
    BoxState state = kBoxOk;
    void* box = house_box(state);
    if (!box) return false;
    g.prepared         = true;
    g.prepared_seq     = seq;
    g.prepared_brought = has_pending_cats();
    reset_recruits();                        // une nouvelle aventure (voir h_ButchDepart)
    k.run_seq = seq;
    if (!g.prepared_brought)
        drop_mixed();                        // une aventure d'avant ne decide rien pour celle-ci
    else if (!prepare_mixed(box))
        log_line("TEAM", "depart ensemble : les chats des invites ne viennent pas, l'hote partira avec "
                         "les siens");
    return true;
}

bool team_copy_depart(const std::vector<uint64_t>& keys) {
    if (keys.empty()) return false;
    BoxState state = kBoxOk;
    void* box = house_box(state);
    auto go = (FnDepartGo)addr_of_call(C_DepartGo);
    if (!box || !go) {
        log_line("TEAM", "!! depart dans la copie impossible : %s", !box ? "boite de depart introuvable"
                                                                         : "fonction du jeu introuvable");
        return false;
    }
    // Un chat que la copie n'a pas (l'hote ne l'avait pas encore sauve) : elle
    // part sans lui ; ses ecrans different alors un peu de ceux de l'hote.
    std::vector<uint64_t> all;
    for (uint64_t key : keys) {
        if (catsync_cat_by_key(key)) all.push_back(key);
        else log_line("TEAM", "!! la copie de la partie de l'hote n'a pas le chat %llu : elle part sans lui",
                      (unsigned long long)key);
    }
    if (all.empty()) return false;
    DepartCapture capture{ box, (uint32_t)all.size(), (uint32_t)all.size(), all.data() };
    if (!safe_depart_go(go, &capture)) {
        log_line("TEAM", "!! le depart du jeu a echoue dans la copie");
        return false;
    }
    return true;
}

void team_forget_peer(uint8_t peer) {
    const size_t before = g.pending.size();
    g.pending.erase(std::remove_if(g.pending.begin(), g.pending.end(),
                                   [&](const PendingTeam& p) { return p.peer == peer; }),
                    g.pending.end());
    if (g.pending.size() != before)
        log_line("TEAM", "%s ne vient plus : ses chats ne partent pas", mm_peer_name(peer).c_str());
}

std::string team_guest_offer_line() {
    const int q = team_quota(false, net_peer_count());
    std::vector<BoxCat> box;
    bool all_read = true;
    const int n = read_box(box, all_read);
    if (n == kNoHouse) return u8"Tu n'es pas dans ta maison :\ntu joueras avec les chats des autres.";
    if (n == kNoBox)
        return u8"Ta boîte de départ n'est pas encore débloquée :\ntu joueras avec les chats des autres.";
    if (n == 0)
        return fmt(u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
                   u8"Pour emmener les tiens : « Pas encore », mets-en %s dans ta boîte\n"
                   u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
                   q > 1 ? u8"1 ou 2" : u8"un");
    if (n > q)
        return fmt(u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
                   u8"réponds « Pas encore », retires-en %d,\n"
                   u8"puis clique sur ton panneau « À l'aventure ! ».", n, q, n - q);
    std::vector<std::string> names;
    for (const BoxCat& b : box) names.push_back(b.info.name);
    if (names.empty()) return u8"Tes chats partent de ta boîte de départ (chez Butch).";
    return u8"Tu emmènes " + join_names(names) + ".";
}

bool team_guest_send(uint8_t seq, std::string& problem) {
    problem.clear();
    const char* head = u8"Départ en aventure\n\n";
    if (!g.on || net_role() != NetRole::Client) {
        problem = std::string(head) + u8"La session n'est pas prête : réessaie dans un instant.";
        return false;
    }
    const int q = team_quota(false, net_peer_count());
    std::vector<BoxCat> box;
    bool all_read = true;
    const int n = read_box(box, all_read);
    if (n > q) { problem = std::string(head) + too_many_text(n, q); return false; }
    if (n > 0 && (!all_read || box.empty())) {
        problem = std::string(head) + u8"Tes chats n'ont pas pu être préparés\npour l'aventure (voir mgmp.log).";
        return false;
    }
    // Un chat dont l'etat final d'une aventure d'avant attend encore d'etre
    // recopie dans sa save (arrive pendant qu'elle etait ouverte) : il
    // repartirait de son etat d'avant cette aventure, et les deux retours se
    // contrediraient (mort ou parti dans l'un, rentre dans l'autre). Recharger
    // sa save recopie d'abord le premier.
    char cur_save[64] = {};
    const bool know_save = savefile_own_save_name(cur_save, sizeof(cur_save));
    std::vector<std::string> waiting;
    for (const BoxCat& b : box) {
        bool held = false;
        for (const GuestDepart& d : k.departs) {
            if (!d.have_results || held) continue;
            if (know_save && !d.save.empty() && _stricmp(d.save.c_str(), cur_save) != 0) continue;
            bool same = false, result = false;
            for (const DepartCat& dc : d.cats)
                if (dc.own_key == b.cat.own_key && dc.id == b.info.id) same = true;
            for (const TeamCat& r : d.results)
                if (r.own_key == b.cat.own_key) result = true;
            held = same && result;
        }
        if (held) waiting.push_back(b.info.name);
    }
    if (!waiting.empty()) {
        const bool many = waiting.size() > 1;
        log_line("TEAM", "!! %s : l'etat final d'une aventure d'avant n'est pas encore recopie -- pas de "
                         "depart avant que la save soit rechargee", join_names(waiting).c_str());
        problem = std::string(head) +
                  fmt(u8"%s %s d'une aventure, mais %s retour\n"
                      u8"n'est pas encore recopié dans ta save : retourne\n"
                      u8"au menu principal et recharge-la avant de %s\n"
                      u8"faire repartir.",
                      join_names(waiting).c_str(), many ? u8"reviennent" : u8"revient",
                      many ? u8"leur" : u8"son", many ? u8"les" : u8"le");
        return false;
    }
    // Pas de maison, pas de boite, boite vide : il vient sans chat (un message
    // vide quand meme, qui remplace ce qu'il aurait envoye plus tot).
    TeamMsg msg;
    msg.kind = kTeamDepart;
    msg.seq  = seq;
    msg.day  = n > 0 ? save_day(director()) : kNoDay;
    std::vector<std::string> names;
    for (const BoxCat& b : box) {
        msg.cats.push_back(b.cat);
        names.push_back(b.info.name);
    }
    if (!send_team(team_encode(msg), -1)) {
        problem = std::string(head) + u8"L'envoi de ta réponse à l'hôte a échoué :\nréessaie.";
        return false;
    }
    // Ce vote remplace ce qu'il avait repondu plus tot au meme vote, et un vote
    // d'avant qui n'est pas parti (annule, « pas encore ») ne partira plus : sa
    // liste serait arrivee avant ce vote-ci. Les departs partis, eux, gardent
    // leur etat final a rendre.
    k.departs.erase(std::remove_if(k.departs.begin(), k.departs.end(),
                                   [](const GuestDepart& d) { return !d.departed; }),
                    k.departs.end());
    if (msg.cats.empty()) {
        log_line("TEAM", "-> ce joueur part sans chat (vote %u)", (unsigned)seq);
        return true;
    }
    if (k.departs.size() >= kMaxDeparts) {
        log_line("TEAM", "!! trop de departs sans retour : le plus ancien est oublie (ses chats restent tels "
                         "qu'a son depart)");
        k.departs.erase(k.departs.begin());
    }
    char own[64] = {};
    GuestDepart d;
    d.seq  = seq;
    d.save = savefile_own_save_name(own, sizeof(own)) ? own : "";
    d.day  = msg.day;
    // Son rangement (mm_gear) : en memoire, ce qu'il equipera au depart ; dans
    // le fichier, pour savoir au retour si sa save a change depuis.
    if (!d.save.empty() && invsync_serialize_storage(d.storage_mem)) {
        if (sqlite3* db = open_own(d.save.c_str(), false)) {
            const int f = team_read_storage(db, d.storage_disk);
            sqlite3_close(db);
            d.storage_ok         = f >= 0;
            d.storage_disk_found = f == 1;
        }
    }
    if (!d.storage_ok) {
        d.storage_mem.clear();
        d.storage_disk.clear();
    }
    for (const BoxCat& b : box) {
        DepartCat dc;
        dc.own_key = b.cat.own_key;
        dc.id      = b.info.id;
        dc.name    = b.info.name;
        dc.raw     = b.cat.data;
        dc.orig    = b.days;
        d.cats.push_back(std::move(dc));
    }
    log_line("TEAM", "-> chats envoyes a l'hote pour le depart : %s (save %s, jour %lld, vote %u) ; "
                     "rangement %s", join_names(names).c_str(), d.save.empty() ? "inconnue" : d.save.c_str(),
             (long long)(d.day == kNoDay ? -1 : d.day), (unsigned)seq,
             d.storage_ok ? fmt("lu (%zu octets)", d.storage_mem.size()).c_str()
                          : "illisible : ses chats ne pourront pas etre equipes au depart");
    k.departs.push_back(std::move(d));
    return true;
}

void team_guest_withdraw(uint8_t seq) {
    GuestDepart* d = departure(seq);
    if (d && !d->departed) d->withdrawn = true;
}

bool team_guest_storage(uint8_t seq, std::vector<uint8_t>& out) {
    out.clear();
    const GuestDepart* d = departure(seq);
    if (!d || !d->storage_ok || d->storage_mem.size() < 4) return false;
    out = d->storage_mem;
    return true;
}

void team_guest_storage_after(uint8_t seq, const std::vector<uint8_t>& post) {
    GuestDepart* d = departure(seq);
    if (!d || !d->storage_ok || post.size() < 4) {
        log_line("TEAM", "!! rangement d'apres le choix des objets (vote %u) sans depart a lui -- ignore",
                 (unsigned)seq);
        return;
    }
    d->storage_post     = post;
    d->storage_post_set = true;
    log_line("TEAM", "rangement d'apres le choix des objets garde pour le retour (vote %u, %zu octets)",
             (unsigned)seq, post.size());
}

bool team_guest_has_cats_in_run() {
    const uint8_t self = net_self();
    for (const TeamOwner& o : k.owners)
        if (o.peer == self) return true;
    return false;
}

bool team_guest_brought_cats() {
    const uint8_t self = net_self();
    for (const TeamOwner& o : k.owners) {
        if (o.peer != self) continue;
        for (const GuestDepart& d : k.departs) {
            if (!d.departed) continue;
            for (const DepartCat& dc : d.cats)
                if (dc.id == o.cat_id) return true;
        }
    }
    return false;
}

std::string team_run_guest_names() {
    std::vector<std::string> names;
    for (const RunCat& rc : k.run) {
        std::string n = rc.owner_name;
        for (char& c : n)
            if (c == '[') c = '(';
            else if (c == ']') c = ')';
        if (n.empty()) continue;
        bool have = false;
        for (const std::string& o : names) have = have || o == n;
        if (!have) names.push_back(n);
    }
    return join_names(names);
}

bool team_owners_active() { return !k.owners.empty(); }

int team_owner_of(uint64_t cat_id) {
    for (const TeamOwner& o : k.owners)
        if (o.cat_id == cat_id) return o.peer;
    return -1;
}

void team_prepare_own_save(const char* name) {
    if (!name || !name[0]) return;
    // Hote : une autre save que celle de l'aventure mixte en cours -- celle-ci ne
    // decide plus rien (ses chats d'invites restent marques dans leur save).
    if (!k.run.empty() && !k.run_save.empty() && _stricmp(k.run_save.c_str(), name) != 0)
        forget_run("une autre save se charge");
    // Une save qui se charge : les recrues d'une aventure en memoire ne la
    // concernent plus (une aventure sans chat d'invite n'a pas de run_save), et
    // la premiere photo de l'equipe se refera.
    reset_recruits();
    k.backed_up = false;                     // une copie de securite par chargement
    const size_t home_before = k.home_infos.size();
    // Ce qui attendait d'etre ajoute a cette save, peut-etre d'une partie d'avant
    // (le jeu ferme entre-temps) : bourses et butin (mm_purse, mm_loot).
    pending_load(name);
    // Invite (maintenant ou avant) : l'etat final de ses chats, depart par depart.
    for (size_t i = 0; i < k.departs.size();) {
        GuestDepart& d = k.departs[i];
        if (!d.save.empty() && _stricmp(d.save.c_str(), name) != 0) { ++i; continue; }
        if (!d.have_results) {
            if (!d.home_loaded) {
                take_baseline(name, d);
                d.home_loaded = true;
                log_line("TEAM", "%s se charge avant l'etat final de tes chats (vote %u) : s'il arrive plus "
                                 "tard, seuls les chats inchanges depuis seront recopies", name, (unsigned)d.seq);
            }
            ++i;
            continue;
        }
        if (apply_results(name, d) == 1) {
            k.departs.erase(k.departs.begin() + (ptrdiff_t)i);
            continue;
        }
        // Pas recopie cette fois : la save va etre jouee, la prochaine fois ne
        // recopiera que les chats qui n'auront pas change.
        if (!d.home_loaded) {
            take_baseline(name, d);
            d.home_loaded = true;
        }
        ++i;
    }
    // Et ce qu'il a rapporte de ses aventures gagnees (mm_purse, mm_loot), et
    // ses recrues.
    apply_payouts(name, home_before);
    apply_loot(name, home_before);
    apply_new_cats(name, home_before);
}

// --- mm_purse ----------------------------------------------------------------------------------------

bool team_run_slots(uint32_t& gen, uint32_t& slots) {
    gen   = k.run_gen;
    slots = 0;
    if (k.run.empty()) return false;         // pas d'aventure mixte (et jamais chez un invite)
    if (!k.host_cats.empty()) slots |= 1u << kHostPeer;    // l'hote, s'il a amene des chats
    for (const RunCat& rc : k.run)
        if (rc.first_owner < 32) slots |= 1u << rc.first_owner;
    return true;
}

uint8_t team_run_seq() { return k.run_seq; }

uint8_t team_slot_of_cat(uint64_t cat_id) {
    for (const RunCat& rc : k.run)
        if (rc.cat_id == cat_id) return rc.first_owner;
    // Une recrue : la bourse de son attributaire, s'il en a une (il a amene des chats).
    for (const Recruit& r : k.recruits) {
        if (r.cat_id != cat_id || r.owner == kHostPeer) continue;
        for (const RunCat& rc : k.run)
            if (rc.owner_name == r.owner_name) return rc.first_owner;
    }
    return kHostPeer;
}

int team_slot_peer(uint8_t slot) {
    if (slot == kHostPeer) return kHostPeer;
    for (const RunCat& rc : k.run)
        if (rc.first_owner == slot) return in_session() ? peer_for(rc) : -1;
    return -1;
}

std::string team_slot_name(uint8_t slot) {
    if (slot != kHostPeer)
        for (const RunCat& rc : k.run)
            if (rc.first_owner == slot) return rc.owner_name;
    return mm_peer_name(slot);
}

void team_note_payout(uint8_t seq, int32_t coins, int32_t food) {
    if (coins < 0) coins = 0;
    if (food < 0) food = 0;
    if (!coins && !food) return;
    Payout p;
    p.seq   = seq;
    p.coins = coins;
    p.food  = food;
    if (const GuestDepart* d = departure(seq)) p.save = d->save;
    if (p.save.empty()) {
        char own[64] = {};
        if (savefile_own_save_name(own, sizeof(own))) p.save = own;
    }
    // La meme fin deux fois (meme vote, meme save) : une seule bourse. Les votes
    // repartent de 1 chez chaque hote : le numero seul ne suffit pas.
    k.payouts.erase(std::remove_if(k.payouts.begin(), k.payouts.end(),
                                   [&](const Payout& o) {
                                       return o.seq == seq && _stricmp(o.save.c_str(), p.save.c_str()) == 0;
                                   }),
                    k.payouts.end());
    if (k.payouts.size() >= kMaxPayouts) {
        log_line("TEAM", "!! trop de bourses en attente : la plus ancienne (%d piece(s), %d nourriture(s), %s) "
                         "est oubliee", k.payouts.front().coins, k.payouts.front().food,
                 k.payouts.front().save.empty() ? "save inconnue" : k.payouts.front().save.c_str());
        k.payouts.erase(k.payouts.begin());
    }
    k.payouts.push_back(p);
    if (!p.save.empty()) pending_store(p.save.c_str());   // gardee meme si le jeu se ferme avant
    log_line("TEAM", "<- ta bourse de l'aventure (vote %u) : %d piece(s), %d de nourriture -- ajoutee a %s a "
                     "son prochain chargement", (unsigned)seq, coins, food,
             p.save.empty() ? "ta save" : p.save.c_str());
    // Deja chez lui, sa save ouverte par le jeu : elle n'y sera qu'au prochain
    // chargement.
    if (mm_house() && !savefile_in_coop())
        say(u8"Retour de l'aventure\n\n"
            u8"Ta bourse de l'aventure (" + purse_amount_text(coins, food) + u8")\n"
            u8"t'attend, mais ta save est déjà ouverte : retourne\n"
            u8"au menu principal et recharge-la pour la recevoir.");
}

bool team_note_loot(uint8_t seq, uint64_t key, const ItemFields& item, const std::string& name) {
    for (const LootItem& l : k.loot)
        if (l.key == key && l.seq == seq) return true;       // deja note
    LootItem l;
    l.seq  = seq;
    l.key  = key;
    l.item = item;
    l.name = name;
    if (const GuestDepart* d = departure(seq)) l.save = d->save;
    if (l.save.empty()) {
        char own[64] = {};
        if (savefile_own_save_name(own, sizeof(own))) l.save = own;
    }
    if (k.loot.size() >= kMaxLoot) {
        log_line("TEAM", "!! trop d'objets de butin en attente : %s n'est pas pris", item.name.c_str());
        return false;
    }
    k.loot.push_back(std::move(l));
    // Garde dans le fichier de la save avant de dire a l'hote « pris » : il le
    // retire alors de son rangement, et l'objet ne doit exister nulle part
    // ailleurs que la, meme si ce joueur ferme son jeu avant de recharger sa save.
    const std::string save = k.loot.back().save;
    if (!save.empty() && !pending_store(save.c_str())) {
        log_line("TEAM", "!! butin : %s n'a pas pu etre garde sur le disque -- il n'est pas pris (il reste chez l'hote)",
                 item.name.c_str());
        k.loot.pop_back();
        return false;
    }
    log_line("TEAM", "<- butin : %s -- ajoute au rangement de %s a son prochain chargement", item.name.c_str(),
             save.empty() ? "ta save" : save.c_str());
    return true;
}

bool team_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_ButchBoxDepart: *detour = (void*)&h_ButchDepart; *original = (void**)&o_ButchDepart; return true;
    case T_HouseReturnCat: *detour = (void*)&h_ReturnCat;   *original = (void**)&o_ReturnCat;   return true;
    case T_RunCleared:     *detour = (void*)&h_RunCleared;  *original = (void**)&o_RunCleared;  return true;
    default: return false;
    }
}

} // namespace mgmp
