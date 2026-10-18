// mm_purse -- Mewgenics Multi : les bourses de l'aventure. Voir mm_purse.h.

#include "mm_purse.h"
#include "mm_purse_core.h"
#include "mm_gift.h"
#include "mm_loot.h"
#include "mm_menu.h"
#include "mm_shop.h"
#include "mm_team.h"
#include "mm_team_merge.h"

#include "mgmp_addresses.h"
#include "mgmp_catsync.h"
#include "mgmp_follow.h"
#include "mgmp_hooks.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"

#include <windows.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_purse.h ----------------------------------------------

constexpr uintptr_t kEvt_Subject  = 0x128;   // WorldEvent : le chat de l'evenement (CatData*)
constexpr uintptr_t kCatData_Id   = 0x00;    // CatData : son id
constexpr uintptr_t kChar_CatData = 0x88;    // Character : son CatData (nul pour un non-chat)
constexpr uintptr_t kPassive_Unit = 0x38;    // un passif de combat : le Character qui le porte

// --- les delais ----------------------------------------------------------------------

constexpr uint64_t kLedgerGapMs   = 300;     // hote : les bourses, au plus souvent quand elles changent
constexpr uint64_t kLedgerBeatMs  = 10000;   // ... et meme sans changement
constexpr uint64_t kLedgerStaleMs = 30000;   // invite : rien recu depuis, il ne les montre plus
constexpr uint64_t kFinalWindowMs = 60000;   // invite : la fin de sa copie et celle de l'hote, au plus a ce temps
constexpr uint64_t kFindMaxAgeMs  = 300000;  // une trouvaille jamais montree : 5 minutes sur la carte
constexpr size_t   kMaxFinds      = 8;       // trouvailles en attente d'etre dites
constexpr size_t   kShownFinds    = 2;       // ... dans une fenetre du vote
constexpr int      kMaxCtx        = 8;       // appels du jeu imbriques suivis (voir Ctx)
constexpr uint64_t kGiveAnswerMs  = 10000;   // invite : la reponse de l'hote a un don, au plus

using FnGain      = void (__fastcall*)(void* world_event, void* args);
using FnVictory   = void (__fastcall*)(void* director);
// Les rappels du jeu dont on ne connait pas tout : on passe les quatre registres
// d'arguments et la valeur rendue, sans rien supposer de plus.
using FnPass      = uint64_t (__fastcall*)(void* a1, void* a2, void* a3, void* a4);
using FnUnitCoins = void (__fastcall*)(void* unit, int amount);
FnGain      o_GainCoins   = nullptr;
FnGain      o_GainFood    = nullptr;
FnVictory   o_RunVictory  = nullptr;
FnPass      o_EventResult = nullptr;   // WorldEvent::setupResult(WorldEvent*)
FnUnitCoins o_UnitCoins   = nullptr;   // Unit::GainCoins(Character*, int)
FnPass      o_EatFood     = nullptr;   // HealthPickup (objet, Character*, multiplicateur)
FnPass      o_BonusFood   = nullptr;   // passif BonusFoodEachBattle (passif)

// Une trouvaille a dire, dans la fenetre du vote du chemin.
struct Find {
    int         owner = -1;          // son joueur, numero de maintenant (-1 : parti)
    std::string owner_name;
    int32_t     coins = 0;
    int32_t     food  = 0;
    std::string cat;                 // vide en combat
    uint8_t     flags = 0;           // kFindSpent, kFindBattle
    uint64_t    at    = 0;
    bool        shown = false;       // deja dans une fenetre du vote
};

// Un appel du jeu suivi, en cours (un crochet) : ce qui change le compte pendant
// lui, hors des appels suivis imbriques, est au chat qu'il nomme. Les appels
// s'imbriquent : setupResult appelle gain_coins ; Unit::GainCoins fait jouer les
// OnGainCoins des passifs, qui peuvent rappeler Unit::GainCoins pour un autre
// chat ; HealthPickup soigne, et un soin peut faire gagner des pieces.
enum CtxKind : uint8_t { kCtxCommand = 0, kCtxResult = 1, kCtxBattle = 2, kCtxShop = 3 };
struct Ctx {
    void*    cat  = nullptr;         // son CatData (pour son nom ; nul en combat)
    uint64_t id   = 0;               // son id (0 : pas de chat -- partage)
    uint8_t  kind = kCtxCommand;
    uint8_t  slot = 0;               // kCtxShop : le joueur qui achete (numero de place)
    std::string label;               // kCtxShop : ce qu'il achete
};

struct State {
    bool told   = false;             // les crochets, dits une fois dans le journal
    bool armed  = false;             // les quatre crochets des evenements et de la fin sont la
    bool battle = false;             // ... et ceux des combats (sinon : partage)

    // Hote : l'aventure mixte suivie.
    bool      tracking   = false;
    bool      ended      = false;    // sa fin (gagnee) est passee : plus rien a suivre
    uint32_t  gen        = 0;        // team_run_slots
    uint32_t  slots      = 0;
    PurseBook book;
    int64_t   seen_coins = 0;        // le compte de l'aventure, tel que vu la derniere fois
    int64_t   seen_food  = 0;
    bool      dirty      = false;
    uint64_t  sent_at    = 0;
    uint32_t  roster_sent = 0;
    uint32_t  finds_made = 0;
    std::vector<std::string> home_notes;   // la fin, une fois dans sa maison
    // Les appels suivis en cours, du plus exterieur au plus interieur.
    Ctx       ctx[kMaxCtx];
    int       depth      = 0;

