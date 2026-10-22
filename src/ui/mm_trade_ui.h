#pragma once
// mm_trade_ui -- Mewgenics Multi : « Échanger », dans les fenetres du jeu (protocole 62).
//
// Plus de fenetre ImGui : la boite de confirmation du jeu (sa police, ses boutons),
// ouverte par mm_menu avec nos textes, mis a jour en direct ; une boite par etape.
// Rien ici ne touche au jeu : l'etat vient de mm_gift (gift_view), les demandes y
// retournent (gift_request). Fil du jeu (mm_menu_pump et mm_menu_wndproc).
//
//   - Ou : a la maison, le bouton « Échanger » du menu pause (Echap ; mm_menu
//     remontre le bouton « Abandonner » que le jeu y cache) ; en aventure mixte,
//     sur la carte, la touche E (pieces et nourriture de sa bourse).
//   - Les etapes : a qui (sautee s'il n'y a qu'un joueur qui peut recevoir), les
//     pieces, la nourriture (un nombre tape, T : tout), les objets du rangement (a
//     la maison : une liste dans le texte, Haut/Bas, Espace, Gauche/Droite, des
//     lettres pour chercher ; 16 au plus), le resume « Donner ... ? », puis
//     l'attente et la reponse dans la meme boite. Entree valide, Echap annule.
//   - Ce qui est choisi reste jusqu'au don passe ou a Echap / « Annuler » : une
//     boite fermee par le jeu (l'hote entre dans une case, un refus) le garde.
//   - Les nouvelles des dons : des fenetres d'information du jeu ; celles de celui
//     qui RECOIT attendent que sa maison ou sa carte soit deja en pause (Echap) --
//     un don recu n'arrete jamais personne.
//   Le clavier passe par mm_menu_wndproc, donc par la fenetre sous-classee de
//   mgmp_ui (ui.enabled, vrai par defaut), comme l'adresse de « Rejoindre ».

#include <windows.h>

namespace mgmp {

// Chaque image, depuis mm_menu_pump (apres gift_pump, avant les autres fenetres) :
// les touches recues, le texte de la boite, la boite suivante, les nouvelles.
void trade_ui_pump();

// Un bouton de notre boite (Action::TradeYes / TradeNo, la boite refermee).
void trade_ui_button(bool yes);

// Le bouton « Échanger » du menu pause de la maison a ete clique (h_TryAbandonRun).
void trade_ui_open_house();

// mm_menu_wndproc : une touche. `box` : notre boite est a l'ecran (toutes les touches
// sont a elle). Sinon, sur la carte, E ouvre la suite. Vrai : le jeu ne la voit pas.
bool trade_ui_key(UINT msg, WPARAM wp, LPARAM lp, bool box);

} // namespace mgmp
