#pragma once
// mm_vote -- Mewgenics Multi : le chemin sur la carte se vote. Les regles, le
// message et les textes sont dans mm_vote_core (portable, teste) ; ici, le
// jeu : le clic sur une case, la carte, les fenetres, le reseau.
//
// LE JEU (1.1.21239)
//   Une case de la carte : MapNode (cree par 0x140226BA0, symbole « Map_Node »).
//     +0x118 sa graine (32 octets), +0x138 son type (MapNodeType), +0x160 son
//     image (position en float a +0x70/+0x74, lue par MapMarker::update),
//     +0x168 deja visitee, +0x16C revelee, +0x170 le MapScreen, +0x178 son
//     Button. MapNode::refresh (0x1402262A0) met le bouton a l'etat 4 (grise),
//     puis a 5 si la case n'est pas visitee, a une voisine visitee, et peut etre
//     atteinte depuis le pion par des cases revelees (0x140226070 -> chemin
//     0x140224050) ; une case visitee reste a 4. La fin d'une case
//     (0x140392FC0) marque visitee la case ou est le pion : voir mgmp_marker.h
//     pour ce que cela demande au suivi d'un invite.
//   Le clic : le lambda du bouton (_Do_call 0x140228700, vtable 0x140F00610,
//     installe par 0x140228370), capture +8 = la case. Pour la maison (type 4)
//     avec d'autres cases encore possibles, il ouvre la confirmation
//     POPUP_TRY_RETURN_HOME_EARLY (0x14077DD30) ; sinon il pose la case comme
//     destination du pion (MapScreen+0xA0 -> +0x60) et efface MapScreen+0x1E7.
//     Le pion y marche (MapMarker::update, slot 10 de la vtable 0x140F14A28)
//     et, a chaque case du chemin, appelle son rappel (0x14039B180) : la
//     Nemesis sur la case (0x140392ED0) lance son combat et arrete le pion ;
//     une case deja visitee est traversee ; sinon EnterNode -- ou mgmp publie la
//     case (ENTERNODE) et ou les invites la suivent -- et le pion s'arrete
//     (destination videe).
//   Button::CanBeClicked (0x14097EAF0) : visible (+0x10), pas cache (+0xF),
//     actif (+0xE), pas « survol seul » (+0x5A), etat != 4, plus la scene (pas
//     en pause) -- ce dernier point est laisse de cote pour verifier un vote :
//     la fenetre du vote met justement la carte en pause.
//
// CE QUE FAIT LE MOD
//   - En aventure ensemble (au moins un invite dans l'aventure de l'hote), le
//     clic d'un joueur sur une case n'est plus un deplacement mais un vote :
//     le pion ne bouge pas. L'hote vote chez lui ; l'invite envoie son vote a
//     l'hote (MM_VOTE kVoteCast), qui le verifie sur sa carte (meme case,
//     cliquable) et tient le tour (VoteHost), puis publie son etat a tous.
//   - Chacun voit le tour dans la fenetre du jeu (sa boite de confirmation),
//     tenue a jour en direct : qui a vote quoi, le compte a rebours. Un joueur
//     qui n'a pas vote peut prendre le meme chemin que les autres (« Même
//     chemin ») ou fermer pour cliquer sur une case (« Choisir ») ; un joueur
//     qui a vote peut fermer pour en changer (« Changer »). Elle revient a son
//     prochain vote, ou au bout de quelques secondes.
//   - Le resultat s'affiche 2,5 s, puis l'hote rejoue le clic du jeu sur la
//     case gagnante (le lambda d'origine, avec la case) : le pion y marche et
//     tout continue comme avant (les invites suivent l'hote dans la case, et
//     leur pion y arrive : mgmp_marker.h). La maison garde la confirmation du
//     jeu, chez l'hote.
//   - Pendant le resultat et la marche du pion, plus de vote. Le tour finit a
//     l'entree dans une case ; ou quand le pion s'arrete sans y entrer
//     (confirmation de la maison refusee, Nemesis) ; ou sur une autre carte ;
//     ou apres 30 s de carte en marche (un menu par-dessus ne compte pas).
//   - Un vote d'invite que l'hote ne prend pas (hote pas sur la carte, case pas
//     cliquable chez lui, tour fini) : l'etat publie juste apres le dit, et la
//     fenetre de l'invite revient avec la raison, meme s'il l'avait fermee pour
//     cliquer (une fenetre a part s'il n'a pas de tour, jamais pendant qu'il
//     est emmene dans une case).
//   - Tant que le joueur n'a pas vote, sa fenetre garde ses boutons quand les
//     votes des autres font ou defont un chemin en tete ; « Même chemin » sans
//     chemin en tete ne vote rien et la rouvre avec « Choisir ».
//   - Seul dans l'aventure (ou hors session) : le clic est celui du jeu.
//   - La Nemesis se joue ensemble depuis le protocole 57 (mm_nemesis.h) : elle
//     marche chez tous, et son combat est publie par l'hote comme une case.
//   - Un vote rejoue pendant que le pion est fige (la Nemesis marche,
//     MapMarker+0x78 : le jeu efface alors la destination a chaque image)
//     attend qu'il soit libre.
//
// LIMITES. Seul le chemin se vote pour l'instant : les evenements et les
// montees de niveau restent decides par l'hote (mgmp_choice). La position
// (« en haut », « à gauche »...) qui distingue deux cases du meme type est lue
// sur la carte telle que le jeu la dessine. Le retour anticipe a la maison est
// recopie chez l'invite (ENTERNODE kEnterEarlyHome, mgmp_follow).

#include <cstdint>
#include <string>

namespace mgmp {

enum Target : int;
struct MmMsg;

// Detour et original de T_MapNodeClick.
bool vote_binding(Target t, void** detour, void*** original);

// Chaque image (mm_menu_pump), avant les fenetres.
void vote_pump();

// A la fin de MapScreen::update (h_MapUpdate), avant le suivi de l'hote.
void vote_map_update(void* map);

// Une case est entree : par l'hote (h_EnterNode), ou par un invite qui le suit.
void vote_on_enter_node();

// MM_VOTE d'un autre joueur.
void vote_on_mm(uint8_t from, const MmMsg& m);

// La fenetre voulue maintenant (mm_menu). `layout` change quand les boutons
// changent (il faut alors la refermer et la rouvrir) ; 0 = pas de fenetre.
struct VotePopup {
    uint32_t    layout = 0;
    std::string text;
    const char* yes = nullptr;     // libelle UTF-8 du premier bouton
    const char* no  = nullptr;     // du second ; nullptr = un seul bouton
};
// `open_layout` : celle de la fenetre deja ouverte (0 aucune), dont les boutons
// sont gardes tant que le joueur n'a pas vote.
bool vote_popup(VotePopup& out, uint32_t open_layout = 0);

// Un bouton de la fenetre presse par le joueur (pas une fermeture du mod).
void vote_popup_button(uint32_t layout, bool yes);

} // namespace mgmp
