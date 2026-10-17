#pragma once
// mm_ai -- Mewgenics Multi : l'IA du jeu joue les chats d'un joueur absent.
//
// LA REGLE (document, « Mort, spectateur, nouveaux chats, deconnexion et IA »).
// Un joueur qui part en plein combat (deconnecte, ou rentre chez lui), ou qui
// n'est pas membre du combat a l'entree dans la case, laisse ses chats a l'hote
// (mgmp_members.h). Avant (builds 27 a 31), c'etait le joueur de l'hote qui les
// jouait ; maintenant c'est l'IA du jeu, jusqu'au combat suivant ou il revient.
//
// LE JEU (1.1.21239). Le jeu a deja une IA qui joue un chat a la place de son
// joueur : celle des passifs Uncontrollable (objet Brain Chip, Deja Vu severe),
// Charmed et ProbeCharmed. Brain::GetChoice (0x140138590) demande a chaque passif
// du chat (entree 111, 0x140768530) une decision a la place du joueur ; pour ces
// trois-la, l'entree 112 (0x140082B00, C_AiChoice) fait, si le cerveau est un
// PlayerBrain, ce que fait le cerveau d'un ennemi (GenericBrain::decide,
// 0x1408B48D0) : les pouvoirs utilisables, puis le choix de l'IA (0x1408B44D0).
// Elle ne lit du passif que son Character (+0x38) : un faux passif de 0x40 octets
// suffit.
//
// CE QUE FAIT LE MOD. Chez l'hote seul, pour un chat d'un absent (members_ai_cats)
// dont c'est le tour : il attend un instant (kAiThinkMs), demande sa decision a
// cette IA, puis la publie comme les siennes (ACTION) -- chez chacun, invites
// compris, c'est la meme TurnAction, rebatie depuis le message. Les invites
// n'appellent jamais l'IA : la decision leur arrive, comme celle d'un joueur.
//
// CE QUE L'APPEL NE DOIT PAS LAISSER (il ne tourne que chez l'hote).
//   - Le flux de la simulation (TLS+0x178) : l'IA y tire (son tirage pour
//     departager, ses chemins). Remis comme avant, et l'enregistreur de vol
//     rembobine (mgmp_rng.h).
//   - L'orientation de fin de tour : si l'IA choisit de finir le tour, elle tourne
//     le chat (sub_14013C460 -> Character::Face, un tirage inline pour
//     departager) -- un effet hors de la TurnAction. Brain+0x2B1 (« auto_orient »)
//     est mis a 0 le temps de l'appel.
//   - L'etat des chats : la cloture d'etat de mgmp_lockstep (PV, case, orientation,
//     elements de chaque chat) entoure l'appel ; une orientation est remise, le
//     reste serait signale.
//   - La decision gardee par le cerveau (Brain+0x220) : l'IA l'ecrit, mais
//     Brain::UpdateDecision la remplace par ce que rend GetChoice, juste apres.
//   - La std::function de la TurnAction rendue : detruite comme le fait le jeu.

#include <cstddef>
#include <cstdint>

#include "mgmp_turnaction.h"

namespace mgmp {

// L'IA du jeu est-elle la (C_AiChoice resolu) ? Sinon, comme avant : le joueur de
// l'hote joue les chats des absents.
bool ai_available();

// La decision de l'IA du jeu pour le chat de ce cerveau (un PlayerBrain), dans
// `out`. Faux si l'appel n'a pas pu se faire ou a plante (`why` dit pourquoi) ;
// vrai sinon, `out` tel que l'IA l'a rendu, sa std::function deja detruite (a
// verifier : type 2 ou 3, mm_ai_core.h). Thread du jeu, dans Brain::GetChoice.
bool ai_choose(void* brain, TurnAction& out, const char** why);

// Pour le journal en fin de session : decisions demandees, tirages remis, appels
// apres lesquels la cloture d'etat a vu un chat change (PV, case, elements, mort :
// une divergence certaine, dite en erreur la premiere fois), appels apres
// lesquels seule une orientation avait bouge (remise, sans consequence). Remis a
// zero a chaque session (lockstep_init).
uint32_t ai_calls();
uint32_t ai_rewinds();
uint32_t ai_fence_hits();
uint32_t ai_faced();
void     ai_reset_counters();

// Avant d'ecrire une TurnAction entiere par-dessus celle que le jeu a rendue
// (h_GetChoice) : sa std::function detruite, s'il y en a une -- sinon elle fuit
// (le destructeur du jeu ne verrait plus qu'un _Impl nul). Faux si la lecture a
// echoue (exception attrapee).
bool ai_clear_function(void* turn_action);

} // namespace mgmp
