// mm_gear -- Mewgenics Multi : le choix des objets au depart ensemble. Voir
// mm_gear.h.

#include "mm_gear.h"
#include "mm_embark.h"
#include "mm_menu.h"
#include "mm_rungear.h"
#include "mm_team.h"
#include "mm_team_merge.h"

#include "mgmp_addresses.h"
#include "mgmp_catsync.h"
#include "mgmp_follow.h"
#include "mgmp_invsync.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"

#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_gear.h -----------------------------------------------

constexpr uintptr_t kItem_Screen   = 0x38;    // InventoryScreen2*
constexpr uintptr_t kItem_Button   = 0x48;    // Button*
constexpr uintptr_t kScr_Selector  = 0x50;    // CatSelector*
constexpr uintptr_t kScr_Mode      = 0x58;    // i32 : 1 le rangement de la maison, 0 le sac
constexpr uintptr_t kScr_BoxCount  = 0xAC;    // u32
constexpr uintptr_t kScr_Current   = 0x108;   // u64, la cle du chat montre
constexpr uintptr_t kSel_Index     = 0x78;    // i32, le rang du chat montre (CatSelector)
constexpr uintptr_t kCat_Id        = 0x00;
constexpr uintptr_t kCat_Class     = 0xC10;   // std::string, la classe (son collier)

constexpr int32_t  kModeStorage    = 1;
constexpr uint64_t kNone           = ~0ull;
constexpr uint32_t kMaxRunKeys     = 16;
constexpr uint64_t kPollMs         = 500;     // invite : une photo toutes les 0,5 s au plus
constexpr uint64_t kGraceMs        = 3000;    // hote, sur la carte : les dernieres photos
constexpr uint32_t kFinalTries     = 10;      // invite : sa derniere photo, si l'envoi echoue
constexpr uint8_t  kGearPeers      = 8;

constexpr char kScene[]   = "StorageItems";
constexpr char kChooser[] = "ClassChooser";

using FnItemUpdate = void (__fastcall*)(void* box);
using FnSelect     = void (__fastcall*)(void* selector, uint64_t key);
FnItemUpdate o_ItemUpdate = nullptr;

// Ce qu'un invite envoie : ses chats tels que sa copie les a, et son rangement.
struct Photo {
    std::vector<uint8_t> storage;
    std::vector<GearCat> cats;
    uint64_t             hash = 0;
};

// Hote : la derniere photo de chaque invite.
struct Peer {
    bool    have  = false;      // une photo avec des donnees
    bool    final = false;      // il ne changera plus rien
    GearMsg msg;
};

struct State {
    GearRole role  = GearRole::Off;
    uint8_t  vote  = 0;
    uint32_t frame = 1;

    // L'ecran des objets.
    void*    screen       = nullptr;   // InventoryScreen2 vu par une boite
    uint32_t screen_frame = 0;
    void*    shown        = nullptr;   // l'ecran ou il a vu d'abord un de ses chats
    bool     chooser_seen = false;     // le choix des colliers, qui vient toujours avant
    bool     seen         = false;     // « StorageItems » ouvert pendant ce depart
    bool     closed       = false;     // ... puis ferme

    // « ce joueur peut-il toucher au chat montre ? », une fois par image
    uint32_t act_frame  = 0;
    void*    act_screen = nullptr;
    uint64_t act_key    = kNone;
    bool     act_ok     = true;

    // invite
    bool     load_pending  = false;    // son rangement est a charger dans la copie
    bool     loaded        = false;    // ... il y est
    bool     frozen        = false;    // derniere photo partie : plus de changement
    bool     final_retry   = false;    // ... son envoi a echoue : encore
    uint32_t final_tries   = 0;
    bool     close_pending = false;    // le « Valider » de l'hote est arrive
    uint32_t snap          = 0;        // numero de la derniere photo envoyee
    uint64_t sent_hash     = 0;
    uint64_t first_hash    = 0;        // la premiere : ses chats tels qu'a l'ouverture
    uint64_t poll_at       = 0;
    bool     close_info    = false;
    bool     load_info     = false;

