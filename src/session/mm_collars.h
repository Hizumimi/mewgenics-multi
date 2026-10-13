#pragma once
// mm_collars -- Mewgenics Multi : les colliers de classe, choisis ensemble au
// depart (scene « ClassChooser »). Chaque joueur regarde le chat qu'il veut,
// ne pose ou ne retire de collier qu'a ses propres chats, et voit en direct
// ceux que posent les autres. Un collier n'existe qu'une fois : le premier qui
// le prend l'a (c'est l'hote qui tranche).
//
// LE JEU (1.1.21239)
//   ClassChooser (vtable 0x140F165D8, init 0x14013D650) :
//     +0x48  CatSelector* : les fleches (CatSelector_Left/Right) changent son
//            rang +0x78 (i32, modulo l'equipe) puis appellent sa selection
//            0x1400E18B0(selecteur, cle) : +0x88 = la cle du chat montre.
//     +0x9C  nombre et +0xA0 tableau des ClassTagBox* : d'abord une par chat
//            qui avait deja une classe (son collier, fixe), puis une par collier
//            de l'inventaire (liste passee a init). La mise en page 0x14013EF10
//            (apres chaque clic) reordonne ce tableau.
//     +0xB8/+0xC0 une copie de chaque CatData de l'equipe (0xC58 octets, dans
//            l'ordre de MewDirector+1472) : +0x00 l'id, +0xC10 la classe
//            d'origine (std::string), +0xC48 la cle.
//     +0xD0  la cle du chat montre, recopiee de CatSelector+0x88 a chaque image
//            (update 0x14013FA20).
//   ClassTagBox (vtable 0x140EE2130) : +0x38 son ClassChooser, +0x48 son
//     Button, +0x58 la classe du collier (std::string), +0x90 la cle du chat qui
//     le porte (-1 : libre). Son update (slot 11, 0x1401402A0) cache et grise
//     (etat 4) les colliers portes par un autre chat que celui montre, nomme le
//     bouton « InventoryButton_Collar » (libre) ou « EquippedButton_Collar ».
//     Son clic (0x1401405A0, appele par le std::function du bouton) agit sur le
//     chat montre (ClassChooser+0xD0), s'il est d'origine « Colorless » : libre,
//     le collier lui est pose (celui qu'il portait est libere, la CatData est
//     remise a sa copie puis recoit la classe, 0x1400BDC70) ; porte, il est
//     retire (0x14013E100 : la CatData reprend sa copie). L'animation joue sur
//     le chat du selecteur (rang +0x78), le son « SuccessfulEquipItem_Collar »
//     ou « SuccessfulUnequipItem_Collar » aussi. Le jeu modifie donc la CatData
//     tout de suite : « Valider » ne fait que fermer (et demande confirmation si
//     un chat de l'equipe est encore « Colorless », 0x140141E40).
//
// CE QUE FAIT LE MOD
//   - L'etat partage est, pour chaque chat d'origine « Colorless », la classe du
//     collier qu'il porte (ou aucune). Deux colliers de meme classe sont
//     interchangeables : on ne suit pas quelle boite va sur quel chat.
//   - Le proprietaire d'un chat est celui de l'equipe mixte (team_owner_of, par
//     l'id de la CatData) ; un chat sans proprietaire connu est a l'hote, comme
//     celui d'un invite qui ne suit pas le depart (parti, en retard).
//   - Les colliers du chat montre sont grises (etat 4, celui du jeu) quand ce
//     joueur ne peut pas y toucher : le detour de ClassTagBox::update
//     (T_ClassTagUpdate) le refait apres le jeu, a chaque image.
//   - Hote : ses clics sur ses chats passent tels quels. Chaque image, ce qui a
//     change part chez les invites (MM_EMBARK kEmbarkCollarSet : la cle, la
//     classe ou rien).
//   - Invite : son clic sur un collier d'un de ses chats ne fait rien chez lui ;
//     il part chez l'hote (kEmbarkCollarReq : poser ou retirer). L'hote verifie
//     (son chat, collier encore libre), puis fait le clic du jeu lui-meme, sur
//     ce chat. Le resultat revient comme les autres changements.
//   - Pour appliquer un changement qui ne vient pas de son propre clic, un pair
//     fait le clic du jeu sur une boite de cette classe en montrant le bon chat
//     le temps de l'appel (ClassChooser+0xD0 et le rang du selecteur), puis
//     remet le chat qu'il regardait (selection du jeu). L'animation et le son
//     sont ceux du jeu, sur le bon chat.
//   - A l'ouverture, chacun voit d'abord un de ses chats ; l'invite le dit a
//     l'hote (kEmbarkChooserOpen), qui renvoie tout l'etat.
//   - « Valider » est a l'hote : mm_embark rejoue son clic chez les invites une
//     fois leurs colliers pareils aux siens (ou apres 5 s d'ecart, choix a
//     l'ecran), puis sa reponse a la confirmation du jeu ; si elle ne s'ouvre que
//     chez l'invite, celui-ci y repond « oui » (l'hote est deja passe). Apres
//     son « Valider », l'hote n'applique plus aucune demande (sauf s'il repond
//     « non » a la confirmation). Celui d'un invite est retenu, avec une
//     fenetre une fois.
//   - Un invite qui ne suit plus le depart (MM_EMBARK kEmbarkGone, ou parti de
//     la session) rend la main a l'hote sur ses chats ; un « pret » arrive apres
//     le depart de l'hote le fait suivre quand meme. Une exception dans le clic
//     du jeu arrete tout : plus aucun clic du jeu, l'hote choisit pour toute
//     l'equipe, l'invite laisse le miroir.
//
// LIMITES. L'invite attend la reponse de l'hote pour voir son collier (un aller
// et retour). Un collier que l'invite n'a pas (inventaire different) ne peut
// pas etre recopie chez lui : il le voit sans, la vraie aventure (celle de
// l'hote, rechargee sur la carte) l'a. Une fenetre ouverte par-dessus le choix
// le met en pause : les changements attendent qu'elle se ferme.