    // Invite : les bourses recues de l'hote.
    bool      have       = false;
    uint8_t   have_gen   = 0;
    uint64_t  have_at    = 0;
    PurseLine lines[kPurseMaxLines];
    uint8_t   n_lines    = 0;
    bool      final_set  = false;    // ce qu'il rapporte (a = 3), pour l'ecran de fin de sa copie
    int32_t   final_coins = 0;
    int32_t   final_food  = 0;
    uint64_t  final_at   = 0;
    uint64_t  victory_at = 0;        // la fin de sa copie
    // Son don a un autre joueur (protocole 62) : un a la fois, jusqu'a la reponse.
    bool      give_pending = false;
    uint8_t   give_seq     = 0;      // jamais 0 (0 : un don de l'hote)
    uint64_t  give_at      = 0;

    // Les deux.
    std::vector<Find> finds;
    bool      was_live = false;      // follow_node_live a l'image d'avant
};
State g;

// --- petits outils ----------------------------------------------------------------------

int popcount(uint32_t m) {
    int n = 0;
    for (; m; m &= m - 1) ++n;
    return n;
}

int32_t clamp32(int64_t v) {
    if (v > INT32_MAX) return INT32_MAX;
    if (v < INT32_MIN) return INT32_MIN;
    return (int32_t)v;
}

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

void* director() {
    const void** slot = (const void**)addr_of_data(D_MewDirectorPtr);
    void* d = nullptr;
    if (!slot || !mem_read(slot, &d, sizeof(d))) return nullptr;
    return d;
}

void* inventory() {
    void* d = director();
    void* inv = nullptr;
    if (!d || !mem_read((const char*)d + kDir_Inventory, &inv, sizeof(inv))) return nullptr;
    return inv;
}

// Le compte de l'aventure : ses pieces et sa nourriture.
bool read_run(int64_t& coins, int64_t& food) {
    void* inv = inventory();
    int32_t c = 0, f = 0;
    if (!inv || !mem_read((const char*)inv + kInv_Coins, &c, sizeof(c)) ||
        !mem_read((const char*)inv + kInv_Food, &f, sizeof(f)))
        return false;
    coins = c;
    food  = f;
    return true;
}

// Les deux, ou aucun : la seconde ecriture ratee remet la premiere.
bool write_run(int64_t coins, int64_t food) {
    void* inv = inventory();
    int32_t old_c = 0;
    if (!inv || !mem_read((const char*)inv + kInv_Coins, &old_c, sizeof(old_c))) return false;
    const int32_t c = clamp32(coins < 0 ? 0 : coins);
    const int32_t f = clamp32(food < 0 ? 0 : food);
    if (!mem_write((char*)inv + kInv_Coins, &c, sizeof(c))) return false;
    if (!mem_write((char*)inv + kInv_Food, &f, sizeof(f))) {
        mem_write((char*)inv + kInv_Coins, &old_c, sizeof(old_c));
        return false;
    }
    return true;
}

// Le nom d'un chat (CatData*), tel que le jeu le serialise.
std::string cat_name(void* cat) {
    std::vector<uint8_t> raw;
    CatInfo info;
    if (cat && catsync_serialize_cat(cat, raw) && cat_info(raw, info) && !info.name.empty()) return info.name;
    return u8"Un chat";
}

// L'id du chat d'un Character (0 : pas un chat).
uint64_t unit_cat_id(void* unit) {
    void*    cat = nullptr;
    uint64_t id  = 0;
    if (!unit || !mem_read((const char*)unit + kChar_CatData, &cat, sizeof(cat)) || !cat) return 0;
    if (!mem_read((const char*)cat + kCatData_Id, &id, sizeof(id))) return 0;
    return id;
}

bool own_line(uint8_t self, int64_t& coins, int64_t& food) {
    for (uint8_t i = 0; i < g.n_lines; ++i)
        if (g.lines[i].peer == self) {
            coins = g.lines[i].coins;
            food  = g.lines[i].food;
            return true;
        }
    return false;
}

// Une trouvaille de plus : ajoutee a celle du meme chat (ou des memes chats en
// combat) pour le meme joueur, si elle n'a pas encore ete montree.
void push_find(Find f) {
    f.at = GetTickCount64();
    for (Find& o : g.finds)
        if (!o.shown && !(f.flags & kFindShop) && o.owner == f.owner && o.owner_name == f.owner_name &&
            o.flags == f.flags && o.cat == f.cat) {
            o.coins = clamp32((int64_t)o.coins + f.coins);
            o.food  = clamp32((int64_t)o.food + f.food);
            o.at    = f.at;
            return;
        }
    if (g.finds.size() >= kMaxFinds) g.finds.erase(g.finds.begin());
    g.finds.push_back(std::move(f));
}

// --- hote : les comptes -------------------------------------------------------------------

// Les bourses, a tous : celles des joueurs la, par leur numero de maintenant.
void host_send(bool force) {
    if (!in_session()) return;
    const uint64_t now    = GetTickCount64();
    const uint32_t roster = roster_mask();
    const bool due = force || roster != g.roster_sent || now - g.sent_at >= kLedgerBeatMs ||
                     (g.dirty && now - g.sent_at >= kLedgerGapMs);
    if (!due) return;
    PurseLine lines[kPurseMaxLines];
    uint8_t   n = 0;
    for (uint8_t s = 0; s < kPurseSlots && n < kPurseMaxLines; ++s) {
        if (!(g.slots & (1u << s))) continue;
        const int peer = team_slot_peer(s);
        if (peer < 0 || peer >= kPurseSlots) continue;
        lines[n].peer  = (uint8_t)peer;
        lines[n].coins = clamp32(g.book.at(s).coins);
        lines[n].food  = clamp32(g.book.at(s).food);
        ++n;
    }
    if (!n) return;
    MmMsg m;
    purse_encode_lines(m, kPurseLedger, (uint8_t)g.gen, lines, n, clamp32(g.seen_coins), clamp32(g.seen_food));
    g.sent_at = now;                         // un envoi rate n'est pas retente a chaque image
    if (net_send_mm(m)) {
        g.dirty       = false;
        g.roster_sent = roster;
    }
}

