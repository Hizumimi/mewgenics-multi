// mm_shop -- Mewgenics Multi : les achats en boutique avec sa bourse. Voir mm_shop.h
// (le jeu, le mod) et mm_shop_core.h (les regles, le message).

#include "mm_shop.h"
#include "mm_shop_core.h"

#include "mm_loot.h"          // le sac et le rangement : objets, retrait, noms
#include "mm_purse.h"         // purse_tracking, purse_slot, purse_charge
#include "mm_purse_core.h"    // purse_safe, purse_of
#include "mm_team.h"          // team_send, team_run_slots, team_slot_peer, team_note_loot
#include "mm_team_merge.h"    // item_digest
#include "mm_menu.h"          // mm_show_info, mm_peer_name, mm_run_peers_mask, mm_house

#include "mgmp_addresses.h"
#include "mgmp_follow.h"      // follow_here_seed, follow_node_live, follow_pending_count
#include "mgmp_hooks.h"       // hooks_is_live
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"       // kHostPeer
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"    // savefile_in_coop
#include "mgmp_session.h"     // session_ready

#include <windows.h>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_shop.h ---------------------------------------------------

constexpr uintptr_t kShop_ItemsBegin = 0x60;   // vector<ShopItem>
constexpr uintptr_t kShop_ItemsEnd   = 0x68;
constexpr uintptr_t kShop_Panel      = 0x40;   // son MenuPanel (Shop::update y ecrit « coins »)
constexpr uintptr_t kShop_Treasure   = 0x79;   // u8 : 1 une salle au tresor (GON treasure_room) --
                                               //   initShop « achete » tout seul chaque objet (0x1407964EF)
constexpr uintptr_t kShop_House      = 0x7B;   // u8 : 1 une boutique de maison
constexpr size_t    kShopItemSize    = 0xC0;
constexpr uintptr_t kItem_Type       = 0x00;
constexpr uintptr_t kItem_Price      = 0x08;
constexpr uintptr_t kItem_Stock      = 0x0C;
constexpr uintptr_t kItem_Amount     = 0x30;
constexpr uintptr_t kItem_Equip      = 0x38;   // un Equipment de 0x60 octets
constexpr size_t    kMaxShopItems    = 64;     // une boutique en a une dizaine au plus

// --- les delais ------------------------------------------------------------------------------

constexpr uint64_t kPendingMs    = 30000;    // invite : une demande sans reponse ; apres, on peut redemander
constexpr uint64_t kReqWaitMs    = 25000;    // hote : une demande que sa boutique (en pause : une fenetre
                                             //   ouverte par-dessus) n'a pas prise -- moins que kPendingMs
constexpr uint64_t kReqGoneMs    = 1500;     // ... et une fois sa scene « Shop » partie
constexpr uint64_t kArriveMs     = 15000;    // hote : un invite de l'aventure qui n'a pas encore dit sa
                                             //   boutique ouverte, depuis l'entree de l'hote, « arrive encore »
constexpr uint64_t kInsideMs     = 2000;     // hote : sa boutique a tourne il y a moins que ca
constexpr size_t   kMaxPending   = 8;        // invite : demandes en attente (une par objet)
constexpr uint64_t kReplayMaxMs  = 120000;   // invite : un achat a rejouer, au plus
constexpr uint64_t kReplayGoneMs = 3000;     // ... et pas plus une fois sorti de cette case
constexpr uint64_t kGiveTimeoutMs = 120000;  // hote : sans reponse de l'acheteur, ses achats restent chez lui
constexpr uint64_t kOpenMs       = 600;      // invite : sa boutique a tourne il y a moins que ca -- ouverte
constexpr uint64_t kHereBeatMs   = 3000;     // ... et il le redit
constexpr uint64_t kHereStaleMs  = 7000;     // hote : rien entendu depuis, il n'y est plus
constexpr uint64_t kExitConfirmMs = 10000;   // hote : le second clic sur « Sortir », dans ce delai
constexpr size_t   kMaxQueue     = 32;
constexpr uintptr_t kBtnName     = kBtn_Name;   // Button : son nom (std::string)

using FnShopBuy    = void (__fastcall*)(void* shop, void* item);
using FnShopUpdate = void (__fastcall*)(void* shop);
FnShopBuy    o_ShopBuy    = nullptr;
FnShopUpdate o_ShopUpdate = nullptr;

// Un objet de la boutique, tel que lu dans le jeu.
struct ItemView {
    int32_t    type   = 0;
    int32_t    price  = 0;
    int32_t    stock  = 0;
    int32_t    amount = 0;
    bool       has_f  = false;    // un objet (genre 1) lisible
    ItemFields f;
    uint64_t   eq_id  = 0;
    uint64_t   digest = 0;
};

struct Req {                      // hote : une demande d'invite, pour sa boutique
    uint8_t  from = 0;
    ShopMsg  m;
    uint64_t at   = 0;
};

struct Replay {                   // invite : un achat a rejouer dans sa copie
    ShopMsg  m;
    uint64_t at        = 0;
    uint64_t left_at   = 0;       // sorti de cette case depuis (0 : pas encore)
};

struct Give {                     // hote : des achats partis chez leur acheteur a la victoire
    uint64_t                 gift = 0;
    void*                    dir  = nullptr;   // la partie d'ou ils viennent (une autre chargee : rien a retirer)
    uint8_t                  peer = 0;
    std::string              who;
    std::vector<uint64_t>    ids, digests;
    std::vector<std::string> labels;
    uint64_t                 at   = 0;
};

struct Erase {                    // hote : un achat garde par son acheteur, a retirer du rangement
    uint64_t    id = 0, digest = 0;
    void*       dir = nullptr;
    std::string label, who;
};

struct Here {                     // hote : la boutique d'un invite
    uint64_t node = 0;
    bool     open = false;
    uint64_t at   = 0;
};

struct Owner {                    // la victoire : un acheteur, tel que l'equipe le connait encore
    uint8_t     slot = 0;
    int         peer = -1;
    std::string name;
};

