// mm_gift -- Mewgenics Multi : les dons entre joueurs (protocole 62). Voir mm_gift.h.

#include "mm_gift.h"
#include "mm_gift_core.h"
#include "mm_loot.h"
#include "mm_menu.h"
#include "mm_npcgift.h"
#include "mm_purse.h"
#include "mm_purse_core.h"
#include "mm_shop.h"
#include "mm_team.h"
#include "mm_team_merge.h"

#include "mgmp_addresses.h"
#include "mgmp_follow.h"
#include "mgmp_invsync.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"

#include <windows.h>
#include <algorithm>
#include <climits>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_gift.h ----------------------------------------------------

constexpr uintptr_t kDir_House   = 0x5A0;   // MewDirector : la maison (House*)
constexpr uintptr_t kHouse_Food  = 0xB0;    // int32
constexpr uintptr_t kHouse_Coins = 0xB4;    // int32

using FnDirSave = void (__fastcall*)(void* director);

// --- les delais ------------------------------------------------------------------------------

constexpr uint64_t kAnswerMs        = kGiftWaitMs + 15000;  // donneur : la reponse, au plus
constexpr uint64_t kLateMs          = 300000;  // ... une reponse en retard compte encore
constexpr uint64_t kHomeGraceMs     = 2000;    // celui qui recoit : hors de sa maison, si ca dure
constexpr uint64_t kPresenceBeatMs  = 10000;   // presence : redite meme sans changement
constexpr uint64_t kPresenceStaleMs = 30000;   // invite : le masque de l'hote, plus vieux, ne compte plus
constexpr uint64_t kViewGapMs       = 400;     // l'etat de la fenetre, au plus souvent
constexpr uint64_t kToastMs         = 8000;    // une nouvelle reste a l'ecran
constexpr size_t   kMaxIncoming     = 8;
constexpr size_t   kMaxLate         = 4;       // dons sans reponse a temps, encore attendus
constexpr size_t   kDoneRing        = 32;
constexpr size_t   kMaxToasts       = 4;
constexpr size_t   kMaxRemovals     = 32;

// MM_GIFT, champ a (la presence).
enum : uint8_t {
    kPresenceMine = 1,   // joueur -> hote : c = 1 il peut recevoir, 0 non
    kPresenceAll  = 2,   // hote -> tous : x = les numeros qui le peuvent
};

// --- l'etat (fil du jeu) ---------------------------------------------------------------------

struct OutItem {
    uint64_t    id     = 0;      // dans le rangement, a l'envoi
    uint64_t    digest = 0;
    ItemFields  f;
    std::string label;
};

// Un don de ce joueur, a la maison, jusqu'a la reponse.
struct Outgoing {
    bool        on      = false;
    uint32_t    id      = 0;
    uint8_t     target  = 0;
    std::string name;            // a qui (UTF-8)
    std::string save;            // la save d'ou il part (son nom)
    int32_t     coins   = 0;
    int32_t     food    = 0;
    std::vector<OutItem> items;
    uint64_t    sent_at = 0;
    bool        late    = false; // pas de reponse a temps : dit, une reponse compte encore
};

// Un don garde chez l'autre : il quitte la maison du donneur -- dans la save d'ou
// il est parti seulement, quand elle est chargee (l'or et la nourriture tout de
// suite, les objets une fois l'ecran des objets ferme). Sans limite de temps : il
// attend le retour du donneur dans cette save.
struct Removal {
    uint32_t    id = 0;
    std::string name;            // qui l'a recu (le nom qu'il donne)
    std::string save;
    int32_t     coins = 0;
    int32_t     food  = 0;
    std::vector<OutItem> items;
    std::string list;            // ce qui a ete donne (UTF-8)
    bool        late       = false;
    bool        money_done = false;
    bool        told_wait  = false;   // « les objets attendent » dit
    size_t      kept = 0;
};

// Un don pour ce joueur, en attente.
struct Incoming {
    GiftMsg  m;
    uint64_t at = 0;
};

struct Toast {
    uint32_t    id     = 0;
    std::string text;
    bool        result = false;
    uint64_t    at     = 0;
};

struct Request {
    TradeMode mode  = TradeMode::None;
    uint8_t   to    = 0;
    int64_t   coins = 0;
    int64_t   food  = 0;
    std::vector<uint64_t> items;
};

struct State {
    bool     told    = false;
    bool     save_ok = false;    // MewDirector::save resolu
    uint32_t next_id = 0;

    Outgoing out;                // le don en cours
    // Ceux restes sans reponse a temps (kMaxLate au plus) : une reponse en retard
    // trouve encore le sien, meme apres un autre don reste sans reponse.
    std::vector<Outgoing> late;
    std::vector<Removal> removals;
    bool     save_due = false;   // la maison a change : sauver des que possible

    std::vector<Incoming> in;
    uint64_t done[kDoneRing] = {};   // dons deja gardes ici : bit 63 | donneur << 32 | numero
    size_t   done_next = 0;

    // La presence.
    int      sent_can     = -1;  // invite : ce qu'il a dit a l'hote (-1 : rien encore)
    uint64_t sent_can_at  = 0;
    uint32_t guest_can    = 0;   // hote : les invites qui peuvent recevoir
    uint32_t mask_sent    = 0;   // hote : le dernier masque envoye
    bool     mask_said    = false;
    uint64_t mask_sent_at = 0;
    uint32_t host_mask    = 0;   // invite : le masque recu de l'hote
    uint64_t host_mask_at = 0;
    bool     have_mask    = false;
    bool     was_session  = false;

    // La fenetre.
    uint64_t    view_at = 0;
    std::map<uint64_t, std::string> labels;   // empreinte -> nom affiche
    std::string status;
    uint32_t    status_id = 0;
    uint32_t    kept_count  = 0;              // les nouvelles `kept` (TradeView::kept_count)
    std::vector<Toast> toasts;
    uint32_t    toast_id = 0;
    std::vector<std::string> popups;          // fenetres du jeu a ouvrir, dans sa maison
    bool        said_house = false;           // deux maisons differentes : dit une fois
};
State g;