// Un changement du compte qui vient d'un chat (son evenement, ou ce qu'il
// ramasse en combat) : a la bourse de son proprietaire (pas de chat : partage),
// et dit a tous.
void attribute(void* cat, uint64_t id, int64_t dc, int64_t df, uint8_t flags, const char* what) {
    if ((!dc && !df) || g.ended) return;
    g.dirty = true;
    if (!id) {
        g.book.share(dc, df);
        log_line("PURSE", "%s sans chat : %+lld piece(s), %+lld nourriture(s), partage", what, (long long)dc,
                 (long long)df);
        return;
    }
    const uint8_t slot = team_slot_of_cat(id);
    g.book.credit(slot, dc, df);
    ++g.finds_made;
    Find f;
    f.owner      = team_slot_peer(slot);
    f.owner_name = team_slot_name(slot);
    f.coins      = clamp32(dc);
    f.food       = clamp32(df);
    f.flags      = flags;
    if (!(flags & kFindBattle)) f.cat = cat_name(cat);
    log_line("PURSE", "%s : %s (chat %llu) %+lld piece(s), %+lld nourriture(s) -> bourse de %s (%lld et %lld)",
             what, (flags & kFindBattle) ? "en combat" : f.cat.c_str(), (unsigned long long)id, (long long)dc,
             (long long)df, f.owner_name.c_str(), (long long)g.book.at(slot).coins, (long long)g.book.at(slot).food);
    // A tous : la trouvaille (si son joueur est la), puis les bourses a jour.
    if (in_session() && f.owner >= 0 && f.owner < kPurseSlots) {
        MmMsg m;
        purse_encode_find(m, (uint8_t)f.owner, f.coins, f.food, f.cat, flags);
        if (!net_send_mm(m)) log_line("PURSE", "!! envoi de la trouvaille impossible");
    }
    push_find(std::move(f));
    host_send(true);
}

// Un achat en boutique (mm_shop) : ce qu'il change au compte va a la bourse de
// l'acheteur, et chacun le voit.
void attribute_shop(uint8_t slot, const std::string& label, int64_t dc, int64_t df) {
    if ((!dc && !df) || g.ended) return;
    g.dirty = true;
    g.book.credit(slot, dc, df);
    ++g.finds_made;
    Find f;
    f.owner      = team_slot_peer(slot);
    f.owner_name = team_slot_name(slot);
    f.coins      = clamp32(dc);
    f.food       = clamp32(df);
    f.flags      = kFindShop;
    f.cat        = label;
    log_line("PURSE", "achat de %s : %+lld piece(s), %+lld nourriture(s) -> bourse de %s (%lld et %lld)",
             label.c_str(), (long long)dc, (long long)df, f.owner_name.c_str(), (long long)g.book.at(slot).coins,
             (long long)g.book.at(slot).food);
    if (in_session() && f.owner >= 0 && f.owner < kPurseSlots) {
        MmMsg m;
        purse_encode_find(m, (uint8_t)f.owner, f.coins, f.food, f.cat, kFindShop);
        if (!net_send_mm(m)) log_line("PURSE", "!! envoi de l'achat impossible");
    }
    push_find(std::move(f));
    host_send(true);
}

// Ce qui a change depuis la derniere fois : au chat de l'appel suivi en cours le
// plus interieur ; hors de tout appel suivi, partage.
void take_pending(const char* what) {
    if (g.ended) return;
    int64_t c = 0, f = 0;
    if (!read_run(c, f)) return;
    const int64_t dc = c - g.seen_coins, df = f - g.seen_food;
    if (!dc && !df) return;
    g.seen_coins = c;
    g.seen_food  = f;
    if (g.depth > 0) {
        const Ctx& t = g.ctx[g.depth - 1];
        if (t.kind == kCtxShop) {
            attribute_shop(t.slot, t.label, dc, df);
            return;
        }
        const uint8_t flags = t.kind == kCtxBattle                             ? kFindBattle
                            : (t.kind == kCtxResult && dc <= 0 && df <= 0)     ? kFindSpent
                                                                               : 0;
        attribute(t.cat, t.id, dc, df, flags, what);
        return;
    }
    g.book.share(dc, df);
    g.dirty = true;
    log_line("PURSE", "partage (%s) : %+lld piece(s), %+lld nourriture(s) (compte : %lld et %lld)", what,
             (long long)dc, (long long)df, (long long)c, (long long)f);
}

void host_pump() {
    uint32_t gen = 0, slots = 0;
    if (!team_run_slots(gen, slots)) {
        if (g.tracking)
            log_line("PURSE", "plus d'aventure mixte : bourses oubliees (%u trouvaille(s) dans celle-ci)",
                     g.finds_made);
        g.tracking = false;
        g.ended    = false;
        g.finds.clear();
        return;
    }
    if (!g.tracking || gen != g.gen) {
        if (g.tracking && g.ended && gen == g.gen) return;
        // Le debut : la premiere image sur la carte. Les provisions du depart
        // sont alors dans le compte ; elles sont a tous.
        if (!follow_on_map()) return;
        int64_t c = 0, f = 0;
        if (!read_run(c, f)) return;
        g.book.reset();
        g.book.set_players(slots, kHostPeer);
        g.book.share(c, f);
        g.tracking    = true;
        g.ended       = false;
        g.gen         = gen;
        g.slots       = g.book.players();
        g.seen_coins  = c;
        g.seen_food   = f;
        g.dirty       = true;
        g.sent_at     = 0;
        g.roster_sent = 0;
        g.finds_made  = 0;
        g.finds.clear();
        g.home_notes.clear();
        log_line("PURSE", "aventure mixte %u : une bourse par joueur qui a amene des chats (%d%s) ; au depart "
                          "%lld piece(s) et %lld nourriture(s), partagees", gen, popcount(g.slots),
                 (g.slots & (1u << kHostPeer)) ? ", l'hote compris" : ", l'hote n'a pas de chat", (long long)c,
                 (long long)f);
        host_send(true);
        return;
    }
    if (g.ended) return;
    take_pending("compte");
    host_send(false);
}

