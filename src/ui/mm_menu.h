// mm_menu.h -- l'interface Mewgenics Multi, construite avec l'interface du jeu.
//
// Pas de fenetre posee par-dessus le jeu : on reutilise ses propres elements.
//
//   - Le bouton « Miauler » du menu principal devient « Multijoueur ». Le menu
//     n'a que quatre emplacements dessines dans swfs/ui.swf (sprite 2593, avec
//     un champ texte « label »), et MenuPanel::register_button plante si on lui
//     demande un emplacement qui n'existe pas : renommer celui-ci est donc la
//     seule facon d'avoir un bouton identique aux autres.
//   - Les fenetres passent par la boite de confirmation du jeu (YesNoPrompt /
//     ConfirmationBox), celle de « Voulez-vous vraiment abandonner ? » : notre
//     texte (en Unicode, donc avec les accents), nos libelles de boutons, ou un
//     seul bouton. Son champ texte « prompt » est mis a jour en direct : liste
//     des joueurs du salon, adresse tapee au clavier.
//
// Parcours : Multijoueur -> « Heberger » ou « Rejoindre ».
//   Heberger  : le salon s'ouvre, l'adresse a donner est affichee (et copiee), la
//               liste des joueurs se remplit ; « Choisir la sauvegarde » ouvre
//               l'ecran des sauvegardes, comme « Jouer ».
//   Rejoindre : on tape l'adresse de l'hote (ou Ctrl+V), Entree ou « Rejoindre » ;
//               le salon montre la connexion puis l'attente du choix de l'hote.
//
// Tout s'execute sur le thread du jeu : les clics ne font que poser une demande,
// traitee au debut de l'image suivante (mm_menu_pump).
#pragma once

#include "mgmp_addresses.h"

#include <windows.h>
#include <cstdint>
#include <functional>

namespace mgmp {

// Fournit le detour et l'original des cibles T_MainMenuInit, T_MeowClick et
// T_ConfirmBuild a install_one (mgmp_hooks.cpp). Faux pour toute autre cible.
bool mm_binding(Target t, void** detour, void*** original);

// Appele au debut de chaque image (h_FrameBegin) : ouvre les fenetres demandees,
// met a jour le texte de la fenetre ouverte. Ouvrir une fenetre ajoute une scene
// a la liste du Director ; le faire pendant que le jeu parcourt cette liste (un
// clic arrive en plein Button::update) fige le jeu. Le debut d'image est entre
// deux choses.
void mm_menu_pump();

// Appele pour chaque bouton du jeu, apres son update (h_ButtonUpdate) : c'est la
// que nous « cliquons » nos boutons, au meme moment que le jeu le ferait.
void mm_on_button_update(void* button);

// Appele en premier par la procedure de fenetre (mgmp_ui.cpp). Vrai = message
// consomme : pendant la saisie de l'adresse, le clavier est a nous.
bool mm_menu_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// Le nom de ce joueur (pseudo Steam, sinon « Joueur »), en UTF-8. Envoye dans le
// HELLO, affiche dans le salon des autres.
const char* mm_local_name();

// Un pair s'est presente (HELLO accepte) : on garde son nom pour le salon.
void mm_on_peer_hello(uint8_t peer, const char* name_utf8);

// Un message MM (MSG_MM) d'un autre joueur : nuit votee, et les fonctions a venir.
struct MmMsg;
void mm_on_message(uint8_t from, const MmMsg& m);

// Appele par le detour de Button::Click, avant le clic. Vrai = clic retenu :
// en session, le panneau de fin de journee (« EndDay_Sign ») attend que tout le
// monde soit pret ; le mod le rejoue ensuite tel quel.
bool mm_on_button_click(void* button);

} // namespace mgmp
