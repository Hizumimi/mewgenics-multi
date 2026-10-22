// mm_loot -- Mewgenics Multi : le defile du butin. Voir mm_loot.h.

#include "mm_loot.h"
#include "mm_loot_core.h"
#include "mm_menu.h"
#include "mm_rungear.h"
#include "mm_shop.h"
#include "mm_team.h"
#include "mm_team_merge.h"

#include "mgmp_addresses.h"
#include "mgmp_hooks.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"

#include <windows.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_loot.h -----------------------------------------------

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
constexpr uintptr_t kNode_Payload = 0x18;    // noeud d'un seau : suivant +0, precedent +8, cle +0x10, Equipment
// L'id donne a la copie d'un objet passee au jeu pour son nom : aucun objet du
// jeu ne l'a (le compteur part de 0), et ce n'est pas -1 (emplacement vide).
constexpr uint64_t  kForeignId    = 0xFFFFFFFFFFFFFFFEull;

// --- les delais et les tailles ----------------------------------------------------------

constexpr uint64_t kStateBeatMs  = 1000;     // hote : l'etat, meme sans changement
constexpr uint64_t kNotReadyMs   = 3000;     // « ne voit plus le defile » apres ce temps sans sa fenetre
constexpr uint64_t kExpectMs     = 15000;    // invite : sa copie a gagne, le defile attendu au plus
constexpr uint64_t kStartFreshMs = 30000;    // invite : un kLootStart recu depuis moins : deja la
constexpr uint64_t kStaleMs      = 30000;    // invite : plus rien de l'hote depuis, le defile est oublie
constexpr uint64_t kNoProgressMs = 300000;   // invite : le defile n'avance plus depuis, il est oublie
constexpr size_t   kMaxDesc      = 600;      // une description plus longue n'est pas montree
constexpr size_t   kMaxItemName  = 63;       // un objet au nom (cle GON) ou au texte plus long ne defile pas :
constexpr size_t   kMaxItemText  = 255;      //   le message le couperait (mm_loot_core)
constexpr int      kMaxDescLines = 8;
constexpr size_t   kMaxNoted     = 256;
constexpr int      kLookPerFrame = 4;        // noms demandes au jeu par image
constexpr uint64_t kChosenShowMs = 700;      // « Ton choix : ... » pas avant (le resultat arrive souvent avant)

// --- les chaines du jeu (MSVC x64, identiques ici) ----------------------------------------

struct GStr {                        // std::string
    union { char buf[16]; char* ptr; };
    uint64_t size;
    uint64_t cap;                    // 15 : tampon interne
};
struct GWStr {                       // std::wstring
    union { wchar_t buf[8]; wchar_t* ptr; };
    uint64_t size;
    uint64_t cap;                    // 7 : tampon interne
};
static_assert(sizeof(GStr) == 32 && sizeof(GWStr) == 32, "chaines MSVC = 32 octets");

// Une copie d'un Equipment (0x60 octets), pour SpawnDatabase::ItemName/ItemTooltip.
struct FakeEquip {
    uint64_t id;
    GStr     name;
    GStr     text;
    uint32_t a, b, c, d;
    uint8_t  e;
    uint8_t  pad[3];
    uint32_t f;
};
static_assert(sizeof(FakeEquip) == 0x60, "Equipment = 0x60 octets");
static_assert(offsetof(FakeEquip, name) == kEq_Name && offsetof(FakeEquip, text) == kEq_Text &&
              offsetof(FakeEquip, a) == kEq_A && offsetof(FakeEquip, b) == kEq_B &&
              offsetof(FakeEquip, c) == kEq_C && offsetof(FakeEquip, d) == kEq_D &&
              offsetof(FakeEquip, e) == kEq_E && offsetof(FakeEquip, f) == kEq_F,
              "disposition d'un Equipment");

using FnItemText = void* (__fastcall*)(void* db, GWStr* out, void* equip);
using FnWTidy    = void (__fastcall*)(GWStr* s);
using FnErase    = uint32_t (__fastcall*)(void* bucket, void* item);