bool tracking_host() { return g.armed && g.tracking && !g.ended && net_role() != NetRole::Client; }

// --- les dons entre bourses (protocole 62) ----------------------------------------------

// Hote : le numero de place d'un joueur la (son numero de maintenant), -1 : aucun.
int slot_of_peer(uint8_t peer) {
    for (uint8_t s = 0; s < kPurseSlots; ++s)
        if (team_slot_peer(s) == (int)peer) return s;
    return -1;
}

// Hote : ce joueur a une bourse dans cette aventure (il a amene des chats ; l'hote,
// toujours : il garde le reste a la fin).
bool slot_has_purse(int s) {
    return s >= 0 && s < (int)kPurseSlots && (s == (int)kHostPeer || (g.slots & (1u << s)) != 0);
}

// Hote : un don de la bourse de `giver` a celle de `to` (numeros de maintenant).
// 0 : fait (les bourses partent ensuite) ; sinon la raison (kPurseNo*).
uint8_t host_transfer(uint8_t giver, uint8_t to, int64_t coins, int64_t food) {
    if (!tracking_host()) return kPurseNoRun;
    const int fs = slot_of_peer(giver), ts = slot_of_peer(to);
    if (!slot_has_purse(fs) || !slot_has_purse(ts) || fs == ts) return kPurseNoPurse;
    take_pending("avant un don");            // le compte a jour d'abord
    if (!g.book.transfer((uint8_t)fs, (uint8_t)ts, coins, food)) return kPurseNoEnough;
    g.dirty = true;
    log_line("PURSE", "don : %s donne %lld piece(s) et %lld nourriture(s) a %s (sa bourse : %lld et %lld ; "
                      "l'autre : %lld et %lld)", team_slot_name((uint8_t)fs).c_str(), (long long)coins,
             (long long)food, team_slot_name((uint8_t)ts).c_str(), (long long)g.book.at((uint8_t)fs).coins,
             (long long)g.book.at((uint8_t)fs).food, (long long)g.book.at((uint8_t)ts).coins,
             (long long)g.book.at((uint8_t)ts).food);
    return 0;
}

// Hote : la demande d'un invite (MM_PURSE a = 4). Le donneur est celui dont vient
// le message, pas un champ du message.
void host_on_give_req(uint8_t from, const MmMsg& m) {
    uint8_t to = 0, seq = 0;
    int32_t coins = 0, food = 0;
    if (from >= kPurseSlots || !purse_decode_give_req(m, to, coins, food, seq)) {
        log_line("PURSE", "!! demande de don illisible (joueur %u) -- ignoree", (unsigned)from);
        return;
    }
    const uint8_t why = to == from ? kPurseNoPurse : host_transfer(from, to, coins, food);
    MmMsg r;
    if (why) {
        purse_encode_give_no(r, from, why, seq);
        log_line("PURSE", "don refuse : %s voulait donner %d piece(s) et %d nourriture(s) a %s (raison %u)",
                 mm_peer_name(from).c_str(), coins, food, mm_peer_name(to).c_str(), (unsigned)why);
        if (!net_send_mm(r)) log_line("PURSE", "!! envoi du refus impossible");
        return;
    }
    purse_encode_given(r, from, to, coins, food, seq);
    if (!net_send_mm(r)) log_line("PURSE", "!! envoi du don impossible (les bourses suivent quand meme)");
    host_send(true);
    if (to == kHostPeer)
        gift_purse_note(purse_given_line(false, true, mm_peer_name(from), "", coins, food), false);
}

// Un appel du jeu suivi : ce qui a change avant lui va a l'appel qui l'entoure
// (ou est partage), ce qui change pendant lui -- hors de ses appels suivis
// imbriques, qui font de meme -- va au chat qu'il nomme.
template <typename Call>
void in_ctx(void* cat, uint64_t id, uint8_t kind, const char* what, Call call) {
    take_pending("avant un appel suivi");
    if (g.depth >= kMaxCtx) {                // trop profond : a l'appel qui l'entoure
        call();
        take_pending(what);
        return;
    }
    Ctx& c = g.ctx[g.depth++];
    c.cat  = cat;
    c.id   = id;
    c.kind = kind;
    call();
    take_pending(what);
    --g.depth;
}

// Un achat (mm_shop) : comme in_ctx, a la bourse du joueur `slot` plutot qu'a
// celle du proprietaire d'un chat.
void in_shop_ctx(uint8_t slot, const std::string& label, void (*call)(void*), void* arg) {
    take_pending("avant un achat");
    if (g.depth >= kMaxCtx) {
        call(arg);
        take_pending("achat (trop profond)");
        return;
    }
    Ctx& c = g.ctx[g.depth++];
    c.cat   = nullptr;
    c.id    = 0;
    c.kind  = kCtxShop;
    c.slot  = slot;
    c.label = label;
    call(arg);
    take_pending("achat");
    --g.depth;
}

// Le chat de l'evenement maintenant (WorldEvent+0x128) et son id.
void event_subject(void* evt, void*& cat, uint64_t& id) {
    cat = nullptr;
    id  = 0;
    if (evt) mem_read((const char*)evt + kEvt_Subject, &cat, sizeof(cat));
    if (cat) mem_read((const char*)cat + kCatData_Id, &id, sizeof(id));
}

// gain_coins / gain_food : ce que la commande fait au compte va au proprietaire
// du chat de l'evenement a ce moment-la (rien : deja 99 pieces, ou rien a perdre).
void gain(FnGain orig, void* evt, void* args, const char* what) {
    if (!orig) return;
    if (!tracking_host()) {
        orig(evt, args);
        return;
    }
    void*    cat = nullptr;
    uint64_t id  = 0;
    event_subject(evt, cat, id);
    in_ctx(cat, id, kCtxCommand, what, [&] { orig(evt, args); });
}

