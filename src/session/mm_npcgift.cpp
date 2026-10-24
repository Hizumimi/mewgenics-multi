// mm_npcgift -- Mewgenics Multi : les dons de l'invite aux PNJ. Voir mm_npcgift.h
// (le jeu, le mod) et mm_npcgift_core.h (les regles, le message).

#include "mm_npcgift.h"
#include "mm_npcgift_core.h"
#include "mm_lang.h"

#include "mm_loot.h"          // le rangement : objets, retrait, noms
#include "mm_npc_state.h"     // npc_name
#include "mm_rungear.h"       // rungear_screen_open
#include "mm_team.h"          // team_send, team_note_loot, team_note_gift
#include "mm_team_merge.h"    // cat_info, item_digest
#include "mm_menu.h"          // mm_in_house, mm_house, mm_show_info, mm_peer_name...

#include "mgmp_catsync.h"     // catsync_serialize_cat
#include "mgmp_hooks.h"       // hooks_is_live
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"    // savefile_in_coop
#include "mgmp_session.h"     // session_ready

#include <windows.h>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {

void __fastcall h_CacheReward_entry(void* prog, int npc);   // plus bas : le detour de CacheInReward

namespace {

using FnNpcFavor    = void (__fastcall*)(void* progress, int npc, void* cat, int amount);
using FnCacheReward = void (__fastcall*)(void* progress, int npc);
FnNpcFavor    o_NpcFavor    = nullptr;
FnCacheReward o_CacheReward = nullptr;

// La progression (MewDirector+0x5A8, voir mm_npc_state.h) et ses PNJ.
constexpr uintptr_t kDir_Progress = 0x5A8;
constexpr uintptr_t kProg_Npcs    = 0x48;
constexpr uintptr_t kNpcStride    = 0x78;
constexpr uintptr_t kNpc_Cur      = 0x18;   // std::string : la recompense visee
constexpr uintptr_t kNpc_Favor    = 0x40;   // u32
constexpr uintptr_t kNpc_Ready    = 0x48;   // u8

// Ce que sub_1401C1650 lit du chat pour Beanies (0x1401C1731, 0x1401C17EF,
// 0x1401C1854) : 4 std::string a partir de +0x810 (pas de 0x20), puis +0x910 et
// +0x938. Le faux chat de l'hote n'a que ca (le reste a zero).
constexpr uintptr_t kCat_Actives  = 0x810;
constexpr uintptr_t kCat_Passive1 = 0x910;
constexpr uintptr_t kCat_Passive2 = 0x938;
constexpr size_t    kFakeCat      = 0x980;

// La maison vivante (D_HouseInstance) : CacheInReward y ajoute pieces et
// nourriture (0x1401C1B3E, 0x1401C1AF9).
constexpr uintptr_t kHouse_Food  = 0xB0;
constexpr uintptr_t kHouse_Coins = 0xB4;

constexpr size_t   kMaxQueue      = 64;
constexpr uint64_t kGiftTimeoutMs = 120000;   // sans reponse du donneur : il garde, l'hote aussi

// Une std::string du jeu, telle que le jeu la lit (tampon interne jusqu'a 15).
struct GStr {
    union { char buf[16]; char* ptr; };
    uint64_t size;
    uint64_t cap;
};
static_assert(sizeof(GStr) == 32, "std::string de MSVC");

void set_gstr(GStr& g, const std::string& s) {
    memset(&g, 0, sizeof(g));
    if (s.size() <= 15) {
        memcpy(g.buf, s.data(), s.size());
        g.buf[s.size()] = 0;
        g.size = s.size();
        g.cap  = 15;
        return;
    }
    g.ptr  = const_cast<char*>(s.c_str());   // lu seulement, pendant l'appel
    g.size = s.size();
    g.cap  = s.size();
}

struct Donation {
    uint8_t                  from = 0;
    std::string              who;            // son nom, pour le journal et les fenetres
    uint8_t                  npc  = 0;
    uint32_t                 favor = 1;
    std::string              cat;
    std::vector<std::string> abilities;
};

struct Gift {                                 // hote : une recompense partie, en attente de reponse
    uint64_t                 id   = 0;
    void*                    prog = nullptr;  // la partie d'ou elle vient (une autre chargee : oubliee)
    uint8_t                  peer = 0;
    std::string              who;
    uint8_t                  npc  = 0;
    int32_t                  coins = 0, food = 0;
    std::vector<uint64_t>    ids, digests;
    std::vector<std::string> labels;
    uint64_t                 at   = 0;
};

struct Erase {                                // hote : un objet garde par son donneur, a retirer
    uint64_t    id = 0, digest = 0;
    void*       prog = nullptr;                // de cette partie seulement
    std::string label, who;
    bool        told = false;
};

struct State {
    bool     told = false;                    // la ligne « arme » est ecrite
    bool     armed = false;                   // la faveur se compte (crochet NPCFAVOR)
    bool     reward_ok = false;               // ... et la recompense va au donneur (CACHEREWARD, la maison)
    // --- hote ---
    void*                 prog = nullptr;     // la progression du Director vu en dernier
    std::vector<Donation> queue;
    // Les dons a un PNJ dont la recompense est deja prete : comptes seulement
    // apres sa prise (le jeu remet la jauge a zero en la prenant -- une faveur
    // ajoutee avant serait perdue). Pour la partie `prog` seulement.
    std::vector<Donation> held;
    NpcDonor              donors[kNpcGiftNpcs];
    std::vector<Gift>     gifts;
    std::vector<Erase>    erases;
    int64_t               owed_coins = 0, owed_food = 0;   // a retirer de sa maison (gardes par un donneur)
    void*                 owed_prog  = nullptr;            // ... de cette partie
    uint64_t              rng = 0;
    uint32_t              applied = 0, rewards = 0;
    // --- invite ---
    uint32_t              sent = 0;
    uint64_t              got_ids[16] = {};   // recompenses deja notees (un message en double)
    uint32_t              got_next = 0;
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

std::string no_markup(std::string s) {
    for (char& c : s)
        if (c == '[') c = '(';
        else if (c == ']') c = ')';
    return s;
}

bool in_session() { return session_ready() && net_active() && net_peer_count() >= 2; }

bool peer_present(uint8_t peer) {
    uint8_t ids[8] = {};
    const uint8_t n = net_peer_count();
    if (!n || !net_peer_ids(ids, 8)) return false;
    for (uint8_t i = 0; i < n && i < 8; ++i)
        if (ids[i] == peer) return true;
    return false;
}

// Le nom affiche d'un PNJ : celui du jeu (NPC_NAME_* de combined.csv ; le Broyeur d'organe, que
// le joueur nomme lui-meme, par le nom de son evenement, EVENT_ORGANGRINDER_NAME).
std::string npc_label(int npc) {
    static const char* const names[kNpcGiftNpcs] = { trk(u8"Dr Beanies"), trk(u8"Butch"),     trk(u8"Tink"),
                                                     trk(u8"Frank"),      trk(u8"Bébé Jack"), trk(u8"Tracy"),
                                                     trk(u8"Broyeur d'organe"), trk(u8"Steven") };
    return npc >= 0 && npc < (int)kNpcGiftNpcs ? tr(names[npc]) : "?";
}

void* progress() {
    const void** slot = (const void**)addr_of_data(D_MewDirectorPtr);
    void*    dir  = nullptr;
    uint64_t tag  = 0, mark = 0;
    void*    prog = nullptr;
    if (!slot || !mem_read(slot, &dir, sizeof(dir)) || !dir) return nullptr;
    if (!mem_read(slot + 1, &tag, sizeof(tag)) ||
        !mem_read((const char*)dir - 8, &mark, sizeof(mark)) || mark != tag)
        return nullptr;
    if (!mem_read((const char*)dir + kDir_Progress, &prog, sizeof(prog))) return nullptr;
    return prog;
}

struct NpcRead {
    std::string cur;
    uint32_t    favor = 0;
    bool        ready = false;
};

bool read_npc(void* prog, int npc, NpcRead& out) {
    if (!prog || npc < 0 || npc >= (int)kNpcGiftNpcs) return false;
    const char* e = (const char*)prog + kProg_Npcs + (uintptr_t)npc * kNpcStride;
    char    cur[96] = {};
    uint8_t ready   = 0;
    if (!mem_read_std_string(e + kNpc_Cur, cur, sizeof(cur)) ||
        !mem_read(e + kNpc_Favor, &out.favor, sizeof(out.favor)) || !mem_read(e + kNpc_Ready, &ready, 1))
        return false;
    out.cur   = cur;
    out.ready = ready != 0;
    return true;
}

// La maison vivante, celle que CacheInReward remplit ; nulle hors de la maison.
void* house_live() {
    const void* const* slot = (const void* const*)addr_of_data(D_HouseInstance);
    void* h = nullptr;
    if (!slot || !mem_read(slot, &h, sizeof(h))) return nullptr;
    return h;
}

bool read_money(void* house, int32_t& coins, int32_t& food) {
    return house && mem_read((const char*)house + kHouse_Coins, &coins, sizeof(coins)) &&
           mem_read((const char*)house + kHouse_Food, &food, sizeof(food));
}

bool storage(std::vector<NpcStored>& out) {
    out.clear();
    std::vector<LootStoredItem> items;
    if (!loot_storage_items(items)) return false;
    for (const LootStoredItem& s : items) out.push_back(NpcStored{ s.id, s.f });
    return true;
}

// --- appels au jeu, sous SEH (rien de C++ a detruire ici) --------------------------------

bool seh_favor(FnNpcFavor fn, void* prog, int npc, void* cat, int amount) {
    __try {
        fn(prog, npc, cat, amount);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool seh_reward(FnCacheReward fn, void* prog, int npc) {
    __try {
        fn(prog, npc);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// --- invite : le don part chez l'hote -----------------------------------------------------

std::string cat_name(void* cat) {
    std::vector<uint8_t> raw;
    CatInfo info;
    if (cat && catsync_serialize_cat(cat, raw) && cat_info(raw, info) && !info.name.empty())
        return no_markup(info.name);
    return std::string();
}

std::vector<std::string> cat_abilities(void* cat) {
    std::vector<std::string> out;
    if (!cat) return out;
    const uintptr_t offs[kNpcGiftAbilities] = { kCat_Actives, kCat_Actives + 0x20, kCat_Actives + 0x40,
                                                kCat_Actives + 0x60, kCat_Passive1, kCat_Passive2 };
    for (uintptr_t off : offs) {
        char buf[80] = {};
        out.push_back(mem_read_std_string((const char*)cat + off, buf, sizeof(buf)) ? std::string(buf)
                                                                                   : std::string("None"));
    }
    return out;
}

bool guest_forwards(void* prog, int npc, int amount) {
    return npcgift_guest_donates() && prog && prog == progress() && npc >= 0 && npc < (int)kNpcGiftNpcs &&
           amount > 0 && (uint32_t)amount <= kNpcGiftMaxFavor;
}

// --- hote : le don applique, la recompense prise -------------------------------------------

void say(const std::string& text) { mm_show_info(text); }

void send_ready(uint8_t npc, const NpcDonor& d) {
    NpcGiftMsg m;
    m.op  = kNpcReady;
    m.npc = npc;
    if (!team_send(npcgift_encode(m), d.peer))
        log_line("PNJ", "!! %s : la recompense prete n'a pas pu etre annoncee a %s", npc_name(npc), d.name.c_str());
}

void apply_donations(void* prog) {
    while (!g.queue.empty()) {
        const Donation dn = g.queue.front();
        g.queue.erase(g.queue.begin());
        NpcRead before, after;
        if (!read_npc(prog, dn.npc, before)) {
            log_line("PNJ", "!! don de %s a %s : PNJ illisible -- le don est perdu", dn.who.c_str(), npc_name(dn.npc));
            continue;
        }
        // Sa recompense est prete et pas encore prise : le jeu remettra sa jauge a
        // zero en la prenant. Le don attend cette prise, puis compte pour la suivante.
        if (before.ready) {
            if (g.held.size() >= kMaxQueue) {
                log_line("PNJ", "!! trop de dons en attente : celui de %s a %s est perdu", dn.who.c_str(),
                         npc_name(dn.npc));
                continue;
            }
            log_line("PNJ", "<- don de %s a %s : sa recompense « %s » est prete -- le don comptera une fois "
                            "qu'elle sera prise", dn.who.c_str(), npc_name(dn.npc), before.cur.c_str());
            g.held.push_back(dn);
            continue;
        }
        // Le faux chat : ses capacites seulement (Beanies), le reste a zero.
        alignas(16) static uint8_t fake[kFakeCat];
        memset(fake, 0, sizeof(fake));
        const std::vector<std::string> ab = npcgift_abilities(dn.abilities);
        const uintptr_t offs[kNpcGiftAbilities] = { kCat_Actives, kCat_Actives + 0x20, kCat_Actives + 0x40,
                                                    kCat_Actives + 0x60, kCat_Passive1, kCat_Passive2 };
        for (size_t i = 0; i < kNpcGiftAbilities; ++i) set_gstr(*(GStr*)(fake + offs[i]), ab[i]);
        if (!seh_favor(o_NpcFavor, prog, dn.npc, dn.npc == 0 ? fake : nullptr, (int)dn.favor)) {
            log_line("PNJ", "!! don de %s a %s : exception dans le jeu -- le don n'est pas compte", dn.who.c_str(),
                     npc_name(dn.npc));
            continue;
        }
        ++g.applied;
        const bool readable = read_npc(prog, dn.npc, after);
        log_line("PNJ", "<- don de %s (%s) a %s : faveur %u -> %u%s", dn.who.c_str(),
                 dn.cat.empty() ? "un chat" : dn.cat.c_str(), npc_name(dn.npc), before.favor,
                 readable ? after.favor : 0, readable && after.ready ? " (recompense prete)" : "");
        if (readable && g.reward_ok &&
            npcgift_after_donation(g.donors[dn.npc], before.ready, after.ready, dn.from, dn.who, before.cur)) {
            log_line("PNJ", "%s : la jauge est pleine grace au don de %s -- la recompense « %s » est pour lui",
                     npc_name(dn.npc), dn.who.c_str(), before.cur.c_str());
        }
    }
}

// Les dons retenus d'un PNJ dont la recompense a ete prise repartent, devant les
// autres, dans leur ordre d'arrivee.
void release_held(void* prog) {
    if (g.held.empty() || !prog) return;
    std::vector<Donation> back;
    for (size_t i = 0; i < g.held.size();) {
        NpcRead r;
        if (read_npc(prog, g.held[i].npc, r) && !r.ready) {
            back.push_back(std::move(g.held[i]));
            g.held.erase(g.held.begin() + (ptrdiff_t)i);
        } else {
            ++i;
        }
    }
    if (back.empty()) return;
    log_line("PNJ", "%zu don(s) retenu(s) comptent maintenant (la recompense a ete prise)", back.size());
    g.queue.insert(g.queue.begin(), std::make_move_iterator(back.begin()), std::make_move_iterator(back.end()));
}

// L'hote peut prendre la recompense d'un donneur : dans sa maison vivante, sans
// fenetre ni ecran des objets ouvert.
bool safe_to_take() {
    void* h = house_live();
    return h && mm_house() && mm_in_house() && !savefile_in_coop() && !mm_other_popup_open() &&
           !mm_game_prompt_open() && !rungear_screen_open() && !loot_storage_busy();
}

void auto_take(void* prog) {
    if (!prog || !g.reward_ok) return;
    for (uint8_t npc = 0; npc < kNpcGiftNpcs; ++npc) {
        NpcDonor& d = g.donors[npc];
        if (d.peer == kNpcNoDonor) continue;
        NpcRead r;
        if (!read_npc(prog, npc, r)) continue;
        if (!r.ready || r.cur != d.reward) {
            log_line("PNJ", "%s : la recompense de %s n'est plus prete (prise autrement) -- oubliee", npc_name(npc),
                     d.name.c_str());
            d = NpcDonor{};
            continue;
        }
        if (!peer_present(d.peer) || mm_peer_name(d.peer) != d.name) continue;   // il reviendra peut-etre
        if (!safe_to_take()) {
            if (!d.told) {
                d.told = true;
                send_ready(npc, d);
                log_line("PNJ", "%s : recompense de %s prete, prise des que tu seras dans ta maison", npc_name(npc),
                         d.name.c_str());
                say(fmt(tr(u8"Récompense prête\n\n"
                        u8"Le chat que %s a donné à %s a rempli\n"
                        u8"sa jauge. La récompense sera prise pour lui\n"
                        u8"dès que tu seras dans ta maison : ses pièces,\n"
                        u8"sa nourriture et ses objets iront à %s,\n"
                        u8"ses déblocages à tout le monde."),
                        no_markup(d.name).c_str(), npc_label(npc).c_str(), no_markup(d.name).c_str()));
            }
            continue;
        }
        log_line("PNJ", "%s : la recompense de %s est prise pour lui", npc_name(npc), d.name.c_str());
        // Le detour lui-meme : la prise, puis le partage.
        h_CacheReward_entry(prog, npc);
        return;                              // une par image
    }
}

uint64_t next_gift_id() {
    if (!g.rng) g.rng = GetTickCount64() * 0x9E3779B97F4A7C15ull ^ (uint64_t)(uintptr_t)&g;
    g.rng ^= g.rng << 13;
    g.rng ^= g.rng >> 7;
    g.rng ^= g.rng << 17;
    return g.rng ? g.rng : 1;
}

// La recompense prise (par le jeu ou par auto_take) : ce qu'elle a donne part
// chez le donneur.
void route_reward(void* prog, int npc, const NpcDonor& donor) {
    void* house = house_live();
    int32_t c0 = 0, f0 = 0, c1 = 0, f1 = 0;
    const bool money0 = read_money(house, c0, f0);
    std::vector<NpcStored> s0, s1;
    const bool st0 = storage(s0);
    if (!seh_reward(o_CacheReward, prog, npc)) {
        log_line("PNJ", "!! %s : exception dans le jeu en prenant la recompense de %s", npc_name(npc),
                 donor.name.c_str());
        return;
    }
    ++g.rewards;
    const bool money1 = money0 && read_money(house_live(), c1, f1);
    const bool st1    = st0 && storage(s1);
    const int32_t coins = money1 ? std::max(0, c1 - c0) : 0;
    const int32_t food  = money1 ? std::max(0, f1 - f0) : 0;
    std::vector<size_t> fresh;
    if (st1) fresh = npcgift_new_items(s0, s1);
    if (fresh.size() > kNpcGiftMaxItems) fresh.resize(kNpcGiftMaxItems);
    if (!money1) log_line("PNJ", "!! %s : or et nourriture de la maison illisibles -- ils restent chez toi", npc_name(npc));
    if (!st1)    log_line("PNJ", "!! %s : rangement illisible -- les objets restent chez toi", npc_name(npc));

    NpcGiftMsg m;
    m.op    = kNpcGift;
    m.npc   = (uint8_t)npc;
    m.gift  = next_gift_id();
    m.coins = coins;
    m.food  = food;
    Gift rec;
    rec.id    = m.gift;
    rec.prog  = prog;
    rec.peer  = donor.peer;
    rec.who   = donor.name;
    rec.npc   = (uint8_t)npc;
    rec.coins = coins;
    rec.food  = food;
    rec.at    = GetTickCount64();
    for (size_t ix : fresh) {
        // Un nom ou un texte que le message couperait : l'objet reste chez l'hote
        // (comme au defile du butin) -- coupe, ce ne serait plus le meme objet.
        if (s1[ix].f.name.size() > kNpcGiftMaxName || s1[ix].f.text.size() > kNpcGiftMaxText) {
            log_line("PNJ", "!! %s : nom ou texte trop long pour le message -- il reste chez toi",
                     s1[ix].f.name.substr(0, 40).c_str());
            continue;
        }
        NpcGiftItem it;
        it.f     = s1[ix].f;
        it.label = loot_item_label(it.f);
        m.items.push_back(it);
        rec.ids.push_back(s1[ix].id);
        rec.digests.push_back(item_digest(it.f));
        rec.labels.push_back(it.label);
    }
    const std::string list = npcgift_list_text(coins, food, rec.labels);
    if (!team_send(npcgift_encode(m), donor.peer)) {
        log_line("PNJ", "!! %s : la recompense de %s (%s) n'a pas pu partir -- elle reste chez toi", npc_name(npc),
                 donor.name.c_str(), list.c_str());
        say(fmt(tr(u8"Récompense de %s\n\n"
                u8"La récompense du don de %s n'a pas pu\n"
                u8"lui être envoyée : elle reste chez toi."), npc_label(npc).c_str(),
                no_markup(donor.name).c_str()));
        return;
    }
    log_line("PNJ", "-> %s : recompense du don de %s envoyee (%s), en attente de sa reponse", npc_name(npc),
             donor.name.c_str(), list.c_str());
    g.gifts.push_back(rec);                   // meme sans rien : sa reponse est attendue
    const bool things = coins || food || !rec.ids.empty();
    say(things ? fmt(tr(u8"Récompense pour %s\n\n"
                     u8"%s a récompensé le chat que %s\n"
                     u8"lui a donné : %s partent chez lui.\n"
                     u8"Les déblocages sont pour tout le monde."),
                     no_markup(donor.name).c_str(), npc_label(npc).c_str(), no_markup(donor.name).c_str(),
                     list.c_str())
               : fmt(tr(u8"Récompense pour tout le monde\n\n"
                     u8"%s a récompensé le chat que %s\n"
                     u8"lui a donné : ses déblocages sont pour\n"
                     u8"tout le monde."),
                     npc_label(npc).c_str(), no_markup(donor.name).c_str()));
}

void on_got(uint8_t from, const NpcGiftMsg& m) {
    for (size_t i = 0; i < g.gifts.size(); ++i) {
        Gift& gf = g.gifts[i];
        if (gf.id != m.gift || gf.peer != from) continue;
        if (gf.prog != g.prog) {
            log_line("PNJ", "<- %s a garde la recompense de %s, mais une autre partie est chargee depuis -- "
                            "rien n'est retire de celle-ci", gf.who.c_str(), npc_name(gf.npc));
            g.gifts.erase(g.gifts.begin() + (ptrdiff_t)i);
            return;
        }
        if (m.money && (gf.coins || gf.food)) {
            if (g.owed_prog != gf.prog) g.owed_coins = g.owed_food = 0;
            g.owed_prog   = gf.prog;
            g.owed_coins += gf.coins;
            g.owed_food  += gf.food;
        } else if (gf.coins || gf.food) {
            log_line("PNJ", "<- %s n'a pas pu garder %s : elles restent chez toi", gf.who.c_str(),
                     npcgift_list_text(gf.coins, gf.food, {}).c_str());
        }
        for (size_t k = 0; k < gf.ids.size(); ++k) {
            if (k < 64 && ((m.kept >> k) & 1u)) {
                Erase e;
                e.id     = gf.ids[k];
                e.digest = gf.digests[k];
                e.prog   = gf.prog;
                e.label  = gf.labels[k];
                e.who    = gf.who;
                g.erases.push_back(e);
            } else {
                log_line("PNJ", "<- %s n'a pas pu garder %s : il reste chez toi", gf.who.c_str(), gf.labels[k].c_str());
            }
        }
        log_line("PNJ", "<- %s a garde la recompense de %s", gf.who.c_str(), npc_name(gf.npc));
        g.gifts.erase(g.gifts.begin() + (ptrdiff_t)i);
        return;
    }
    log_line("PNJ", "<- reponse de %s pour une recompense qui ne lui etait pas envoyee -- ignoree",
             mm_peer_name(from).c_str());
}

// Les recompenses dont le donneur est parti sans repondre (ou ne repond pas) :
// tout reste chez l'hote.
void drop_gone_gifts() {
    const uint64_t now = GetTickCount64();
    for (size_t i = g.gifts.size(); i-- > 0;) {
        const Gift& gf = g.gifts[i];
        const bool gone = !in_session() || !peer_present(gf.peer);
        if (!gone && now - gf.at < kGiftTimeoutMs) continue;
        log_line("PNJ", "!! %s %s sans repondre pour la recompense de %s : %s reste(nt) chez toi", gf.who.c_str(),
                 gone ? "est parti" : "ne repond pas", npc_name(gf.npc),
                 npcgift_list_text(gf.coins, gf.food, gf.labels).c_str());
        g.gifts.erase(g.gifts.begin() + (ptrdiff_t)i);
    }
}

void run_erases() {
    if (g.erases.empty() || loot_storage_busy()) return;
    void* prog = progress();                  // le Director vivant (verifie par son etiquette)
    if (!prog || !mm_in_game()) return;       // au menu : plus tard, dans la partie
    for (const Erase& e : g.erases) {
        if (e.prog != prog) {
            log_line("PNJ", "!! %s : garde par %s, mais une autre partie est chargee -- rien n'est retire",
                     e.label.c_str(), e.who.c_str());
            continue;
        }
        std::string why;
        if (loot_storage_take(e.id, e.digest, why))
            log_line("PNJ", "%s : retire de ton rangement (%s l'a)", e.label.c_str(), e.who.c_str());
        else
            log_line("PNJ", "!! %s : garde par %s, mais il n'a pas pu quitter ton rangement (%s) -- il est aussi chez toi",
                     e.label.c_str(), e.who.c_str(), why.c_str());
    }
    g.erases.clear();
}

// Les pieces et la nourriture gardees par un donneur quittent la maison de
// l'hote, des qu'elle est la (pas en dessous de 0 : il a pu les depenser).
void pay_owed() {
    if (!g.owed_coins && !g.owed_food) return;
    void* prog = progress();
    if (!prog) return;
    if (prog != g.owed_prog) {
        log_line("PNJ", "!! or et nourriture gardes par un donneur : une autre partie est chargee -- rien n'est "
                        "retire de celle-ci");
        g.owed_coins = g.owed_food = 0;
        return;
    }
    void* house = house_live();
    int32_t coins = 0, food = 0;
    if (!house || !mm_house() || !read_money(house, coins, food)) return;
    const int32_t nc = (int32_t)std::max<int64_t>(0, (int64_t)coins - g.owed_coins);
    const int32_t nf = (int32_t)std::max<int64_t>(0, (int64_t)food - g.owed_food);
    if (!mem_write((char*)house + kHouse_Coins, &nc, sizeof(nc)) || !mem_write((char*)house + kHouse_Food, &nf, sizeof(nf))) {
        log_line("PNJ", "!! or et nourriture de la maison : ecriture impossible -- ils restent chez toi");
    } else {
        log_line("PNJ", "maison : or %d -> %d, nourriture %d -> %d (gardes par les donneurs)", coins, nc, food, nf);
    }
    g.owed_coins = 0;
    g.owed_food  = 0;
}

void host_pump() {
    void* prog = progress();
    // Un autre Director : une save (re)chargee -- les donneurs d'une autre partie
    // ne comptent plus (le monde a change).
    if (prog && prog != g.prog) {
        if (g.prog) {
            for (NpcDonor& d : g.donors)
                if (d.peer != kNpcNoDonor) {
                    log_line("PNJ", "une autre partie se charge : la recompense de %s n'est plus a lui", d.name.c_str());
                    d = NpcDonor{};
                }
            if (!g.held.empty())
                log_line("PNJ", "!! une autre partie se charge : %zu don(s) retenu(s) pour l'ancienne sont perdus",
                         g.held.size());
            g.held.clear();
        }
        g.prog = prog;
    }
    drop_gone_gifts();
    run_erases();
    pay_owed();
    release_held(prog);
    if (prog && mm_in_game() && !g.queue.empty()) apply_donations(prog);
    auto_take(prog);
}

// --- messages --------------------------------------------------------------------------------

void guest_on_gift(const NpcGiftMsg& m) {
    // Un message en double : deja note (la reponse repart quand meme).
    bool seen = false;
    for (uint64_t id : g.got_ids) seen = seen || id == m.gift;
    uint64_t kept = 0;
    bool money = true;
    std::vector<std::string> labels;
    for (size_t i = 0; i < m.items.size() && i < 64; ++i) {
        const uint64_t key = m.gift ^ (0x9E3779B97F4A7C15ull * (uint64_t)(i + 1));
        // Le nom dans la langue de ce joueur ; sans les noms du jeu (repli sur la cle GON),
        // celui de l'hote (dans sa langue).
        std::string label = loot_item_label(m.items[i].f);
        if ((label.empty() || label.compare(0, m.items[i].f.name.size(), m.items[i].f.name) == 0) &&
            !m.items[i].label.empty())
            label = m.items[i].label;
        if (team_note_loot(kNpcGiftSeq, key, m.items[i].f, label)) {
            kept |= 1ull << i;
            labels.push_back(label);
        }
    }
    if ((m.coins || m.food) && !seen) money = team_note_gift(m.coins, m.food);
    if (!seen) {
        g.got_ids[g.got_next] = m.gift;
        g.got_next = (g.got_next + 1) % (sizeof(g.got_ids) / sizeof(g.got_ids[0]));
    }
    NpcGiftMsg r;
    r.op    = kNpcGot;
    r.npc   = m.npc;
    r.gift  = m.gift;
    r.money = money ? 1 : 0;
    r.kept  = kept;
    if (!team_send(npcgift_encode(r), kHostPeer))
        log_line("PNJ", "!! reponse a l'hote impossible pour la recompense de %s", npc_name(m.npc));
    const std::string list = npcgift_list_text(money ? m.coins : 0, money ? m.food : 0, labels);
    log_line("PNJ", "<- recompense de %s pour ton don : %s", npc_name(m.npc), list.c_str());
    if (seen) return;
    const bool things = (money && (m.coins || m.food)) || !labels.empty();
    // Chez lui, sa save ouverte (il donne depuis sa maison) : la fenetre
    // « Recharger » (mm_menu) ; ailleurs, ils arrivent a son retour.
    if (things && mm_offer_reload(fmt(tr(u8"%s te remercie pour ton chat : %s\n(ses déblocages sont pour tout le monde)."),
                                      npc_label(m.npc).c_str(), list.c_str())))
        return;
    say(things ? fmt(tr(u8"Récompense de %s\n\n"
                     u8"%s te remercie pour ton chat : %s.\n"
                     u8"Ils arrivent dans ta maison au prochain\n"
                     u8"chargement de ta save (au retour de\n"
                     u8"l'aventure). Ses déblocages sont pour\n"
                     u8"tout le monde."),
                     npc_label(m.npc).c_str(), npc_label(m.npc).c_str(), list.c_str())
               : fmt(tr(u8"Récompense de %s\n\n"
                     u8"Ton chat a rempli la jauge de %s :\n"
                     u8"sa récompense, des déblocages, est pour\n"
                     u8"tout le monde. Elle arrive dans ta save\n"
                     u8"à son prochain chargement."),
                     npc_label(m.npc).c_str(), npc_label(m.npc).c_str()));
}

} // namespace

// Le detour de CacheInReward, appele aussi par auto_take.
void __fastcall h_CacheReward_entry(void* prog, int npc) {
    const bool ours = npc >= 0 && npc < (int)kNpcGiftNpcs && prog && prog == progress();
    if (net_role() != NetRole::Host || !in_session() || !ours) {
        NpcRead r0;
        const bool was_ready = ours && read_npc(prog, npc, r0) && r0.ready;
        o_CacheReward(prog, npc);
        // Prise hors session (ou par le jeu seul) : un donneur note pour elle ne
        // l'aura pas -- sinon il prendrait la suivante, remplie par l'hote.
        if (was_ready && g.donors[npc].peer != kNpcNoDonor) {
            log_line("PNJ", "%s : recompense prise hors session -- elle reste chez toi, %s ne l'aura pas",
                     npc_name(npc), g.donors[npc].name.c_str());
            g.donors[npc] = NpcDonor{};
        }
        // Les dons retenus pour ce PNJ comptent maintenant (pour la suivante) : a
        // la prochaine image (npcgift_pump), pas dans l'appel du jeu.
        if (was_ready && prog == g.prog && !g.held.empty()) release_held(prog);
        return;
    }
    NpcRead r;
    if (!read_npc(prog, npc, r) || !r.ready) {   // rien a prendre (ou illisible : le jeu decide)
        o_CacheReward(prog, npc);
        return;
    }
    const NpcDonor donor = g.donors[npc];
    const uint8_t  owner = npcgift_reward_owner(donor, r.cur);
    if (owner == kNpcNoDonor) {
        g.donors[npc] = NpcDonor{};
        o_CacheReward(prog, npc);
        return;
    }
    g.donors[npc] = NpcDonor{};
    if (!peer_present(owner) || mm_peer_name(owner) != donor.name) {
        log_line("PNJ", "%s : %s n'est plus la -- sa recompense reste chez toi", npc_name(npc), donor.name.c_str());
        o_CacheReward(prog, npc);
        say(fmt(tr(u8"Récompense de %s\n\n"
                u8"%s, qui avait rempli la jauge de %s,\n"
                u8"n'est plus là : la récompense reste chez toi."),
                npc_label(npc).c_str(), no_markup(donor.name).c_str(), npc_label(npc).c_str()));
        return;
    }
    route_reward(prog, npc, donor);
}

namespace {

void __fastcall h_NpcFavor(void* prog, int npc, void* cat, int amount) {
    // Invite chez lui : la faveur va au PNJ de l'hote, pas a sa copie.
    if (guest_forwards(prog, npc, amount)) {
        NpcGiftMsg m;
        m.op        = kNpcDonate;
        m.npc       = (uint8_t)npc;
        m.favor     = (uint32_t)amount;
        m.cat       = cat_name(cat);
        m.abilities = cat_abilities(cat);
        if (team_send(npcgift_encode(m), kHostPeer)) {
            ++g.sent;
            log_line("PNJ", "-> don de %s a %s envoye a l'hote (sa faveur compte chez lui)",
                     m.cat.empty() ? "un chat" : m.cat.c_str(), npc_name(npc));
            return;
        }
        log_line("PNJ", "!! don a %s : envoi a l'hote impossible -- compte ici seulement", npc_name(npc));
        o_NpcFavor(prog, npc, cat, amount);
        return;
    }
    // Hote : son propre don. Si sa jauge se remplit, la recompense est la sienne.
    if (net_role() == NetRole::Host && in_session() && prog && prog == progress() && npc >= 0 &&
        npc < (int)kNpcGiftNpcs) {
        NpcRead before, after;
        const bool ok = read_npc(prog, npc, before);
        o_NpcFavor(prog, npc, cat, amount);
        if (ok && read_npc(prog, npc, after))
            npcgift_after_donation(g.donors[npc], before.ready, after.ready, kNpcNoDonor, std::string(), before.cur);
        return;
    }
    o_NpcFavor(prog, npc, cat, amount);
}

} // namespace

// ---------------------------------------------------------------------------

bool npcgift_storage_refs() { return net_role() != NetRole::Client && (!g.gifts.empty() || !g.erases.empty()); }

bool npcgift_guest_donates() {
    return net_role() == NetRole::Client && in_session() && !savefile_in_coop() && mm_in_house() &&
           hooks_is_live(T_NpcFavor) && o_NpcFavor != nullptr;
}

void npcgift_pump() {
    if (!g.told) {
        g.told      = true;
        g.armed     = hooks_is_live(T_NpcFavor) && o_NpcFavor != nullptr;
        g.reward_ok = g.armed && hooks_is_live(T_CacheReward) && o_CacheReward != nullptr &&
                      addr_of_data(D_HouseInstance);
        log_line("PNJ", g.reward_ok ? "arme -- les dons des invites aux PNJ comptent chez l'hote, et leurs "
                                      "recompenses vont au donneur"
                      : g.armed     ? "!! crochet CACHEREWARD ou maison introuvable : les dons des invites comptent "
                                      "chez l'hote, mais leurs recompenses restent a l'hote"
                                    : "!! crochet NPCFAVOR introuvable : le tuyau d'un invite refuse ses chats en "
                                      "session, comme avant");
    }
    if (!g.armed) return;
    if (net_role() == NetRole::Host) host_pump();
    if (!in_session() && !g.queue.empty()) {
        // Les chats sont deja partis de chez l'invite : leurs dons comptent quand
        // meme, dans la partie ou ils sont arrives (sinon, ils sont perdus).
        void* prog = progress();
        if (prog && prog == g.prog && mm_in_game()) apply_donations(prog);
        if (!g.queue.empty())
            log_line("PNJ", "!! session finie : %zu don(s) d'invite pas comptes (pas de partie chargee) -- perdus",
                     g.queue.size());
        g.queue.clear();
    }
}

void npcgift_on_team(uint8_t from, const uint8_t* p, size_t n) {
    NpcGiftMsg m;
    if (!npcgift_decode(p, n, m)) {
        log_line("PNJ", "!! message des dons aux PNJ illisible (%zu octets) -- ignore", n);
        return;
    }
    const bool host = net_role() == NetRole::Host;
    switch (m.op) {
    case kNpcDonate: {
        if (!host || from == kHostPeer) break;
        if (g.queue.size() >= kMaxQueue) {
            log_line("PNJ", "!! trop de dons en attente : celui de %s a %s est perdu", mm_peer_name(from).c_str(),
                     npc_name(m.npc));
            break;
        }
        Donation d;
        d.from      = from;
        d.who       = mm_peer_name(from);
        d.npc       = m.npc;
        d.favor     = m.favor;
        d.cat       = m.cat;
        d.abilities = m.abilities;
        g.queue.push_back(std::move(d));
        break;
    }
    case kNpcGot:
        if (host && from != kHostPeer) on_got(from, m);
        break;
    case kNpcReady:
        if (host || from != kHostPeer) break;
        log_line("PNJ", "<- %s : ta recompense est prete, l'hote la prendra pour toi chez lui", npc_name(m.npc));
        say(fmt(tr(u8"Récompense prête\n\n"
                u8"Ton chat a rempli la jauge de %s !\n"
                u8"%s prendra la récompense pour toi\n"
                u8"dès qu'il sera dans sa maison."),
                npc_label(m.npc).c_str(), no_markup(mm_peer_name(kHostPeer)).c_str()));
        break;
    case kNpcGift:
        if (host || from != kHostPeer) break;
        guest_on_gift(m);
        break;
    default:
        break;
    }
}

bool npcgift_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_NpcFavor:     *detour = (void*)&h_NpcFavor;          *original = (void**)&o_NpcFavor;    return true;
    case T_CacheReward:  *detour = (void*)&h_CacheReward_entry; *original = (void**)&o_CacheReward; return true;
    default: return false;
    }
}

} // namespace mgmp
