#pragma once
// mm_gear -- Mewgenics Multi : le choix des objets au depart ensemble (scene
// « StorageItems »). Chacun equipe ses propres chats avec les objets de son
// propre rangement ; personne ne touche aux chats des autres.
//
// LE JEU (1.1.21239)
//   L'ecran : InventoryScreen2 (vtable 0x140F13D00, init 0x140349F70), ouvert
//     par 0x1403B0BB0 en mode 1 (+0x58 : 1 = rangement de la maison, 0 = le sac
//     de l'aventure). +0x50 son CatSelector, +0x108 la cle du chat montre
//     (recopiee de CatSelector+0x88 a chaque image, update 0x14034D130).
//     +0xAC/+0xB0 le nombre et le tableau de ses boites.
//   Ses boites : InventoryItemBox (vtable 0x140F11828), une par objet, creees a
//     l'ouverture seulement (0x140351880). +0x38 l'ecran, +0x40 l'image, +0x48
//     le Button, +0x58 l'id de l'objet (Equipment+0x00, un compteur du jeu, qui
//     ne survit pas a une relecture). Leur update (slot 11, 0x14034E280) cache
//     et grise (etat 4) l'objet porte par un autre chat que celui montre, rend
//     l'etat 5 aux autres ; un objet que l'ecran ne retrouve plus (carte id ->
//     chat refaite a chaque image par 0x14034BB50) fait retirer et detruire sa
//     boite, dans cet update meme. Un clic sur un objet agit sur le chat montre
//     (equiper depuis le rangement, ou rendre au rangement).
//   « Valider » (CloseButton, 0x14034D870) : en mode 1, pas de confirmation ;
//     l'ecran se ferme et le jeu sauve.
//   Les colliers ne viennent pas du rangement (0x1401C5CA0, la progression,
//     MewDirector+0x5A8) : remplacer le rangement ne change pas leur choix.
//   Un objet porte reste sur le chat, meme rentre a la maison (CatData+0x9B0,
//     serialise avec le chat). A la fin d'une aventure, le sac part au rangement
//     de l'hote (0x1402DF760 « on_store ») ; les objets portes s'usent
//     (0x1402DF420) et restent sur les chats qui rentrent.
//
// CE QUE FAIT LE MOD
//   - Invite, a « J'arrive » (mm_team) : son rangement est lu deux fois, tel que
//     le jeu l'a en memoire (ce qu'il va equiper) et tel que sa save l'a sur le
//     disque (pour savoir au retour si elle a change depuis).
//   - Invite, son depart fait dans la copie : le rangement de la copie (celui de
//     l'hote) est remplace par le sien (invsync_load_storage), a la premiere
//     image sans ecran des objets ouvert (celui de la maison porte le meme nom
//     de scene, et le jeu vide le rangement avant de le remplir) -- donc avant
//     celui du depart, qui vient toujours apres le choix des colliers. Il y
//     equipe ses chats comme d'habitude, avec ses objets, en voyant d'abord un
//     de ses chats (l'hote aussi).
//   - Les deux : sur l'ecran des objets, les objets du chat montre sont grises
//     (etat 4, celui du jeu) quand ce n'est pas un chat de ce joueur, et leur
//     clic est retenu. Les fleches et le tri restent a chacun, ainsi que les
//     confirmations du jeu en equipant. Les clics de l'hote ne sont plus rejoues
//     chez l'invite, sauf « Valider » (mm_embark) ; celui de l'invite est
//     retenu, avec une fenetre une fois, mais c'est son « fini » : sa derniere
//     photo part. Le premier « Valider » de l'hote est retenu, avec une
//     fenetre, si un invite qui suit n'a pas fini : le suivant passe.
//   - Invite : toutes les 0,5 s, une photo de ses chats (serialises par le jeu)
//     et de son rangement ; si elle a change, elle part chez l'hote (MSG_TEAM
//     kTeamGear, numerotee). Quand il valide, que le « Valider » de l'hote
//     arrive ou que son ecran se ferme, une derniere photo (refaite si l'envoi
//     echoue), puis ses objets sont grises. Rien a equiper (son rangement pas
//     charge) : « fini » des l'ouverture.
//   - Hote : il garde la derniere photo de chaque invite. Arrive sur la carte,
//     juste avant de publier son aventure (gear_hold_publish), il attend 3 s au
//     plus les dernieres photos de ceux qui suivent, puis applique chacune a ses
//     chats d'invite (catsync_deserialize_cat), tout ou rien : chaque chat doit
//     etre dans l'equipe, a ce joueur, le meme (id) et garder sa classe. Il
//     repond a chacun (kTeamGearAck) le numero applique et le rangement de cette
//     photo. Une photo arrivee plus tard, ou d'un invite parti, n'est pas
//     appliquee ; entre dans une case avant, il n'applique plus rien.
//   - Invite : ce rangement deviendra celui de sa save au retour (mm_team, dans
//     la meme transaction que l'etat final de ses chats), si chacun des chats
//     partis est recopie et si sa save n'a pas change depuis. Aucune photo
//     appliquee : ses chats partent tels qu'au depart et son rangement ne change
//     pas. Toute reponse fige ses objets ; une fenetre le lui dit s'il equipait
//     encore, ou si ce qu'il avait change n'a pas ete repris.
//
// LIMITES. On ne voit pas en direct les objets choisis par les autres : l'hote
// voit les chats de l'invite tels qu'au depart jusqu'a la carte, l'invite ceux
// de l'hote aussi. Un chat d'invite arrive dans l'aventure tel que la copie de
// l'invite l'a, y compris les capacites tirees au hasard par le jeu a la
// fermeture du choix des colliers (la copie ne tire pas les memes que l'hote) :
// celles que l'invite a vues. Un invite dont la photo n'est pas appliquee les
// retrouve telles que l'hote les a tirees. Un invite qui ne suit pas le depart
// (parti, en retard) ne peut pas equiper ses chats, et l'hote ne le fait pas a
// sa place. Si la save de l'invite a change depuis son depart, son rangement
// n'est pas touche au retour : les objets partis avec ses chats y restent
// aussi. Sans reponse de l'hote (session coupee juste apres), de meme.

