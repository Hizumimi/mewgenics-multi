// mm_nemesis.cpp -- Mewgenics Multi (protocole 57) : la Nemesis a plusieurs.
// Voir mm_nemesis.h.

#include "mm_nemesis.h"

#include "mgmp_addresses.h"
#include "mgmp_follow.h"
#include "mgmp_log.h"
#include "mgmp_marker.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_rng.h"
#include "mgmp_savefile.h"
#include "mm_menu.h"            // mm_run_peers_mask
#include "mm_vote.h"            // vote_on_enter_node
#include "mgmp_hooks.h"         // hooks_is_live

#include <windows.h>

#include <cstring>

namespace mgmp {
namespace {

// MapScreen (TickNemesis 0x140393590, TryBeginNemesisFight 0x140392ED0).
constexpr uintptr_t kMap_Entity     = 0x18;    // son entite ; entite+8 = la scene
constexpr uintptr_t kEntity_Scene   = 0x08;
constexpr uintptr_t kScene_Dying    = 0x4B0;   // u8 : Scene::NewEntity refuse alors (0x14096B3E6)
constexpr uintptr_t kMap_Pawn       = 0xA0;    // le pion du joueur (MapMarker)
constexpr uintptr_t kMap_Nemesis    = 0xA8;    // le pion de la Nemesis, ou nul
constexpr uintptr_t kMap_NemBattles = 0x218;   // u32
constexpr uintptr_t kMap_NemSpawns  = 0x21C;   // u32
constexpr uintptr_t kMap_NemAdvance = 0x220;   // u32
// MapMarker (vtable 0x140F14A28, 0xC0 octets).
constexpr uintptr_t kMk_Frozen      = 0x78;    // u8 : le pion du joueur, fige pendant la marche
constexpr uintptr_t kMk_Speed       = 0x70;    // double : 2.0 par defaut, 1.0 pour la Nemesis
constexpr uintptr_t kMk_Callback    = 0x80;    // std::function<bool(MapNode*)> (0x40 octets)
constexpr uintptr_t kFn_Impl        = 0x38;    // std::function MSVC : _Impl apres 0x38 octets de stockage
constexpr uintptr_t kNode_Seed0     = 0x118;   // MapNode : premier mot de sa graine (mgmp_follow)

constexpr uint64_t  kNemesisSalt    = 0x4E454D4553495321ull;   // « NEMESIS! »
constexpr uint64_t  kArmMs          = 30000;   // un combat arme qui ne part pas : oublie

using FnFight     = bool  (__fastcall*)(void* map);
using FnFightGo   = void  (__fastcall*)(void* impl);
using FnTick      = void  (__fastcall*)(void* map);
using FnNewEntity = void* (__fastcall*)(void* scene);
using FnMarker    = void* (__fastcall*)(void* scene, void* entity, void* unused, void** node);
using FnSwap      = void  (__fastcall*)(void* a, void* b);
using FnDestroy   = void  (__fastcall*)(void* component);
using FnDelete    = void  (__fastcall*)(void* impl, bool dealloc);

FnFight   o_Fight   = nullptr;
FnFightGo o_FightGo = nullptr;
FnTick    o_Tick    = nullptr;

struct State {
    uint64_t armed_id   = 0;    // le prochain rappel du combat : son flux vient de cet id
    uint64_t armed_at   = 0;
    uint32_t published  = 0;    // hote : combats publies
    uint32_t replayed   = 0;    // invite : combats rejoues
    uint32_t refused    = 0;    // invite : ses propres combats de la Nemesis retenus
    uint32_t reseeded   = 0;
    uint32_t spawned    = 0;    // invite : Nemesis apparues sans tirage
    uint32_t removed    = 0;
    uint32_t snapped    = 0;
    uint32_t ticks      = 0;    // TickNemesis vus en aventure ensemble
    bool     said_refuse = false;
    bool     said_calls  = false;
    bool     said_off    = false;   // crochets ou appels absents : pas de Nemesis ensemble
    bool     said_space  = false;   // une fin de case qui n'est pas celle de la case suivie
    uint64_t said_wait_for = 0;
    uint64_t last_id     = 0;       // hote : le dernier combat publie (pas deux fois)
    uint64_t last_at     = 0;
    uint64_t said_replay_fail = 0;  // invite : ce combat qui ne part pas, deja dit
};
State g;

template <class T> bool rd(const void* base, uintptr_t off, T& out) {
    return base && mem_read((const uint8_t*)base + off, &out, sizeof(out));
}
template <class T> bool wr(void* base, uintptr_t off, const T& v) {
    return base && mem_write((uint8_t*)base + off, &v, sizeof(v));
}

bool in_coop_copy() { return net_role() == NetRole::Client && savefile_in_coop(); }
bool host_with_guests() { return net_role() == NetRole::Host && net_active() && mm_run_peers_mask() != 0; }

// --- appels proteges (pas d'objet C++ dans ces fonctions : __try) ---------------

bool seh_new_entity(FnNewEntity f, void* scene, void** out) {
    __try { *out = f(scene); return true; } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool seh_marker(FnMarker f, void* scene, void* entity, void** node, void** out) {
    __try { *out = f(scene, entity, nullptr, node); return true; } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
// Le rappel de la marche pose dans le pion, comme TickNemesis (0x140393D25-0x140393D6D) :
// une std::function locale {vtable du lambda_2, MapScreen}, _Impl sur elle-meme,
// echangee avec celle du pion ; ce qu'elle tient apres (vide d'habitude) est
// detruit comme le fait le jeu.
bool seh_set_callback(FnSwap swap, uintptr_t vt, void* map, void* marker) {
    __try {
        alignas(16) uint8_t fn[0x40];
        memset(fn, 0, sizeof(fn));
        *(uintptr_t*)fn          = vt;
        *(void**)(fn + 8)        = map;
        *(void**)(fn + kFn_Impl) = fn;
        swap(fn, (uint8_t*)marker + kMk_Callback);
        void* impl = *(void**)(fn + kFn_Impl);
        if (impl) {
            void** v = *(void***)impl;
            ((FnDelete)v[4])(impl, impl != (void*)fn);
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
bool seh_destroy(FnDestroy f, void* component) {
    __try { f(component); return true; } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// Le flux de la simulation (TLS+0x178, xoshiro256) d'apres l'id du combat :
// splitmix64, comme on derive des graines ailleurs. Jamais tout a zero.
void reseed(uint64_t id) {
    uint64_t* s = rng_global_stream();
    if (!s) return;
    uint64_t x = id;
    for (int i = 0; i < 4; ++i) {
        x += 0x9E3779B97F4A7C15ull;
        uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        s[i] = z ^ (z >> 31);
    }
    if (!(s[0] | s[1] | s[2] | s[3])) s[0] = 1;
}

void arm(uint64_t id) {
    g.armed_id = id;
    g.armed_at = GetTickCount64();
}

// Le pion du joueur n'est plus fige (le rappel de la marche de la Nemesis le fait
// a chaque case : 0x14039B067).
void unfreeze_pawn(void* map) {
    void* pawn = nullptr;
    const uint8_t no = 0;
    if (rd(map, kMap_Pawn, pawn) && pawn) wr(pawn, kMk_Frozen, no);
}

// La Nemesis apparait sur `node`, sans tirage (TickNemesis 0x140393CDC-0x140393D6D).
bool spawn_at(void* map, void* node) {
    auto new_entity = (FnNewEntity)addr_of_call(C_SceneNewEntity);
    auto add_marker = (FnMarker)addr_of_data(D_NemesisMarkerFn);
    auto swap       = (FnSwap)addr_of_call(C_FunctionSwap);
    const uintptr_t vt = addr_of_data(D_NemesisWalkVt);
    if (!new_entity || !add_marker || !swap || !vt) {
        if (!g.said_calls) {
            g.said_calls = true;
            log_line("NEMESIS", "!! fonctions du jeu introuvables (Scene::NewEntity, pion de la Nemesis, "
                                "std::function) : la Nemesis de l'hote ne peut pas apparaitre chez ce joueur");
        }
        return false;
    }
    void* entity = nullptr;
    void* scene  = nullptr;
    uint8_t dying = 1;
    if (!rd(map, kMap_Entity, entity) || !entity || !rd(entity, kEntity_Scene, scene) || !scene ||
        !rd(scene, kScene_Dying, dying) || dying)
        return false;
    void* fresh = nullptr;
    if (!seh_new_entity(new_entity, scene, &fresh) || !fresh) return false;
    void* slot   = node;
    void* marker = nullptr;
    if (!seh_marker(add_marker, scene, fresh, &slot, &marker) || !marker) return false;
    const double speed = 1.0;
    if (!wr(map, kMap_Nemesis, marker) || !wr(marker, kMk_Speed, speed)) return false;
    if (!seh_set_callback(swap, vt, map, marker)) {
        log_line("NEMESIS", "!! la Nemesis est apparue mais son rappel de marche n'a pas pu etre pose");
        return false;
    }
    return true;
}

// La Nemesis retiree (la fin de case : 0x140393016).
bool remove_nemesis(void* map, void* marker) {
    auto destroy = (FnDestroy)addr_of_call(C_ComponentDestroy);
    if (!destroy || !seh_destroy(destroy, marker)) return false;
    void* const none = nullptr;
    wr(map, kMap_Nemesis, none);
    unfreeze_pawn(map);
    return true;
}

// --- detours ---------------------------------------------------------------------

// Ce qu'il faut pour jouer la Nemesis ensemble : les deux crochets de son combat
// et les appels qui la font apparaitre ou la retirent chez un invite. Sans eux
// (une autre version du jeu), comme avant le protocole 57 : pas de Nemesis en
// aventure ensemble.
bool coop_ready() {
    return hooks_is_live(T_NemesisFight) && hooks_is_live(T_NemesisFightGo) && o_Fight && o_FightGo &&
           addr_of_call(C_SceneNewEntity) && addr_of_call(C_FunctionSwap) && addr_of_call(C_ComponentDestroy) &&
           addr_of_data(D_NemesisMarkerFn) && addr_of_data(D_NemesisWalkVt);
}

void __fastcall h_Tick(void* map) {
    const bool guest = in_coop_copy();
    const bool coop  = guest || host_with_guests();
    if (coop && !coop_ready()) {
        if (!g.said_off) {
            g.said_off = true;
            log_line("NEMESIS", "!! crochets ou fonctions du jeu absents : pas de Nemesis en aventure ensemble "
                                "(comme avant le protocole 57)");
        }
        return;
    }
    // Chez un invite, seulement a la fin de la case qu'il suit : Espace sur un pion
    // au repos appelle aussi la fin de case (0x14038F508), chez lui seul -- son
    // TickNemesis avancerait la Nemesis et tirerait dans le flux sans l'hote.
    if (guest && !follow_node_end_counts()) {
        if (!g.said_space) {
            g.said_space = true;
            log_line("NEMESIS", "fin de case hors d'une case suivie (Espace ?) -- pas de TickNemesis chez cet invite");
        }
        return;
    }
    uint32_t spawns_before = 0;
    if (coop) rd(map, kMap_NemSpawns, spawns_before);
    o_Tick(map);
    if (!coop) return;
    ++g.ticks;
    NemesisState s;
    if (!nemesis_read(map, s)) return;
    if (s.spawns != spawns_before)
        log_line("NEMESIS", "la Nemesis apparait sur la case %u (apparition %u, apres %u combat(s))",
                 s.node, s.spawns, s.battles);
    else if (s.node != kNemNoNode)
        log_line("NEMESIS", "la Nemesis avance (case %u, avance %u)", s.node, s.advance);
}

bool __fastcall h_Fight(void* map) {
    // Invite dans la copie : son combat de la Nemesis vient de l'hote (le suivi le
    // rejoue, par l'original). Ici, c'est le rappel d'une marche chez lui.
    if (in_coop_copy()) {
        ++g.refused;
        if (!g.said_refuse) {
            g.said_refuse = true;
            log_line("NEMESIS", "combat de la Nemesis : chez un invite, il attend celui de l'hote");
        }
        return false;
    }
    const bool started = o_Fight(map);
    if (!started || !host_with_guests() || !coop_ready()) return started;
    // Hote, des invites dans l'aventure : le combat part comme une case.
    void* pawn = nullptr;
    void* node = nullptr;
    if (!rd(map, kMap_Pawn, pawn) || !pawn || !rd(pawn, kMk_At, node)) node = nullptr;
    uint32_t spawns = 0;
    rd(map, kMap_NemSpawns, spawns);
    const uint64_t id = node ? nemesis_battle_id(map, node)
                             : kNemesisSalt ^ ((uint64_t)spawns << 32) ^ GetTickCount64();
    // Le meme combat deux fois (le jeu ne le fait pas aujourd'hui) : une publication.
    const uint64_t now = GetTickCount64();
    if (id == g.last_id && now - g.last_at < kArmMs) return started;
    g.last_id = id;
    g.last_at = now;
    arm(id);
    if (follow_on_nemesis_fight(map, node, id)) ++g.published;
    return started;
}

void __fastcall h_FightGo(void* impl) {
    if (g.armed_id && GetTickCount64() - g.armed_at < kArmMs) {
        reseed(g.armed_id);
        ++g.reseeded;
        log_line("NEMESIS", "combat de la Nemesis %016llx : son flux vient de son id, chez tous",
                 (unsigned long long)g.armed_id);
    }
    g.armed_id = 0;
    o_FightGo(impl);
}

} // namespace

// ---------------------------------------------------------------------------------

bool nemesis_read(void* map, NemesisState& out) {
    out = NemesisState{};
    void* marker = nullptr;
    if (!map || !rd(map, kMap_NemBattles, out.battles) || !rd(map, kMap_NemSpawns, out.spawns) ||
        !rd(map, kMap_NemAdvance, out.advance) || !rd(map, kMap_Nemesis, marker))
        return false;
    out.node = kNemNoNode;
    if (marker) {
        void* node = nullptr;
        uint32_t index = 0;
        if (!rd(marker, kMk_At, node) || !node || !follow_index_of_node(map, node, index)) return false;
        out.node = index;
    }
    out.valid = true;
    return true;
}

void nemesis_fill(const NemesisState& s, EnterNodeMsg& m) {
    m.nem_flags   = s.valid ? kNemValid : 0;
    m.nem_battles = s.battles;
    m.nem_spawns  = s.spawns;
    m.nem_advance = s.advance;
    m.nem_node    = s.node;
}

NemesisState nemesis_from(const EnterNodeMsg& m) {
    NemesisState s;
    s.valid   = (m.nem_flags & kNemValid) != 0;
    s.battles = m.nem_battles;
    s.spawns  = m.nem_spawns;
    s.advance = m.nem_advance;
    s.node    = m.nem_node;
    return s;
}

NemApply nemesis_apply(void* map, const NemesisState& s, uint64_t waited_ms, const char* why) {
    if (!map || !s.valid) return NemApply::Done;      // rien de l'hote : comme avant
    void* marker = nullptr;
    if (!rd(map, kMap_Nemesis, marker)) return NemApply::Failed;
    void* node = nullptr;
    if (s.node != kNemNoNode) {
        node = follow_node_at(map, s.node);
        if (!node) {
            log_line("NEMESIS", "!! case %u de la Nemesis de l'hote absente de cette carte (%s)", s.node, why);
            return NemApply::Failed;
        }
        if (marker) {
            void* at     = nullptr;
            void* next   = nullptr;
            void* target = nullptr;
            if (!rd(marker, kMk_At, at) || !rd(marker, kMk_Next, next) || !rd(marker, kMk_Target, target))
                return NemApply::Failed;
            const bool walking = next != nullptr || target != nullptr;
            if (at != node && walking && waited_ms < kNemesisWalkWaitMs) {
                // Elle marche encore (la marche suit les images) : on lui laisse
                // le temps d'arriver.
                if (g.said_wait_for != (uint64_t)(uintptr_t)node) {
                    g.said_wait_for = (uint64_t)(uintptr_t)node;
                    log_line("NEMESIS", "la Nemesis de ce joueur marche encore -- la case %u de l'hote "
                                        "attend qu'elle arrive (%s)", s.node, why);
                }
                return NemApply::Wait;
            }
            // Ailleurs (au repos, ou trop longtemps en marche), ou sur la case mais
            // encore en marche (elle en repartirait) : au repos sur celle de l'hote.
            if (at != node || walking) {
                if (!marker_place(marker, node)) return NemApply::Failed;
                unfreeze_pawn(map);
                ++g.snapped;
                log_line("NEMESIS", "la Nemesis de ce joueur est recalee au repos sur la case %u, comme chez "
                                    "l'hote (%s)", s.node, why);
            }
        }
    }
    // L'ecart de ses compteurs avec ceux de l'hote : sa fin de case n'a pas fait le
    // meme TickNemesis (ou il arrive en cours de carte). NODEHASH ne les compte pas.
    {
        uint32_t b = 0, sp = 0, adv = 0;
        rd(map, kMap_NemBattles, b);
        rd(map, kMap_NemSpawns, sp);
        rd(map, kMap_NemAdvance, adv);
        if (b != s.battles || sp != s.spawns)
            log_line("NEMESIS", "compteurs de la Nemesis recopies de l'hote : combats %u -> %u, apparitions %u -> %u, "
                                "avance %u -> %u (%s)", b, s.battles, sp, s.spawns, adv, s.advance, why);
    }
    // Les compteurs de l'hote : la fin de case fera ici le meme TickNemesis.
    wr(map, kMap_NemBattles, s.battles);
    wr(map, kMap_NemSpawns, s.spawns);
    wr(map, kMap_NemAdvance, s.advance);
    g.said_wait_for = 0;
    if (s.node == kNemNoNode) {
        if (!marker) return NemApply::Done;
        if (!remove_nemesis(map, marker)) {
            log_line("NEMESIS", "!! la Nemesis de ce joueur n'a pas pu etre retiree (l'hote n'en a pas) (%s)", why);
            return NemApply::Failed;
        }
        ++g.removed;
        log_line("NEMESIS", "la Nemesis de ce joueur est retiree : l'hote n'en a pas (%s)", why);
        return NemApply::Done;
    }
    if (marker) return NemApply::Done;
    if (!spawn_at(map, node)) {
        log_line("NEMESIS", "!! la Nemesis de l'hote (case %u) n'a pas pu apparaitre chez ce joueur (%s)",
                 s.node, why);
        return NemApply::Failed;
    }
    ++g.spawned;
    log_line("NEMESIS", "la Nemesis de l'hote apparait chez ce joueur sur la case %u, sans tirage (%s)",
             s.node, why);
    return NemApply::Done;
}

uint64_t nemesis_battle_id(void* map, void* node) {
    uint32_t spawns = 0;
    uint64_t seed   = 0;
    rd(node, kNode_Seed0, seed);       // MapNode+0x118 : la graine de la case, la meme chez tous
    rd(map, kMap_NemSpawns, spawns);
    return seed ^ kNemesisSalt ^ ((uint64_t)spawns * 0x9E3779B97F4A7C15ull);
}

bool nemesis_replay_fight(void* map, uint64_t id) {
    if (!o_Fight || !map) return false;
    arm(id);
    const bool ok = o_Fight(map);     // la trampoline : pas de detour, pas de refus
    if (ok) {
        ++g.replayed;
        vote_on_enter_node();         // comme une case suivie : le vote en cours ne vaut plus
    } else {
        g.armed_id = 0;
        if (g.said_replay_fail != id) {
            g.said_replay_fail = id;
            log_line("NEMESIS", "le combat de la Nemesis de l'hote ne part pas encore chez ce joueur (pas de "
                                "Nemesis sur la case de son pion, ou une transition en cours) -- nouvel essai");
        }
    }
    return ok;
}

bool nemesis_binding(Target t, void** detour, void*** original) {
    switch (t) {
    case T_TickNemesis:    *detour = (void*)&h_Tick;    *original = (void**)&o_Tick;    return true;
    case T_NemesisFight:   *detour = (void*)&h_Fight;   *original = (void**)&o_Fight;   return true;
    case T_NemesisFightGo: *detour = (void*)&h_FightGo; *original = (void**)&o_FightGo; return true;
    default: return false;
    }
}

void nemesis_shutdown() {
    if (!g.ticks && !g.published && !g.replayed && !g.refused && !g.spawned && !g.removed && !g.snapped) return;
    log_line("NEMESIS", "fin : %u TickNemesis en aventure ensemble, %u combat(s) publie(s), %u rejoue(s), "
                        "%u retenu(s), %u flux recharge(s), %u apparition(s) sans tirage, %u retrait(s), "
                        "%u recalage(s)",
             g.ticks, g.published, g.replayed, g.refused, g.reseeded, g.spawned, g.removed, g.snapped);
    g = State{};
}

} // namespace mgmp
