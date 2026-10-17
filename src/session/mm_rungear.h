#pragma once
// mm_rungear -- Mewgenics Multi : les objets pendant l'aventure, en direct. Un
// seul sac pour l'equipe (celui du jeu) ; chacun n'equipe et ne desequipe que ses
// propres chats ; chaque mouvement d'objet part chez les autres tout de suite.
//
// LE JEU (1.1.21239)
//   L'ecran des objets : InventoryScreen2 (vtable 0x140F13D00), mode 0 (+0x58) en
//     aventure : le sac de l'aventure. +0x108 la cle du chat montre. Ses boites
//     (InventoryItemBox) sont creees a l'ouverture pour chaque objet du sac et des
//     chats de l'equipe, et retrouvent leur objet par son ID (Equipment+0x00) a
//     chaque image (0x14034BB50 : la carte id -> chat ; une boite dont l'id n'est
//     plus nulle part est retiree et detruite).
//   Un clic : InventoryItemBox::click (confirmations du jeu), puis son lambda_1
//     (0x14034F0A0, T_InvItemAct) : objet porte -> rendu au sac (CatData, 0x1400CA510,
//     T_CatUnequip) ; objet du sac -> equipe au chat montre (0x1400CA2E0,
//     T_CatEquip) ; un livre (genre 5) -> sa capacite apprise, ou un
//     AbilityChooser dont le rappel (0x140352520 / 0x140352430, T_InvLearn*)
//     remplace celle choisie, puis le livre quitte le sac.
//   Equiper et rendre deplacent l'objet entre le sac (std::unordered_map<id,
//   Equipment>, Inventory+56) et l'emplacement du chat (CatData+0x9B0, cinq de
//   0x60 octets) en GARDANT son id : les boites de l'ecran le suivent, meme ouvert.
//
// AVANT (build 20). Une « session » par ecran ouvert, une photo des chats et du sac
// a la fermeture, prise ou refusee par l'hote, des retours en arriere ; et un
// bogue de relecture qui dupliquait les objets rendus au sac (mgmp_catsync,
// clear_equipment).
//
// CE QUE FAIT LE MOD (en aventure ensemble, hors du choix des objets du depart)
//   - Chez chacun, les objets du chat montre sont grises et leurs clics retenus
//     quand ce chat n'est pas a ce joueur ; le lambda_1 verifie encore le chat
//     vise (le porteur de l'objet, sinon le chat montre) au moment d'agir.
//   - Un mouvement fait par ce joueur (equiper, rendre au sac, lire un livre) est
//     note pendant l'appel du jeu : l'objet par son EMPREINTE (les champs que le
//     jeu sauve, pas son id), l'emplacement, ce qu'il tenait avant ; pour un
//     livre, le chat tel qu'il est apres. Il part aussitot (MSG_TEAM kTeamItemOp) :
//     l'hote l'envoie a tous ; un invite le joue chez lui tout de suite et l'envoie
//     a l'hote.
//   - L'hote prend le mouvement d'un invite sur la carte, hors d'une case : chat a
//     lui, objet encore la (sinon refus : un autre l'a pris avant) ; il le joue avec
//     la meme fonction du jeu, l'envoie aux autres et repond (kTeamItemAck).
//   - Chez un invite, les mouvements de l'hote se jouent dans l'ordre, sur la
//     carte, ecran des objets ouvert ou non (les boites suivent les ids). Une
//     poussee de l'hote (entree dans une case, retour sur la carte) contient tout
//     ce qu'il avait fait avant : un mouvement arrive avant une poussee appliquee
//     est laisse (follow_note_arrival). Une poussee identique a son etat est
//     laissee sans relire (catsync/invsync_drop_identical), meme sac ouvert.
//   - Un mouvement refuse, ou un mouvement de l'hote qui ne se joue pas ici :
//     l'invite demande l'etat de l'hote (kTeamItemSyncReq, apres ses propres
//     mouvements) et le recoit (kTeamItemSync : sac, poubelle, chats) ; il le
//     relit sac ferme, sur la carte ; d'ici la, son ecran est grise. A chaque
//     entree dans une case, l'hote pousse TOUS les chats et le sac : ce qui
//     aurait diverge sur la carte est remis avant la case.
//   - Un invite n'equipe que sur la carte, sans case de l'hote a suivre, l'hote
//     revenu de la case que l'invite a suivie en dernier (MM_RUNREV, drapeau
//     « pose »), sans poussee ni etat de l'hote en attente. L'hote, dans une case,
//     garde ses changements pour sa poussee de retour sur la carte.
//   - « Sur la carte » veut dire ENTRE DEUX CASES (follow_node_live,
//     follow_inside_node, aucune case a suivre), pas « la carte tourne » : l'ecran
//     des objets met la carte en pause (comme toute fenetre du jeu), et un test
//     sur follow_on_map prenait un hote sac ouvert pour un hote dans une case --
//     aucun de ses mouvements ne partait -- et un invite sac ouvert pour un invite
//     dans une case -- tous ses clics etaient retenus (build 22). La carte qui
//     tourne ne sert plus qu'a la poussee d'apres une case, aux poussees gardees et
//     aux fenetres de ce module.
//   - L'invite ne suit l'hote dans une case qu'une fois son ecran des objets
//     ferme.
//   - Un livre lu ailleurs pendant qu'une confirmation ou un AbilityChooser de
//     ce livre est ouvert ici : la boite et le noeud du sac qu'ils tiennent sont
//     detruits (InventoryItemBox::update retire une boite sans objet). Le oui de
//     la confirmation (un objet qui n'est plus nulle part) et le rappel du livre
//     (son objet n'est plus ce noeud du sac) sont retenus, avec une fenetre.
//   - Un livre lu ne touche pas aux objets du chat : si ceux du chat recu ne sont
//     pas ceux d'ici (un mouvement refuse que son auteur ne sait pas encore), le
//     livre est refuse et le chat remis.
//   - Un etat de l'hote arrive entre l'envoi d'un mouvement et sa reponse (la
//     poussee entiere pour un joueur qui arrive) l'a fait sans lui : a la
//     reponse, l'invite redemande l'etat de l'hote. Un mouvement que l'hote joue
//     en partie (refuse, le jeu change quand meme) : ses poussees le disent a tous.
//
// LIMITES. Les livres (genre 5) et ce que le jeu fait d'autre pendant un clic
// d'objet passent par le chat serialise du proprietaire. Deux joueurs qui
// prennent le meme objet a la meme fraction de seconde : le second est refuse,
// son sac se remet a jour quand il le ferme. Pendant une case (boutique...),
// un invite ne change pas ses objets.

