#pragma once
// mm_shop -- Mewgenics Multi : les achats en boutique avec sa bourse. Les regles et
// le message : mm_shop_core.h ; ici, le jeu et le reseau.
//
// LE JEU (1.1.21239)
//   - Une boutique est un composant Shop (vtable 0x140F03A88) dans la scene
//     « Shop » (celle d'une case boutique, ou ouverte par un evenement :
//     WorldEvent::shop_now). Shop::update (entree 10 de sa vtable, 0x140797EA0,
//     T_ShopUpdate) tourne a chaque image tant qu'elle est ouverte.
//     Shop+0x58 : le MewDirector ; +0x60/+0x68 : ses objets (vector<ShopItem>,
//     0xC0 octets chacun) ; +0x7B : la boutique d'une maison (1 : Jack, Tracy) ou
//     d'une aventure (0) ; +0x98 : les achats qu'il reste (une boutique a achats
//     comptes ; a 0, tout ce qui reste passe « vendu »).
//   - ShopItem : +0x00 son genre (1 objet, 12 bonbon, 13 familier, 15 meuble,
//     16 nourriture, 17 pieces, 18 collier vierge), +0x08 son prix, +0x0C son
//     stock, +0x10 le nom d'un familier, +0x30 la quantite (nourriture, pieces),
//     +0x38 l'objet lui-meme (un Equipment de 0x60 octets, son id en tete).
//   - Le bouton d'achat (un rappel, 0x14079A3F0) : prix <= pieces (aventure :
//     Inventory+0x148 ; maison : House+0xB4), le son « Shop_BuyItem », puis
//     Shop::BuyItem(Shop*, ShopItem&) (0x14079A600, T_ShopBuy) ; sinon
//     « Shop_CantAfford ». BuyItem reverifie le prix, puis Shop::ObtainItem
//     (0x14079ACB0) donne la chose -- un objet : Inventory::insert_item dans le sac
//     (une copie, le meme id) ; nourriture et pieces : ajoutees au compte de
//     l'aventure ; un bonbon : le choix du chat (0x14079C5C0) ; un familier :
//     MewDirector+0x628 -- puis son rappel (0x14079A6B0) : pieces -= prix
//     (aventure : bornees a 0..99), stock--, Shop+0x98--, l'affichage.
//
// AVANT. L'hote achetait seul pour l'equipe, le prix partage entre toutes les
// bourses. Ce qu'un invite achetait n'arrivait que dans sa copie : la poussee de
// l'hote apres la case l'effacait (son objet et ses pieces). Un familier achete
// par l'hote n'existait pas dans les copies.
//
// CE QUE FAIT LE MOD (aventure mixte en session, les deux crochets la)
//   - Invite (copie de l'aventure de l'hote) : BuyItem ne fait rien chez lui ; il
//     demande l'achat a l'hote (kShopBuy : la case, la place de l'objet, son
//     empreinte). Une demande a la fois par objet. Un refus s'affiche dans une
//     fenetre du jeu, sur la boutique.
//   - Hote : il prend la demande dans Shop::update de sa boutique (elle y est
//     vivante), verifie la case, l'objet et la bourse du joueur (mm_purse), puis
//     achete par le jeu (BuyItem), le prix pris dans la bourse de l'acheteur
//     (purse_charge). L'hote qui achete lui-meme paie avec SA bourse (refus si
//     elle ne suffit pas, comme pour un invite). L'objet arrive dans le sac
//     commun, note a son acheteur (son id dans le sac de l'hote).
//   - Chaque achat (de l'hote ou d'un invite) part chez tous (kShopSold) : la
//     copie de chacun le rejoue par le jeu (BuyItem, appele dans Shop::update de
//     sa boutique, apres avoir remis le compte de l'aventure a celui de l'hote) :
//     l'objet passe vendu, le compte baisse, l'objet arrive dans le sac, un
//     familier rejoint l'equipe. Un bonbon (le choix du chat) n'est pas rejoue :
//     le compte seul ; le chat monte de niveau dans la poussee d'apres la case.
//   - Chaque invite dit a l'hote si sa boutique est ouverte (kShopHere) : l'hote
//     qui clique « Sortir » pendant qu'un invite achete encore voit une fenetre
//     (une fois sorti, il ne pourrait plus acheter pour lui) ; un second clic
//     sort quand meme.
//   - Les salles au tresor sont des boutiques aussi (Shop+0x79 : initShop y
//     « achete » tout seul chaque objet, gratuit) : laissees au jeu, comme avant.
//   - Un bonbon achete par l'hote pour le chat d'un invite : l'hote choisit sa
//     montee de niveau (la copie de l'invite n'a pas l'ecran : shop_host_inside).
//   - Victoire : un objet achete encore dans le sac ne defile pas (mm_loot) ; il
//     rentre chez son acheteur (kShopGive -> kShopGot, comme un objet gagne au
//     defile) ; achete par l'hote, ou son acheteur parti, il reste chez l'hote.
//     Equipe sur un chat, il rentre avec le chat.
//
// LIMITES. Un invite n'achete pas de bonbon ni de familier (l'hote, avec sa
// bourse). Un objet achete puis donne a un chat d'un autre joueur rentre avec ce
// chat. La note des achats ne vit que dans la memoire de l'hote (comme les
// bourses).

#include <cstdint>
#include <string>

namespace mgmp {

enum Target : int;

// Detours et originaux de T_ShopBuy et T_ShopUpdate.
bool shop_binding(Target t, void** detour, void*** original);

// Chaque image (mm_menu_pump).
void shop_pump();

// Un message MSG_TEAM kTeamShop.
void shop_on_team(uint8_t from, const uint8_t* data, size_t size);

// La victoire (h_RunVictory, mm_purse) : avant la fonction du jeu (l'equipe est
// encore connue) et apres (le sac est dans le rangement de l'hote). Hote seul.
void shop_victory_before();
void shop_victory_after();

// mm_loot, a la victoire : cet objet (son id dans le sac de l'hote) a ete achete
// -- il ne defile pas.
bool shop_claims(uint64_t id);

// Hote : il est dans sa boutique d'aventure en ce moment (elle a tourne il y a
// moins de 2 s, dans la case ou il est). Une montee de niveau qui s'ouvre alors
// vient d'un bonbon qu'il a achete : c'est lui qui la choisit, meme pour le chat
// d'un invite -- la copie de l'invite n'a pas l'ecran (mgmp_choice, decider_of).
bool shop_host_inside();

// Un clic de ce joueur (Button::Click, mm_on_button_click). Vrai = retenu : chez
// l'hote, « Sortir » (Shop_ExitButton) d'une boutique ou un invite achete encore
// -- une fenetre le dit ; un second clic dans les 10 s sort quand meme.
bool shop_on_click(void* button);

} // namespace mgmp
