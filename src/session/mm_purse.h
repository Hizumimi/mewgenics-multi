#pragma once
// mm_purse -- Mewgenics Multi : a chacun ses trouvailles. Une bourse par joueur
// pendant l'aventure ; ce qu'un chat trouve est a son proprietaire, qui le
// rapporte dans SA maison.
//
// LE JEU (1.1.21239)
//   L'aventure a ses propres pieces et sa propre nourriture, a cote de celles de
//   la maison : Inventory+0x148 (adventure_coins, 99 au plus) et +0x14C
//   (adventure_food), lues avec l'aventure (0x1402E1E90). Les evenements les
//   changent, comme les combats (recompenses, pieces ramassees), la boutique et
//   les boites.
//   Un evenement execute ses commandes dans WorldEvent::spin (0x1409173A0) ;
//   « gain_coins » et « gain_food » (gagner ou perdre) y sont chacune un appel :
//   0x14092BAD0 (T_EventGainCoins) et 0x14092C330 (T_EventGainFood), avec le
//   WorldEvent en premier argument. WorldEvent+0x128 est le chat de l'evenement
//   (CatData*, « {Catname} a obtenu 15 pièces ! ») : ecrit par WorldEvent::init
//   (un chat au hasard, 0x1409150F8) et par le choix du chat (0x14093C250). Le
//   resultat d'un choix passe par WorldEvent::setupResult (0x14091C650,
//   T_EventResult ; un rappel du clip, ou WorldEvent::init quand le clip va droit
//   a la fin) : le prix du choix s'il coute des pieces (0x14091C84D, issue
//   « Success » seulement), puis spin sur ses effets. Les commandes reward,
//   random_pool, conditional_reward... rappellent spin ; shop_now n'ouvre qu'une
//   boutique.
//   En combat, un chat ramasse des pieces par Unit::GainCoins (0x140114E30,
//   T_UnitGainCoins : MoneyPickup, Bounty, AwardCoinsOnDeath... ; il fait jouer
//   les OnGainCoins des passifs, qui peuvent en rappeler un autre) et de la
//   nourriture par HealthPickup (0x1402A12B0, T_PickupFood : soigne, ou garde de
//   la nourriture) et le passif BonusFoodEachBattle (0x14033D4B0). Character+0x88 :
//   son CatData.
//   Une aventure gagnee rentre par 0x1403B5430 (T_RunVictory, MewDirector* seul
//   argument) : le jeu y ajoute les pieces et la nourriture de l'aventure a
//   celles de la maison (House+0xB4 et +0xB0, 0x1403B5809) puis les remet a zero
//   (0x1403B5C74) -- puis les retours des chats et ClearRunState (T_RunCleared,
//   mm_team). Une defaite ou un abandon (0x1403B4750) n'ajoute rien a la maison
//   (0x1401C7D00 garde une partie du compte en objets pour une aventure d'apres,
//   qui les partagera).
//
// AVANT. Une seule bourse, celle de l'aventure : ce qu'un chat d'invite trouvait
// augmentait le compte de tous, et tout finissait dans la maison de l'hote.
//
// CE QUE FAIT LE MOD (l'hote tient les comptes, en aventure mixte -- mm_team)
//   - Le compte du jeu ne change pas pendant l'aventure (la boutique, les
//     evenements, les combats voient la meme bourse chez tous) ; l'hote tient a
//     cote une bourse par joueur qui a amene des chats (l'hote seulement s'il en
//     a amene), dont la somme est ce compte.
//   - Ce qu'un gain_coins ou un gain_food fait au compte va a la bourse du
//     proprietaire du chat de l'evenement a ce moment-la (un chat d'un joueur
//     parti reste a lui ; une recrue, a l'hote) ; ce que le resultat d'un choix
//     change d'autre (son prix), au proprietaire du chat de l'evenement au debut
//     du resultat ; ce qu'un chat ramasse en combat, a son proprietaire. Ces
//     appels s'imbriquent : chacun prend ce qui change pendant lui, hors des
//     appels suivis qu'il contient. Tout le
//     reste -- le compte a la premiere image de carte, recompenses des combats,
//     boutique (aussi celle d'un evenement), boites, evenement sans chat, un
//     voleur ennemi -- est partage a parts egales (les unites qui restent : un
//     gain a la bourse la plus maigre, une perte a la plus garnie). Une bourse
//     peut passer sous zero : une dette, que ses trouvailles suivantes
//     remboursent (mm_purse_core.h).
//   - L'hote envoie les bourses (MM_PURSE a = 1) quand elles changent et toutes
//     les 10 s, et chaque trouvaille (a = 2). Chacun voit, dans la fenetre du
//     vote du chemin (pas de fenetre de plus), les trouvailles depuis la case
//     d'avant -- a qui elles vont -- et sa bourse.
//   - Aventure gagnee : juste avant que le jeu ne vide l'aventure dans la maison
//     de l'hote, chaque invite encore la recoit sa bourse (positive, dans la
//     limite du compte), retiree du compte -- un message a lui seul (a = 3),
//     avant l'etat final des chats ; un envoi rate, sa bourse reste a l'hote.
//     L'invite l'ajoute a l'or et a la nourriture de SA maison avec ses chats
//     (mm_team). L'hote rapporte le reste, comme le jeu ; une fenetre lui dit,
//     une fois dans sa maison, ce que chacun a rapporte. Chez l'invite, la copie
//     de l'aventure montre sa bourse a l'ecran de fin.
//   - Il faut les quatre crochets des evenements et de la fin : sans l'un d'eux,
//     pas de bourses (tout va a l'hote, comme avant). Sans ceux des combats, ce
//     qui y est ramasse est partage.
//
// LIMITES. Les boites de meubles restent a l'hote. Un invite parti avant la fin
// ne recoit rien (sa bourse reste a l'hote). Les bourses ne vivent que dans la
// memoire de l'hote : apres un redemarrage en pleine aventure, plus d'aventure
// mixte (mm_team), donc plus de bourses. Ce qu'un invite rapporte vit dans sa
// memoire jusqu'au chargement de sa save : s'il ferme le jeu avant, c'est perdu
// (comme l'etat final de ses chats).

#include <cstdint>
#include <string>

namespace mgmp {

enum Target : int;
struct MmMsg;

// Detours et originaux de T_RunVictory, T_EventGainCoins, T_EventGainFood,
// T_EventResult, T_UnitGainCoins, T_PickupFood et T_BonusFoodPassive.
bool purse_binding(Target t, void** detour, void*** original);

// Chaque image (mm_menu_pump).
void purse_pump();

// MM_PURSE (invite).
void purse_on_mm(uint8_t from, const MmMsg& m);

// Pour la fenetre du vote du chemin : les trouvailles depuis la case d'avant
// (« \nFrédéric a trouvé 15 pièces : c'est pour ta bourse. ») et
// « \nTa bourse : 12 pièces et 3 nourritures. », ou "" hors d'une aventure mixte.
// `mark` : une fenetre s'ouvre ou se met a jour avec ce texte -- ces trouvailles
// sont dites, et s'en iront a la case suivante.
std::string purse_vote_line(bool mark);

} // namespace mgmp