struct State {
    bool     told  = false;
    bool     armed = false;       // les deux crochets
    bool     replaying = false;   // un BuyItem a nous : le detour le laisse passer
    uint64_t frame = 0;
    // --- hote ---
    ShopLedger         ledger;
    bool               ledger_on  = false;
    uint32_t           ledger_gen = 0;
    std::deque<Req>    reqs;
    uint32_t           bought = 0, refused = 0;
    // la victoire
    bool               snap = false;
    std::vector<ShopOwned> snap_owned;
    std::vector<uint64_t>  claimed;        // les ids achetes a la derniere victoire : ils ne defilent pas
    std::vector<Owner> snap_owners;
    uint8_t            snap_seq = 0;
    std::vector<Give>  gives;
    std::vector<Erase> erases;
    std::vector<std::string> home_notes;
    uint64_t           rng = 0;
    Here               here[8];                // la boutique de chaque invite (kShopHere)
    uint64_t           exit_asked_at = 0;      // « Sortir » retenu une fois, a ce moment...
    uint64_t           exit_closed_at = 0;     // ... et sa fenetre refermee (la boutique tourne de nouveau)
    void*              host_shop_ptr   = nullptr; // sa boutique d'aventure (le composant)...
    uint64_t           host_shop_node  = 0;    // ... sa case...
    uint64_t           host_shop_since = 0;    // ... depuis (sa premiere image)
    uint64_t           host_shop_at    = 0;    // ... et sa derniere image
    // --- invite ---
    std::deque<Replay> replays;
    struct Pending { uint64_t node = 0; uint16_t index = 0; uint64_t at = 0; };
    std::vector<Pending> pending;              // ses demandes sans reponse, une par objet
    uint64_t           got_ids[16] = {};
    uint32_t           got_next = 0;
    uint64_t           my_shop_node = 0;       // sa boutique d'aventure : sa case...
    uint64_t           my_shop_at   = 0;       // ... et sa derniere image
    uint64_t           here_node    = 0;       // ce qu'il a dit a l'hote
    bool               here_open    = false;
    uint64_t           here_at      = 0;
    uint32_t           asked = 0, replayed = 0;
};
State g;

// --- petits outils ----------------------------------------------------------------------------

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

bool is_host()    { return net_role() == NetRole::Host; }
bool is_guest()   { return net_role() == NetRole::Client; }
bool in_session() { return session_ready() && net_active() && net_peer_count() >= 2; }

bool peer_present(uint8_t peer) {
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (!n || !net_peer_ids(ids, 8)) return false;
    for (uint8_t i = 0; i < n && i < 8; ++i)
        if (ids[i] == peer) return true;
    return false;
}

// Hote : des invites jouent son aventure (leurs copies ont la meme boutique).
bool host_synced() { return is_host() && in_session() && mm_run_peers_mask() != 0; }
// Invite : il est dans la copie de l'aventure de l'hote (ses achats ne se font pas chez lui).
bool guest_copy()  { return is_guest() && savefile_in_coop() && savefile_adventure_is_loaded(); }

// Le Director vivant : son etiquette (ptr-8) est celle notee a cote de son
// emplacement global (+8), jamais remis a zero (voir mm_npc.cpp).
void* director() {
    const void* const* slot = (const void* const*)addr_of_data(D_MewDirectorPtr);
    const uint8_t* d = nullptr;
    uint64_t tag = 0, mark = 0;
    if (!slot || !mem_read(slot, &d, sizeof(d)) || !d) return nullptr;
    if (!mem_read(slot + 1, &tag, sizeof(tag)) || !mem_read(d - 8, &mark, sizeof(mark)) || mark != tag)
        return nullptr;
    return (void*)d;
}

void* inventory() {
    void* d = director();
    void* inv = nullptr;
    if (!d || !mem_read((const char*)d + kDir_Inventory, &inv, sizeof(inv))) return nullptr;
    return inv;
}

bool read_run(int32_t& coins, int32_t& food) {
    void* inv = inventory();
    return inv && mem_read((const char*)inv + kInv_Coins, &coins, sizeof(coins)) &&
           mem_read((const char*)inv + kInv_Food, &food, sizeof(food));
}

bool write_run(int32_t coins, int32_t food) {
    void* inv = inventory();
    if (coins < 0) coins = 0;
    if (food < 0) food = 0;
    return inv && mem_write((char*)inv + kInv_Coins, &coins, sizeof(coins)) &&
           mem_write((char*)inv + kInv_Food, &food, sizeof(food));
}

// Une boutique a laisser au jeu : celle d'une maison (Jack, Tracy), ou une salle
// au tresor (initShop y « achete » tout seul chaque objet, gratuit : comme avant,
// chacun ramasse dans sa copie, et la poussee de l'hote apres la case remet les
// sacs -- relecture de la build 30).
bool special_shop(void* shop) {
    uint8_t h = 1, t = 1;
    return !shop || !mem_read((const char*)shop + kShop_House, &h, 1) || h != 0 ||
           !mem_read((const char*)shop + kShop_Treasure, &t, 1) || t != 0;
}

// Les objets de la boutique : [begin, end), verifies.
bool shop_items(void* shop, uintptr_t& begin, uintptr_t& end) {
    begin = end = 0;
    if (!shop || !mem_read((const char*)shop + kShop_ItemsBegin, &begin, sizeof(begin)) ||
        !mem_read((const char*)shop + kShop_ItemsEnd, &end, sizeof(end)))
        return false;
    if (!begin || end < begin || (end - begin) % kShopItemSize || (end - begin) / kShopItemSize > kMaxShopItems)
        return false;
    return true;
}

bool read_item(void* item, ItemView& v) {
    v = ItemView{};
    if (!item || !mem_read((const char*)item + kItem_Type, &v.type, 4) ||
        !mem_read((const char*)item + kItem_Price, &v.price, 4) ||
        !mem_read((const char*)item + kItem_Stock, &v.stock, 4) ||
        !mem_read((const char*)item + kItem_Amount, &v.amount, 4))
        return false;
    uint64_t eqd = 0;
    if (v.type == kShopTypeItem) {
        v.has_f = loot_read_equipment((const char*)item + kItem_Equip, v.f, &v.eq_id) && !v.f.name.empty();
        if (!v.has_f) return false;
        eqd = item_digest(v.f);
    }
    v.digest = shop_item_digest(v.type, v.price, (v.type == kShopTypeFood || v.type == kShopTypeCoins) ? v.amount : 0,
                                eqd);
    return true;
}

void* item_at(uintptr_t begin, uint16_t index) { return (void*)(begin + (uintptr_t)index * kShopItemSize); }

