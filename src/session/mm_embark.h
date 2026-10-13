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
//   4. Le choix des colliers (mm_collars) : chacun regarde le chat qu'il veut
//      et choisit les colliers de ses propres chats ; ceux que posent les
//      autres apparaissent en direct. Seul « Valider » de l'hote part chez les
//      invites (MM_EMBARK kEmbarkClick : la scene, le nom du bouton et son rang
//      parmi les boutons du meme nom), rejoue une fois leurs colliers a jour ;
//      celui d'un invite est retenu. La confirmation du jeu (« un chat sans
//      collier ? », scene « YesNoPrompt ») suit le meme chemin ; l'hote dit
//      avec son « Valider » s'il l'aura (champ c), et si elle ne s'ouvre que
//      chez l'invite, il y repond « oui ».
//      Le choix des objets (« StorageItems », mm_gear) : chacun equipe ses
//      chats avec son propre rangement (l'invite a charge le sien dans sa
//      copie) ; ses confirmations du jeu sont les siennes. Seul « Valider » de
//      l'hote est rejoue chez les invites ; juste avant, leur derniere photo
//      (chats et rangement) part chez lui, qui l'applique sur la carte.
//   5. L'hote arrive sur la carte et publie son aventure comme avant (fresh) :
//      l'invite recharge celle-ci -- c'est la sienne qui fait foi. Son depart
//      a lui n'etait qu'un miroir.
//
// LIMITES. Un clic que l'invite ne retrouve pas dans les 30 secondes (ou plus
// aucun signe de l'hote pendant 5 minutes) arrete le miroir : il peut alors
// finir les ecrans lui-meme pour rejoindre la carte (et le dit a l'hote, qui
// peut alors choisir les colliers de ses chats ; leurs objets restent a
// l'invite, dont la derniere photo part quand il valide lui-meme). Un clic que l'hote fait
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

// MM_EMBARK, champ a. Le champ b est toujours le numero du vote.
enum EmbarkMsg : uint8_t {
    kEmbarkReady       = 1,  // invite -> hote : il a charge la maison de l'hote
    kEmbarkClick       = 2,  // hote -> invites : un clic a rejouer (texte « scene|nom »,
                             //   x = rang parmi les boutons de ce nom, y = numero ; pour
                             //   « Valider » des colliers, c = 1 : la confirmation du jeu
                             //   s'ouvre chez l'hote, 0 : non, 2 : on ne sait pas)
    kEmbarkCollarReq   = 3,  // invite -> hote : c = 1 poser / 0 retirer, x/y = cle du
                             //   chat (bas/haut), texte = classe du collier
    kEmbarkCollarSet   = 4,  // hote -> invites : c = 1 porte / 0 aucun, x/y = cle,
                             //   texte = classe
    kEmbarkChooserOpen = 5,  // invite -> hote : son choix des colliers est ouvert
    kEmbarkGone        = 6,  // invite -> hote : il ne suit plus le depart
    kEmbarkAlive       = 7,  // hote -> invites : toujours dans les ecrans du depart (toutes
                             //   les 20 s : au choix des objets, aucun clic ne part)
};

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
// Le « Valider » de l'hote est deja arrive (pas encore rejoue) : le choix des
// colliers se ferme bientot chez lui.
bool embark_guest_close_queued();

// Hote, pendant les ecrans du depart : cet invite suit le depart (il avait
// charge la maison de l'hote, ou son « pret » est arrive juste apres), il est
// toujours la et n'a pas dit qu'il ne suivait plus (kEmbarkGone).
bool embark_follower_active(uint8_t peer);

// --- les deux -------------------------------------------------------------------

void embark_on_mm(uint8_t from, const MmMsg& m);
// Avant chaque clic de bouton (mm_on_button_click). Vrai = clic retenu.
bool embark_on_button_click(void* button);
// Apres chaque update de bouton (mm_on_button_update).
void embark_on_button_update(void* button);

} // namespace mgmp
