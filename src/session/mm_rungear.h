#pragma once
// mm_rungear -- Mewgenics Multi : les objets pendant l'aventure. Un seul sac
// pour l'equipe (celui du jeu) ; chacun n'equipe et ne desequipe que ses
// propres chats ; ce que l'un change arrive chez les autres.
//
// LE JEU (1.1.21239)
//   L'ecran des objets : InventoryScreen2 (vtable 0x140F13D00), le meme qu'au
//     depart (mm_gear) ; en aventure, mode 0 (+0x58), le sac de l'aventure.
//     +0x108 la cle du chat montre, recopiee de son CatSelector (+0x50, +0x88)
//     a chaque image par son update (slot 10, 0x14034D130) ; son destructeur
//     est le slot 5 (0x140387310). Ses boites (InventoryItemBox, +0x20)
//     tiennent les Equipment du sac et des chats : tant qu'il est ouvert, rien
//     ne doit relire le sac ni un chat (le lecteur du jeu vide puis remplit,
//     et libere les objets -- voir mgmp_invsync.h).
//   Un clic sur un objet (boutons « InventoryButton_* » et « EquippedButton_* »)
//     agit sur le chat montre : equiper depuis le sac, ou rendre au sac.
//
// AVANT. L'hote poussait ses chats et son sac a chaque case (mgmp_catsync,
// mgmp_invsync) et rien ne revenait : ce qu'un invite changeait disparaissait a
// la case suivante -- ou pire, restait chez lui seul (l'hote ne renvoie pas un
// chat qu'il n'a pas change) et il entrait en combat avec d'autres objets que
// chez l'hote. Et chacun pouvait toucher aux chats de tous.
//
// CE QUE FAIT LE MOD (en aventure ensemble, hors du choix des objets du depart)
//   - Chez chacun, sur l'ecran des objets, les objets sont grises et leurs
//     clics retenus quand le chat montre n'est pas a ce joueur (proprietaire :
//     l'equipe mixte, team_owner_of ; l'hote a defaut).
//   - L'hote tient la REVISION de l'aventure (MM_RUNREV, avec la graine de sa
//     derniere case et deux drapeaux) : +1 a chaque case ou il entre, a chaque
//     poussee qui change quelque chose, a chaque photo refusee. Elle part apres
//     les poussees qu'elle couvre (le meme canal), et toutes les 2 s.
//   - L'hote pousse ses chats et son sac : en entrant dans une case (comme
//     avant), une fois de retour sur la carte apres la case (drapeau « pose » :
//     son etat d'apres la case est parti), quand son ecran des objets se ferme,
//     et avant et apres chaque photo d'invite.
//   - Un invite n'a une SESSION (il peut equiper ses chats) que sur la carte,
//     sans case de l'hote a suivre, sans poussee gardee, sans photo sans
//     reponse, l'hote pose apres la case que l'invite a suivie en dernier, et
//     sans ecran des objets ouvert chez l'hote. Sinon l'ecran reste grise (une
//     fenetre, apres coup, dit pourquoi) ; la session s'ouvre d'elle-meme quand
//     plus rien ne l'empeche, si rien n'a bouge depuis l'ouverture.
//   - A la fermeture d'une session qui a change quelque chose, une photo de
//     ses chats, du sac et de la poubelle part chez l'hote (MSG_TEAM
//     kTeamRunGear) avec la revision de l'ouverture. L'hote la prend sur la
//     carte, sans ecran ouvert, apres avoir pousse ses propres changements, si
//     sa revision n'a pas bouge (le sac et la poubelle, puis ses chats d'invite :
//     chacun a lui, le meme, meme classe ; tout ou rien) et pousse le tout ;
//     sinon il refuse et renvoie tout (kTeamRunGearAck).
//   - L'invite garde la photo d'ouverture. Ses objets y reviennent quand la
//     sienne ne sera jamais prise : l'hote entre dans une case avant de
//     repondre (ou pendant la session), pas de reponse en 20 s, refus (puis
//     les poussees de l'hote par-dessus), envoi impossible. Un ecran sans
//     session dont le sac a bouge (un tri) revient aussi comme a l'ouverture.
//   - L'invite ne suit l'hote dans une case qu'une fois son ecran des objets
//     ferme ; juste avant, ce qui ne sera pas pris revient (voir ci-dessus).
//   - Les poussees gardees (mgmp_catsync, mgmp_invsync, mgmp_runhist) portent
//     leur rang d'arrivee (follow_epoch) : avant une case, seules celles
//     d'avant son ENTERNODE s'appliquent. L'invite applique les autres sur la
//     carte des que c'est sans danger.
//   - Les fenetres de ce module ne s'ouvrent pas quand l'invite va suivre
//     l'hote, et se referment s'il le doit (kInfoYieldToFollow) : sa carte
//     arretee, l'hote l'attendrait.
//   - Un ecran dont l'update ne tourne plus depuis 120 images et dont la vtable
//     a change (detruit sans passer par le slot 5) est oublie : sinon l'invite
//     attendrait sa fermeture pour toujours avant de suivre l'hote. Sans les
//     deux crochets, rien de tout cela (comme avant).
//
// LIMITES. Pas de fusion : deux joueurs qui changent le sac en meme temps, le
// second est refuse (il recommence). Ce que l'invite change pendant une case
// (boutique...) ne compte pas : il le refait sur la carte. L'hote attend, dans
// un combat, que l'invite ferme son sac pour le suivre.

#include <cstddef>
#include <cstdint>

namespace mgmp {

enum Target : int;
struct MmMsg;

// Detours et originaux de T_InvScreenUpdate et T_InvScreenDtor.
bool rungear_binding(Target t, void** detour, void*** original);

// Une aventure ensemble (au moins un invite chez l'hote ; l'invite : dans la
// copie de l'hote), hors du choix des objets du depart.
bool rungear_active();

// Un ecran des objets est ouvert chez ce joueur.
bool rungear_screen_open();

// Les poussees de l'hote doivent etre gardees, pas appliquees a leur arrivee :
// un ecran des objets ouvert, ou une photo de ce joueur sans reponse ou refusee
// (ses objets vont revenir comme a l'ouverture, puis les poussees par-dessus).
bool rungear_hold_pushes();

// L'ecran `screen` (InventoryScreen2) : ce joueur peut-il toucher aux objets du
// chat qu'il montre ? Vrai hors d'une aventure ensemble.
bool rungear_can_touch(void* screen);

// Un clic de ce joueur (Button::Click). Vrai = retenu.
bool rungear_on_click(void* button);

// Chaque image (mm_menu_pump).
void rungear_pump();

// Hote : il entre dans la case de graine `seed` (follow_on_enter_node), apres
// ses poussees et avant ENTERNODE.
void rungear_on_node_entered(uint64_t seed);

// Invite : il suit l'hote dans la case de graine `seed` (follow_map_update,
// juste avant EnterNode).
void rungear_on_followed(uint64_t seed);

// Invite : faut-il attendre avant de suivre l'hote dans une case
// (follow_map_update) ? Oui tant qu'un ecran des objets est ouvert. Sinon, ce
// qui ne sera jamais pris chez l'hote (photo sans reponse, session fermee
// apres son entree) revient comme a l'ouverture, ici, avant de suivre.
bool rungear_hold_follow();

// MM_RUNREV (invite) ; MSG_TEAM kTeamRunGear (hote) / kTeamRunGearAck (invite).
void rungear_on_mm(uint8_t from, const MmMsg& m);
void rungear_on_team(uint8_t from, const uint8_t* data, size_t size);

} // namespace mgmp