std::string item_label(const ItemView& v) {
    switch (v.type) {
    case kShopTypeItem:     return v.has_f ? loot_item_label(v.f) : std::string(u8"un objet");
    case kShopTypeFood:
    case kShopTypeCoins:    return shop_amount_label(v.type, v.amount);
    case kShopTypeLevelUp:  return u8"un bonbon";
    case kShopTypeFamiliar: return u8"un familier";
    default:                return u8"un article";
    }
}

// Rejoue-t-on cet achat dans une copie ? Un bonbon ouvrirait le choix du chat.
bool replayable(int32_t type) {
    return type == kShopTypeItem || type == kShopTypeFood || type == kShopTypeCoins || type == kShopTypeFamiliar;
}

std::vector<uint64_t> bag_ids() {
    std::vector<uint64_t> out;
    std::vector<LootStoredItem> bag;
    if (loot_bag_items(bag))
        for (const LootStoredItem& b : bag) out.push_back(b.id);
    return out;
}

// Hote : le numero de place (celui des bourses) du joueur `peer` maintenant, -1
// s'il n'en a pas (il n'a pas amene de chats).
int slot_of_peer(uint8_t peer) {
    uint32_t gen = 0, slots = 0;
    if (!team_run_slots(gen, slots)) return peer == kHostPeer ? kHostPeer : -1;
    if (peer == kHostPeer) return kHostPeer;
    for (uint8_t s = 0; s < 32; ++s)
        if ((slots & (1u << s)) && s != kHostPeer && team_slot_peer(s) == (int)peer) return s;
    return -1;
}

void say(const std::string& text) { mm_show_info(text); }

uint64_t next_gift_id() {
    if (!g.rng) g.rng = GetTickCount64() * 0x9E3779B97F4A7C15ull ^ (uint64_t)(uintptr_t)&g;
    g.rng ^= g.rng << 13;
    g.rng ^= g.rng >> 7;
    g.rng ^= g.rng << 17;
    return g.rng ? g.rng : 1;
}

// --- appels au jeu, sous SEH (rien de C++ a detruire ici) --------------------------------------

