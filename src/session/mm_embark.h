#pragma once
// mm_embark -- Mewgenics Multi : le depart ensemble. Les invites qui ont dit
// « J'arrive » voient le depart de l'hote comme lui : le choix des colliers de
// classe (scene « ClassChooser »), puis celui des objets du rangement (scene
// « StorageItems », une fois le desert debloque).
//
// LE JEU (1.1.21239). Le panneau « À l'aventure ! » appelle ButchBox::depart,
// puis C_DepartGo -> GoOnAnAdventure (0x1403B13F0) -> DepartTransition
// (0x1403C9F90) ; a la fin de la transition, 0x1403CCB30 ouvre la scene
// « ClassChooser » (ClassChooser::init 0x14013D650 : les colliers, puis une
// etiquette ClassTagBox par chat, 0x140142C10). Sa fermeture (0x140142A60,
// QuickSwipeTransition) applique les classes (0x14013FD40), sauve
// l'aventure, puis 0x1403B0BB0 ouvre « StorageItems »
// (MewDirector::DoHouseStorageEquipFlow 0x1403C49A0) si mapflag_DesertUnlocked,
// et enfin « embark » (la cinematique) et la carte. Le choix des colliers ne se
// reprend pas depuis une save : pour le voir, l'invite doit faire ce depart
// lui-meme, dans une copie de la partie de l'hote.
//
// LE DEROULEMENT
//   1. Le vote passe : l'hote prepare l'equipe (mm_team : les chats des invites
//      sont ecrits dans sa save) puis publie sa save telle quelle -- la maison
//      (embark_begin). Il attend que chaque invite qui vient l'ait chargee.
//   2. L'invite, deja en route vers le menu depuis « J'arrive », charge cette
//      copie (mgmp_savefile) et se retrouve dans la maison de l'hote ; il le
//      dit (MM_EMBARK, a = 1).
//   3. Tout le monde est la (ou l'hote n'attend plus) : l'hote part (le clic du
//      panneau est rejoue), et envoie l'equipe -- les cles, dans l'ordre du jeu
//      (MSG_TEAM kTeamEmbark). L'invite fait le meme depart dans sa copie
//      (team_copy_depart).
//   4. Pendant le choix des colliers et des objets, chaque clic de l'hote dans
//      ces deux scenes part chez les invites (MM_EMBARK, a = 2 : la scene, le
//      nom du bouton et son rang parmi les boutons du meme nom) et y est rejoue
//      au meme bouton. L'invite regarde : ses propres clics y sont retenus.
//      (Etape 2 a venir : il choisira les colliers de ses chats.)
//   5. L'hote arrive sur la carte et publie son aventure comme avant (fresh) :
//      l'invite recharge celle-ci -- c'est la sienne qui fait foi. Son depart
//      a lui n'etait qu'un miroir.
//
// LIMITES. Un clic que l'invite ne retrouve pas dans les 30 secondes (ou plus
// aucun clic de l'hote pendant 5 minutes) arrete le miroir : il peut alors
// finir les ecrans lui-meme pour rejoindre la carte. Un clic que l'hote fait
// au clavier, hors de l'update de son bouton, peut etre mal place. Un invite
// qui charge trop lentement (2 minutes) : l'hote part sans l'attendre, et
// l'invite le rejoint sur la carte, comme avant. Le miroir compte comme la
// copie de l'aventure pour les autres modules : si la premiere publication de
// l'hote sur la carte echouait et qu'il entrait aussitot dans une case, cette
// case serait appliquee au miroir (sans suite : la vraie copie la remplace).

#include <cstdint>
#include <string>
#include <vector>

namespace mgmp {

struct MmMsg;

void embark_init();          // session prete
void embark_shutdown();

// Chaque image (mm_menu_pump), avant les boutons.
void embark_frame_begin();
void embark_pump();

// --- hote (mm_menu, mm_team) ------------------------------------------------------

// Le vote `seq` est passe ; `joiners` = masque des invites qui ont dit
// « J'arrive ». Prepare l'equipe et publie la save. Faux : pas de depart
// ensemble (l'hote part tout de suite, comme avant).
bool embark_begin(uint8_t seq, uint32_t joiners);
// L'hote attend son equipe dans sa maison.
bool embark_waiting();
// Il peut partir : tous sont la, ou l'attente est finie.
bool embark_should_go();
// « Ne pas attendre ».
void embark_no_wait();
// Le texte de la fenetre d'attente.
std::string embark_wait_text();
// Le depart du jeu vient d'avoir lieu (mm_team, apres C_DepartGo) : le miroir
// des ecrans du depart commence.
void embark_host_departed(uint8_t seq);
// Le vote est annule.
void embark_cancel(const char* why);

// --- invite (mm_menu, mm_team) -------------------------------------------------

// Il a dit « J'arrive » au vote `seq` / il ne vient plus.
void embark_guest_joining(uint8_t seq);
void embark_guest_cancel(const char* why);
// L'equipe de l'hote (kTeamEmbark) : il part, l'invite le suit.
void embark_on_go(uint8_t seq, const std::vector<uint64_t>& keys);
// Il suit le depart de l'hote dans sa copie : pas de « Retour a la maison ».
bool embark_guest_active();

// --- les deux -------------------------------------------------------------------

void embark_on_mm(uint8_t from, const MmMsg& m);
// Avant chaque clic de bouton (mm_on_button_click). Vrai = clic retenu.
bool embark_on_button_click(void* button);
// Apres chaque update de bouton (mm_on_button_update).
void embark_on_button_update(void* button);

} // namespace mgmp