// --- partage avec la fenetre (son fil) : un verrou ------------------------------------------

SRWLOCK g_lock = SRWLOCK_INIT;
struct Lock {
    Lock() { AcquireSRWLockExclusive(&g_lock); }
    ~Lock() { ReleaseSRWLockExclusive(&g_lock); }
    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;
};
TradeView g_view;                // le dernier etat publie
uint32_t  g_serial  = 0;
bool      g_window  = false;
bool      g_has_req = false;
Request   g_req;

// --- petits outils ---------------------------------------------------------------------------

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

bool present(uint8_t id) { return id < 32 && (roster_mask() & (1u << id)) != 0; }

void* director() {
    const void** slot = (const void**)addr_of_data(D_MewDirectorPtr);
    void* d = nullptr;
    if (!slot || !mem_read(slot, &d, sizeof(d))) return nullptr;
    return d;
}

// La maison : *(MewDirector+0x5A0), qui est aussi la maison vivante (D_HouseInstance,
// pose a sa creation, remis a 0 par son destructeur). Les deux, ou rien.
void* house() {
    void* d = director();
    void* h = nullptr;
    if (!d || !mem_read((const char*)d + kDir_House, &h, sizeof(h)) || !h) return nullptr;
    const void* const* slot = (const void* const*)addr_of_data(D_HouseInstance);
    void* live = nullptr;
    if (!slot || !mem_read(slot, &live, sizeof(live)) || live != h) {
        if (!g.said_house) {
            g.said_house = true;
            log_line("DON", "!! la maison du Director (%p) n'est pas la maison vivante (%p) -- pas de don a la "
                            "maison tant que c'est le cas", h, live);
        }
        return nullptr;
    }
    return h;
}

bool read_money(void* h, int32_t& coins, int32_t& food) {
    return h && mem_read((const char*)h + kHouse_Coins, &coins, sizeof(coins)) &&
           mem_read((const char*)h + kHouse_Food, &food, sizeof(food));
}

// Les deux, ou aucun : la seconde ecriture ratee remet la premiere.
bool write_money(void* h, int32_t coins, int32_t food) {
    int32_t old = 0;
    if (!h || !mem_read((const char*)h + kHouse_Coins, &old, sizeof(old))) return false;
    if (!mem_write((char*)h + kHouse_Coins, &coins, sizeof(coins))) return false;
    if (!mem_write((char*)h + kHouse_Food, &food, sizeof(food))) {
        mem_write((char*)h + kHouse_Coins, &old, sizeof(old));
        return false;
    }
    return true;
}

// La save chargee par ce joueur (son nom ; "" : le mod ne la connait pas).
std::string save_id() {
    char b[64] = {};
    if (savefile_own_save_name(b, sizeof(b)) || savefile_host_save_name(b, sizeof(b))) return b;
    return "";
}

// Ce joueur est dans SA maison (pas la copie de l'hote), sans depart ni defense en
// cours : il peut donner et recevoir. `session` : en session aussi. Sinon `why`
// (pour la fenetre, UTF-8).
bool at_home(bool session, std::string* why) {
    auto no = [&](const char* w) {
        if (why) *why = w;
        return false;
    };
    if (session && !in_session()) return no(u8"pas de partie à plusieurs en cours");
    if (!g.save_ok) return no(u8"une fonction du jeu manque, les échanges à la maison sont coupés");
    if (savefile_in_coop()) return no(u8"tu es dans la partie de l'hôte");
    if (!mm_house() || !mm_in_house()) return no(u8"tu n'es pas dans ta maison");
    if (net_role() == NetRole::Client && !savefile_has_own_slot()) return no(u8"ta save n'est pas chargée");
    if (mm_departure_busy()) return no(u8"un départ en aventure est en cours");
    if (team_house_defense_running()) return no(u8"ta maison est attaquée");
    if (!house()) return no(u8"ta maison est illisible");
    return true;
}

// Le donneur est de retour dans la save d'ou son don est parti, dans sa maison :
// ce qu'il a donne peut la quitter.
bool giver_here(const std::string& save) {
    return !save.empty() && !savefile_in_coop() && mm_house() && mm_in_house() && house() &&
           _stricmp(save_id().c_str(), save.c_str()) == 0;
}

// Un ecran des objets ou une fenetre du jeu ouverte : rien ne se range ni ne se retire.
bool screens_busy() { return loot_storage_busy(); }

// Hote : le defile, les achats ou les dons aux PNJ tiennent des objets de son
// rangement par leur id -- le relire les changerait, et ces objets sont promis.
bool storage_held() {
    return net_role() != NetRole::Client && (loot_storage_refs() || npcgift_storage_refs() || shop_storage_refs());
}

// Son rangement attend ce que ses chats rapportent d'une aventure de l'hote : ils
// y sont encore (il en est parti avant la fin), ou ils sont rentres mais ne sont pas
// encore recopies dans sa save (au prochain chargement, team_guest_storage_pending).
// mm_team ne le remet a jour que s'il n'a pas change depuis le depart.
bool cats_away() {
    return (net_role() == NetRole::Client && team_guest_has_cats_in_run()) || team_guest_storage_pending();
}

// Les objets de son rangement ne peuvent pas changer de maison maintenant ;
// `why` (UTF-8).
bool items_blocked(std::string* why) {
    if (cats_away()) {
        if (why)
            *why = u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont "
                   u8"rentrés, recharge ta save depuis le menu)";
        return true;
    }
    if (storage_held()) {
        if (why) *why = u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)";
        return true;
    }
    return false;
}

// Un objet que le message couperait (son nom ou son texte trop longs) : ce ne
// serait plus le meme objet chez l'autre -- il ne se donne pas.
bool item_fits(const ItemFields& f) { return f.name.size() <= kGiftMaxName && f.text.size() <= kGiftMaxText; }

