#pragma once
// mm_nemesis -- Mewgenics Multi (protocole 57) : la Nemesis a plusieurs.
//
// CE QUE FAIT LE JEU (build 1.1.21239)
//   - MapScreen::TickNemesis (0x140393590), appelee par la fin de case
//     (0x140392FC0) apres un combat (types 5 a 9) : MapScreen+0x218 compte les
//     combats. Sans Nemesis, quand ce compte atteint « head_start » (GON de la
//     carte, cle « nemesis » ; puis « repeat », ou jamais avec « never »), elle
//     apparait sur une case tiree au sort dans le FLUX DE LA SIMULATION
//     (randint, TLS+0x178) : une entite neuve, son pion (MapMarker, icone
//     « MapNemesisIcon ») a MapScreen+0xA8, +0x21C compte les apparitions, et le
//     rappel de sa marche (lambda_2, vtable 0x140F14DC0) dans le pion+0x80. Deja
//     la : +0x218 = 0, +0x220 = « advance », et elle fait un pas vers le pion du
//     joueur (0x140224050 -> pion de la Nemesis+0x58), pion du joueur fige
//     (+0x78) pendant la marche. Rien de cela n'est sauve avec la carte.
//   - MapScreen::TryBeginNemesisFight (0x140392ED0) : la Nemesis sur la case du
//     pion -> une CircleTransition dont le rappel (0x14039D510) construit son
//     combat (« nemesis », GON levels) et le lance (tirage d'embuscade dans le
//     flux). Appelee par le rappel de la marche de la Nemesis et par celui du
//     pion, avant EnterNode : pas d'ENTERNODE, pas de graine de case.
//   - La fin de case d'apres ce combat detruit la Nemesis (0x14094EBA0) au lieu
//     de marquer la case visitee.
//
// CE QUE FAIT LE MOD
//   - TickNemesis tourne chez tous. Chaque ENTERNODE de l'hote porte sa Nemesis
//     d'avant la case (compteurs, case ou elle est) ; l'invite la recopie juste
//     avant d'entrer (nemesis_apply) : meme etat + meme flux = meme TickNemesis a
//     la fin de case. Arrive en cours de carte, il n'a pas de Nemesis : elle
//     apparait chez lui sur la case de celle de l'hote, sans tirage ; une Nemesis
//     qui marche encore vers une autre case est attendue un moment, puis recalee.
//   - Le combat : chez l'hote, des invites dans l'aventure, TryBeginNemesisFight
//     qui part publie le combat comme une case (mgmp_follow, ENTERNODE
//     kEnterNemesis : la case du pion, l'id du combat, les membres). Chez un
//     invite dans la copie, son propre combat de la Nemesis est retenu ; le suivi
//     amene son pion sur la case et rejoue celui de l'hote (nemesis_replay_fight).
//   - Le flux de la simulation de ce combat vient de son id, chez tous (le
//     rappel du combat le recharge, comme EnterNode le fait depuis la graine
//     d'une case) : un invite arrive en cours de route a le meme combat.

#include <cstdint>

namespace mgmp {

enum Target : int;
struct EnterNodeMsg;

// La Nemesis d'une carte, telle que l'hote la joint a une case.
struct NemesisState {
    bool     valid   = false;          // l'hote a pu la lire
    uint32_t battles = 0;              // MapScreen+0x218
    uint32_t spawns  = 0;              // MapScreen+0x21C
    uint32_t advance = 0;              // MapScreen+0x220
    uint32_t node    = 0xFFFFFFFFu;    // l'index de sa case, ou kNemNoNode (pas de Nemesis)
};

// Hote : la Nemesis de `map` maintenant. Faux si la carte ne se lit pas.
bool nemesis_read(void* map, NemesisState& out);
void nemesis_fill(const NemesisState& s, EnterNodeMsg& m);
NemesisState nemesis_from(const EnterNodeMsg& m);

// Invite : l'etat de l'hote recopie sur sa carte -- compteurs ; sa Nemesis
// apparue, retiree ou recalee sur la case de celle de l'hote. `waited_ms` : depuis
// quand la case attend ; une Nemesis qui marche encore vers une autre case est
// attendue jusqu'a kNemesisWalkWaitMs (Wait), puis recalee.
enum class NemApply { Done, Wait, Failed };
constexpr uint64_t kNemesisWalkWaitMs  = 4000;
// Invite : le combat de la Nemesis de l'hote qui ne se rejoue pas (etat
// impossible a recopier, TryBeginNemesisFight qui refuse) est reessaye ce temps.
constexpr uint64_t kNemesisFightRetryMs = 6000;
NemApply nemesis_apply(void* map, const NemesisState& s, uint64_t waited_ms, const char* why);

// L'id du combat de la Nemesis sur la case `node` de `map` : la graine de la case,
// un sel et le compte des apparitions (une Nemesis ne se bat qu'une fois).
uint64_t nemesis_battle_id(void* map, void* node);

// Invite : le combat de la Nemesis de l'hote, rejoue (TryBeginNemesisFight,
// l'original) ; son flux viendra de `id`. Vrai si le combat part.
bool nemesis_replay_fight(void* map, uint64_t id);

// Les detours (T_TickNemesis, T_NemesisFight, T_NemesisFightGo).
bool nemesis_binding(Target t, void** detour, void*** original);

// Une ligne au journal en fin de session.
void nemesis_shutdown();

} // namespace mgmp