#include <cstddef>
#include <cstdint>

namespace mgmp {

enum Target : int;
struct MmMsg;

// Detours et originaux de T_InvScreenUpdate, T_InvScreenDtor, T_NodeEnd,
// T_InvItemAct, T_InvLearnActive, T_InvLearnPassive, T_CatEquip et T_CatUnequip.
bool rungear_binding(Target t, void** detour, void*** original);

// Une aventure ensemble (au moins un invite chez l'hote ; l'invite : dans la
// copie de l'hote), hors du choix des objets du depart.
bool rungear_active();

// Un ecran des objets est ouvert chez ce joueur.
bool rungear_screen_open();

// Les poussees de l'hote doivent etre gardees, pas appliquees a leur arrivee :
// un ecran des objets ouvert (ses boites tiennent les objets que la relecture
// remplacerait).
bool rungear_hold_pushes();

// L'ecran `screen` (InventoryScreen2) : ce joueur peut-il toucher aux objets du
// chat qu'il montre ? Vrai hors d'une aventure ensemble.
bool rungear_can_touch(void* screen);

// Un clic de ce joueur (Button::Click). Vrai = retenu.
bool rungear_on_click(void* button);

// Chaque image (mm_menu_pump).
void rungear_pump();

// Hote : juste avant ses poussees d'entree dans une case (follow_on_enter_node) :
// elles seront entieres (tous les chats, le sac), pas seulement ce qui a change.
void rungear_before_node_push();

// Hote : il entre dans la case de graine `seed` (follow_on_enter_node), apres
// ses poussees et avant ENTERNODE.
void rungear_on_node_entered(uint64_t seed);

// Invite : il suit l'hote dans la case de graine `seed` (follow_map_update,
// juste avant EnterNode, apres les poussees d'avant la case).
void rungear_on_followed(uint64_t seed);

// Invite : faut-il attendre avant de suivre l'hote dans une case
// (follow_map_update) ? Oui tant qu'un ecran des objets est ouvert.
bool rungear_hold_follow();

// MM_RUNREV (invite) ; MSG_TEAM kTeamItemOp, kTeamItemAck, kTeamItemSync,
// kTeamItemSyncReq.
void rungear_on_mm(uint8_t from, const MmMsg& m);
void rungear_on_team(uint8_t from, const uint8_t* data, size_t size);

} // namespace mgmp
