#pragma once
// mm_trade_ui -- Mewgenics Multi : le bouton « Échanger » et sa fenetre (protocole 62).
//
// Une fenetre ImGui, dessinee par mgmp_ui au meme endroit que le panneau de
// diagnostic (la prise de SDL_GL_SwapWindow), mais sans lui : elle tourne aussi
// panneau cache. Elle ne fait que montrer l'etat publie par mm_gift (gift_view) et
// lui laisser des demandes (gift_request) -- rien ici ne touche au jeu, et rien de
// ce qu'elle fait n'entre dans la simulation des combats.
//
//   - Le bouton : en session, dans sa maison, ou en aventure mixte sur la carte
//     quand on a une bourse. On peut le deplacer (par son bord).
//   - La fenetre : a qui donner, combien de pieces et de nourriture, et a la maison
//     les objets du rangement (16 au plus, une recherche par nom) ; « Donner », puis
//     la reponse dans la fenetre.
//   - Les nouvelles (un don recu, ou la reponse quand la fenetre est fermee) : en
//     haut de l'ecran quelques secondes, sans prendre la souris.

struct ImFont;

namespace mgmp {

// ImGui doit tourner : le bouton, la fenetre ou une nouvelle a montrer.
bool trade_ui_wanted();

// Entre ImGui::NewFrame et ImGui::Render. `font` : la police des textes accentues
// (nulle : celle d'ImGui).
void trade_ui_draw(ImFont* font);

} // namespace mgmp