#include <cstdint>

namespace mgmp {

enum Target : int;

enum class GearRole { Off, Host, Guest };

// mm_embark : le depart est fait (hote : le sien ; invite : celui de sa copie --
// son rangement y est charge ici).
void gear_begin(GearRole role, uint8_t vote);
void gear_end(const char* why);

// Chaque image (embark_pump).
void gear_pump();

// Un clic de ce joueur sur un bouton de « StorageItems », hors rejeu
// (embark_on_button_click). Vrai = clic retenu.
bool gear_on_click(void* button, const char* name);

// Invite : le « Valider » de l'hote est arrive (pas encore rejoue) : derniere
// photo a la prochaine image, puis plus de changement.
void gear_guest_close();
// Invite : sa copie arrive sur la carte, le depart ensemble finit : la
// derniere photo part maintenant si ce n'est fait.
void gear_guest_flush();

// Hote, sur la carte, juste avant de publier son aventure (mgmp_savefile) :
// vrai = pas encore (il attend les dernieres photos des invites) ; faux = les
// photos sont appliquees, ou il n'y en a pas.
bool gear_hold_publish();

// MSG_TEAM kTeamGear (hote) / kTeamGearAck (invite).
void gear_on_team(uint8_t from, const uint8_t* data, size_t size);

// Detour et original de T_InvItemUpdate. Hors du depart, le meme detour grise
// les objets en aventure (mm_rungear).
bool gear_binding(Target t, void** detour, void*** original);

// Le choix des objets du depart est en cours (hote ou invite).
bool gear_role_active();

} // namespace mgmp
