// mm_menu.h -- l'interface Mewgenics Multi, construite avec l'interface du jeu.
//
// Pas de fenetre posee par-dessus le jeu : on reutilise ses propres elements.
//
//   - Le bouton « Miauler » du menu principal devient « Multijoueur ». Le menu
//     n'a que quatre emplacements dessines dans swfs/ui.swf (sprite 2593, avec
//     un champ texte « label »), et MenuPanel::register_button plante si on lui
//     demande un emplacement qui n'existe pas : renommer celui-ci est donc la
//     seule facon d'avoir un bouton identique aux autres.
//   - Les messages passent par la fenetre de confirmation du jeu (YesNoPrompt /
//     ConfirmationBox), celle de « Voulez-vous vraiment abandonner ? ». On lui
//     donne notre texte en francais, nos libelles de boutons, ou un seul bouton.
//
// Tout s'execute sur le thread du jeu, depuis ses propres fonctions.
#pragma once

#include "mgmp_addresses.h"

#include <functional>

namespace mgmp {

// Fournit le detour et l'original des cibles T_MainMenuInit, T_MeowClick et
// T_ConfirmBuild a install_one (mgmp_hooks.cpp). Faux pour toute autre cible.
bool mm_binding(Target t, void** detour, void*** original);

// Ouvre une fenetre du jeu.
//   texte    : UTF-8, peut contenir \n, &nbsp; et les balises [c:red]...[/c]
//   oui/non  : libelles des boutons (UTF-8) ; nullptr garde « Oui » / « Non »
//   un_seul  : vrai -> un seul bouton (le « Oui », renomme par `oui`)
//   on_oui / on_non : actions des boutons (peuvent etre vides)
// Faux si les fonctions du jeu necessaires n'ont pas ete trouvees.
bool mm_show_popup(const char* texte, const char* oui, const char* non, bool un_seul,
                   std::function<void()> on_oui = {}, std::function<void()> on_non = {});

// La fenetre « Multijoueur » ouverte par le bouton du menu principal.
void mm_open_multiplayer_menu();

} // namespace mgmp