    // hote
    Peer     peers[kGearPeers];
    bool     applied      = false;     // les photos sont appliquees (ou abandonnees)
    bool     hold_started = false;
    uint64_t hold_from    = 0;
    bool     marker_ok    = false;
    uint32_t marker       = 0;
    bool     close_warned = false;     // « X equipe encore ses chats », une fois

    uint32_t sent = 0, received = 0, applied_n = 0, failed = 0, late = 0, blocked = 0;
};

State g;

// Une fenetre pour ce joueur, des que possible (gardee apres la fin du choix :
// la reponse de l'hote peut arriver ensuite) ; et, pour elle, si sa derniere
// photo de ce vote differait de la premiere (il a change quelque chose).
std::string g_notice;
uint8_t     g_final_vote    = 0;
bool        g_final_changed = false;

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

uint64_t fnv(const void* p, size_t n, uint64_t h) {
    const uint8_t* b = (const uint8_t*)p;
    for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 0x100000001B3ull; }
    return h;
}

bool is_item_button(const char* name) {
    return strncmp(name, "InventoryButton_", 16) == 0 || strncmp(name, "EquippedButton_", 15) == 0;
}

// L'equipe de l'aventure (MewDirector+1472), dans l'ordre du selecteur.
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

bool peer_present(uint8_t peer) {
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (!n || !net_peer_ids(ids, 8)) return false;
    for (uint8_t i = 0; i < n && i < 8; ++i)
        if (ids[i] == peer) return true;
    return false;
}

// Le proprietaire d'un chat de l'equipe : celui de l'equipe mixte, l'hote a
// defaut. -1 : pas de chat a cette cle.
int owner_of_key(uint64_t key, uint64_t* id_out = nullptr) {
    void* cat = catsync_cat_by_key(key);
    uint64_t id = 0;
    if (!cat || !mem_read((const char*)cat + kCat_Id, &id, sizeof(id))) return -1;
    if (id_out) *id_out = id;
    const int o = team_owner_of(id);
    return o < 0 ? (int)kHostPeer : o;
}

// Ce joueur peut-il equiper le chat montre, ou lui retirer ses objets ? Seul
// son proprietaire, et l'invite seulement avec son propre rangement dans la
// copie, avant sa derniere photo. Pas de chat montre : l'hote, comme le jeu.
bool can_act(uint64_t key) {
    if (g.role == GearRole::Guest && (g.frozen || !g.loaded)) return false;
    const int owner = owner_of_key(key);
    if (owner < 0) return g.role == GearRole::Host;
    return g.role == GearRole::Host ? owner == (int)kHostPeer : owner == (int)net_self();
}

bool act_ok(void* screen) {
    uint64_t key = kNone;
    if (!screen || !mem_read((const char*)screen + kScr_Current, &key, sizeof(key)))
        return g.role == GearRole::Host;
    if (g.act_frame == g.frame && g.act_screen == screen && g.act_key == key) return g.act_ok;
    g.act_frame  = g.frame;
    g.act_screen = screen;
    g.act_key    = key;
    g.act_ok     = can_act(key);
    return g.act_ok;
}