bool seh_buy(FnShopBuy fn, void* shop, void* item) {
    __try {
        fn(shop, item);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

struct BuyCall {
    void* shop = nullptr;
    void* item = nullptr;
    bool  ok   = false;
};

void buy_trampoline(void* p) {
    BuyCall* c = (BuyCall*)p;
    g.replaying = true;
    c->ok = seh_buy(o_ShopBuy, c->shop, c->item);
    g.replaying = false;
}

// --- hote : l'achat ---------------------------------------------------------------------------

void send_sold(const ShopMsg& base, const ItemView& v, uint8_t buyer, int32_t c0, int32_t f0, int32_t c1, int32_t f1,
               const std::string& label) {
    if (!host_synced()) return;
    ShopMsg s;
    s.op           = kShopSold;
    s.node         = base.node;
    s.index        = base.index;
    s.digest       = v.digest;
    s.price        = v.price;
    s.buyer        = buyer;
    s.coins_before = c0;
    s.food_before  = f0;
    s.coins_after  = c1;
    s.food_after   = f1;
    s.label        = label;
    if (!team_send(shop_encode(s), -1)) log_line("SHOP", "!! envoi de l'achat aux invites impossible");
}

// Achete l'objet `index` de la boutique vivante pour le joueur de place `slot`
// (`buyer` : son numero maintenant), le prix pris dans sa bourse. Vrai si le jeu
// l'a achete.
bool buy_for(void* shop, uintptr_t begin, uint16_t index, const ItemView& v, int slot, uint8_t buyer,
             const char* who) {
    void* item = item_at(begin, index);
    const std::string label = item_label(v);
    const size_t n0 = v.type == kShopTypeItem ? shop_count(bag_ids(), v.eq_id) : 0;
    int32_t c0 = 0, f0 = 0, c1 = 0, f1 = 0;
    const bool run0 = read_run(c0, f0);
    BuyCall call{ shop, item, false };
    if (slot >= 0) purse_charge((uint8_t)slot, label, &buy_trampoline, &call);
    else           buy_trampoline(&call);
    if (!call.ok) {
        log_line("SHOP", "!! %s : exception dans le jeu en achetant %s", who, label.c_str());
        return false;
    }
    // Achete : son stock a baisse (de 1, ou a 0 quand c'etait le dernier achat
    // permis -- le jeu passe alors tout ce qui reste a 0).
    ItemView after;
    if (!read_item(item, after) || after.stock >= v.stock) {
        log_line("SHOP", "%s : le jeu n'a pas achete %s (stock %d -> %d)", who, label.c_str(), v.stock, after.stock);
        return false;
    }
    const bool run1 = run0 && read_run(c1, f1);
    if (v.type == kShopTypeItem && slot >= 0) {
        // L'objet arrive dans le sac avec l'id de celui de la boutique (insert_item
        // le copie) : un exemplaire de plus de cet id.
        const size_t n1 = shop_count(bag_ids(), v.eq_id);
        for (size_t k = n0; k < n1; ++k) g.ledger.add(v.eq_id, (uint8_t)slot, label);
        if (n1 <= n0)
            log_line("SHOP", "%s : %s achete, mais pas retrouve dans le sac (devenu autre chose ?)", who,
                     label.c_str());
    }
    ++g.bought;
    log_line("SHOP", "%s achete %s (place %u) pour %d piece(s) ; compte %d -> %d piece(s), %d -> %d nourriture(s)%s",
             who, label.c_str(), (unsigned)index, v.price, c0, c1, f0, f1, slot >= 0 ? "" : " (pas de bourses)");
    ShopMsg base;
    base.node  = follow_here_seed();
    base.index = index;
    if (run1) send_sold(base, v, buyer, c0, f0, c1, f1, label);
    else      log_line("SHOP", "!! compte de l'aventure illisible : l'achat n'est pas rejoue chez les invites");
    return true;
}

// Une demande d'invite, dans Shop::update de la boutique vivante.
void host_take(void* shop, const Req& r) {
    ShopMsg done;
    done.op    = kShopDone;
    done.node  = r.m.node;
    done.index = r.m.index;
    const std::string who = mm_peer_name(r.from);

    ShopLook look;
    ItemView v;
    uintptr_t begin = 0, end = 0;
    look.shop_here = follow_here_seed() == r.m.node && shop_items(shop, begin, end);
    if (look.shop_here && r.m.index < (end - begin) / kShopItemSize && read_item(item_at(begin, r.m.index), v)) {
        look.present = true;
        look.digest  = v.digest;
        look.type    = v.type;
        look.price   = v.price;
        look.stock   = v.stock;
    }
    const int slot = slot_of_peer(r.from);
    int64_t coins = 0, food = 0;
    bool has = false;
    const bool mixed = purse_tracking();
    if (slot >= 0) purse_slot((uint8_t)slot, coins, food, has);
    ShopResult res = shop_check(r.m, look, mixed, slot >= 0 && has, coins);
    if (res == ShopResult::Bought && !buy_for(shop, begin, r.m.index, v, slot, r.from, who.c_str()))
        res = ShopResult::Failed;
    if (res != ShopResult::Bought) {
        ++g.refused;
        log_line("SHOP", "<- %s veut acheter l'objet %u de la case %016llx : refuse (%u)", who.c_str(),
                 (unsigned)r.m.index, (unsigned long long)r.m.node, (unsigned)res);
    }
    if (slot >= 0) purse_slot((uint8_t)slot, coins, food, has);
    done.result = res;
    done.price  = look.price;
    done.purse  = (int32_t)std::max<int64_t>(INT32_MIN, std::min<int64_t>(INT32_MAX, coins));
    if (!team_send(shop_encode(done), r.from)) log_line("SHOP", "!! reponse a %s impossible", who.c_str());
}

// L'hote achete lui-meme (le bouton de sa boutique).
void host_own_buy(void* shop, void* item) {
    uintptr_t begin = 0, end = 0;
    uint16_t index = 0;
    ItemView v;
    if (!shop_items(shop, begin, end) || !shop_index_of(begin, end, (uintptr_t)item, kShopItemSize, index) ||
        !read_item(item, v)) {
        log_line("SHOP", "!! achat de l'hote : objet illisible -- achete par le jeu sur ta bourse, pas rejoue "
                         "chez les invites");
        BuyCall call{ shop, item, false };
        purse_charge(kHostPeer, u8"un article", &buy_trampoline, &call);   // sans bourses suivies : le jeu seul
        return;
    }
    if (!purse_tracking()) {             // pas de bourses : le jeu, puis les copies
        buy_for(shop, begin, index, v, -1, kShopNoBuyer, "l'hote");
        return;
    }
    int64_t coins = 0, food = 0;
    bool has = false;
    purse_slot(kHostPeer, coins, food, has);
    const ShopResult res = shop_host_check(has, coins, v.price);
    if (res != ShopResult::Bought) {
        ++g.refused;
        log_line("SHOP", "l'hote veut acheter %s (%d piece(s)) : sa bourse ne suffit pas (%lld)", item_label(v).c_str(),
                 v.price, (long long)coins);
        say(shop_refusal_text(res, true, v.price, coins));
        return;
    }
    buy_for(shop, begin, index, v, kHostPeer, kHostPeer, "l'hote");
}

// --- invite : la demande, puis le rejeu --------------------------------------------------------

void guest_ask(void* shop, void* item) {
    uintptr_t begin = 0, end = 0;
    uint16_t index = 0;
    ItemView v;
    const uint64_t node = follow_here_seed();
    if (!g.armed || !node || !shop_items(shop, begin, end) ||
        !shop_index_of(begin, end, (uintptr_t)item, kShopItemSize, index) || !read_item(item, v)) {
        log_line("SHOP", "!! achat : objet ou case illisible -- pas d'achat");
        say(shop_local_failure_text());
        return;
    }
    if (!shop_guest_type(v.type)) {
        log_line("SHOP", "achat de %s : seul l'hote achete ce genre (%d)", item_label(v).c_str(), v.type);
        say(shop_refusal_text(ShopResult::NotItem, false, v.price, 0));
        return;
    }
    const uint64_t now = GetTickCount64();
    // Deja demande, sans reponse (l'hote a peut-etre une fenetre ouverte, sa
    // boutique en pause) : un second clic n'achete pas deux fois.
    for (const State::Pending& p : g.pending)
        if (p.node == node && p.index == index) {
            log_line("SHOP", "achat de %s deja demande a l'hote -- en attente de sa reponse", item_label(v).c_str());
            return;
        }
    if (g.pending.size() >= kMaxPending) {
        log_line("SHOP", "!! trop d'achats en attente de l'hote -- celui-ci attendra un autre clic");
        return;
    }
    ShopMsg m;
    m.op     = kShopBuy;
    m.node   = node;
    m.index  = index;
    m.digest = v.digest;
    m.price  = v.price;
    if (!team_send(shop_encode(m), kHostPeer)) {
        log_line("SHOP", "!! demande d'achat impossible a envoyer");
        say(shop_local_failure_text());
        return;
    }
    ++g.asked;
    g.pending.push_back(State::Pending{ node, index, now });
    log_line("SHOP", "-> achat de %s (place %u, %d piece(s)) demande a l'hote", item_label(v).c_str(), (unsigned)index,
             v.price);
}

// Dans Shop::update de sa copie : les achats de l'hote pour cette case, dans l'ordre.
void guest_replay(void* shop) {
    const uint64_t here = follow_here_seed();
    while (!g.replays.empty() && g.replays.front().m.node == here) {
        const ShopMsg m = g.replays.front().m;
        g.replays.pop_front();
        uintptr_t begin = 0, end = 0;
        ItemView v;
        if (!shop_items(shop, begin, end) || m.index >= (end - begin) / kShopItemSize ||
            !read_item(item_at(begin, m.index), v) || v.digest != m.digest) {
            log_line("SHOP", "!! achat de l'hote (%s, place %u) : pas le meme objet dans ta boutique -- pas rejoue "
                             "(ton sac suivra apres la case)", m.label.c_str(), (unsigned)m.index);
            write_run(m.coins_after, m.food_after);
            continue;
        }
        if (v.stock <= 0) {
            log_line("SHOP", "!! achat de l'hote (%s) : deja vendu dans ta boutique", m.label.c_str());
            write_run(m.coins_after, m.food_after);
            continue;
        }
        if (replayable(v.type)) {
            write_run(m.coins_before, m.food_before);
            BuyCall call{ shop, item_at(begin, m.index), false };
            buy_trampoline(&call);
            ItemView after;
            const bool done = call.ok && read_item(item_at(begin, m.index), after) && after.stock < v.stock;
            if (!done) log_line("SHOP", "!! achat de l'hote (%s) : le jeu ne l'a pas rejoue ici", m.label.c_str());
            else ++g.replayed;
        }
        write_run(m.coins_after, m.food_after);
        const std::string buyer = m.buyer == kShopNoBuyer ? std::string("l'hote")
                                : m.buyer == net_self()   ? std::string("toi")
                                                          : mm_peer_name(m.buyer);
        log_line("SHOP", "<- %s achete par %s (%d piece(s)) : %s", m.label.c_str(), buyer.c_str(), m.price,
                 replayable(v.type) ? "rejoue dans ta boutique" : "le compte seulement (un bonbon n'est pas rejoue)");
    }
}

void guest_on_done(const ShopMsg& m) {
    for (size_t i = 0; i < g.pending.size(); ++i)
        if (g.pending[i].node == m.node && g.pending[i].index == m.index) {
            g.pending.erase(g.pending.begin() + (ptrdiff_t)i);
            break;
        }
    if (m.result == ShopResult::Bought) {
        log_line("SHOP", "<- achat fait chez l'hote (%d piece(s)) : ta bourse a maintenant %d piece(s)", m.price,
                 m.purse);
        return;
    }
    log_line("SHOP", "<- achat refuse par l'hote (%u)", (unsigned)m.result);
    say(shop_refusal_text(m.result, false, m.price, m.purse));
}

void guest_on_give(const ShopMsg& m) {
    bool seen = false;
    for (uint64_t id : g.got_ids) seen = seen || id == m.gift;
    uint64_t kept = 0;
    std::vector<std::string> labels;
    for (size_t i = 0; i < m.items.size() && i < 64; ++i) {
        const uint64_t key = m.gift ^ (0xC2B2AE3D27D4EB4Full * (uint64_t)(i + 1));
        const std::string label = m.items[i].label.empty() ? m.items[i].f.name : m.items[i].label;
        if (team_note_loot(m.seq, key, m.items[i].f, label)) {
            kept |= 1ull << i;
            labels.push_back(label);
        }
    }
    if (!seen) {
        g.got_ids[g.got_next] = m.gift;
        g.got_next = (g.got_next + 1) % (sizeof(g.got_ids) / sizeof(g.got_ids[0]));
    }
    ShopMsg r;
    r.op   = kShopGot;
    r.gift = m.gift;
    r.kept = kept;
    if (!team_send(shop_encode(r), kHostPeer)) log_line("SHOP", "!! reponse a l'hote impossible pour tes achats");
    std::string list;
    for (size_t i = 0; i < labels.size(); ++i) list += (i ? ", " : "") + labels[i];
    log_line("SHOP", "<- tes achats de l'aventure rentrent avec toi : %s%s", list.empty() ? "rien" : list.c_str(),
             labels.size() < m.items.size() ? " (certains n'ont pas pu etre gardes : ils restent chez l'hote)" : "");
}

// --- hote : la victoire, puis les reponses -----------------------------------------------------

void host_on_got(uint8_t from, const ShopMsg& m) {
    for (size_t i = 0; i < g.gives.size(); ++i) {
        Give& gv = g.gives[i];
        if (gv.gift != m.gift || gv.peer != from) continue;
        for (size_t k = 0; k < gv.ids.size(); ++k) {
            if (k < 64 && ((m.kept >> k) & 1u)) {
                Erase e;
                e.id     = gv.ids[k];
                e.digest = gv.digests[k];
                e.dir    = gv.dir;
                e.label  = gv.labels[k];
                e.who    = gv.who;
                g.erases.push_back(e);
            } else {
                log_line("SHOP", "<- %s n'a pas pu garder %s : il reste chez toi", gv.who.c_str(), gv.labels[k].c_str());
            }
        }
        log_line("SHOP", "<- %s a repondu pour ses achats", gv.who.c_str());
        g.gives.erase(g.gives.begin() + (ptrdiff_t)i);
        return;
    }
    log_line("SHOP", "<- reponse de %s pour des achats qui ne lui etaient pas envoyes -- ignoree",
             mm_peer_name(from).c_str());
}

void drop_gone_gives() {
    const uint64_t now = GetTickCount64();
    for (size_t i = g.gives.size(); i-- > 0;) {
        const Give& gv = g.gives[i];
        const bool gone = !in_session() || !peer_present(gv.peer);
        if (!gone && now - gv.at < kGiveTimeoutMs) continue;
        log_line("SHOP", "!! %s %s sans repondre pour ses achats : ils restent chez toi", gv.who.c_str(),
                 gone ? "est parti" : "ne repond pas");
        g.gives.erase(g.gives.begin() + (ptrdiff_t)i);
    }
}

void run_erases() {
    if (g.erases.empty() || loot_storage_busy()) return;
    void* dir = director();
    if (!dir || !mm_in_game()) return;       // au menu : plus tard, dans la partie
    for (const Erase& e : g.erases) {
        if (e.dir != dir) {
            log_line("SHOP", "!! %s : garde par %s, mais une autre partie est chargee -- rien n'est retire",
                     e.label.c_str(), e.who.c_str());
            continue;
        }
        std::string why;
        if (loot_storage_take(e.id, e.digest, why))
            log_line("SHOP", "%s : retire de ton rangement (%s l'a achete et l'a)", e.label.c_str(), e.who.c_str());
        else
            log_line("SHOP", "!! %s : garde par %s, mais il n'a pas pu quitter ton rangement (%s) -- il est aussi chez toi",
                     e.label.c_str(), e.who.c_str(), why.c_str());
    }
    g.erases.clear();
}

// Hote : les achats notes suivent l'aventure mixte ; une autre (ou plus aucune) :
// oublies.
void ledger_pump() {
    uint32_t gen = 0, slots = 0;
    const bool on = team_run_slots(gen, slots);
    if (on && (!g.ledger_on || gen != g.ledger_gen)) {
        if (g.ledger.size()) log_line("SHOP", "nouvelle aventure mixte : %zu achat(s) de la precedente oublies", g.ledger.size());
        g.ledger.clear();
        g.ledger_on  = true;
        g.ledger_gen = gen;
    } else if (!on && g.ledger_on) {
        if (g.ledger.size()) log_line("SHOP", "fin de l'aventure mixte : %zu achat(s) notes oublies", g.ledger.size());
        g.ledger.clear();
        g.ledger_on = false;
    }
}

void host_pump(uint64_t now) {
    ledger_pump();
    // Les demandes que sa boutique ne prendra pas : il n'est plus dans cette case
    // (tout de suite), ou sa boutique ne tourne plus depuis longtemps (une fenetre
    // ouverte par-dessus la met en pause : on l'attend un moment).
    for (size_t i = 0; i < g.reqs.size();) {
        const Req r = g.reqs[i];
        const bool here  = follow_node_live() && follow_here_seed() == r.m.node;
        const bool shop  = here && (mm_scene_alive("Shop") || (g.host_shop_at && now - g.host_shop_at < kOpenMs));
        // Sa boutique de cette case ne s'est pas encore ouverte (un evenement qui
        // l'ouvre, une transition) : on l'attend aussi.
        const bool early = here && g.host_shop_node != r.m.node;
        if ((shop || early) ? now - r.at <= kReqWaitMs : now - r.at <= kReqGoneMs) {
            ++i;
            continue;
        }
        g.reqs.erase(g.reqs.begin() + (ptrdiff_t)i);
        ShopMsg done;
        done.op     = kShopDone;
        done.node   = r.m.node;
        done.index  = r.m.index;
        done.result = shop ? ShopResult::Busy : ShopResult::NoShop;
        ++g.refused;
        log_line("SHOP", "<- %s veut acheter dans une boutique %s : refuse", mm_peer_name(r.from).c_str(),
                 shop ? "en pause chez toi depuis trop longtemps" : "ou tu n'es plus");
        team_send(shop_encode(done), r.from);
    }
    drop_gone_gives();
    run_erases();
    if (!g.home_notes.empty() && mm_house() && !mm_info_busy()) {
        say(g.home_notes.front());
        g.home_notes.erase(g.home_notes.begin());
    }
}

// Invite : sa boutique ouverte ou fermee, dite a l'hote (qui ne sort pas sans le
// confirmer pendant qu'un invite achete encore).
void guest_tell_here(uint64_t now) {
    // Ouverte : vue dans cette case, et sa scene « Shop » encore la -- une fenetre
    // par-dessus met la boutique en pause (plus de Shop::update), elle n'en est pas
    // moins ouverte (relecture de la build 30).
    const bool     seen = g.my_shop_at && g.my_shop_node && g.my_shop_node == follow_here_seed();
    const bool     open = g.armed && guest_copy() && seen &&
                          (now - g.my_shop_at < kOpenMs || (follow_node_live() && mm_scene_alive("Shop")));
    const uint64_t node = open ? g.my_shop_node : g.here_node;
    if (open == g.here_open && node == g.here_node && !(open && now - g.here_at >= kHereBeatMs)) return;
    if (!open && !g.here_open) return;
    ShopMsg m;
    m.op   = kShopHere;
    m.node = node ? node : 1;
    m.open = open ? 1 : 0;
    g.here_at = now;                         // un envoi rate n'est pas retente a chaque image
    if (!team_send(shop_encode(m), kHostPeer)) return;
    if (open != g.here_open)
        log_line("SHOP", "-> ta boutique est %s (l'hote le sait)", open ? "ouverte" : "fermee");
    g.here_open = open;
    g.here_node = node;
}

void guest_pump(uint64_t now) {
    for (size_t i = g.pending.size(); i-- > 0;)
        if (now - g.pending[i].at >= kPendingMs) {
            log_line("SHOP", "!! l'hote n'a pas repondu a un achat -- tu peux recliquer");
            g.pending.erase(g.pending.begin() + (ptrdiff_t)i);
        }
    guest_tell_here(now);
    if (!guest_copy()) {
        g.replays.clear();
        return;
    }
    // Les achats d'une case ou il n'est plus (ou ne sera pas) : la poussee de
    // l'hote d'apres la case remet son sac.
    const uint64_t here = follow_here_seed();
    for (Replay& r : g.replays)
        if (r.m.node != here && follow_pending_count() == 0 && !r.left_at) r.left_at = now;
    while (!g.replays.empty()) {
        const Replay& r = g.replays.front();
        if (now - r.at < kReplayMaxMs && !(r.left_at && now - r.left_at > kReplayGoneMs)) break;
        log_line("SHOP", "achat de l'hote (%s) pas rejoue : tu n'es pas dans cette boutique", r.m.label.c_str());
        g.replays.pop_front();
    }
}

// Hote : les invites dont la boutique de cette case est ouverte (« Ami et Lou »).
std::string guests_still_here(uint64_t now, int& count) {
    count = 0;
    std::vector<std::string> names;
    const uint64_t node = follow_here_seed();
    const uint32_t run  = mm_run_peers_mask();
    for (uint8_t p = 1; p < 8; ++p) {
        const Here& h = g.here[p];
        if (!node || !peer_present(p)) continue;
        // Il y achete : sa boutique de cette case est ouverte.
        const bool shopping = h.open && h.node == node && now - h.at <= kHereStaleMs;
        // Il arrive encore : il joue l'aventure, n'a encore rien dit de cette case
        // (ni ouverte ni fermee), et l'hote vient d'y entrer -- un invite qui suit
        // l'hote ne l'a pas encore rejoint (son sac ouvert, sa transition).
        const bool arriving = (run & (1u << p)) && h.node != node && g.host_shop_node == node &&
                              now - g.host_shop_since <= kArriveMs;
        if (shopping || arriving) names.push_back(mm_peer_name(p));
    }
    count = (int)names.size();
    std::string out;
    for (size_t i = 0; i < names.size(); ++i) {
        std::string n = names[i];
        for (char& c : n)
            if (c == '[') c = '(';
            else if (c == ']') c = ')';
        out += (i == 0 ? "" : (i + 1 == names.size() ? " et " : ", ")) + n;
    }
    return out;
}

// Le compteur de pieces de la boutique montre la bourse de ce joueur, pas le
// compte de l'equipe : Shop::update y ecrit le compte a chaque image (le champ
// « coins » de son MenuPanel, Shop+0x40, 0x140797F9D), on le recrit juste apres.
// Sans bourse a lui : le compte du jeu, tel quel.
void show_purse(void* shop) {
    int64_t coins = 0, food = 0;
    if (!purse_mine(coins, food)) return;
    void* panel = nullptr;
    if (!mem_read((const char*)shop + kShop_Panel, &panel, sizeof(panel)) || !panel) return;
    char buf[24];
    snprintf(buf, sizeof(buf), "%lld", (long long)coins);
    mm_panel_set_text(panel, "coins", buf);
}

// --- les detours -------------------------------------------------------------------------------

void __fastcall h_ShopBuy(void* shop, void* item) {
    if (g.replaying || special_shop(shop)) {
        o_ShopBuy(shop, item);
        return;
    }
    if (guest_copy()) {                  // jamais chez lui : chez l'hote
        guest_ask(shop, item);
        return;
    }
    // Hote : avec des bourses, il paie avec la sienne (meme si les invites sont
    // deconnectes un moment) ; sans, le jeu achete et les copies rejouent.
    if (g.armed && is_host() && (purse_tracking() || host_synced())) {
        host_own_buy(shop, item);
        return;
    }
    o_ShopBuy(shop, item);
}

void __fastcall h_ShopUpdate(void* shop) {
    o_ShopUpdate(shop);
    if (!g.armed || special_shop(shop)) return;
    if (is_host() || guest_copy()) show_purse(shop);
    if (is_host()) {
        const uint64_t now  = GetTickCount64();
        const uint64_t node = follow_here_seed();
        // Une autre boutique : une autre case, ou un autre composant. (Pas un trou
        // dans les images : une fenetre par-dessus -- celle du « Sortir » retenu, le
        // sac, un bonbon -- met la boutique en pause, c'est la meme.)
        if (node != g.host_shop_node || shop != g.host_shop_ptr) {
            g.host_shop_node  = node;
            g.host_shop_ptr   = shop;
            g.host_shop_since = now;
            g.exit_asked_at   = 0;
            g.exit_closed_at  = 0;
        }
        g.host_shop_at = now;
        // La fenetre du « Sortir » retenu est refermee : la boutique tourne de nouveau.
        if (g.exit_asked_at && !g.exit_closed_at && !mm_info_busy()) g.exit_closed_at = now;
        while (!g.reqs.empty()) {
            const Req r = g.reqs.front();
            g.reqs.pop_front();
            host_take(shop, r);
        }
    } else if (guest_copy()) {
        g.my_shop_node = follow_here_seed();
        g.my_shop_at   = GetTickCount64();
        if (!g.replays.empty()) guest_replay(shop);
    }
}

} // namespace

// ---------------------------------------------------------------------------------------------

void shop_pump() {
    ++g.frame;
    if (!g.told) {
        g.told  = true;
        g.armed = hooks_is_live(T_ShopBuy) && hooks_is_live(T_ShopUpdate);
        if (g.armed)
            log_line("SHOP", "arme -- en aventure ensemble, l'invite achete chez l'hote avec sa bourse, et chacun "
                             "voit les achats dans sa boutique");
        else
            log_line("SHOP", "!! crochet %s absent : l'hote achete seul (avec la bourse de tous)%s",
                     hooks_is_live(T_ShopBuy) ? "SHOPUPD" : "SHOPBUY",
                     hooks_is_live(T_ShopBuy) ? ", l'invite ne peut pas acheter" : "");
    }
    const uint64_t now = GetTickCount64();
    if (is_host()) host_pump(now);
    else if (is_guest()) guest_pump(now);
}

void shop_on_team(uint8_t from, const uint8_t* data, size_t size) {
    ShopMsg m;
    if (!shop_decode(data, size, m)) {
        log_line("SHOP", "!! message de boutique illisible de %s -- ignore", mm_peer_name(from).c_str());
        return;
    }
    if (is_host()) {
        switch (m.op) {
        case kShopBuy:
            if (g.reqs.size() >= kMaxQueue) {
                log_line("SHOP", "!! trop de demandes d'achat en attente -- celle de %s est refusee",
                         mm_peer_name(from).c_str());
                ShopMsg done;
                done.op = kShopDone; done.node = m.node; done.index = m.index; done.result = ShopResult::Failed;
                team_send(shop_encode(done), from);
                return;
            }
            g.reqs.push_back(Req{ from, m, GetTickCount64() });
            return;
        case kShopGot:
            host_on_got(from, m);
            return;
        case kShopHere:
            if (from < 8) {
                const bool was = g.here[from].open;
                g.here[from] = Here{ m.node, m.open != 0, GetTickCount64() };
                if (was != g.here[from].open)
                    log_line("SHOP", "<- la boutique de %s est %s", mm_peer_name(from).c_str(),
                             m.open ? "ouverte" : "fermee");
            }
            return;
        default:
            log_line("SHOP", "!! message de boutique inattendu chez l'hote (%u) -- ignore", (unsigned)m.op);
            return;
        }
    }
    if (from != kHostPeer) return;
    switch (m.op) {
    case kShopDone:
        guest_on_done(m);
        return;
    case kShopSold:
        if (!guest_copy()) return;
        if (g.replays.size() >= kMaxQueue) g.replays.pop_front();
        g.replays.push_back(Replay{ m, GetTickCount64(), 0 });
        return;
    case kShopGive:
        guest_on_give(m);
        return;
    default:
        log_line("SHOP", "!! message de boutique inattendu (%u) -- ignore", (unsigned)m.op);
        return;
    }
}

void shop_victory_before() {
    g.snap = false;
    g.snap_owned.clear();
    g.snap_owners.clear();
    g.claimed.clear();
    if (!is_host() || !g.ledger.size()) return;
    g.snap       = true;
    g.snap_owned = g.ledger.all();
    g.snap_seq   = team_run_seq();
    for (const ShopOwned& o : g.snap_owned) {
        bool known = false;
        for (const Owner& w : g.snap_owners) known = known || w.slot == o.slot;
        if (known) continue;
        Owner w;
        w.slot = o.slot;
        w.peer = o.slot == kHostPeer ? (int)kHostPeer : team_slot_peer(o.slot);
        w.name = o.slot == kHostPeer ? std::string() : team_slot_name(o.slot);
        g.snap_owners.push_back(w);
    }
    log_line("SHOP", "victoire : %zu achat(s) note(s) dans l'aventure", g.snap_owned.size());
}

bool shop_storage_refs() { return net_role() != NetRole::Client && (!g.gives.empty() || !g.erases.empty()); }

bool shop_claims(uint64_t id) {
    return std::find(g.claimed.begin(), g.claimed.end(), id) != g.claimed.end();
}

void shop_victory_after() {
    if (!g.snap) return;
    // Achetes, ils ne defilent pas (mm_loot, juste apres) -- meme ceux qui restent
    // chez l'hote.
    for (const ShopOwned& o : g.snap_owned) g.claimed.push_back(o.id);
    std::vector<LootStoredItem> store;
    if (!loot_storage_items(store)) {
        log_line("SHOP", "!! victoire : rangement illisible -- tous les achats restent chez toi");
        g.snap = false;
        return;
    }
    std::string note;
    std::vector<uint64_t> stored_ids;
    for (const LootStoredItem& s : store) stored_ids.push_back(s.id);
    const std::vector<int> match = shop_match_owned(g.snap_owned, stored_ids);   // un exemplaire par achat
    for (const Owner& w : g.snap_owners) {
        std::vector<const LootStoredItem*> mine;
        std::vector<std::string> labels;
        for (size_t i = 0; i < g.snap_owned.size(); ++i) {
            const ShopOwned& o = g.snap_owned[i];
            if (o.slot != w.slot || match[i] < 0) continue;
            const LootStoredItem& s = store[(size_t)match[i]];
            // Un nom ou un texte que le message couperait : il reste chez l'hote
            // (comme au defile) -- coupe, ce ne serait plus le meme objet.
            if (w.slot != kHostPeer && (s.f.name.size() > kShopMaxName || s.f.text.size() > kShopMaxText)) {
                log_line("SHOP", "!! %s : nom ou texte trop long pour le message -- il reste chez toi",
                         s.f.name.substr(0, 40).c_str());
                continue;
            }
            mine.push_back(&s);
            labels.push_back(purse_safe(o.label.empty() ? loot_item_label(s.f) : o.label));
        }
        if (mine.empty()) continue;
        std::string list;
        for (size_t i = 0; i < labels.size(); ++i) list += (i ? ", " : "") + labels[i];
        const std::string who = purse_safe(w.name);
        if (w.slot == kHostPeer) {
            log_line("SHOP", "victoire : tes achats restent chez toi (%s)", list.c_str());
            continue;
        }
        if (w.peer < 0 || !in_session() || !peer_present((uint8_t)w.peer)) {
            log_line("SHOP", "victoire : %s n'est plus la -- ses achats restent chez toi (%s)", w.name.c_str(), list.c_str());
            note += fmt(u8"\n%s n'est plus là : ses achats restent chez toi (%s).", who.c_str(), list.c_str());
            continue;
        }
        bool sent_all = true;
        for (size_t first = 0; first < mine.size(); first += kShopMaxItems) {
            ShopMsg m;
            m.op   = kShopGive;
            m.seq  = g.snap_seq;
            m.gift = next_gift_id();
            Give gv;
            gv.gift = m.gift;
            gv.dir  = director();
            gv.peer = (uint8_t)w.peer;
            gv.who  = w.name;
            gv.at   = GetTickCount64();
            for (size_t i = first; i < mine.size() && i < first + kShopMaxItems; ++i) {
                ShopGiveItem it;
                it.f     = mine[i]->f;
                it.label = labels[i];
                m.items.push_back(it);
                gv.ids.push_back(mine[i]->id);
                gv.digests.push_back(item_digest(mine[i]->f));
                gv.labels.push_back(labels[i]);
            }
            if (!team_send(shop_encode(m), w.peer)) {
                log_line("SHOP", "!! victoire : les achats de %s n'ont pas pu partir -- ils restent chez toi", w.name.c_str());
                sent_all = false;
                continue;
            }
            log_line("SHOP", "-> victoire : les achats de %s partent chez lui (%zu objet(s)), en attente de sa reponse",
                     w.name.c_str(), gv.ids.size());
            g.gives.push_back(std::move(gv));
        }
        if (sent_all)
            note += fmt(u8"\n%s rapporte ses achats : %s.", who.c_str(), list.c_str());
        else
            note += u8"\nLes achats " + purse_of(w.name) + u8" n'ont pas pu lui être envoyés : ils restent chez toi.";
    }
    if (!note.empty()) g.home_notes.push_back(u8"Achats de l'aventure\n" + note);
    g.snap = false;
}

bool shop_host_inside() {
    if (!g.armed || !is_host() || !g.host_shop_node) return false;
    if (!follow_node_live() || follow_here_seed() != g.host_shop_node) return false;
    // Sa scene « Shop » est la, meme en pause : un bonbon ouvre la montee de niveau
    // par-dessus et met la boutique en pause (0x14079C96F) -- plus de Shop::update
    // tant que l'hote choisit (relecture de la build 30).
    return mm_scene_alive("Shop") || (g.host_shop_at && GetTickCount64() - g.host_shop_at < kInsideMs);
}

bool shop_on_click(void* button) {
    if (!g.armed || !button || !is_host() || !in_session()) return false;
    char name[64] = {};
    if (!mem_read_std_string((const char*)button + kBtnName, name, sizeof(name))) return false;
    if (strcmp(name, "Shop_ExitButton") != 0) return false;
    if (!shop_host_inside()) return false;    // une boutique de maison, une salle au tresor
    const uint64_t now = GetTickCount64();
    int count = 0;
    const std::string names = guests_still_here(now, count);
    if (!count) return false;
    // Le second clic : dans les 10 s apres la fermeture de la fenetre (le temps de
    // la lire ne compte pas -- elle met la boutique en pause).
    if (g.exit_asked_at && (!g.exit_closed_at || now - g.exit_closed_at < kExitConfirmMs)) {
        log_line("SHOP", "l'hote sort de la boutique quand meme (%s n'a pas fini)", names.c_str());
        g.exit_asked_at  = 0;
        g.exit_closed_at = 0;
        return false;
    }
    g.exit_asked_at  = now;
    g.exit_closed_at = 0;
    log_line("SHOP", "« Sortir » retenu : %s achete encore dans cette boutique", names.c_str());
    say(shop_wait_text(names, count > 1));
    return true;
}

bool shop_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_ShopBuy:    *detour = (void*)&h_ShopBuy;    *original = (void**)&o_ShopBuy;    return true;
    case T_ShopUpdate: *detour = (void*)&h_ShopUpdate; *original = (void**)&o_ShopUpdate; return true;
    default: return false;
    }
}

} // namespace mgmp