// Les appels du jeu, sous SEH (rien de C++ a derouler ici).
bool seh_item_text(FnItemText fn, void* db, GWStr* out, FakeEquip* eq) {
    __try {
        fn(db, out, eq);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool seh_wtidy(FnWTidy fn, GWStr* s) {
    __try {
        fn(s);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool seh_erase(FnErase fn, void* bucket, void* item, uint32_t* out) {
    __try {
        *out = fn(bucket, item);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// --- l'etat ---------------------------------------------------------------------------------

struct Item {
    ItemFields  f;
    uint64_t    id     = 0;          // hote : son id dans le rangement
    uint64_t    digest = 0;
    std::string name;                // affiche (UTF-8, balises du jeu possibles)
    std::string desc;
    bool        looked = false;      // nom et description demandes au jeu
};

// Hote : un objet parti chez son gagnant, sa reponse attendue (tant qu'il est la :
// une reponse tardive compte encore).
struct Give {
    uint32_t    parade = 0;
    uint8_t     index  = 0;
    uint8_t     peer   = 0;
    uint64_t    id     = 0;
    uint64_t    digest = 0;
    std::string name;
    std::string who;
    uint64_t    at     = 0;
};

// Hote : un objet pris par son gagnant, a retirer du rangement des que rien ne le
// tient a l'ecran (l'ecran des objets, une confirmation du jeu sur lui).
struct Erase {
    uint8_t     index  = 0;
    uint64_t    id     = 0;
    uint64_t    digest = 0;
    std::string name;
    std::string who;
    bool        told   = false;      // « attend » deja dit
};

// Un objet du sac a la victoire (hote).
struct Stored {
    void*      payload = nullptr;
    uint64_t   id      = 0;
    ItemFields f;
};

struct State {
    bool     told     = false;
    bool     armed    = false;       // T_RunVictory et C_BucketErase sont la
    bool     names_ok = false;       // les noms viennent du jeu (sinon : la cle GON)

    // Les deux : le defile en cours, dont ce joueur est.
    bool     on       = false;
    uint32_t parade   = 0;
    uint8_t  seq      = 0;
    uint32_t players  = 0;
    std::vector<Item> items;
    uint32_t hidden   = 0;           // la fenetre masquee par le joueur (son layout)
    bool     step2    = false;       // « Je le veux » appuye...
    uint8_t  step2_index = 0;        // ... pour cet objet
    uint64_t chose_at    = 0;        // son dernier choix...
    uint8_t  chose_index = 0;        // ... pour cet objet
    bool     summary_closed = false;
    bool     ready_now    = false;   // il voit le defile (anti-rebond compris)
    uint64_t unseen_since = 0;       // sa fenetre n'est plus la depuis (0 : elle est la)

    // Hote.
    bool     snap = false;           // la victoire en cours : le sac et les joueurs
    std::vector<uint64_t> snap_ids;
    uint32_t snap_players = 0;
    uint8_t  snap_seq     = 0;
    uint32_t next_parade  = 0;
    LootHost host;
    LootRng  rng{0};
    bool     dirty   = false;
    uint64_t sent_at = 0;
    LootPhase last_phase = LootPhase::Idle;
    uint8_t   last_index = 0;
    std::vector<Give>  gives;
    std::vector<Erase> erases;
    uint32_t given = 0, kept = 0;

    // Invite.
    bool     have_state = false;
    LootState st;
    uint64_t st_at      = 0;
    uint64_t heard_at   = 0;
    uint64_t progress_at = 0;        // le dernier changement d'objet ou de phase
    uint64_t received   = 0;         // les objets que l'hote lui a donnes (un bit par objet)
    uint64_t start_at   = 0;         // le dernier kLootStart recu
    uint8_t  my_choice[kLootMaxItems] = {};
    bool     ready_sent = false;     // ce qu'il a dit a l'hote
    bool     brought    = false;     // il a amene des chats a l'aventure en cours
    uint64_t expect_until = 0;       // sa copie a gagne : le defile est attendu jusque-la
    std::vector<uint64_t> noted;     // (defile << 8) | objet : deja pris
    LootPhase seen_phase = LootPhase::Idle;
    uint8_t   seen_index = 0xFF;
};
State g;

// --- petits outils ----------------------------------------------------------------------

bool is_host()  { return net_role() == NetRole::Host; }
bool is_guest() { return net_role() == NetRole::Client; }
bool in_session() { return net_active() && net_peer_count() >= 2; }

uint32_t roster_mask() {
    uint8_t ids[8] = {};
    uint32_t mask = 0;
    const uint8_t n = net_peer_count();
    if (n && net_peer_ids(ids, 8))
        for (uint8_t i = 0; i < n && i < 8; ++i)
            if (ids[i] < 32) mask |= 1u << ids[i];
    return mask;
}

std::string peer_label(uint8_t p) { return mm_peer_name(p); }

const char* phase_name(LootPhase p) {
    switch (p) {
    case LootPhase::Waiting:  return "attente";
    case LootPhase::Choosing: return "choix";
    case LootPhase::Result:   return "resultat";
    case LootPhase::Done:     return "fin";
    default:                  return "rien";
    }
}

std::string mask_names(uint32_t mask) {
    std::vector<std::string> v;
    for (uint8_t p = 0; p < kLootSlots; ++p)
        if (mask & (1u << p)) v.push_back(peer_label(p));
    return v.empty() ? std::string("personne") : loot_join(v);
}

bool send_msg(const LootMsg& m, int peer) { return team_send(loot_encode(m), peer); }

// --- lire le jeu ---------------------------------------------------------------------------

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

// L'inventaire du Director vivant. L'emplacement global du Director n'est jamais
// remis a zero (un nouveau a chaque chargement) : son etiquette (ptr-8) doit etre
// celle notee a cote de l'emplacement (+8), comme le jeu le verifie lui-meme
// (mm_npc.cpp) -- sinon, au menu, le retrait d'un objet toucherait un inventaire
// libere (relecture de la build 29).
void* inventory() {
    const void* const* slot = (const void* const*)addr_of_data(D_MewDirectorPtr);
    const uint8_t* dir = nullptr;
    uint64_t tag = 0, mark = 0;
    void* inv = nullptr;
    if (!slot || !mem_read(slot, &dir, sizeof(dir)) || !dir) return nullptr;
    if (!mem_read(slot + 1, &tag, sizeof(tag)) || !mem_read(dir - 8, &mark, sizeof(mark)) || mark != tag)
        return nullptr;
    if (!mem_read(dir + kDir_Inventory, &inv, sizeof(inv))) return nullptr;
    return inv;
}

// Un seau de l'inventaire (kInv_Backpack, kInv_Storage), dans l'ordre de sa liste.
bool read_bucket(uintptr_t which, std::vector<Stored>& out) {
    out.clear();
    uint8_t* inv = (uint8_t*)inventory();
    if (!inv) return false;
    uint8_t* bucket = inv + which;
    uint8_t* head = nullptr;
    uint64_t count = 0;
    if (!mem_read(bucket + kBucket_Head, &head, sizeof(head)) || !head) return false;
    if (!mem_read(bucket + kBucket_Count, &count, sizeof(count)) || count > 4096) return false;
    uint8_t* n = nullptr;
    if (!mem_read(head, &n, sizeof(n))) return false;
    for (uint32_t guard = 0; n && n != head; ++guard) {
        if (guard > count + 4) return false;           // une liste qui ne revient pas a sa tete
        Stored s;
        s.payload = n + kNode_Payload;
        if (read_item(s.payload, s.f, &s.id) && !s.f.name.empty()) out.push_back(std::move(s));
        if (!mem_read(n, &n, sizeof(n))) return false;
    }
    return true;
}

// --- le nom d'un objet, par le jeu -----------------------------------------------------------

std::string wide_to_utf8(const wchar_t* w, size_t n) {
    if (!w || !n) return std::string();
    const int len = WideCharToMultiByte(CP_UTF8, 0, w, (int)n, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string out((size_t)len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, (int)n, &out[0], len, nullptr, nullptr);
    return out;
}

// La chaine `s` vue par le jeu : dans le tampon interne si elle y tient, sinon
// son pointeur vers `s` (le jeu ne fait que la lire, `s` vit pendant l'appel).
void set_gstr(GStr& g, const std::string& s) {
    memset(&g, 0, sizeof(g));
    if (s.size() <= 15) {
        memcpy(g.buf, s.data(), s.size());
        g.buf[s.size()] = 0;
        g.size = s.size();
        g.cap  = 15;
        return;
    }
    g.ptr  = const_cast<char*>(s.c_str());
    g.size = s.size();
    g.cap  = s.size();
}

// Le texte que la fonction du jeu `c` (C_ItemName, C_ItemTooltip) ecrit pour cet
// objet. Faux si elle n'a pas pu (et alors plus aucun appel : g.names_ok).
bool item_text(Call c, const ItemFields& f, std::string& out) {
    out.clear();
    auto fn   = (FnItemText)addr_of_call(c);
    auto tidy = (FnWTidy)addr_of_call(C_WStrTidy);
    const void* const* slot = (const void* const*)addr_of_data(D_SpawnDatabase);
    void* db = nullptr;
    if (!fn || !tidy || !slot || !mem_read(slot, &db, sizeof(db)) || !db) return false;
    const std::string name = f.name, text = f.text;
    FakeEquip eq;
    memset(&eq, 0, sizeof(eq));
    eq.id = kForeignId;
    set_gstr(eq.name, name);
    set_gstr(eq.text, text);
    eq.a = f.a;
    eq.b = f.b;
    eq.c = f.c;
    eq.d = f.d;
    eq.e = f.e;
    eq.f = f.f;
    GWStr w;
    memset(&w, 0, sizeof(w));
    w.cap = 7;
    if (!seh_item_text(fn, db, &w, &eq)) {
        log_line("BUTIN", "!! le jeu n'a pas pu nommer l'objet %s (exception) -- les cles des objets seront "
                          "montrees a la place de leur nom", f.name.c_str());
        g.names_ok = false;
        return false;
    }
    const wchar_t* p = w.cap > 7 ? w.ptr : w.buf;
    if (p && w.size && w.size <= 4096) out = wide_to_utf8(p, (size_t)w.size);
    if (!seh_wtidy(tidy, &w)) {
        log_line("BUTIN", "!! liberation du nom d'un objet impossible (exception) -- plus de noms du jeu");
        g.names_ok = false;
    }
    return !out.empty();
}

// Sans le jeu : la cle GON, et son texte (un livre : sa capacite).
std::string fallback_name(const ItemFields& f) {
    std::string s = f.name + (f.text.empty() ? std::string() : " (" + f.text + ")");
    for (char& ch : s)
        if (ch == '[') ch = '(';
        else if (ch == ']') ch = ')';
    return s;
}

// La description : sans le nom s'il la commence, sans lignes vides au bout ;
// trop longue, elle n'est pas montree (couper une balise la casserait).
std::string trim_desc(std::string d, const std::string& name) {
    for (char& ch : d)
        if (ch == '\r') ch = '\n';
    if (!name.empty() && d.compare(0, name.size(), name) == 0) {
        d.erase(0, name.size());
        while (!d.empty() && (d[0] == '\n' || d[0] == ' ')) d.erase(0, 1);
    }
    while (!d.empty() && (d.back() == '\n' || d.back() == ' ')) d.pop_back();
    const int lines = 1 + (int)std::count(d.begin(), d.end(), '\n');
    if (d.size() > kMaxDesc || lines > kMaxDescLines) return std::string();
    return d;
}

void look_items() {
    int budget = kLookPerFrame;
    for (Item& it : g.items) {
        if (it.looked) continue;
        if (budget-- <= 0) break;
        it.looked = true;
        std::string name, desc;
        if (g.names_ok && item_text(C_ItemName, it.f, name)) it.name = name;
        else it.name = fallback_name(it.f);
        if (g.names_ok && item_text(C_ItemTooltip, it.f, desc)) it.desc = trim_desc(desc, it.name);
    }
}

std::string item_name(size_t i) {
    if (i >= g.items.size()) return "?";
    return g.items[i].name.empty() ? fallback_name(g.items[i].f) : g.items[i].name;
}

// --- le defile ---------------------------------------------------------------------------

void reset_parade() {
    g.on             = false;
    g.parade         = 0;
    g.seq            = 0;
    g.players        = 0;
    g.items.clear();
    g.hidden         = 0;
    g.step2          = false;
    g.step2_index    = 0;
    g.chose_at       = 0;
    g.chose_index    = 0;
    g.summary_closed = false;
    g.ready_now      = false;
    g.unseen_since   = 0;
    g.dirty          = false;
    g.sent_at        = 0;
    g.last_phase     = LootPhase::Idle;
    g.last_index     = 0;
    g.have_state     = false;
    g.st             = LootState{};
    g.st_at          = 0;
    g.heard_at       = 0;
    g.progress_at    = 0;
    g.received       = 0;
    memset(g.my_choice, 0, sizeof(g.my_choice));
    g.ready_sent     = false;
    g.seen_phase     = LootPhase::Idle;
    g.seen_index     = 0xFF;
}

LootView make_view(uint64_t now) {
    LootView v;
    v.name = peer_label;
    for (size_t i = 0; i < g.items.size(); ++i) {
        v.item_names.push_back(item_name(i));
        v.item_descs.push_back(g.items[i].desc);
    }
    if (is_host()) {
        v.st      = g.host.state(now);
        v.self    = kHostPeer;
        v.host    = kHostPeer;
        v.is_host = true;
        if (v.st.phase == LootPhase::Choosing) v.my_choice = g.host.choice_of(kHostPeer);
    } else {
        v.self    = net_self();
        v.host    = kHostPeer;
        v.is_host = false;
        v.at_home = mm_house() && !savefile_in_coop();
        v.track_received = true;
        v.received       = g.received;
        if (g.have_state) {
            v.st = g.st;
            // Le compte a rebours continue ici tant que l'hote voit le defile
            // (sinon son temps est arrete) ; son prochain etat le recale.
            if ((g.st.ready & (1u << kHostPeer)) && now > g.st_at) {
                const uint64_t el = now - g.st_at;
                v.st.ms_left = el >= v.st.ms_left ? 0 : (uint32_t)(v.st.ms_left - el);
            }
        } else {
            v.st.parade  = g.parade;
            v.st.phase   = LootPhase::Waiting;
            v.st.players = g.players;
            v.st.winners.assign(g.items.size(), kLootUndecided);
        }
        if (v.st.phase == LootPhase::Choosing && v.st.index < kLootMaxItems) v.my_choice = g.my_choice[v.st.index];
    }
    v.step2 = g.step2 && g.step2_index == v.st.index && v.st.phase == LootPhase::Choosing;
    return v;
}

uint32_t layout_id(const LootScreen& s, const LootState& st) {
    if (s.layout == kLayNone) return 0;
    return ((g.parade & 0xFFu) << 24) | ((uint32_t)st.index << 16) | ((uint32_t)s.layout << 8) | 0x5Au;
}

uint32_t current_layout(uint64_t now) {
    if (!g.on || g.summary_closed) return 0;
    const LootView v = make_view(now);
    return layout_id(loot_screen(v), v.st);
}

// Ce joueur voit-il le defile ? Sa fenetre est ouverte, ou il l'a masquee lui-meme
// et rien d'autre n'est a l'ecran de sa maison. « Plus » seulement apres
// kNotReadyMs : la fenetre se referme et se rouvre a chaque changement de boutons.
void update_ready(uint64_t now) {
    const bool sees = mm_loot_popup_open() ||
                      (g.hidden && g.hidden == current_layout(now) && mm_house() && !mm_other_popup_open());
    if (sees) {
        g.unseen_since = 0;
        g.ready_now    = true;
        return;
    }
    if (!g.unseen_since) g.unseen_since = now;
    if (now - g.unseen_since >= kNotReadyMs) g.ready_now = false;
}

// --- hote ------------------------------------------------------------------------------------

void broadcast_state(uint64_t now) {
    LootMsg m;
    m.op     = kLootState;
    m.parade = g.parade;
    m.state  = g.host.state(now);
    g.sent_at = now;                         // un envoi rate n'est pas retente a chaque image
    if (send_msg(m, -1)) g.dirty = false;
}

void host_finish(const char* why) {
    if (g.on)
        log_line("BUTIN", "defile %u : %s (%u objet(s) partis chez les invites ; %u n'ont pas pu partir et "
                          "restent chez toi)", g.parade, why, g.given, g.kept);
    g.host.stop();
    reset_parade();
}

void host_abort(const char* why) {
    if (!g.on) return;
    log_line("BUTIN", "!! defile %u arrete : %s -- les objets pas encore partages restent chez toi", g.parade, why);
    host_finish("arrete");
}

void host_start(std::vector<Item> items, uint32_t players, uint8_t seq, uint64_t now) {
    if (g.on) host_abort("une nouvelle victoire");
    if (!g.next_parade) g.next_parade = ((uint32_t)(now & 0xFFFFFFu) << 8) | 1u;
    const uint32_t parade = g.next_parade++;
    LootMsg m;
    m.op      = kLootStart;
    m.parade  = parade;
    m.seq     = seq;
    m.players = players;
    const bool guests = (players & ~(1u << kHostPeer)) != 0;
    if (!guests || items.empty()) {
        // Les invites qui attendaient le defile sont liberes.
        send_msg(m, -1);
        log_line("BUTIN", "victoire : %s -- tout reste chez toi",
                 items.empty() ? "aucun objet du sac dans le rangement" : "aucun invite de l'aventure n'est la");
        return;
    }
    for (const Item& it : items) m.items.push_back(it.f);
    reset_parade();
    g.on      = true;
    g.parade  = parade;
    g.seq     = seq;
    g.players = players;
    g.items   = std::move(items);
    g.given   = 0;
    g.kept    = 0;
    g.rng     = LootRng(now * 0x9E3779B97F4A7C15ull ^ ((uint64_t)parade << 32) ^ (uint64_t)(uintptr_t)&g);
    std::vector<ItemFields> fields;
    for (const Item& it : g.items) fields.push_back(it.f);
    g.host.start(parade, seq, std::move(fields), players, kHostPeer, now);
    g.last_phase = g.host.phase();
    g.last_index = g.host.index();
    if (!send_msg(m, -1)) {
        log_line("BUTIN", "!! defile : envoi aux invites impossible");
        host_abort("envoi impossible");
        return;
    }
    std::string names;
    for (const Item& it : g.items) names += (names.empty() ? "" : ", ") + it.f.name;
    log_line("BUTIN", "defile %u : %zu objet(s) (%s) entre %s -- vote du depart %u", parade, g.items.size(),
             names.c_str(), mask_names(players).c_str(), (unsigned)seq);
    g.dirty = true;
}

// L'objet `ix` part chez son gagnant (un invite) ; il quitte le rangement quand
// celui-ci repond l'avoir pris.
// Un objet du rangement par son id, verifie par son empreinte.
bool stored_item(uint64_t id, uint64_t digest, Stored* out, std::string& why) {
    std::vector<Stored> store;
    if (!read_bucket(kInv_Storage, store)) { why = "rangement illisible"; return false; }
    for (const Stored& s : store) {
        if (s.id != id) continue;
        if (item_digest(s.f) != digest) { why = "l'objet de cet id a change"; return false; }
        if (out) *out = s;
        return true;
    }
    why = "plus dans le rangement";
    return false;
}

void host_give(uint8_t ix, uint8_t peer) {
    const std::string who = peer_label(peer);
    if (ix >= g.items.size()) return;
    if (!(roster_mask() & (1u << peer))) {
        ++g.kept;
        log_line("BUTIN", "objet %u (%s) : %s n'est plus la -- il reste chez toi", ix + 1, g.items[ix].f.name.c_str(),
                 who.c_str());
        return;
    }
    // Encore la, tel qu'au debut du defile ? (L'hote a pu le prendre entre-temps,
    // sa fenetre masquee.) Sinon il n'est pas donne : sans doublon.
    std::string why;
    if (!stored_item(g.items[ix].id, g.items[ix].digest, nullptr, why)) {
        ++g.kept;
        log_line("BUTIN", "!! objet %u (%s) : %s -- il n'est pas donne a %s", ix + 1, g.items[ix].f.name.c_str(),
                 why.c_str(), who.c_str());
        return;
    }
    LootMsg m;
    m.op     = kLootGive;
    m.parade = g.parade;
    m.index  = ix;
    if (!send_msg(m, peer)) {
        ++g.kept;
        log_line("BUTIN", "!! objet %u (%s) : envoi a %s impossible -- il reste chez toi", ix + 1,
                 g.items[ix].f.name.c_str(), who.c_str());
        return;
    }
    Give gv;
    gv.parade = g.parade;
    gv.index  = ix;
    gv.peer   = peer;
    gv.id     = g.items[ix].id;
    gv.digest = g.items[ix].digest;
    gv.name   = g.items[ix].f.name;
    gv.who    = who;
    gv.at     = GetTickCount64();
    g.gives.push_back(gv);
    log_line("BUTIN", "-> objet %u (%s, id %llu) donne a %s, en attente de sa reponse", ix + 1, gv.name.c_str(),
             (unsigned long long)gv.id, who.c_str());
}

void host_transition(LootPhase ph, uint8_t ix) {
    switch (ph) {
    case LootPhase::Choosing:
        log_line("BUTIN", "objet %u/%zu : %s (joueurs prets : %s)", ix + 1, g.items.size(), item_name(ix).c_str(),
                 mask_names(g.host.ready() & g.host.players()).c_str());
        break;
    case LootPhase::Result: {
        if (ix >= g.host.results().size()) break;
        const LootResult& r = g.host.results()[ix];
        std::string choices;
        for (const LootRoll& x : r.rolls) {
            if (!choices.empty()) choices += ", ";
            choices += peer_label(x.peer) + " " + loot_choice_word(x.choice);
            if (x.roll) choices += " " + std::to_string(x.roll);
        }
        if (r.winner == kLootNobody) {
            log_line("BUTIN", "objet %u (%s) : personne n'en veut, il reste chez toi (%s)", ix + 1,
                     g.items[ix].f.name.c_str(), choices.c_str());
            break;
        }
        log_line("BUTIN", "objet %u (%s) : a %s (%s%s) -- %s", ix + 1, g.items[ix].f.name.c_str(),
                 peer_label(r.winner).c_str(), loot_choice_word(r.category), r.rerolls ? ", egalite departagee" : "",
                 choices.c_str());
        if (r.winner != kHostPeer) host_give(ix, r.winner);
        break;
    }
    case LootPhase::Done:
        log_line("BUTIN", "defile %u fini", g.parade);
        break;
    default:
        break;
    }
}

// La reponse d'un gagnant : l'objet quitte le rangement (pris), ou reste.
void host_got(uint8_t from, const LootMsg& m) {
    for (size_t i = 0; i < g.gives.size(); ++i) {
        Give& gv = g.gives[i];
        if (gv.parade != m.parade || gv.index != m.index || gv.peer != from) continue;
        if (!m.ok) {
            ++g.kept;
            log_line("BUTIN", "<- %s n'a pas pu prendre l'objet %u (%s) : il reste chez toi", gv.who.c_str(),
                     gv.index + 1, gv.name.c_str());
        } else {
            Erase e;
            e.index  = gv.index;
            e.id     = gv.id;
            e.digest = gv.digest;
            e.name   = gv.name;
            e.who    = gv.who;
            g.erases.push_back(e);
            log_line("BUTIN", "<- %s a pris l'objet %u (%s)", gv.who.c_str(), gv.index + 1, gv.name.c_str());
        }
        g.gives.erase(g.gives.begin() + (ptrdiff_t)i);
        return;
    }
    log_line("BUTIN", "<- reponse de %s pour l'objet %u du defile %u, qui ne lui etait pas donne -- ignoree",
             peer_label(from).c_str(), m.index + 1, m.parade);
}

// Les objets donnes a un joueur parti avant de repondre : ils restent chez l'hote.
void drop_gives() {
    const uint32_t roster = in_session() ? roster_mask() : 0;
    for (size_t i = g.gives.size(); i-- > 0;) {
        if (roster & (1u << g.gives[i].peer)) continue;
        ++g.kept;
        log_line("BUTIN", "!! %s est parti sans repondre pour l'objet %u (%s) : il reste chez toi",
                 g.gives[i].who.c_str(), g.gives[i].index + 1, g.gives[i].name.c_str());
        g.gives.erase(g.gives.begin() + (ptrdiff_t)i);
    }
}

// Les objets pris par leur gagnant quittent le rangement -- pas tant que l'ecran
// des objets ou une confirmation du jeu est ouvert : le jeu y tient peut-etre
// cet objet (une boite, la souris), et le retrait libere sa memoire.
void run_erases() {
    if (g.erases.empty()) return;
    if (rungear_screen_open() || mm_game_prompt_open()) {
        for (Erase& e : g.erases)
            if (!e.told) {
                e.told = true;
                log_line("BUTIN", "objet %u (%s) : il quittera ton rangement a la fermeture de l'ecran des objets",
                         e.index + 1, e.name.c_str());
            }
        return;
    }
    auto erase = (FnErase)addr_of_call(C_BucketErase);
    uint8_t* inv = (uint8_t*)inventory();
    for (const Erase& e : g.erases) {
        Stored s;
        std::string why;
        uint32_t n = 0;
        bool ok = false;
        if (!inv || !erase) why = "fonction du jeu ou inventaire introuvable";
        else if (!stored_item(e.id, e.digest, &s, why)) {}
        else if (!seh_erase(erase, inv + kInv_Storage, s.payload, &n)) why = "exception";
        else if (n != 1) why = "le jeu n'a rien retire";
        else ok = true;
        if (ok) {
            ++g.given;
            log_line("BUTIN", "objet %u (%s) : retire de ton rangement (%s l'a)", e.index + 1, e.name.c_str(),
                     e.who.c_str());
        } else {
            ++g.kept;
            log_line("BUTIN", "!! objet %u (%s) : pris par %s, mais il n'a pas pu quitter ton rangement (%s) -- il "
                              "est aussi chez toi", e.index + 1, e.name.c_str(), e.who.c_str(), why.c_str());
        }
    }
    g.erases.clear();
}

void host_pump(uint64_t now) {
    drop_gives();
    run_erases();
    if (!g.on) return;
    if (!in_session()) {
        host_abort("la session est finie");
        return;
    }
    const uint32_t roster = roster_mask();
    for (uint8_t p = 1; p < kLootSlots; ++p)
        if ((g.host.players() & (1u << p)) && !(roster & (1u << p))) {
            g.host.gone(p);
            g.dirty = true;
            log_line("BUTIN", "%s a quitte la session : son tour passe", peer_label(p).c_str());
        }
    const bool was = (g.host.ready() & (1u << kHostPeer)) != 0;
    update_ready(now);
    g.host.set_ready(kHostPeer, g.ready_now, now);
    if (was != g.ready_now) {
        g.dirty = true;
        if (g.host.running())
            log_line("BUTIN", g.ready_now ? "tu vois le defile" : "tu ne vois plus le defile : le temps s'arrete");
    }
    if (g.host.tick(now, g.rng)) g.dirty = true;
    const LootPhase ph = g.host.phase();
    const uint8_t   ix = g.host.index();
    if (ph != g.last_phase || ix != g.last_index) {
        g.last_phase = ph;
        g.last_index = ix;
        host_transition(ph, ix);
        g.dirty = true;
    }
    if (g.dirty || now - g.sent_at >= kStateBeatMs) broadcast_state(now);
}

void host_on(uint8_t from, const LootMsg& m) {
    if (m.op == kLootGot) {
        host_got(from, m);
        return;
    }
    if (!g.on || m.parade != g.parade) return;           // un defile d'avant
    const uint64_t now = GetTickCount64();
    switch (m.op) {
    case kLootReady:
        if (!(g.host.players() & (1u << from))) return;
        g.host.set_ready(from, m.ready != 0, now);
        g.dirty = true;
        log_line("BUTIN", "<- %s %s le defile", peer_label(from).c_str(), m.ready ? "voit" : "ne voit plus");
        break;
    case kLootChoose:
        if (g.host.choose(from, m.index, m.choice)) {
            g.dirty = true;
            log_line("BUTIN", "<- %s : %s pour l'objet %u", peer_label(from).c_str(), loot_choice_word(m.choice),
                     m.index + 1);
        } else {
            log_line("BUTIN", "<- choix de %s pour l'objet %u refuse (objet %u en cours, phase %s)",
                     peer_label(from).c_str(), m.index + 1, g.host.index() + 1, phase_name(g.host.phase()));
        }
        break;
    default:
        log_line("BUTIN", "!! message du defile inattendu (%u) de %s", (unsigned)m.op, peer_label(from).c_str());
        break;
    }
}

// --- invite --------------------------------------------------------------------------------

void guest_abort(const char* why) {
    if (!g.on) return;
    log_line("BUTIN", "!! defile %u oublie : %s", g.parade, why);
    // L'hote ne doit plus attendre ce joueur a chaque objet. Sans condition sur
    // ready_sent : un choix suffit a le marquer pret chez l'hote. Au mieux : perdu
    // si le lien est coupe, comme avant.
    if (in_session()) {
        LootMsg m;
        m.op     = kLootReady;
        m.parade = g.parade;
        m.ready  = 0;
        if (send_msg(m, kHostPeer)) log_line("BUTIN", "-> tu ne vois plus le defile");
    }
    reset_parade();
}

void guest_start(const LootMsg& m) {
    const uint64_t now = GetTickCount64();
    g.expect_until = 0;
    g.start_at     = now;
    reset_parade();
    const uint8_t self = net_self();
    if (m.items.empty()) {
        log_line("BUTIN", "<- fin de l'aventure : pas de butin a partager");
        return;
    }
    if (self >= kLootSlots || !(m.players & (1u << self))) {
        log_line("BUTIN", "<- defile du butin (%zu objet(s)) entre %s : tu n'en es pas (pas de chat a toi dans "
                          "l'aventure)", m.items.size(), mask_names(m.players).c_str());
        return;
    }
    g.on       = true;
    g.parade   = m.parade;
    g.seq      = m.seq;
    g.players  = m.players;
    g.heard_at = now;
    g.progress_at = now;
    for (const ItemFields& f : m.items) {
        Item it;
        it.f      = f;
        it.digest = item_digest(f);
        g.items.push_back(std::move(it));
    }
    std::string names;
    for (const Item& it : g.items) names += (names.empty() ? "" : ", ") + it.f.name;
    log_line("BUTIN", "<- defile %u : %zu objet(s) (%s) entre %s", g.parade, g.items.size(), names.c_str(),
             mask_names(g.players).c_str());
}

void guest_state(const LootMsg& m) {
    if (!g.on || m.parade != g.parade) return;
    const uint64_t now = GetTickCount64();
    g.st         = m.state;
    g.st_at      = now;
    g.heard_at   = now;
    g.have_state = true;
    if (g.st.phase != g.seen_phase || g.st.index != g.seen_index) {
        g.seen_phase  = g.st.phase;
        g.seen_index  = g.st.index;
        g.progress_at = now;
        if (g.st.phase == LootPhase::Result) {
            const LootResult& r = g.st.result;
            log_line("BUTIN", "<- objet %u (%s) : %s", g.st.index + 1,
                     g.st.index < g.items.size() ? g.items[g.st.index].f.name.c_str() : "?",
                     r.winner == kLootNobody ? "personne n'en veut, il reste chez l'hote"
                                             : ("a " + peer_label(r.winner) + " (" + loot_choice_word(r.category) +
                                                ")").c_str());
        } else if (g.st.phase == LootPhase::Done) {
            log_line("BUTIN", "<- defile %u fini", g.parade);
        }
    }
}

void guest_give(const LootMsg& m) {
    LootMsg r;
    r.op     = kLootGot;
    r.parade = m.parade;
    r.index  = m.index;
    if (!g.on || m.parade != g.parade || m.index >= g.items.size()) {
        log_line("BUTIN", "!! objet %u du defile %u donne, mais ce defile n'est pas le tien ici -- il reste chez "
                          "l'hote", m.index + 1, m.parade);
        send_msg(r, kHostPeer);
        return;
    }
    const uint64_t key = ((uint64_t)m.parade << 8) | m.index;
    bool ok = std::find(g.noted.begin(), g.noted.end(), key) != g.noted.end();
    if (!ok) {
        ok = team_note_loot(g.seq, key, g.items[m.index].f, item_name(m.index));
        if (ok) {
            if (g.noted.size() >= kMaxNoted) g.noted.erase(g.noted.begin());
            g.noted.push_back(key);
        }
    }
    r.ok = ok ? 1 : 0;
    if (ok && m.index < 64) g.received |= 1ull << m.index;
    if (!send_msg(r, kHostPeer))
        log_line("BUTIN", "!! reponse a l'hote impossible pour l'objet %u", m.index + 1);
    log_line("BUTIN", "<- objet %u (%s) : %s", m.index + 1, g.items[m.index].f.name.c_str(),
             ok ? "il est a toi, il ira dans ton rangement" : "!! il n'a pas pu etre pris, il reste chez l'hote");
}

// Sa copie a gagne : s'il avait amene des chats, le defile de l'hote est attendu
// (la fenetre « Rentrer » aussi) -- sauf s'il est deja arrive.
void guest_victory() {
    const uint64_t now = GetTickCount64();
    const bool brought = g.brought;
    g.brought = false;
    if (!brought || !in_session()) return;
    if (g.start_at && now - g.start_at < kStartFreshMs) return;
    g.expect_until = now + kExpectMs;
    log_line("BUTIN", "ta copie de l'aventure est gagnee : le defile du butin de l'hote est attendu");
}

void guest_pump(uint64_t now) {
    // Il a amene des chats a l'aventure en cours (retenu jusqu'a la victoire de sa
    // copie, que l'hote ait deja dit ou non la fin) ; plus dans la copie : oublie.
    if (!savefile_in_coop()) g.brought = false;
    else if (mm_guest_in_run() && team_guest_brought_cats()) g.brought = true;   // pas une recrue seule
    if (g.expect_until && now >= g.expect_until) {
        log_line("BUTIN", "!! le defile du butin n'est pas venu de l'hote -- tu peux rentrer");
        g.expect_until = 0;
    }
    if (!g.on) return;
    if (!in_session()) {
        guest_abort("la session est finie");
        return;
    }
    const bool done = g.have_state && g.st.phase == LootPhase::Done;
    if (!done && now - g.heard_at >= kStaleMs) {
        guest_abort("plus rien de l'hote depuis 30 s");
        return;
    }
    // L'hote ne fait plus avancer le defile (absent, hors de sa maison) : on ne
    // retient plus ce joueur ; un objet gagne ensuite restera chez l'hote.
    if (!done && now - g.progress_at >= kNoProgressMs) {
        guest_abort("le defile n'avance plus depuis 5 minutes");
        return;
    }
    update_ready(now);
    if (!done && g.ready_now != g.ready_sent) {
        LootMsg m;
        m.op     = kLootReady;
        m.parade = g.parade;
        m.ready  = g.ready_now ? 1 : 0;
        if (send_msg(m, kHostPeer)) {
            g.ready_sent = g.ready_now;
            log_line("BUTIN", "-> tu %s le defile", g.ready_now ? "vois" : "ne vois plus");
        }
    }
}

void guest_on(uint8_t from, const LootMsg& m) {
    if (from != kHostPeer) return;
    switch (m.op) {
    case kLootStart: guest_start(m); break;
    case kLootState: guest_state(m); break;
    case kLootGive:  guest_give(m); break;
    default:
        log_line("BUTIN", "!! message du defile inattendu (%u) de l'hote", (unsigned)m.op);
        break;
    }
}

void make_choice(uint8_t index, uint8_t choice) {
    g.step2       = false;
    g.chose_at    = GetTickCount64();
    g.chose_index = index;
    if (is_host()) {
        if (g.host.choose(kHostPeer, index, choice)) {
            g.dirty = true;
            log_line("BUTIN", "ton choix : %s pour l'objet %u", loot_choice_word(choice), index + 1);
        } else {
            log_line("BUTIN", "ton choix pour l'objet %u n'a pas ete pris (objet %u, phase %s)", index + 1,
                     g.host.index() + 1, phase_name(g.host.phase()));
        }
        return;
    }
    if (index >= kLootMaxItems) return;
    g.my_choice[index] = choice;
    LootMsg m;
    m.op     = kLootChoose;
    m.parade = g.parade;
    m.index  = index;
    m.choice = choice;
    if (send_msg(m, kHostPeer)) log_line("BUTIN", "-> ton choix : %s pour l'objet %u", loot_choice_word(choice), index + 1);
    else log_line("BUTIN", "!! ton choix pour l'objet %u n'a pas pu partir", index + 1);
}

} // namespace

// ---------------------------------------------------------------------------------------------

void loot_pump() {
    if (!g.told) {
        g.told     = true;
        g.armed    = hooks_is_live(T_RunVictory) && addr_of_call(C_BucketErase) != 0;
        g.names_ok = addr_of_call(C_ItemName) && addr_of_call(C_ItemTooltip) && addr_of_call(C_WStrTidy) &&
                     addr_of_data(D_SpawnDatabase);
        if (!g.armed)
            log_line("BUTIN", "!! crochet RUNVICTORY ou fonction du jeu (retrait d'un objet) absent : pas de defile "
                              "du butin, tout reste a l'hote comme avant");
        else
            log_line("BUTIN", "arme -- defile du butin a la victoire d'une aventure mixte%s",
                     g.names_ok ? "" : " (noms des objets : leur cle, fonctions du jeu absentes)");
    }
    if (!g.armed) return;
    const uint64_t now = GetTickCount64();
    look_items();
    if (is_host()) {
        host_pump(now);
    } else if (is_guest()) {
        guest_pump(now);
    } else {
        if (g.on) log_line("BUTIN", "plus de session : le defile %u s'arrete", g.parade);
        if (g.on || g.host.running()) {
            g.host.stop();
            reset_parade();
        }
        g.expect_until = 0;
        g.brought      = false;
        g.gives.clear();
        run_erases();                        // pris avant la fin de la session : ils partent quand meme
    }
}

void loot_on_team(uint8_t from, const uint8_t* data, size_t size) {
    if (!g.armed) return;
    LootMsg m;
    if (!loot_decode(data, size, m)) {
        log_line("BUTIN", "!! message du defile illisible (%zu octets) de %u -- ignore", size, (unsigned)from);
        return;
    }
    if (is_host()) host_on(from, m);
    else if (is_guest()) guest_on(from, m);
}

void loot_victory_before() {
    if (!g.armed) return;
    g.snap = false;
    g.snap_ids.clear();
    if (is_guest()) {
        guest_victory();
        return;
    }
    if (!is_host() || !in_session()) return;
    uint32_t gen = 0, slots = 0;
    if (!team_run_slots(gen, slots)) return;             // pas une aventure mixte
    std::vector<Stored> bag;
    if (!read_bucket(kInv_Backpack, bag)) {
        log_line("BUTIN", "!! victoire : sac illisible -- pas de defile, tout reste chez toi");
        return;
    }
    uint32_t players = 1u << kHostPeer;
    for (uint8_t s = 0; s < 32; ++s) {
        if (!(slots & (1u << s)) || s == kHostPeer) continue;
        const int p = team_slot_peer(s);
        if (p > 0 && p < kLootSlots) players |= 1u << p;
    }
    g.snap         = true;
    g.snap_players = players;
    g.snap_seq     = team_run_seq();
    for (const Stored& b : bag) g.snap_ids.push_back(b.id);
    log_line("BUTIN", "victoire : %zu objet(s) dans le sac ; joueurs du defile : %s", bag.size(),
             mask_names(players).c_str());
}

void loot_victory_after() {
    if (!g.snap) return;
    g.snap = false;
    const uint64_t now = GetTickCount64();
    std::vector<Stored> store;
    std::vector<Item> items;
    if (!read_bucket(kInv_Storage, store)) {
        log_line("BUTIN", "!! victoire : rangement illisible -- pas de defile, tout reste chez toi");
        host_start({}, g.snap_players, g.snap_seq, now);
        return;
    }
    size_t claimed = 0;
    for (uint64_t id : g.snap_ids)
        for (const Stored& s : store)
            if (s.id == id) {
                // Achete en boutique : il rentre chez son acheteur (mm_shop), sans defiler.
                if (shop_claims(id)) {
                    ++claimed;
                    break;
                }
                if (s.f.name.size() > kMaxItemName || s.f.text.size() > kMaxItemText) {
                    log_line("BUTIN", "!! %s : nom ou texte trop long pour le defile -- il reste chez toi",
                             s.f.name.substr(0, 40).c_str());
                    break;
                }
                Item it;
                it.f      = s.f;
                it.id     = id;
                it.digest = item_digest(s.f);
                items.push_back(std::move(it));
                break;
            }
    if (claimed)
        log_line("BUTIN", "%zu objet(s) achete(s) en boutique : ils ne defilent pas (ils sont a leur acheteur)",
                 claimed);
    if (items.size() + claimed < g.snap_ids.size())
        log_line("BUTIN", "%zu objet(s) du sac pas dans le rangement (devenus autre chose en rentrant)",
                 g.snap_ids.size() - items.size() - claimed);
    if (items.size() > kLootMaxItems) {
        log_line("BUTIN", "!! %zu objets : seuls les %zu premiers defilent", items.size(), kLootMaxItems);
        items.resize(kLootMaxItems);
    }
    host_start(std::move(items), g.snap_players, g.snap_seq, now);
}

bool loot_popup(LootPopup& out) {
    if (!g.armed || !g.on || g.summary_closed) return false;
    // Dans une maison seulement (la sienne, ou celle de la copie de l'aventure
    // chez un invite) : pas par-dessus l'ecran de fin ni pendant un chargement.
    if (!mm_house()) return false;
    // Invite : rien avant le premier etat de l'hote (il part juste apres le debut).
    if (!is_host() && !g.have_state) return false;
    const uint64_t now = GetTickCount64();
    const LootView v = make_view(now);
    const LootScreen s = loot_screen(v);
    const uint32_t id = layout_id(s, v.st);
    if (!id || id == g.hidden) return false;
    // Son choix fait : la fenetre « en attente des autres » attend un peu -- le
    // dernier a choisir verrait sinon une fenetre s'ouvrir et se refermer aussitot.
    if (s.layout == kLayChosen && g.chose_index == v.st.index && now - g.chose_at < kChosenShowMs) return false;
    out.layout = id;
    out.text   = s.text;
    out.yes    = s.yes;
    out.no     = s.no;
    return true;
}

void loot_popup_button(uint32_t layout, bool yes) {
    if (!g.on || ((layout >> 24) & 0xFFu) != (g.parade & 0xFFu)) return;
    const uint8_t kind  = (uint8_t)((layout >> 8) & 0xFFu);
    const uint8_t index = (uint8_t)((layout >> 16) & 0xFFu);
    const uint64_t now  = GetTickCount64();
    switch (kind) {
    case kLayWait:
        if (is_host() && yes) {
            if (g.host.start_now(now)) {
                g.dirty = true;
                log_line("BUTIN", "tu commences le defile sans attendre (prets : %s)",
                         mask_names(g.host.ready() & g.host.players()).c_str());
            }
        } else {
            g.hidden = layout;
        }
        break;
    case kLayChoose:
        if (yes) {
            g.step2       = true;
            g.step2_index = index;
        } else {
            make_choice(index, kLootPass);
        }
        break;
    case kLayNeedGreed:
        make_choice(index, yes ? kLootNeed : kLootGreed);
        break;
    case kLayChosen:
    case kLayResult:
        g.hidden = layout;
        break;
    case kLaySummary:
        g.summary_closed = true;
        if (is_host()) host_finish("fini");
        else reset_parade();
        break;
    default:
        break;
    }
}

bool loot_holds_home() {
    if (!g.armed || !is_guest()) return false;
    if (g.expect_until && GetTickCount64() < g.expect_until) return true;
    return g.on && !g.summary_closed;
}

bool loot_host_busy() { return g.armed && is_host() && g.on; }

bool loot_storage_refs() { return is_host() && (g.on || !g.gives.empty() || !g.erases.empty()); }

// --- le rangement, pour les dons aux PNJ (mm_npcgift) -------------------------------------------

bool loot_storage_items(std::vector<LootStoredItem>& out) {
    out.clear();
    std::vector<Stored> store;
    if (!read_bucket(kInv_Storage, store)) return false;
    for (const Stored& s : store) out.push_back(LootStoredItem{ s.id, s.f });
    return true;
}

bool loot_storage_busy() { return rungear_screen_open() || mm_game_prompt_open(); }

bool loot_storage_take(uint64_t id, uint64_t digest, std::string& why) {
    if (loot_storage_busy()) { why = "un ecran des objets ou une confirmation est ouvert"; return false; }
    auto erase = (FnErase)addr_of_call(C_BucketErase);
    uint8_t* inv = (uint8_t*)inventory();
    Stored s;
    uint32_t n = 0;
    if (!inv || !erase) { why = "fonction du jeu ou inventaire introuvable"; return false; }
    if (!stored_item(id, digest, &s, why)) return false;
    if (!seh_erase(erase, inv + kInv_Storage, s.payload, &n)) { why = "exception"; return false; }
    if (n != 1) { why = "le jeu n'a rien retire"; return false; }
    return true;
}

bool loot_bag_items(std::vector<LootStoredItem>& out) {
    out.clear();
    std::vector<Stored> bag;
    if (!read_bucket(kInv_Backpack, bag)) return false;
    for (const Stored& s : bag) out.push_back(LootStoredItem{ s.id, s.f });
    return true;
}

bool loot_read_equipment(const void* eq, ItemFields& f, uint64_t* id) { return read_item(eq, f, id); }

std::string loot_item_label(const ItemFields& f) {
    std::string name;
    // g.names_ok : les fonctions du jeu sont la (loot_pump) et n'ont jamais leve d'exception.
    if (g.told && g.names_ok && item_text(C_ItemName, f, name) && !name.empty()) return name;
    return fallback_name(f);
}

} // namespace mgmp