#include <cstdint>

namespace mgmp {

struct MmMsg;
enum Target : int;

enum class CollarRole { Off, Host, Guest };

// mm_embark : le choix commence -- hote : il vient de partir ; invite : il est
// dans la maison de l'hote, pret a faire le meme depart. `seq` : le vote.
void collars_begin(CollarRole role, uint8_t seq);
void collars_end(const char* why);

// Chaque image (embark_pump, au debut de l'image).
void collars_pump();

// Un clic de ce joueur sur un bouton de « ClassChooser », hors rejeu
// (embark_on_button_click). Vrai = clic retenu. Les fleches passent toujours ;
// chez l'invite, « Valider » est retenu.
bool collars_on_click(void* button, const char* name);

// Invite : ses colliers sont ceux de l'hote (vrai aussi sans choix ouvert). Le
// « Valider » de l'hote n'est rejoue qu'alors -- ou quand l'ecart dure depuis
// 5 secondes de choix a l'ecran (le temps en pause ne compte pas).
bool collars_synced();
bool collars_sync_overdue();

// Hote, a son « Valider » : les derniers changements partent tout de suite, et
// plus aucune demande n'est appliquee (sauf s'il repond « non » a la
// confirmation du jeu : collars_cancel_close).
void collars_flush();
void collars_cancel_close();
// Hote : son « Valider » ouvrira-t-il la confirmation du jeu (un chat de
// l'equipe encore « Colorless », le test meme de 0x140141E40) ? 1 oui, 0 non,
// -1 on ne sait pas (un chat illisible).
int collars_host_prompt_expected();

// Le clic du jeu a leve une exception : plus aucun clic du jeu d'ici la fin
// (l'invite laisse le miroir, l'hote choisit pour toute l'equipe).
bool collars_broken();

// MM_EMBARK, a = kEmbarkCollarReq / kEmbarkCollarSet / kEmbarkChooserOpen.
void collars_on_mm(uint8_t from, const MmMsg& m);

// Detour et original de T_ClassTagUpdate.
bool collars_binding(Target t, void** detour, void*** original);

} // namespace mgmp