// Les objets du rangement promis par ses dons pas encore finis (le don en cours,
// ceux restes sans reponse a temps, ceux qui attendent d'etre retires) : ils ne se
// redonnent pas. Par leur id, sinon par leur empreinte (le rangement relu entre-temps).
std::vector<bool> promised_mask(const std::vector<LootStoredItem>& store) {
    std::vector<const OutItem*> promised;
    auto add = [&](const std::vector<OutItem>& v) {
        for (const OutItem& o : v) promised.push_back(&o);
    };
    if (g.out.on) add(g.out.items);
    for (const Outgoing& o : g.late) add(o.items);
    for (const Removal& r : g.removals) add(r.items);
    std::vector<bool> mask(store.size(), false);
    if (promised.empty()) return mask;
    std::vector<uint64_t> dig(store.size());
    for (size_t j = 0; j < store.size(); ++j) dig[j] = item_digest(store[j].f);
    std::vector<bool> found(promised.size(), false);
    for (size_t p = 0; p < promised.size(); ++p)
        for (size_t j = 0; j < store.size() && !found[p]; ++j)
            if (!mask[j] && store[j].id == promised[p]->id && dig[j] == promised[p]->digest) {
                mask[j]  = true;
                found[p] = true;
            }
    for (size_t p = 0; p < promised.size(); ++p)
        for (size_t j = 0; j < store.size() && !found[p]; ++j)
            if (!mask[j] && dig[j] == promised[p]->digest) {
                mask[j]  = true;
                found[p] = true;
            }
    return mask;
}