void __fastcall h_GainCoins(void* evt, void* args) { gain(o_GainCoins, evt, args, "gain_coins"); }
void __fastcall h_GainFood(void* evt, void* args) { gain(o_GainFood, evt, args, "gain_food"); }

// setupResult : le resultat du choix (son prix en pieces, puis ses effets par
// WorldEvent::spin). Ce qui change le compte pendant ce temps, hors gain_coins /
// gain_food (qui suivent le chat du moment), va au proprietaire du chat de
// l'evenement.
uint64_t __fastcall h_EventResult(void* evt, void* a2, void* a3, void* a4) {
    if (!o_EventResult) return 0;
    if (!tracking_host()) return o_EventResult(evt, a2, a3, a4);
    void*    cat = nullptr;
    uint64_t id  = 0;
    event_subject(evt, cat, id);
    uint64_t r = 0;
    in_ctx(cat, id, kCtxResult, "resultat d'un evenement", [&] { r = o_EventResult(evt, a2, a3, a4); });
    return r;
}

// En combat : ce qu'un chat ramasse (Unit::GainCoins, HealthPickup, le passif
// BonusFoodEachBattle) va a son proprietaire ; un non-chat, partage. Le chat est
// lu avant l'appel.
template <typename Call>
void unit_gain(void* unit, const char* what, Call call) {
    if (!g.battle || !tracking_host()) {
        call();
        return;
    }
    in_ctx(nullptr, unit_cat_id(unit), kCtxBattle, what, call);
}

void __fastcall h_UnitCoins(void* unit, int amount) {
    if (!o_UnitCoins) return;
    // Unit::GainCoins sert aussi aux pertes (un montant negatif) : « en combat »,
    // pas « ramassees ».
    unit_gain(unit, "pieces en combat", [&] { o_UnitCoins(unit, amount); });
}

uint64_t __fastcall h_EatFood(void* pickup, void* unit, void* a3, void* a4) {
    if (!o_EatFood) return 0;
    uint64_t r = 0;
    unit_gain(unit, "nourriture ramassee", [&] { r = o_EatFood(pickup, unit, a3, a4); });
    return r;
}

uint64_t __fastcall h_BonusFood(void* passive, void* a2, void* a3, void* a4) {
    if (!o_BonusFood) return 0;
    void* unit = nullptr;
    if (passive) mem_read((const char*)passive + kPassive_Unit, &unit, sizeof(unit));
    uint64_t r = 0;
    unit_gain(unit, "nourriture du combat", [&] { r = o_BonusFood(passive, a2, a3, a4); });
    return r;
}

// L'aventure est gagnee : chacun recoit sa bourse, l'hote garde le reste.
void host_victory() {
    if (!tracking_host()) return;
    take_pending("fin de l'aventure");
    g.ended = true;
    int64_t c = 0, f = 0;
    if (!read_run(c, f)) {
        log_line("PURSE", "!! fin de l'aventure : compte illisible -- tout reste a l'hote");
        return;
    }
    uint32_t paid = 0;
    for (uint8_t s = 0; s < kPurseSlots; ++s)
        if (s != kHostPeer && (g.slots & (1u << s)) && team_slot_peer(s) >= 0) paid |= 1u << s;
    const PursePayout p = g.book.settle(paid, c, f);
    int64_t     out_c = 0, out_f = 0;
    unsigned    sent  = 0;
    std::string note  = u8"Fin de l'aventure\n\nChacun rapporte sa bourse dans sa maison :";
    for (uint8_t s = 0; s < kPurseSlots; ++s) {
        if (s == kHostPeer || !(g.slots & (1u << s))) continue;
        const std::string who = purse_safe(team_slot_name(s));
        if (!(paid & (1u << s))) {
            note += "\n" + who + u8" n'est plus là : sa bourse reste chez toi.";
            log_line("PURSE", "fin : %s n'est plus la -- sa bourse (%lld et %lld) reste a l'hote", who.c_str(),
                     (long long)g.book.at(s).coins, (long long)g.book.at(s).food);
            continue;
        }
        if (!p.coins[s] && !p.food[s]) {
            note += "\n" + who + u8" : rien.";
            continue;
        }
        // Un message a lui seul : retire du compte d'abord (le jeu vide ensuite
        // le reste dans la maison de l'hote), remis si l'envoi rate.
        const int peer = team_slot_peer(s);
        PurseLine line;
        line.peer  = (uint8_t)peer;
        line.coins = p.coins[s];
        line.food  = p.food[s];
        MmMsg m;
        purse_encode_lines(m, kPursePayout, team_run_seq(), &line, 1, 0, 0);
        uint8_t  buf[512];
        const uint32_t len = enc_mm(buf, sizeof(buf), m);
        bool ok = len && write_run(c - out_c - line.coins, f - out_f - line.food);
        if (ok && !net_send_peer((uint8_t)peer, buf, len)) {
            write_run(c - out_c, f - out_f);
            ok = false;
        }
        if (!ok) {
            note += "\n" + who + u8" n'a pas pu recevoir sa bourse : elle reste chez toi.";
            log_line("PURSE", "!! fin : la bourse de %s (%d et %d) n'a pas pu partir -- elle reste a l'hote",
                     who.c_str(), line.coins, line.food);
            continue;
        }
        out_c += line.coins;
        out_f += line.food;
        ++sent;
        note += "\n" + who + " : " + purse_amount_text(line.coins, line.food) + ".";
    }
    note += u8"\nToi : " + purse_amount_text(c - out_c, f - out_f) + ".";
    g.home_notes.push_back(note);
    log_line("PURSE", "fin de l'aventure : %u invite(s) payes, %lld piece(s) et %lld nourriture(s) parties chez "
                      "eux ; l'hote rapporte %lld et %lld", sent, (long long)out_c, (long long)out_f,
             (long long)(c - out_c), (long long)(f - out_f));
}

