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
#include <string>

namespace mgmp {

// Fournit le detour et l'original des cibles T_MainMenuInit, T_MeowClick,
// T_ConfirmBuild et T_HouseHud a install_one (mgmp_hooks.cpp). Faux pour toute
// autre cible.
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

// L'hote a quitte son aventure (MSG_HOSTLEFT, apres mgmp_leave). Ce message
// arrive une ou deux secondes avant MM_HOST_RUN 0 : l'invite note tout de suite
// que l'aventure est finie, et qu'il doit rentrer s'il est dans la copie --
// avant que mgmp_leave ne le ramene au menu, ou la copie ne compterait plus.
void mm_on_host_left();

// La maison du jeu est la (scene « House » vivante) : celle du joueur, ou celle
// de la copie d'une aventure chez un invite (voir savefile_in_coop).
bool mm_in_house();
// Une partie est chargee : la maison, la carte, un combat ou un evenement.
bool mm_in_game();
// Une scene de ce nom est chargee (et n'est pas en train de disparaitre).
bool mm_scene_alive(const char* name);
// ... et n'est pas en pause (une fenetre ou le menu pause par-dessus).
bool mm_scene_running(const char* name);
// Les scenes chargees, « House, Map, ... », pour le journal.
std::string mm_scene_list();

// La maison a l'ecran : son composant House (celui que refait le panneau a
// chaque image), verifie -- il appartient a la scene « House » vivante. Nul
// ailleurs.
void* mm_house();
// Une fenetre d'information du jeu (« D'accord »), des qu'une fenetre peut
// s'ouvrir. Texte UTF-8, en francais.
// kInfoYieldToFollow : chez un invite, la fenetre ne s'ouvre pas tant qu'il a
// une case de l'hote a suivre ou qu'il n'est pas sur la carte, et se referme
// (pour revenir apres) si l'hote entre dans une case pendant qu'elle est
// ouverte -- elle arreterait sa carte, et l'hote l'attendrait.
constexpr int kInfoYieldToFollow = 1;
void mm_show_info(const std::string& text_utf8, int flags = 0);
// Une fenetre d'information attend ou est ouverte : la suivante attendra.
bool mm_info_busy();
// La derniere confirmation du jeu lui-meme (boite YesNoPrompt qui n'est pas
// l'une de nos fenetres) : ouverte, et ce bouton est son « oui » (1), son
// « non » (2), ou ni l'un ni l'autre (0). Ses deux boutons s'appellent
// « GenericButton » (nom par defaut, 0x140142F00) : seul le panneau les
// distingue, releve quand le jeu construit la boite (h_ConfirmBuild).
int  mm_game_prompt_button(void* button);
bool mm_game_prompt_open();
// Le vote du chemin (mm_vote) : sa fenetre est ouverte ; hote, les autres
// joueurs dans l'aventure (connectes) ; invite, il est dans l'aventure de l'hote.
bool     mm_vote_popup_open();
// Le defile du butin (mm_loot) : sa fenetre est ouverte ; une autre fenetre (la
// notre, une information qui attend, une confirmation du jeu) est ouverte.
bool     mm_loot_popup_open();
bool     mm_other_popup_open();
uint32_t mm_run_peers_mask();
bool     mm_guest_in_run();
// Invite : il a choisi « Rentrer » dans « Quitter l'aventure » et n'est pas
// encore sorti de la copie -- il ne suit plus l'hote dans ses cases.
bool     mm_guest_quitting();
// Le nom d'un joueur de la session (pseudo Steam, « Joueur N » a defaut).
std::string mm_peer_name(uint8_t id);

// Ecrit un texte (UTF-8, balises du jeu permises : [ ] a eviter dans les noms)
// dans le champ `child` (15 caracteres au plus) du clip d'un MenuPanel du jeu,
// comme MenuPanel::set_text. Faux si le champ n'existe pas. Thread du jeu.
bool mm_panel_set_text(void* panel, const char* child, const std::string& text_utf8);
// Le texte du champ `child` de ce panneau (UTF-8, balises comprises), pour le
// remettre apres : 1 lu, 0 pas de champ de ce nom dans le clip (MenuPanel::set_text
// n'y ecrit rien non plus), -1 un champ illisible (`why`, s'il est donne, dit
// pourquoi). Thread du jeu.
int mm_panel_read_text(void* panel, const char* child, std::string& out_utf8, std::string* why = nullptr);
inline bool mm_panel_get_text(void* panel, const char* child, std::string& out_utf8) {
    return mm_panel_read_text(panel, child, out_utf8) == 1;
}

// Le panneau de la maison clique dans cette image : 1 « Fin de journée »
// (EndDay_Sign), 2 « À l'aventure ! » (Depart_Sign), 0 aucun. Les deux
// appellent la meme action du jeu (ButchBox::depart, puis la fin de journee si
// la boite est vide) : mm_team ne s'occupe que du second.
int mm_sign_click_kind();

// Hote, le vote de depart passe : personne ne peut partir (mm_team -- boite de
// l'hote videe pendant le vote, et aucun chat d'invite). Les invites qui
// attendaient l'aventure en sont liberes, et une fenetre leur dit pourquoi.
void mm_depart_called_off();

// Appele par le detour de Button::Click, avant le clic. Vrai = clic retenu :
// en session, le panneau de fin de journee (« EndDay_Sign ») attend que tout le
// monde soit pret ; le mod le rejoue ensuite tel quel. Avec des chats dans
// l'equipe, le meme panneau s'appelle « Depart_Sign » (« À l'aventure ! ») :
// chez l'hote, son clic ouvre le vote de depart ; chez un invite, il ne part pas.
//
// Les joueurs qui ne sont pas encore prets le voient : leur panneau dit
// « 1 joueur attend » et une fenetre le leur annonce, avec « Passer la nuit ».
bool mm_on_button_click(void* button);

} // namespace mgmp