bool seh_save(FnDirSave fn, void* dir) {
    __try {
        fn(dir);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Sauver la partie de ce joueur (MewDirector::save), dans sa maison seulement.
bool save_now(const char* what) {
    const FnDirSave fn = (FnDirSave)addr_of_call(C_DirectorSave);
    void* d = director();
    const char* why = nullptr;
    if (!fn || !d) why = "fonction du jeu introuvable";
    else if (!at_home(false, nullptr)) why = "pas dans sa maison";
    else if (screens_busy()) why = "un ecran est ouvert";
    if (why) {
        log_line("DON", "%s : pas de sauvegarde maintenant (%s) -- plus tard", what, why);
        return false;
    }
    if (!seh_save(fn, d)) {
        log_line("DON", "!! %s : la sauvegarde a leve une exception", what);
        return false;
    }
    log_line("DON", "%s : partie sauvee", what);
    return true;
}

std::string name_of(uint8_t id) { return mm_peer_name(id); }

// Le nom d'un objet tel que le jeu de ce joueur l'affiche (appeler le jeu coute :
// garde par empreinte).
std::string label_of(const ItemFields& f) {
    const uint64_t d = item_digest(f);
    const auto it = g.labels.find(d);
    if (it != g.labels.end()) return it->second;
    std::string l = loot_item_label(f);
    if (l.empty()) l = f.name;
    if (g.labels.size() >= 4096) g.labels.clear();
    g.labels.emplace(d, l);
    return l;
}

// Une nouvelle : pour la fenetre (`status` ; `kept` : son don est passe) et/ou une
// fenetre du jeu (mm_trade_ui).
void note(const std::string& text, bool status, bool toast, bool result, bool kept = false) {
    if (status) {
        g.status      = text;
        ++g.status_id;
        if (kept) ++g.kept_count;
    }
    if (toast) {
        Toast t;
        t.id     = ++g.toast_id;
        t.text   = text;
        t.result = result;
        t.at     = GetTickCount64();
        g.toasts.push_back(std::move(t));
        if (g.toasts.size() > kMaxToasts) g.toasts.erase(g.toasts.begin());
    }
    g.view_at = 0;               // publiee a la fin de cette image
}

uint64_t done_key(uint8_t giver, uint32_t id) { return (1ull << 63) | ((uint64_t)giver << 32) | id; }

bool was_done(uint8_t giver, uint32_t id) {
    const uint64_t k = done_key(giver, id);
    for (uint64_t d : g.done)
        if (d == k) return true;
    return false;
}

void remember_done(uint8_t giver, uint32_t id) {
    g.done[g.done_next] = done_key(giver, id);
    g.done_next = (g.done_next + 1) % kDoneRing;
}

// Peut recevoir maintenant (la presence) ; sans nouvelles fraiches, on essaie.
bool peer_can(uint8_t id) {
    if (id >= 32) return false;
    if (net_role() == NetRole::Client) {
        if (!g.have_mask || GetTickCount64() - g.host_mask_at > kPresenceStaleMs) return true;
        return ((g.host_mask >> id) & 1u) != 0;
    }
    return ((g.guest_can >> id) & 1u) != 0;
}

// Une reponse a un don : invite -> l'hote (qui relaie), hote -> le donneur.
void reply(const GiftMsg& offer, uint8_t op, uint8_t why) {
    GiftMsg r;
    r.op     = op;
    r.id     = offer.id;
    r.giver  = offer.giver;
    r.target = offer.target;
    r.why    = why;
    if (op == kGiftGot) {
        const char* me  = mm_local_name();
        r.receiver_name = me ? me : "";
    }
    const int dest = net_role() == NetRole::Client ? (int)kHostPeer : (int)offer.giver;
    if (!team_send(gift_encode(r), dest))
        log_line("DON", "!! reponse au don %u impossible (connexion)", offer.id);
}

// --- celui qui recoit ------------------------------------------------------------------------

// Les objets ajoutes sont bien dans le rangement relu (par empreinte : le jeu leur
// donne de nouveaux ids).
bool storage_has(const std::vector<uint8_t>& before, const std::vector<ItemFields>& add) {
    std::vector<uint8_t>    now_blob;
    std::vector<ItemFields> was, now;
    if (!invsync_serialize_storage(now_blob) || !gift_storage_items(before, was) || !gift_storage_items(now_blob, now))
        return false;
    std::map<uint64_t, size_t> need;
    for (const ItemFields& f : add) ++need[item_digest(f)];
    for (const auto& kv : need) {
        size_t had = 0, have = 0;
        for (const ItemFields& f : was) had += item_digest(f) == kv.first;
        for (const ItemFields& f : now) have += item_digest(f) == kv.first;
        if (have < had + kv.second) return false;
    }
    return true;
}

void restore_storage(const std::vector<uint8_t>& before) {
    if (invsync_load_storage(before)) log_line("DON", "rangement remis comme avant");
    else log_line("DON", "!! le rangement n'a pas pu etre remis comme avant");
}

// Le don dans sa maison : les objets au rangement (marques « nouveaux »), l'or et la
// nourriture a la maison, puis sauver. Faux (et pourquoi) : rien n'est garde.
bool apply_offer(const GiftMsg& m, uint8_t& why, bool& saved) {
    why   = kGiftWhyFailed;
    saved = false;
    void* h = house();
    int32_t hc = 0, hf = 0;
    if (!read_money(h, hc, hf)) {
        log_line("DON", "!! don %u : maison illisible", m.id);
        return false;
    }
    std::vector<uint8_t> before;
    if (!m.items.empty()) {
        if (!invsync_serialize_storage(before)) {
            log_line("DON", "!! don %u : rangement illisible", m.id);
            return false;
        }
        std::vector<uint8_t>    blob = before;
        std::vector<ItemFields> add;
        for (const GiftItem& it : m.items) {
            ItemFields f = it.f;
            f.c |= kItemSeenBits;            // « nouvel objet », comme un objet qui vient d'etre cree
            add.push_back(std::move(f));
        }
        std::string err;
        if (!gift_storage_append(blob, add, &err)) {
            log_line("DON", "!! don %u : %s", m.id, err.c_str());
            return false;
        }
        if (!invsync_load_storage(blob)) {
            log_line("DON", "!! don %u : le jeu n'a pas relu le rangement", m.id);
            restore_storage(before);
            return false;
        }
        if (!storage_has(before, add)) {
            log_line("DON", "!! don %u : les objets ne sont pas dans le rangement relu", m.id);
            restore_storage(before);
            return false;
        }
    }
    if (m.coins || m.food) {
        const int32_t nc = (int32_t)std::min<int64_t>(INT32_MAX, (int64_t)hc + m.coins);
        const int32_t nf = (int32_t)std::min<int64_t>(INT32_MAX, (int64_t)hf + m.food);
        if (!write_money(h, nc, nf)) {
            log_line("DON", "!! don %u : or et nourriture de la maison : ecriture impossible", m.id);
            if (!m.items.empty()) restore_storage(before);
            return false;
        }
        log_line("DON", "maison : or %d -> %d, nourriture %d -> %d (don %u)", hc, nc, hf, nf, m.id);
    }
    saved = save_now("don recu");
    return true;
}

void incoming_pump() {
    if (g.in.empty()) return;
    const uint64_t now  = GetTickCount64();
    const bool     home = at_home(true, nullptr);
    for (size_t i = 0; i < g.in.size();) {
        const GiftMsg m = g.in[i].m;
        const uint64_t waited = now - g.in[i].at;
        const std::string giver = name_of(m.giver).empty() ? m.giver_name : name_of(m.giver);
        // Le donneur est parti : il ne retirerait rien de chez lui -- pas de don.
        if (!present(m.giver)) {
            log_line("DON", "don %u de %s : il n'est plus la -- oublie", m.id, giver.c_str());
            g.in.erase(g.in.begin() + (ptrdiff_t)i);
            continue;
        }
        const bool grace = !home && waited < kHomeGraceMs;
        const bool held  = screens_busy() || (!m.items.empty() && storage_held());
        uint8_t why = 0;
        GiftStep step = gift_receiver_step(home || grace, held || grace, waited, why);
        // Des objets, et ses chats encore dans l'aventure de l'hote : refuse tout de suite.
        if (step != GiftStep::Refuse && !grace && !m.items.empty() && cats_away()) {
            step = GiftStep::Refuse;
            why  = kGiftWhyRun;
        }
        if (step == GiftStep::Wait) {
            ++i;
            continue;
        }
        bool saved = false;
        if (step == GiftStep::Apply && !apply_offer(m, why, saved)) step = GiftStep::Refuse;
        if (step == GiftStep::Refuse) {
            log_line("DON", "don %u de %s refuse (raison %u)", m.id, giver.c_str(), (unsigned)why);
            reply(m, kGiftRefused, why);
            g.in.erase(g.in.begin() + (ptrdiff_t)i);
            continue;
        }
        remember_done(m.giver, m.id);
        reply(m, kGiftGot, 0);
        std::vector<std::string> labels;
        for (const GiftItem& it : m.items) labels.push_back(label_of(it.f));
        const std::string list = gift_list_text(m.coins, m.food, labels);
        log_line("DON", "<- don %u de %s garde : %d piece(s), %d nourriture(s), %zu objet(s)%s", m.id, giver.c_str(),
                 m.coins, m.food, m.items.size(), saved ? " (partie sauvee)" : "");
        if (g.popups.size() >= kMaxIncoming) g.popups.erase(g.popups.begin());   // elles attendent une pause
        g.popups.push_back(gift_received_text(giver, list, saved));
        g.in.erase(g.in.begin() + (ptrdiff_t)i);
    }
}

void on_offer(uint8_t from, const GiftMsg& m, const uint8_t* raw, size_t n) {
    if (net_role() == NetRole::Host) {
        // Un invite ne donne qu'en son nom.
        if (m.giver != from) {
            log_line("DON", "!! don au nom du joueur %u venu du joueur %u -- ignore", (unsigned)m.giver, (unsigned)from);
            return;
        }
        // Pas pour l'hote : relaye.
        if (m.target != net_self()) {
            if (!present(m.target) || !team_send(std::vector<uint8_t>(raw, raw + n), m.target)) {
                log_line("DON", "don %u de %s : %u n'est plus la", m.id, name_of(from).c_str(), (unsigned)m.target);
                reply(m, kGiftRefused, kGiftWhyGone);
            } else {
                log_line("DON", "don %u de %s relaye a %s", m.id, name_of(from).c_str(), name_of(m.target).c_str());
            }
            return;
        }
    } else if (from != kHostPeer || m.target != net_self()) {
        log_line("DON", "!! don %u pour le joueur %u recu ici -- ignore", m.id, (unsigned)m.target);
        return;
    }
    // Deja garde (le meme don une seconde fois) : la reponse repart, rien de plus.
    if (was_done(m.giver, m.id)) {
        reply(m, kGiftGot, 0);
        return;
    }
    for (const Incoming& in : g.in)
        if (in.m.giver == m.giver && in.m.id == m.id) return;
    if (!g.save_ok) {
        reply(m, kGiftRefused, kGiftWhyOff);
        return;
    }
    if (g.in.size() >= kMaxIncoming) {
        reply(m, kGiftRefused, kGiftWhyBusy);
        return;
    }
    Incoming in;
    in.m  = m;
    in.at = GetTickCount64();
    g.in.push_back(std::move(in));
    log_line("DON", "<- don %u de %s : %d piece(s), %d nourriture(s), %zu objet(s)", m.id, name_of(m.giver).c_str(),
             m.coins, m.food, m.items.size());
}

// --- le donneur ------------------------------------------------------------------------------

void house_request(const Request& r) {
    // `ui` pour la fenetre, `log` pour le journal (sans accents).
    auto fail = [&](const std::string& ui, const char* log) {
        note(ui, true, false, true);
        log_line("DON", "pas de don : %s", log);
    };
    std::string why;
    if (!at_home(true, &why)) return fail(u8"Pas d'échange maintenant : " + why + ".", "pas dans sa maison");
    if (g.out.on) return fail(u8"Ton don précédent attend encore sa réponse.", "un don attend sa reponse");
    const std::string save = save_id();
    if (save.empty()) return fail(u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.", "save inconnue");
    const uint8_t self = net_self();
    if (r.to == self || !present(r.to)) return fail(u8"Choisis un joueur de la partie.", "destinataire absent");
    const std::string name = name_of(r.to);
    if (!peer_can(r.to))
        return fail(u8"Pas d'échange maintenant : " + purse_safe(name) + u8" n'est pas dans sa maison.",
                    "destinataire pas dans sa maison");
    if (r.coins < 0 || r.food < 0 || r.coins > kGiftMaxMoney || r.food > kGiftMaxMoney)
        return fail(u8"Ce montant n'est pas possible.", "montant hors bornes");
    if (!r.coins && !r.food && r.items.empty())
        return fail(u8"Choisis des pièces, de la nourriture ou des objets à donner.", "rien a donner");
    if (r.items.size() > kGiftMaxItems) return fail(u8"16 objets au plus par don.", "trop d'objets");
    if (!r.items.empty() && items_blocked(&why))
        return fail(u8"Pas d'objets maintenant : " + why + ".", "objets bloques");
    int32_t hc = 0, hf = 0;
    if (!read_money(house(), hc, hf)) return fail(u8"Ta maison est illisible : pas de don.", "maison illisible");
    if (r.coins > hc) return fail(u8"Tu n'as pas autant de pièces.", "pas assez de pieces");
    if (r.food > hf) return fail(u8"Tu n'as pas autant de nourriture.", "pas assez de nourriture");
    std::vector<LootStoredItem> store;
    if (!r.items.empty() && !loot_storage_items(store))
        return fail(u8"Ton rangement est illisible : pas de don.", "rangement illisible");
    const std::vector<bool> promised = promised_mask(store);
    std::vector<OutItem> items;
    for (uint64_t id : r.items) {
        const LootStoredItem* s = nullptr;
        for (size_t j = 0; j < store.size(); ++j)
            if (store[j].id == id) {
                s = &store[j];
                if (promised[j])
                    return fail(u8"Un objet choisi est déjà promis à un don pas encore fini.", "objet deja promis");
                break;
            }
        bool twice = false;
        for (const OutItem& o : items) twice = twice || o.id == id;
        if (!s || twice)
            return fail(u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.", "objet disparu");
        if (!item_fits(s->f)) return fail(u8"Un objet choisi ne peut pas se donner.", "objet trop long pour le message");
        OutItem o;
        o.id     = s->id;
        o.digest = item_digest(s->f);
        o.f      = s->f;
        o.label  = label_of(s->f);
        items.push_back(std::move(o));
    }
    GiftMsg m;
    m.op     = kGiftOffer;
    m.id     = ++g.next_id;
    m.giver  = self;
    m.target = r.to;
    m.coins  = (int32_t)r.coins;
    m.food   = (int32_t)r.food;
    std::vector<std::string> labels;
    for (const OutItem& o : items) {
        GiftItem gi;
        gi.f     = o.f;
        gi.label = o.label;
        m.items.push_back(std::move(gi));
        labels.push_back(o.label);
    }
    const char* me = mm_local_name();
    m.giver_name = me ? me : "";
    const int dest = net_role() == NetRole::Client ? (int)kHostPeer : (int)r.to;
    if (!team_send(gift_encode(m), dest)) return fail(u8"Le don n'a pas pu partir (connexion).", "envoi impossible");
    Outgoing o;
    o.on      = true;
    o.id      = m.id;
    o.target  = r.to;
    o.name    = name;
    o.save    = save;
    o.coins   = m.coins;
    o.food    = m.food;
    o.items   = std::move(items);
    o.sent_at = GetTickCount64();
    g.out     = std::move(o);
    const std::string list = gift_list_text(m.coins, m.food, labels);
    note(u8"Don envoyé à " + purse_safe(name) + u8" (" + list + u8") : en attente de sa réponse…", true, false, true);
    log_line("DON", "-> don %u a %s : %d piece(s), %d nourriture(s), %zu objet(s)", m.id, name.c_str(), m.coins,
             m.food, m.items.size());
}

void run_request(const Request& r) {
    std::string why;
    if (purse_give(r.to, r.coins, r.food, why)) {
        // L'hote : fait (purse_give l'a dit) ; un invite attend la reponse de l'hote.
        if (net_role() == NetRole::Client)
            note(u8"Don demandé à l'hôte : en attente de sa réponse…", true, false, true);
        return;
    }
    note(u8"Pas de don : " + why + ".", true, false, true);
    log_line("DON", "pas de don de bourse (refuse avant l'envoi)");
}

// Ce qui a ete donne quitte la maison du donneur : dans la save d'ou le don est
// parti, l'or et la nourriture tout de suite (pas sous 0 : il a pu en depenser),
// les objets une fois l'ecran des objets et les fenetres du jeu fermes (par leur
// id, ou par leur empreinte si le rangement a ete relu entre-temps). Une autre save
// chargee, ou ailleurs que dans sa maison : plus tard.
void removals_pump() {
    for (size_t i = 0; i < g.removals.size();) {
        Removal& r = g.removals[i];
        if (!giver_here(r.save)) {
            ++i;
            continue;
        }
        if (!r.money_done) {
            r.money_done = true;
            if (r.coins || r.food) {
                void* h = house();
                int32_t hc = 0, hf = 0;
                if (read_money(h, hc, hf)) {
                    const int32_t nc = (int32_t)std::max<int64_t>(0, (int64_t)hc - r.coins);
                    const int32_t nf = (int32_t)std::max<int64_t>(0, (int64_t)hf - r.food);
                    if (write_money(h, nc, nf)) {
                        g.save_due = true;
                        log_line("DON", "maison : or %d -> %d, nourriture %d -> %d (don %u a %s)", hc, nc, hf, nf,
                                 r.id, r.name.c_str());
                    } else {
                        log_line("DON", "!! don %u : or et nourriture de la maison : ecriture impossible -- ils "
                                        "restent chez toi", r.id);
                    }
                } else {
                    log_line("DON", "!! don %u : maison illisible -- l'or et la nourriture restent chez toi", r.id);
                }
            }
        }
        if (!r.items.empty()) {
            const bool screen = screens_busy();
            if (screen || mm_departure_busy() || team_house_defense_running() || items_blocked(nullptr)) {
                if (!r.told_wait) {
                    r.told_wait = true;
                    note(purse_safe(r.name) +
                             (screen ? u8" a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran "
                                       u8"des objets sera fermé."
                                     : u8" a bien reçu ton don : tes objets quitteront ton rangement dès que possible "
                                       u8"(après le départ, la défense de ta maison ou le retour de tes chats)."),
                         true, false, true, true);
                }
                ++i;
                continue;
            }
            std::vector<LootStoredItem> store;
            const bool        ok = loot_storage_items(store);
            std::vector<bool> used(store.size(), false);
            for (const OutItem& it : r.items) {
                int pick = -1;
                for (size_t j = 0; ok && pick < 0 && j < store.size(); ++j)
                    if (!used[j] && store[j].id == it.id && item_digest(store[j].f) == it.digest) pick = (int)j;
                for (size_t j = 0; ok && pick < 0 && j < store.size(); ++j)
                    if (!used[j] && item_digest(store[j].f) == it.digest) pick = (int)j;
                std::string w = ok ? "il n'est plus dans le rangement" : "rangement illisible";
                if (pick >= 0) {
                    used[(size_t)pick] = true;
                    if (loot_storage_take(store[(size_t)pick].id, it.digest, w)) {
                        log_line("DON", "%s : retire de ton rangement (%s l'a)", it.f.name.c_str(), r.name.c_str());
                        continue;
                    }
                }
                ++r.kept;
                log_line("DON", "!! %s : garde par %s, mais il n'a pas pu quitter ton rangement (%s) -- il est aussi "
                                "chez toi", it.f.name.c_str(), r.name.c_str(), w.c_str());
            }
            r.items.clear();
            g.save_due = true;
        }
        note(gift_done_text(r.name, r.list, r.kept, r.late), true, true, true, true);
        g.removals.erase(g.removals.begin() + (ptrdiff_t)i);
    }
    // La maison a change : sa sauvegarde, des que rien ne l'empeche (sinon le jeu
    // sauvera a son prochain arret).
    if (g.save_due && at_home(false, nullptr) && !screens_busy()) {
        g.save_due = false;
        save_now("don fait");
    }
}

void on_answer(uint8_t from, const GiftMsg& m, const uint8_t* raw, size_t n) {
    if (net_role() == NetRole::Host) {
        // Seul celui qui recoit repond.
        if (m.target != from) {
            log_line("DON", "!! reponse au don %u du joueur %u au nom du joueur %u -- ignoree", m.id, (unsigned)from,
                     (unsigned)m.target);
            return;
        }
        // Un don d'un invite : relaye.
        if (m.giver != net_self()) {
            if (!present(m.giver) || !team_send(std::vector<uint8_t>(raw, raw + n), m.giver))
                log_line("DON", "!! reponse au don %u : le donneur (%u) n'est plus la", m.id, (unsigned)m.giver);
            return;
        }
    } else if (from != kHostPeer || m.giver != net_self()) {
        return;
    }
    // Le don en cours, ou l'un de ceux restes sans reponse a temps : il est fini.
    Outgoing taken;
    const auto late = std::find_if(g.late.begin(), g.late.end(),
                                   [&](const Outgoing& x) { return x.id == m.id && x.target == m.target; });
    if (g.out.on && g.out.id == m.id && g.out.target == m.target) {
        taken = std::move(g.out);
        g.out = Outgoing{};
    } else if (late != g.late.end()) {
        taken = std::move(*late);
        g.late.erase(late);
    } else {
        log_line("DON", "reponse a un don inconnu ou fini (%u) -- ignoree", m.id);
        return;
    }
    const Outgoing* o = &taken;
    if (m.op == kGiftRefused) {
        log_line("DON", "<- don %u refuse par %s (raison %u)", o->id, o->name.c_str(), (unsigned)m.why);
        note(gift_refused_text(o->name, m.why), true, true, true);
        return;
    }
    // Garde chez l'autre : ce qui a ete donne quitte sa maison (removals_pump). Le
    // nom est celui que donne celui qui l'a recu.
    Removal rm;
    rm.id    = o->id;
    rm.name  = m.receiver_name.empty() ? o->name : m.receiver_name;
    rm.save  = o->save;
    rm.coins = o->coins;
    rm.food  = o->food;
    rm.late  = o->late;
    std::vector<std::string> labels;
    for (const OutItem& it : o->items) labels.push_back(it.label);
    rm.list  = gift_list_text(o->coins, o->food, labels);
    rm.items = std::move(taken.items);
    log_line("DON", "<- don %u garde par %s", rm.id, rm.name.c_str());
    if (g.removals.size() >= kMaxRemovals) {
        log_line("DON", "!! trop de dons a retirer : le plus ancien (%u) reste aussi chez toi", g.removals.front().id);
        g.removals.erase(g.removals.begin());
    }
    g.removals.push_back(std::move(rm));
    const std::string who  = g.removals.back().name;
    const std::string save = g.removals.back().save;
    removals_pump();
    // Pas dans la save d'ou il est parti (une autre, le menu, une aventure) : il le
    // saura, et le retrait attend son retour.
    if (!giver_here(save))
        note(purse_safe(who) + u8" a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour "
                               u8"dans cette save.", true, true, true, true);
}

void outgoing_pump() {
    const uint64_t now = GetTickCount64();
    if (g.out.on) {
        const bool gone = !in_session() || !present(g.out.target);
        if (gone || now - g.out.sent_at > kAnswerMs) {
            log_line("DON", "!! don %u a %s : %s -- rien n'a quitte ta maison", g.out.id, g.out.name.c_str(),
                     gone ? "il n'est plus la" : "pas de reponse");
            if (gone) {
                note(gift_refused_text(g.out.name, kGiftWhyGone), true, true, true);
                g.out = Outgoing{};
            } else {
                note(u8"Pas de réponse " + purse_of(g.out.name) + u8" pour ton don : rien n'a quitté ta maison.", true,
                     true, true);
                if (g.late.size() >= kMaxLate) {
                    log_line("DON", "!! trop de dons sans reponse : le plus ancien (%u) n'attend plus la sienne",
                             g.late.front().id);
                    g.late.erase(g.late.begin());
                }
                g.out.late = true;
                g.late.push_back(std::move(g.out));
                g.out = Outgoing{};
            }
        }
    }
    g.late.erase(std::remove_if(g.late.begin(), g.late.end(),
                                [&](const Outgoing& o) {
                                    return now - o.sent_at > kLateMs || !in_session() || !present(o.target);
                                }),
                 g.late.end());
    removals_pump();
}

// --- la presence -----------------------------------------------------------------------------

void presence_pump() {
    const uint64_t now = GetTickCount64();
    const bool     can = at_home(true, nullptr);
    if (net_role() == NetRole::Client) {
        if ((int)can != g.sent_can || now - g.sent_can_at >= kPresenceBeatMs) {
            MmMsg m{};
            m.kind = MM_GIFT;
            m.a    = kPresenceMine;
            m.c    = can ? 1 : 0;
            if (net_send_mm(m)) g.sent_can = can ? 1 : 0;
            g.sent_can_at = now;
        }
        return;
    }
    if (net_role() != NetRole::Host) return;
    g.guest_can &= roster_mask();
    uint32_t mask = g.guest_can & ~(1u << kHostPeer);
    if (can) mask |= 1u << kHostPeer;
    if (!g.mask_said || mask != g.mask_sent || now - g.mask_sent_at >= kPresenceBeatMs) {
        MmMsg m{};
        m.kind = MM_GIFT;
        m.a    = kPresenceAll;
        m.x    = mask;
        if (net_send_mm(m)) {
            g.mask_sent = mask;
            g.mask_said = true;
        }
        g.mask_sent_at = now;
    }
}

void session_end() {
    if (!g.in.empty())
        log_line("DON", "fin de la session : %zu don(s) recu(s) en attente sont oublies", g.in.size());
    g.in.clear();
    g.sent_can  = -1;
    g.guest_can = 0;
    g.mask_said = false;
    g.have_mask = false;
}

// --- la fenetre ------------------------------------------------------------------------------

void publish(bool window) {
    const uint64_t now = GetTickCount64();
    if (g.view_at && now - g.view_at < kViewGapMs) return;
    g.view_at = now ? now : 1;
    TradeView v;
    const uint8_t self = net_self();
    std::string   why;
    std::vector<PurseHolder> holders;
    if (!in_session()) {
        v.mode     = TradeMode::None;
        v.why_none = u8"Les échanges marchent pendant une partie à plusieurs.";
    } else if (at_home(true, &why)) {
        v.mode = TradeMode::House;
        v.show = true;
        int32_t hc = 0, hf = 0;
        if (read_money(house(), hc, hf)) {
            v.coins = hc;
            v.food  = hf;
        }
        v.have_text = u8"Dans ta maison : " + purse_amount_text(v.coins, v.food) + ".";
        uint8_t ids[8] = {};
        const uint8_t n = net_peer_count();
        if (n && net_peer_ids(ids, 8))
            for (uint8_t i = 0; i < n && i < 8; ++i) {
                if (ids[i] == self) continue;
                TradePeer p;
                p.id   = ids[i];
                p.name = name_of(ids[i]);
                p.can  = peer_can(ids[i]);
                if (!p.can) p.why = purse_safe(p.name) + u8" n'est pas dans sa maison.";
                v.peers.push_back(std::move(p));
            }
        std::string iw;
        v.items_ok = !items_blocked(&iw);
        if (!v.items_ok) v.items_why = u8"Pas d'objets maintenant : " + iw + ".";
        if (window && v.items_ok) {
            std::vector<LootStoredItem> store;
            if (loot_storage_items(store)) {
                v.items_ready = true;
                const std::vector<bool> promised = promised_mask(store);
                for (size_t j = 0; j < store.size(); ++j) {
                    if (promised[j] || !item_fits(store[j].f)) continue;
                    TradeItem t;
                    t.id    = store[j].id;
                    t.label = label_of(store[j].f);
                    v.items.push_back(std::move(t));
                }
                std::stable_sort(v.items.begin(), v.items.end(), [](const TradeItem& a, const TradeItem& b) {
                    return _stricmp(a.label.c_str(), b.label.c_str()) < 0;
                });
            }
        }
        v.pending = g.out.on;
    } else if (purse_holders(holders)) {
        const PurseHolder* mine = nullptr;
        for (const PurseHolder& h : holders)
            if (h.peer == self) mine = &h;
        if (mine) {
            v.mode      = TradeMode::Run;
            v.coins     = mine->coins;
            v.food      = mine->food;
            v.have_text = u8"Ta bourse : " + purse_balance_text(mine->coins, mine->food) + ".";
            for (const PurseHolder& h : holders) {
                if (h.peer == self) continue;
                TradePeer p;
                p.id   = h.peer;
                p.name = name_of(h.peer);
                p.can  = present(h.peer);
                if (!p.can) p.why = purse_safe(p.name) + u8" n'est plus dans la partie.";
                v.peers.push_back(std::move(p));
            }
            v.pending = purse_give_pending();
            v.show    = follow_on_map();     // sur la carte : pas en plein combat ou evenement
        } else {
            v.mode     = TradeMode::None;
            v.why_none = u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).";
        }
    } else {
        v.mode     = TradeMode::None;
        v.why_none = u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. "
                     u8"Pour l'instant : " + why + ".";
    }
    v.status      = g.status;
    v.status_id   = g.status_id;
    v.kept_count  = g.kept_count;
    g.toasts.erase(std::remove_if(g.toasts.begin(), g.toasts.end(),
                                  [&](const Toast& t) { return now - t.at >= kToastMs; }),
                   g.toasts.end());
    for (const Toast& t : g.toasts) {
        TradeToast tt;
        tt.id     = t.id;
        tt.text   = t.text;
        tt.result = t.result;
        v.toasts.push_back(std::move(tt));
    }
    Lock l;
    v.serial = ++g_serial;
    g_view   = std::move(v);
}

// « Cadeau de ... » : celui qui recoit n'est pas arrete par la nouvelle. Elle attend
// que sa maison soit deja en pause (le menu pause), sans autre fenetre (mm_info_quiet).
void popups_pump() {
    if (g.popups.empty()) return;
    if (!mm_house() || !mm_info_quiet()) return;
    mm_show_info(g.popups.front());
    g.popups.erase(g.popups.begin());
}

} // namespace

// ---------------------------------------------------------------------------------------------

void gift_pump() {
    if (!g.told) {
        g.told    = true;
        g.save_ok = addr_of_call(C_DirectorSave) != 0;
        // Les numeros de dons partent d'un nombre au hasard : un donneur qui relance
        // son jeu ne reprend pas ceux d'avant (celui qui recoit les retient).
        g.next_id = (uint32_t)(GetTickCount64() * 2654435761ull) ^ ((uint32_t)GetCurrentProcessId() << 12);
        log_line("DON", g.save_ok ? "arme -- les dons entre joueurs, a la maison et en aventure (bourses)"
                                  : "!! MewDirector::save introuvable : pas de dons a la maison (ceux des bourses, "
                                    "en aventure, marchent)");
    }
    const bool session = in_session();
    if (!session && g.was_session) session_end();
    g.was_session = session;

    Request r;
    bool    have   = false;
    bool    window = false;
    {
        Lock l;
        if (g_has_req) {
            r         = std::move(g_req);
            g_req     = Request{};
            g_has_req = false;
            have      = true;
        }
        window = g_window;
    }
    if (have) {
        if (r.mode == TradeMode::House) house_request(r);
        else if (r.mode == TradeMode::Run) run_request(r);
    }
    if (session) {
        presence_pump();
        incoming_pump();
    }
    outgoing_pump();
    popups_pump();
    publish(window);
}

void gift_on_team(uint8_t from, const uint8_t* p, size_t n) {
    GiftMsg m;
    if (!gift_decode(p, n, m)) {
        log_line("DON", "!! don illisible (%zu octets, joueur %u) -- ignore", n, (unsigned)from);
        return;
    }
    if (m.op == kGiftOffer) on_offer(from, m, p, n);
    else on_answer(from, m, p, n);
}

void gift_on_mm(uint8_t from, const MmMsg& m) {
    if (m.kind != MM_GIFT) return;
    if (net_role() == NetRole::Host) {
        if (m.a != kPresenceMine || from >= 32 || from == kHostPeer) return;
        const uint32_t bit = 1u << from;
        const bool     can = m.c != 0;
        if (((g.guest_can & bit) != 0) != can)
            log_line("DON", "%s %s", name_of(from).c_str(),
                     can ? "peut recevoir des dons (dans sa maison)" : "ne peut plus recevoir de dons");
        g.guest_can = can ? (g.guest_can | bit) : (g.guest_can & ~bit);
        return;
    }
    if (net_role() == NetRole::Client && from == kHostPeer && m.a == kPresenceAll) {
        g.host_mask    = m.x;
        g.host_mask_at = GetTickCount64();
        g.have_mask    = true;
    }
}

void gift_purse_note(const std::string& text, bool mine, bool kept) { note(text, mine, true, mine, kept); }

bool gift_house_open() { return at_home(true, nullptr); }

bool gift_view(TradeView& out, uint32_t have) {
    Lock l;
    if (g_view.serial == have) return false;
    out = g_view;
    return true;
}

void gift_window_open(bool open) {
    Lock l;
    g_window = open;
}

void gift_request(TradeMode mode, uint8_t to, int64_t coins, int64_t food, const std::vector<uint64_t>& item_ids) {
    Lock l;
    g_req.mode  = mode;
    g_req.to    = to;
    g_req.coins = coins;
    g_req.food  = food;
    g_req.items = item_ids;
    g_has_req   = true;
}

} // namespace mgmp