// Invite, dans la copie : l'ecran de fin montre ce que CE joueur rapporte.
void guest_victory() {
    if (!g.armed || !savefile_in_coop()) return;
    const uint64_t now = GetTickCount64();
    int64_t mine_c = 0, mine_f = 0;
    if (g.final_set && now - g.final_at <= kFinalWindowMs) {
        mine_c = g.final_coins;              // l'hote a fini avant : ce qu'il a envoye
        mine_f = g.final_food;
    } else if (g.have && now - g.have_at <= kLedgerStaleMs) {
        own_line(net_self(), mine_c, mine_f);   // pas de ligne : il n'a pas amene de chat
    } else {
        g.final_set = false;
        return;                              // pas de bourses : l'ecran du jeu, tel quel
    }
    g.final_set  = false;
    g.victory_at = now;
    int64_t c = 0, f = 0;
    if (!read_run(c, f)) return;
    if (mine_c > c) mine_c = c;
    if (mine_f > f) mine_f = f;
    if (mine_c < 0) mine_c = 0;
    if (mine_f < 0) mine_f = 0;
    if (write_run(mine_c, mine_f))
        log_line("PURSE", "fin de l'aventure (copie) : l'ecran montre ta bourse, %lld piece(s) et %lld "
                          "nourriture(s) (compte de l'equipe : %lld et %lld)", (long long)mine_c, (long long)mine_f,
                 (long long)c, (long long)f);
    g.have = false;
}

void __fastcall h_RunVictory(void* dir) {
    if (net_role() == NetRole::Client) guest_victory();
    else host_victory();
    // Le defile du butin (mm_loot) : le sac et les joueurs avant la fonction du
    // jeu (ClearRunState, a l'interieur, oublie l'equipe), le rangement apres.
    // Les achats en boutique (mm_shop) : notes avant (l'equipe encore connue),
    // envoyes a leurs acheteurs apres, avant le defile (qui ne les montre pas).
    loot_victory_before();
    shop_victory_before();
    o_RunVictory(dir);
    shop_victory_after();
    loot_victory_after();
}

// --- les fenetres ------------------------------------------------------------------------

// Les trouvailles deja montrees s'en vont quand une nouvelle case commence ;
// celles qui ne l'ont jamais ete, apres 5 minutes sur la carte (le temps d'une
// case ne compte pas : un long combat ne les efface pas).
void finds_pump() {
    const bool live = follow_node_live();
    const bool entered = live && !g.was_live;
    g.was_live = live;
    if (g.finds.empty()) return;
    const bool guest = net_role() == NetRole::Client;
    if (guest ? !mm_guest_in_run() : !g.tracking) {
        g.finds.clear();
        return;
    }
    const uint64_t now = GetTickCount64();
    for (size_t i = g.finds.size(); i-- > 0;) {
        Find& f = g.finds[i];
        if (live) f.at = now;
        if ((entered && f.shown) || (!live && now - f.at > kFindMaxAgeMs))
            g.finds.erase(g.finds.begin() + (ptrdiff_t)i);
    }
}

// Hote : la fin, une fois dans sa maison.
void home_pump() {
    if (g.home_notes.empty() || net_role() == NetRole::Client) return;
    if (!mm_house() || mm_info_busy()) return;
    mm_show_info(g.home_notes.front());
    g.home_notes.erase(g.home_notes.begin());
}

} // namespace

// ---------------------------------------------------------------------------------------------

void purse_pump() {
    if (!g.told) {
        g.told   = true;
        g.armed  = hooks_is_live(T_RunVictory) && hooks_is_live(T_EventGainCoins) &&
                   hooks_is_live(T_EventGainFood) && hooks_is_live(T_EventResult);
        g.battle = g.armed && hooks_is_live(T_UnitGainCoins) && hooks_is_live(T_PickupFood) &&
                   hooks_is_live(T_BonusFoodPassive);
        if (!g.armed)
            log_line("PURSE", "!! un crochet manque (RUNVICTORY, EVTCOINS, EVTFOOD, EVTRESULT) : pas de bourses, "
                              "tout va a l'hote comme avant");
        else
            log_line("PURSE", "arme -- une bourse par joueur en aventure mixte ; ce qu'un chat trouve dans un "
                              "evenement%s va a son proprietaire", g.battle ? " ou ramasse en combat"
                                                                            : " (en combat : partage, crochet absent)");
    }
    if (!g.armed) return;
    if (net_role() != NetRole::Client) host_pump();
    finds_pump();
    home_pump();
    // Invite : la reponse a son don tarde (une reponse en retard est dite quand
    // meme a son arrivee).
    if (g.give_pending && GetTickCount64() - g.give_at > kGiveAnswerMs) {
        g.give_pending = false;
        log_line("PURSE", "!! don %u : pas de reponse de l'hote", (unsigned)g.give_seq);
        gift_purse_note(u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la "
                        u8"fenêtre du chemin.", true);
    }
}