bool safe_select(FnSelect f, void* sel, uint64_t key) {
    __try { f(sel, key); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// A l'ouverture, chacun voit d'abord un de ses chats (le premier de l'equipe
// est souvent a un autre) : comme les fleches, le rang du selecteur puis sa
// selection du jeu.
void show_own_cat(void* screen) {
    g.shown = screen;
    void* sel = nullptr;
    uint64_t cur = kNone;
    if (!mem_read((const char*)screen + kScr_Selector, &sel, sizeof(sel)) || !sel ||
        !mem_read((const char*)screen + kScr_Current, &cur, sizeof(cur)))
        return;
    const int self = (int)net_self();
    if (owner_of_key(cur) == self) return;
    std::vector<uint64_t> keys;
    if (!run_keys(keys)) return;
    for (size_t i = 0; i < keys.size(); ++i) {
        if (owner_of_key(keys[i]) != self) continue;
        auto select = (FnSelect)addr_of_call(C_CatSelectorSelect);
        const int32_t idx = (int32_t)i;
        if (!select || !mem_write((char*)sel + kSel_Index, &idx, sizeof(idx))) return;
        if (safe_select(select, sel, keys[i]))
            log_line("OBJETS", "ce joueur voit d'abord son chat %llu", (unsigned long long)keys[i]);
        else
            log_line("OBJETS", "!! la selection de son chat %llu a echoue", (unsigned long long)keys[i]);
        return;
    }
}

// --- invite : les photos -----------------------------------------------------------------

// Ses chats de l'equipe, tels que sa copie les a, et son rangement. Faux si
// quelque chose ne se lit pas.
bool take_photo(Photo& ph) {
    ph = Photo{};
    if (!invsync_serialize_storage(ph.storage) || ph.storage.size() < 4) return false;
    std::vector<uint64_t> keys;
    if (!run_keys(keys)) return false;
    const int self = (int)net_self();
    for (uint64_t key : keys) {
        if (owner_of_key(key) != self) continue;
        GearCat c;
        c.key = key;
        if (!catsync_serialize_cat(catsync_cat_by_key(key), c.data) || c.data.empty()) return false;
        ph.cats.push_back(std::move(c));
    }
    uint64_t h = fnv(ph.storage.data(), ph.storage.size(), 0xCBF29CE484222325ull);
    for (const GearCat& c : ph.cats) {
        h = fnv(&c.key, sizeof(c.key), h);
        h = fnv(c.data.data(), c.data.size(), h);
    }
    ph.hash = h;
    return true;
}

bool send_msg(const GearMsg& m) {
    const bool ok = team_send(team_gear_encode(m), (int)kHostPeer);
    if (ok) ++g.sent;
    return ok;
}

void send_photo(const Photo& ph, bool final, const char* why) {
    GearMsg m;
    m.vote     = g.vote;
    m.snap     = ++g.snap;
    m.final    = final;
    m.has_data = true;
    m.storage  = ph.storage;
    m.cats     = ph.cats;
    const bool ok = send_msg(m);
    if (ok) g.sent_hash = ph.hash;
    if (m.snap == 1) g.first_hash = ph.hash;
    if (final) {
        g_final_vote    = g.vote;
        g_final_changed = ph.hash != g.first_hash;
    }
    log_line("OBJETS", "%s photo %u%s : %zu chat(s), rangement %zu octets (%s)", ok ? "->" : "!! (non envoyee)",
             m.snap, final ? " (derniere)" : "", ph.cats.size(), ph.storage.size(), why);
}

// La derniere : ses chats et son rangement tels qu'il les laisse. Sans rien a
// lui (pas de chat, rangement pas charge), seulement « fini ». Plus aucun
// changement ensuite ; un envoi qui echoue est refait (kFinalTries fois).
void send_final(const char* why) {
    g.frozen      = true;
    g.act_frame   = 0;
    g.final_retry = false;
    ++g.final_tries;
    Photo ph;
    bool ok;
    if (g.loaded && take_photo(ph) && !ph.cats.empty()) {
        const uint32_t before = g.sent;
        send_photo(ph, true, why);
        ok = g.sent != before;
    } else {
        GearMsg m;
        m.vote  = g.vote;
        m.snap  = g.snap;
        m.final = true;
        ok = send_msg(m);
        log_line("OBJETS", "%s fini, rien a appliquer (%s)", ok ? "->" : "!! (non envoye)", why);
    }
    if (!ok && g.final_tries < kFinalTries) {
        g.final_retry = true;
        g.poll_at     = GetTickCount64();
    }
}

// Son rangement a la place de celui de la copie (celui de l'hote), avant
// l'ecran des objets : ses boites sont faites a l'ouverture. Jamais pendant
// qu'un ecran des objets est ouvert (celui de la maison, s'il l'etait encore
// au depart) : le jeu vide le rangement avant de le remplir.
void guest_load() {
    if (mm_scene_alive(kScene)) {
        if (g.chooser_seen && !mm_scene_alive(kChooser)) {
            g.load_pending = false;
            log_line("OBJETS", "!! l'ecran des objets est deja ouvert : rangement de ce joueur pas charge, "
                               "il regarde");
        }
        return;
    }
    g.load_pending = false;
    std::vector<uint8_t> mine;
    if (!team_guest_storage(g.vote, mine)) {
        log_line("OBJETS", "pas de rangement a ce joueur pour ce depart (pas de chat, ou illisible a "
                           "« J'arrive ») : il regarde");
        return;
    }
    g.loaded = invsync_load_storage(mine);
    log_line("OBJETS", g.loaded ? "rangement de ce joueur charge dans la copie (%zu octets)"
                                : "!! rangement de ce joueur pas charge dans la copie (%zu octets) : il ne "
                                  "pourra pas equiper ses chats", mine.size());
}

void guest_pump() {
    if (g.load_pending) guest_load();
    const bool alive = mm_scene_alive(kScene);
    // Celui du depart vient apres le choix des colliers, le rangement charge.
    if (alive && !g.seen && g.chooser_seen && !g.load_pending) {
        g.seen = true;
        log_line("OBJETS", "choix des objets ouvert : %s", g.loaded
            ? "ce joueur equipe ses chats avec son propre rangement"
            : "rangement de ce joueur pas charge, il regarde");
        if (!g.loaded && !g.load_info && team_guest_has_cats_in_run()) {
            g.load_info = true;
            mm_show_info(u8"Choix des objets\n\n"
                         u8"Ton rangement n'a pas pu être chargé ici :\n"
                         u8"tes chats partent avec les objets qu'ils portent.");
        }
    }
    if (!g.seen) return;
    const uint64_t now = GetTickCount64();
    if (g.frozen) {
        if (g.final_retry && now - g.poll_at >= kPollMs) send_final("nouvel essai");
        return;
    }
    if (g.close_pending) { send_final("« Valider » de l'hote"); return; }
    if (!alive)          { send_final("ecran ferme"); return; }
    // Rien a equiper ici (can_act) : « fini » tout de suite, l'hote ne l'attend pas.
    if (!g.loaded)       { send_final("rien a equiper ici"); return; }
    if (g.poll_at && now - g.poll_at < kPollMs) return;
    g.poll_at = now;
    Photo ph;
    if (!take_photo(ph) || ph.cats.empty()) return;
    if (g.snap && ph.hash == g.sent_hash) return;
    send_photo(ph, false, g.snap ? "changement" : "premiere");
}

// --- hote : appliquer -------------------------------------------------------------------

struct Applied {
    void*                cat = nullptr;
    uint64_t             id  = 0;
    char                 cls[64] = {};
    std::vector<uint8_t> backup;
    const GearCat*       in  = nullptr;
};

// La photo d'un invite sur ses chats de l'aventure, tout ou rien : chaque chat
// doit etre dans l'equipe, a lui, le meme (id) et garder sa classe (son
// collier, choisi ensemble) ; au moindre ecart, tous reprennent leur etat
// d'avant.
bool apply_cats(uint8_t from, const std::vector<GearCat>& cats, std::string& why) {
    std::vector<uint64_t> keys;
    if (!run_keys(keys)) { why = "equipe illisible"; return false; }
    std::vector<Applied> items;
    for (const GearCat& c : cats) {
        bool in_run = false, twice = false;
        for (uint64_t k : keys) in_run |= k == c.key;
        for (const Applied& a : items) twice |= a.in->key == c.key;
        if (!in_run || twice) { why = fmt("chat %llu %s", (unsigned long long)c.key,
                                          twice ? "deux fois" : "hors de l'equipe"); return false; }
        Applied a;
        a.in  = &c;
        a.cat = catsync_cat_by_key(c.key);
        CatInfo info;
        if (!a.cat || !mem_read((const char*)a.cat + kCat_Id, &a.id, sizeof(a.id)) ||
            !cat_info(c.data, info) || info.id != a.id) {
            why = fmt("chat %llu : pas le meme chat ici", (unsigned long long)c.key);
            return false;
        }
        if (team_owner_of(a.id) != (int)from) {
            why = fmt("chat %llu : pas a lui", (unsigned long long)c.key);
            return false;
        }
        if (!mem_read_std_string((const char*)a.cat + kCat_Class, a.cls, sizeof(a.cls)) ||
            !catsync_serialize_cat(a.cat, a.backup) || a.backup.empty()) {
            why = fmt("chat %llu illisible ici", (unsigned long long)c.key);
            return false;
        }
        items.push_back(std::move(a));
    }
    size_t i = 0;
    for (; i < items.size(); ++i) {
        const Applied& a = items[i];
        uint64_t id = 0;
        char cls[64] = {};
        if (!catsync_deserialize_cat(a.cat, a.in->data)) {
            why = fmt("chat %llu : le jeu ne l'a pas relu", (unsigned long long)a.in->key);
            break;
        }
        if (!mem_read((const char*)a.cat + kCat_Id, &id, sizeof(id)) || id != a.id ||
            !mem_read_std_string((const char*)a.cat + kCat_Class, cls, sizeof(cls)) || strcmp(cls, a.cls) != 0) {
            why = fmt("chat %llu : classe %s ici, %s chez lui", (unsigned long long)a.in->key, a.cls, cls);
            break;
        }
    }
    if (i == items.size()) return true;
    for (size_t j = 0; j <= i && j < items.size(); ++j)
        if (!catsync_deserialize_cat(items[j].cat, items[j].backup))
            log_line("OBJETS", "!! chat %llu : son etat d'avant n'a pas pu etre remis",
                     (unsigned long long)items[j].in->key);
    return false;
}

bool apply_photo(uint8_t from, const GearMsg& m, std::string& why) {
    return apply_cats(from, m.cats, why);
}

void send_ack(uint8_t peer, uint32_t snap, const std::vector<uint8_t>* storage) {
    GearAck a;
    a.vote        = g.vote;
    a.snap        = snap;
    a.has_storage = storage != nullptr;
    if (storage) a.storage = *storage;
    const bool ok = team_send(team_gear_ack_encode(a), peer);
    log_line("OBJETS", "%s %s : photo %u appliquee", ok ? "->" : "!! (non envoye)", mm_peer_name(peer).c_str(),
             snap);
}

void apply_all(const char* why) {
    g.applied = true;
    unsigned n = 0;
    for (uint8_t p = 1; p < kGearPeers; ++p) {
        Peer& x = g.peers[p];
        if (!x.have) continue;
        const std::string who = mm_peer_name(p);
        // Sans la reponse, son rangement ne changerait pas chez lui : ses objets
        // seraient en double.
        if (!peer_present(p)) {
            log_line("OBJETS", "photo %u de %s pas appliquee : il a quitte la session", x.msg.snap, who.c_str());
            continue;
        }
        std::string err;
        if (apply_photo(p, x.msg, err)) {
            ++g.applied_n;
            ++n;
            log_line("OBJETS", "photo %u de %s appliquee a ses %zu chat(s)%s", x.msg.snap, who.c_str(),
                     x.msg.cats.size(), x.final ? "" : " (pas sa derniere)");
            send_ack(p, x.msg.snap, &x.msg.storage);
        } else {
            ++g.failed;
            log_line("OBJETS", "!! photo %u de %s pas appliquee (%s) : ses chats partent tels qu'au depart",
                     x.msg.snap, who.c_str(), err.c_str());
            send_ack(p, 0, nullptr);
        }
    }
    log_line("OBJETS", "equipement des invites : %u photo(s) appliquee(s) (%s)", n, why);
}

// Plus rien a appliquer (trop tard) : leurs chats partent tels qu'au depart.
void give_up(const char* why) {
    g.applied = true;
    for (uint8_t p = 1; p < kGearPeers; ++p)
        if (g.peers[p].have) {
            ++g.failed;
            send_ack(p, 0, nullptr);
        }
    log_line("OBJETS", "!! photos des invites pas appliquees (%s)", why);
}

void host_pump() {
    const bool alive = mm_scene_alive(kScene);
    if (alive && !g.seen && g.chooser_seen) {
        g.seen = true;
        log_line("OBJETS", "choix des objets ouvert : chacun equipe ses chats avec son rangement");
    }
    if (g.seen && !alive && !g.closed) {
        g.closed = true;
        log_line("OBJETS", "choix des objets ferme chez l'hote : les photos des invites seront appliquees "
                           "sur la carte, avant la publication de l'aventure");
    }
}

// --- le detour --------------------------------------------------------------------------

// `count` : le nombre de boites de l'ecran avant l'update du jeu.
void after_item_update(void* screen, void* button, uint32_t count) {
    if (g.role == GearRole::Off) {
        // Mewgenics Multi : en aventure ensemble (mm_rungear), quel que soit le mode.
        if (rungear_can_touch(screen)) return;
    } else {
        int32_t mode = 0;
        if (!mem_read((const char*)screen + kScr_Mode, &mode, sizeof(mode)) || mode != kModeStorage) return;
        g.screen       = screen;
        g.screen_frame = g.frame;
        if (act_ok(screen)) return;
    }
    // Pas son chat : ses objets grises, comme le jeu grise ceux des autres
    // chats (Button::update n'envoie aucun clic dans cet etat). L'update ne
    // retire jamais que sa propre boite : une de moins, c'est elle -- detruite.
    uint32_t now = 0;
    if (!mem_read((const char*)screen + kScr_BoxCount, &now, sizeof(now)) || now != count) return;
    int32_t state = 0;
    if (!mem_read((const char*)button + kBtn_State, &state, sizeof(state)) || state == kBtnState_Disabled) return;
    const int32_t disabled = kBtnState_Disabled;
    mem_write((char*)button + kBtn_State, &disabled, sizeof(disabled));
}

// Hote, « Valider » : un invite qui suit le depart n'a pas encore envoye sa
// derniere photo (son « Valider ») -- le clic est retenu une fois, avec une
// fenetre ; le suivant passe.
bool host_close_early() {
    if (g.close_warned) return false;
    std::string who;
    unsigned n = 0;
    for (uint8_t p = 1; p < kGearPeers; ++p) {
        if (!embark_follower_active(p) || g.peers[p].final) continue;
        who += (n ? ", " : "") + mm_peer_name(p);
        ++n;
    }
    if (!n) return false;
    g.close_warned = true;
    log_line("OBJETS", "« Valider » de l'hote retenu une fois : %s n'a pas encore valide", who.c_str());
    mm_show_info(fmt(u8"Choix des objets\n\n"
                     u8"%s %s encore %s chats.\n"
                     u8"Clique de nouveau sur « Valider » pour partir\n"
                     u8"quand même.", who.c_str(), n > 1 ? u8"équipent" : u8"équipe", n > 1 ? u8"leurs" : u8"ses"));
    return true;
}

void __fastcall h_ItemUpdate(void* box) {
    // Lus avant : l'update du jeu peut retirer et detruire la boite.
    void* screen = nullptr;
    void* button = nullptr;
    uint32_t count = 0;
    const bool on = (g.role != GearRole::Off || rungear_active()) && box &&
                    mem_read((const char*)box + kItem_Screen, &screen, sizeof(screen)) && screen &&
                    mem_read((const char*)box + kItem_Button, &button, sizeof(button)) &&
                    mem_read((const char*)screen + kScr_BoxCount, &count, sizeof(count));
    o_ItemUpdate(box);
    if (on && button) after_item_update(screen, button, count);
}

} // namespace

// ---------------------------------------------------------------------------------------

void gear_begin(GearRole role, uint8_t vote) {
    const uint32_t frame = g.frame;
    g = State{};
    g.frame = frame;
    g.role  = role;
    g.vote  = vote;
    // Son rangement : a la premiere image (gear_pump), sans ecran des objets ouvert.
    g.load_pending = role == GearRole::Guest;
    log_line("OBJETS", role == GearRole::Host
        ? "choix des objets partage (vote %u) : chacun equipe ses chats, l'hote applique ceux des invites"
        : "choix des objets partage (vote %u) : ce joueur equipe ses chats", (unsigned)vote);
}

void gear_end(const char* why) {
    if (g.role == GearRole::Off) return;
    log_line("OBJETS", "fin du choix des objets (%s) : %u photo(s) envoyee(s), %u recue(s), %u appliquee(s), "
                       "%u refusee(s), %u en retard, %u clic(s) retenu(s)", why ? why : "?", g.sent, g.received,
             g.applied_n, g.failed, g.late, g.blocked);
    const uint32_t frame = g.frame;
    g = State{};
    g.frame = frame;
}

void gear_pump() {
    ++g.frame;
    if (!g_notice.empty() && !mm_info_busy()) {
        mm_show_info(g_notice);
        g_notice.clear();
    }
    if (g.role == GearRole::Off) return;
    if (mm_scene_alive(kChooser)) g.chooser_seen = true;
    if (g.role == GearRole::Guest) guest_pump();
    else                           host_pump();
    // L'ecran vu par ses boites a l'image d'avant, pas encore montre.
    if (g.role != GearRole::Off && g.screen && g.screen != g.shown && g.screen_frame + 1 >= g.frame &&
        mm_scene_alive(kScene))
        show_own_cat(g.screen);
}

bool gear_role_active() { return g.role != GearRole::Off; }

bool gear_on_click(void* button, const char* name) {
    if (g.role == GearRole::Off || !button || !name) return false;
    if (strcmp(name, "CloseButton") == 0) {
        if (g.role == GearRole::Host) return host_close_early();
        // Retenu (l'hote valide pour tous), mais c'est son « fini » : sa derniere
        // photo part tout de suite, et ses objets sont grises.
        const bool now = g.seen && !g.frozen && !g.close_pending;
        log_line("OBJETS", "« Valider » de ce joueur retenu : c'est l'hote qui valide%s",
                 now ? " -- derniere photo" : "");
        if (now) send_final("« Valider » de ce joueur");
        if (now && !g.close_info) {
            g.close_info = true;
            mm_show_info(fmt(u8"Choix des objets\n\n"
                             u8"Prêt ! Tes chats partent avec ces objets :\n"
                             u8"%s validera le départ pour toute l'équipe.",
                             mm_peer_name(kHostPeer).c_str()));
        }
        return true;
    }
    if (!is_item_button(name)) return false;          // fleches, tri : a chacun
    void* screen = g.screen && g.screen_frame + 2 >= g.frame ? g.screen : nullptr;
    if (screen ? act_ok(screen) : g.role == GearRole::Host) return false;
    if (++g.blocked <= 3 || g.blocked % 20 == 0)
        log_line("OBJETS", "clic retenu (%s) : %s", name, g.role == GearRole::Guest && g.frozen
                 ? "l'hote a valide" : "ce chat n'est pas a ce joueur");
    return true;
}

void gear_guest_close() {
    if (g.role != GearRole::Guest || g.frozen || g.close_pending) return;
    g.close_pending = true;
    log_line("OBJETS", "« Valider » de l'hote arrive : derniere photo, puis plus de changement");
}

void gear_guest_flush() {
    if (g.role == GearRole::Guest && g.seen && !g.frozen) send_final("arrive sur la carte");
}

bool gear_hold_publish() {
    if (g.role != GearRole::Host || g.applied) return false;
    bool any = false;
    for (uint8_t p = 1; p < kGearPeers; ++p) any |= g.peers[p].have;
    if (!g.seen && !any) {                 // pas de choix des objets (desert pas debloque)
        g.applied = true;
        return false;
    }
    const uint64_t now = GetTickCount64();
    if (!g.hold_started) {
        g.hold_started = true;
        g.hold_from    = now;
        g.marker_ok    = follow_marker_node(g.marker);
    }
    // Entre dans une case depuis : ses chats y ont joue, une photo d'avant les
    // remettrait en arriere.
    uint32_t m = 0;
    if (g.marker_ok && follow_marker_node(m) && m != g.marker) {
        give_up("l'hote est deja entre dans une case");
        return false;
    }
    // Les dernieres photos de ceux qui suivent, quelques secondes au plus.
    if (g.marker_ok && now - g.hold_from < kGraceMs)
        for (uint8_t p = 1; p < kGearPeers; ++p)
            if (embark_follower_active(p) && !g.peers[p].final) return true;
    apply_all(now - g.hold_from < kGraceMs ? "toutes les photos sont la" : "attente finie");
    return false;
}

void gear_on_team(uint8_t from, const uint8_t* data, size_t size) {
    const uint8_t kind = team_kind(data, size);
    if (kind == kTeamGearAck) {
        // Invite : le rangement de la photo que l'hote a appliquee -- meme apres
        // la fin du choix chez lui (il peut etre deja sur la carte).
        GearAck a;
        if (net_role() != NetRole::Client || from != kHostPeer || !team_gear_ack_decode(data, size, a)) {
            log_line("OBJETS", "!! reponse de l'hote inattendue ou illisible -- ignoree");
            return;
        }
        const bool applied = a.snap && a.has_storage;
        // Encore en train d'equiper ses chats (l'hote n'a pas attendu sa derniere
        // photo) : plus rien ne compte desormais, il le saura.
        const bool mine    = g.role == GearRole::Guest && g.vote == a.vote;
        const bool editing = mine && g.seen && !g.frozen;
        if (mine) {
            g.frozen        = true;
            g.final_retry   = false;
            g.close_pending = false;
            g.act_frame     = 0;
        }
        const std::string host = mm_peer_name(kHostPeer);
        if (!applied) {
            log_line("OBJETS", "<- l'hote n'a pas applique les objets de tes chats (vote %u) : ils partent tels "
                               "qu'au depart, ton rangement ne change pas", (unsigned)a.vote);
            // Rien de change depuis l'ouverture : rien a dire.
            if (!editing && g_final_vote == a.vote && !g_final_changed) return;
            g_notice = fmt(u8"Choix des objets\n\n"
                           u8"Les objets choisis pour tes chats n'ont pas pu\n"
                           u8"être repris dans l'aventure de %s : tes chats\n"
                           u8"partent avec ceux qu'ils avaient, ton rangement\n"
                           u8"ne change pas.", host.c_str());
            return;
        }
        log_line("OBJETS", "<- l'hote a applique la photo %u (vote %u)%s", a.snap, (unsigned)a.vote,
                 editing ? " -- ce joueur equipait encore : la suite ne compte plus" : "");
        team_guest_storage_after(a.vote, a.storage);
        if (editing)
            g_notice = fmt(u8"Choix des objets\n\n"
                           u8"%s a déjà validé le départ : tes chats partent\n"
                           u8"avec les objets choisis jusqu'ici, la suite\n"
                           u8"ne compte plus.", host.c_str());
        return;
    }
    GearMsg m;
    if (net_role() != NetRole::Host || from == kHostPeer || from >= kGearPeers ||
        !team_gear_decode(data, size, m)) {
        log_line("OBJETS", "!! photo inattendue ou illisible de %u (%zu octets) -- ignoree", (unsigned)from, size);
        return;
    }
    const std::string who = mm_peer_name(from);
    if (g.role != GearRole::Host || m.vote != g.vote) {
        log_line("OBJETS", "photo %u de %s pour un autre depart (vote %u) -- ignoree", m.snap, who.c_str(),
                 (unsigned)m.vote);
        return;
    }
    ++g.received;
    if (g.applied) {
        ++g.late;
        log_line("OBJETS", "!! photo %u de %s arrivee apres l'application -- ignoree", m.snap, who.c_str());
        return;
    }
    Peer& x = g.peers[from];
    if (m.final) x.final = true;
    if (m.has_data && (!x.have || m.snap > x.msg.snap)) {
        log_line("OBJETS", "<- photo %u de %s%s : %zu chat(s)", m.snap, who.c_str(), m.final ? " (derniere)" : "",
                 m.cats.size());
        x.msg  = std::move(m);
        x.have = true;
    } else if (m.final) {
        log_line("OBJETS", "<- %s a fini (rien de neuf)", who.c_str());
    }
}

bool gear_binding(Target t, void** detour, void*** original) {
    if (t != T_InvItemUpdate) return false;
    *detour   = (void*)&h_ItemUpdate;
    *original = (void**)&o_ItemUpdate;
    return true;
}

} // namespace mgmp