void purse_on_mm(uint8_t from, const MmMsg& m) {
    if (net_role() == NetRole::Host) {
        if (m.a == kPurseGiveReq && g.armed) host_on_give_req(from, m);
        return;
    }
    if (net_role() != NetRole::Client || from != kHostPeer) return;
    switch (m.a) {
        case kPurseLedger: {
            PurseLine lines[kPurseMaxLines];
            const uint8_t n = purse_decode_lines(m, lines, kPurseMaxLines);
            if (!n) {
                log_line("PURSE", "!! bourses illisibles -- ignorees");
                return;
            }
            const bool fresh = !g.have || g.have_gen != m.b;
            memcpy(g.lines, lines, sizeof(PurseLine) * n);
            g.n_lines  = n;
            g.have     = true;
            g.have_gen = m.b;
            g.have_at  = GetTickCount64();
            if (fresh) {
                int64_t c = 0, f = 0;
                const std::string mine = own_line(net_self(), c, f) ? "la tienne : " + purse_balance_text(c, f)
                                                                    : std::string("pas de bourse a toi");
                log_line("PURSE", "<- bourses de l'aventure (%u joueur(s)) ; %s", (unsigned)n, mine.c_str());
            }
            break;
        }
        case kPurseFind: {
            Find f;
            uint8_t owner = 0, flags = 0;
            if (!purse_decode_find(m, owner, f.coins, f.food, f.cat, &flags)) {
                log_line("PURSE", "!! trouvaille illisible -- ignoree");
                return;
            }
            if (!mm_guest_in_run()) return;
            f.owner      = owner;
            f.owner_name = mm_peer_name(owner);
            f.flags      = flags & (kFindSpent | kFindBattle | kFindShop);
            log_line("PURSE", "<- %s : %+d piece(s), %+d nourriture(s) -> bourse de %s",
                     (f.flags & kFindBattle) ? "en combat" : f.cat.c_str(), f.coins, f.food, f.owner_name.c_str());
            push_find(std::move(f));
            break;
        }
        case kPursePayout: {
            PurseLine lines[kPurseMaxLines];
            const uint8_t n = purse_decode_lines(m, lines, kPurseMaxLines);
            if (!n) {
                log_line("PURSE", "!! fin des bourses illisible -- ignoree");
                return;
            }
            const uint8_t self = net_self();
            int32_t coins = 0, food = 0;
            for (uint8_t i = 0; i < n; ++i)
                if (lines[i].peer == self) {
                    coins = lines[i].coins;
                    food  = lines[i].food;
                }
            g.finds.clear();
            // Pour l'ecran de fin de sa copie, si elle n'y est pas deja passee.
            const uint64_t now = GetTickCount64();
            if (!g.victory_at || now - g.victory_at > kFinalWindowMs) {
                g.final_set   = true;
                g.final_coins = coins;
                g.final_food  = food;
                g.final_at    = now;
            }
            if (coins > 0 || food > 0) team_note_payout(m.b, coins, food);
            else log_line("PURSE", "<- fin de l'aventure : rien a rapporter pour toi");
            break;
        }
        case kPurseGiven: {
            uint8_t giver = 0, to = 0, seq = 0;
            int32_t coins = 0, food = 0;
            if (!purse_decode_given(m, giver, to, coins, food, seq)) {
                log_line("PURSE", "!! don illisible -- ignore");
                return;
            }
            const uint8_t self    = net_self();
            const bool    from_me = giver == self;
            const bool    to_me   = to == self;
            if (from_me && g.give_pending && seq == g.give_seq) g.give_pending = false;
            log_line("PURSE", "<- don de bourse : %s donne %d piece(s) et %d nourriture(s) a %s",
                     mm_peer_name(giver).c_str(), coins, food, mm_peer_name(to).c_str());
            if (from_me || to_me)
                gift_purse_note(purse_given_line(from_me, to_me, mm_peer_name(giver), mm_peer_name(to), coins, food),
                                from_me);
            break;
        }
        case kPurseGiveNo: {
            uint8_t giver = 0, why = 0, seq = 0;
            if (!purse_decode_give_no(m, giver, why, seq)) {
                log_line("PURSE", "!! refus de don illisible -- ignore");
                return;
            }
            if (giver != net_self()) return;
            if (g.give_pending && seq == g.give_seq) g.give_pending = false;
            log_line("PURSE", "<- ton don n'est pas passe (raison %u)", (unsigned)why);
            gift_purse_note(u8"Ton don n'est pas passé : " + purse_give_no_text(why) + ".", true);
            break;
        }
        default:
            log_line("PURSE", "!! message de bourse inconnu (%u) -- ignore", (unsigned)m.a);
            break;
    }
}

std::string purse_vote_line(bool mark) {
    if (!g.armed) return "";
    const bool guest = net_role() == NetRole::Client;
    int64_t c = 0, f = 0;
    bool purse = true;
    if (guest) {
        if (!mm_guest_in_run()) return "";
        purse = g.have && GetTickCount64() - g.have_at <= kLedgerStaleMs && own_line(net_self(), c, f);
    } else {
        if (!g.tracking || g.ended) return "";
        c = g.book.at(kHostPeer).coins;
        f = g.book.at(kHostPeer).food;
        // Sans chat a lui, l'hote n'a de bourse que si quelque chose y est alle.
        purse = (g.slots & (1u << kHostPeer)) || c || f;
    }
    std::string out;
    const int self = guest ? (int)net_self() : (int)kHostPeer;
    size_t shown = 0;
    for (Find& fd : g.finds) {
        if (shown == kShownFinds) {
            out += u8"\n(et d'autres trouvailles)";
            break;
        }
        const bool mine = fd.owner == self;
        out += "\n" + ((fd.flags & kFindBattle) ? purse_battle_line(fd.coins, fd.food, mine, fd.owner_name)
                      : (fd.flags & kFindShop)  ? purse_shop_line(fd.cat, fd.coins, fd.food, mine, fd.owner_name)
                                                : purse_find_line(fd.cat, fd.coins, fd.food, mine, fd.owner_name,
                                                                  (fd.flags & kFindSpent) != 0));
        if (mark) fd.shown = true;
        ++shown;
    }
    if (purse) out += u8"\nTa bourse : " + purse_balance_text(c, f) + ".";
    return out;
}

bool purse_tracking() { return tracking_host(); }

bool purse_slot(uint8_t slot, int64_t& coins, int64_t& food, bool& has) {
    coins = food = 0;
    has   = false;
    if (!tracking_host() || slot >= kPurseSlots) return false;
    coins = g.book.at(slot).coins;
    food  = g.book.at(slot).food;
    has   = (g.slots & (1u << slot)) != 0 || coins != 0 || food != 0;
    return true;
}

bool purse_mine(int64_t& coins, int64_t& food) {
    coins = food = 0;
    if (!g.armed) return false;
    if (net_role() == NetRole::Client) {
        return mm_guest_in_run() && g.have && GetTickCount64() - g.have_at <= kLedgerStaleMs &&
               own_line(net_self(), coins, food);
    }
    if (!tracking_host()) return false;
    coins = g.book.at(kHostPeer).coins;
    food  = g.book.at(kHostPeer).food;
    return (g.slots & (1u << kHostPeer)) || coins || food;
}

bool purse_holders(std::vector<PurseHolder>& out) {
    out.clear();
    if (!g.armed) return false;
    auto add = [&](uint8_t peer, int64_t coins, int64_t food) {
        for (const PurseHolder& h : out)
            if (h.peer == peer) return;
        PurseHolder h;
        h.peer  = peer;
        h.coins = coins;
        h.food  = food;
        out.push_back(h);
    };
    if (net_role() == NetRole::Client) {
        if (!mm_guest_in_run() || !g.have || GetTickCount64() - g.have_at > kLedgerStaleMs) return false;
        for (uint8_t i = 0; i < g.n_lines; ++i) add(g.lines[i].peer, g.lines[i].coins, g.lines[i].food);
        // L'hote sans chat n'a pas de ligne, mais il recoit (il garde le reste a la fin).
        add(kHostPeer, 0, 0);
    } else {
        if (!tracking_host()) return false;
        for (uint8_t s = 0; s < kPurseSlots; ++s) {
            if (!slot_has_purse(s)) continue;
            const int peer = team_slot_peer(s);
            if (peer < 0 || peer >= (int)kPurseSlots) continue;
            add((uint8_t)peer, g.book.at(s).coins, g.book.at(s).food);
        }
    }
    std::sort(out.begin(), out.end(), [](const PurseHolder& a, const PurseHolder& b) { return a.peer < b.peer; });
    return !out.empty();
}

bool purse_give(uint8_t to, int64_t coins, int64_t food, std::string& why) {
    why.clear();
    if (!g.armed) {
        why = u8"les bourses ne marchent pas dans cette partie";
        return false;
    }
    if (coins < 0 || food < 0 || (!coins && !food)) {
        why = u8"rien à donner";
        return false;
    }
    if (coins > kPurseGiveMax || food > kPurseGiveMax) {
        why = u8"c'est trop d'un coup";
        return false;
    }
    const bool    guest = net_role() == NetRole::Client;
    const uint8_t self  = guest ? net_self() : (uint8_t)kHostPeer;
    if (to == self || to >= kPurseSlots) {
        why = u8"choisis un autre joueur";
        return false;
    }
    if (!guest) {
        const uint8_t no = host_transfer(kHostPeer, to, coins, food);
        if (no) {
            why = purse_give_no_text(no);
            return false;
        }
        MmMsg r;
        purse_encode_given(r, kHostPeer, to, (int32_t)coins, (int32_t)food, 0);
        if (!net_send_mm(r)) log_line("PURSE", "!! envoi du don impossible (les bourses suivent quand meme)");
        host_send(true);
        gift_purse_note(purse_given_line(true, false, "", mm_peer_name(to), coins, food), true);
        return true;
    }
    if (g.give_pending) {
        why = u8"ton don précédent attend encore la réponse de l'hôte";
        return false;
    }
    int64_t mc = 0, mf = 0;
    if (!purse_mine(mc, mf)) {
        why = u8"tu n'as pas de bourse dans cette aventure";
        return false;
    }
    if (coins > mc || food > mf) {
        why = purse_give_no_text(kPurseNoEnough);
        return false;
    }
    bool target = to == kHostPeer;
    for (uint8_t i = 0; i < g.n_lines; ++i) target = target || g.lines[i].peer == to;
    if (!target) {
        why = purse_give_no_text(kPurseNoPurse);
        return false;
    }
    uint8_t seq = (uint8_t)(g.give_seq + 1);
    if (!seq) seq = 1;
    MmMsg m;
    purse_encode_give_req(m, to, (int32_t)coins, (int32_t)food, seq);
    if (!net_send_mm(m)) {
        why = u8"envoi impossible (connexion)";
        return false;
    }
    g.give_seq     = seq;
    g.give_pending = true;
    g.give_at      = GetTickCount64();
    log_line("PURSE", "-> don %u : %lld piece(s) et %lld nourriture(s) a %s, demande a l'hote", (unsigned)seq,
             (long long)coins, (long long)food, mm_peer_name(to).c_str());
    return true;
}

bool purse_give_pending() { return g.give_pending; }

void purse_charge(uint8_t slot, const std::string& label, void (*call)(void*), void* arg) {
    if (!call) return;
    if (!tracking_host() || slot >= kPurseSlots) {
        call(arg);
        return;
    }
    in_shop_ctx(slot, label, call, arg);
}

bool purse_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_RunVictory:      *detour = (void*)&h_RunVictory;  *original = (void**)&o_RunVictory;  return true;
    case T_EventGainCoins:  *detour = (void*)&h_GainCoins;   *original = (void**)&o_GainCoins;   return true;
    case T_EventGainFood:   *detour = (void*)&h_GainFood;    *original = (void**)&o_GainFood;    return true;
    case T_EventResult:     *detour = (void*)&h_EventResult; *original = (void**)&o_EventResult; return true;
    case T_UnitGainCoins:   *detour = (void*)&h_UnitCoins;   *original = (void**)&o_UnitCoins;   return true;
    case T_PickupFood:  *detour = (void*)&h_EatFood;     *original = (void**)&o_EatFood;     return true;
    case T_BonusFoodPassive:*detour = (void*)&h_BonusFood;   *original = (void**)&o_BonusFood;   return true;
    default: return false;
    }
}

} // namespace mgmp
