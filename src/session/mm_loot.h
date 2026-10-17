#pragma once
// mm_loot -- Mewgenics Multi : le defile du butin. Les regles, le message et les
// fenetres sont dans mm_loot_core.h ; ici, le jeu et le reseau.
//
// LE JEU (1.1.21239)
//   - La victoire (0x1403B5430, T_RunVictory, detournee par mm_purse) range les
//     objets du sac (Inventory+0x38) dans le rangement de la maison
//     (Inventory+0x78) en gardant leur id ; ceux qui deviennent autre chose au
//     rangement (GON on_store, des pieces par exemple) n'y sont pas, ceux qu'un
//     chat porte rentrent avec lui. loot_victory_before releve les ids du sac et
//     les joueurs de l'aventure AVANT la fonction du jeu (ClearRunState, a
//     l'interieur, oublie l'equipe) ; loot_victory_after retrouve ces ids dans
//     le rangement : ce sont ces objets qui defilent, dans l'ordre du sac.
//   - Un objet gagne par un invite quitte le rangement de l'hote (C_BucketErase,
//     l'objet par son id, verifie par son empreinte) quand l'invite repond
//     l'avoir pris (kLootGot) ; chez l'invite, il est ajoute au rangement de SA
//     save quand elle se charge (team_note_loot puis team_apply_loot, apres le
//     retour de ses chats). Pas de reponse, ou l'invite ne peut pas le prendre :
//     il reste chez l'hote (un doublon plutot qu'une perte).
//   - Le nom et la description d'un objet : SpawnDatabase::ItemName et
//     SpawnDatabase::ItemTooltip (0x1407B4BE0, 0x1407B5480 : rendent une
//     std::wstring par valeur, que _Tidy_deallocate 0x140052290 libere sur le
//     tas du jeu), appeles sur une copie de l'objet faite ici -- ils ne font que
//     lire ses champs (nom, texte, usure, b pour un objet lie a un chat). Le
//     SpawnDatabase est le global 0x1413C79D0.
//
// LE DEROULEMENT
//   - Hote : a la victoire d'une aventure mixte, en session, le defile part
//     (kLootStart a tous les invites, meme sans objet : ceux qui attendaient sont
//     liberes). Il avance chez lui (LootHost) et publie son etat (kLootState) a
//     chaque changement et chaque seconde. Il est « pret » tant que sa fenetre
//     du defile est ouverte (ou masquee par lui, rien d'autre a l'ecran) : sinon
//     le temps s'arrete pour tous. Ni nuit ni depart tant que le defile n'est pas
//     fini (loot_host_busy).
//   - Invite : il dit a l'hote s'il voit le defile (kLootReady, a chaque
//     changement ; « plus pret » seulement apres 1,5 s, pour qu'une fenetre qui se
//     referme et se rouvre ne compte pas), envoie ses choix (kLootChoose) et
//     prend les objets qu'il gagne (kLootGive -> kLootGot). Sa copie gagne :
//     s'il avait amene des chats, la fenetre « Rentrer » attend le defile
//     (loot_holds_home), 15 s au plus s'il ne vient pas.
//
// LES FENETRES : celles du jeu (mm_menu), comme le vote du chemin -- loot_popup
// dit la fenetre voulue maintenant, son `layout` change quand ses boutons
// changent (mm_menu la referme et ouvre la suivante).

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "mm_team_merge.h"   // ItemFields

namespace mgmp {

// Chaque image (mm_menu_pump), avant les fenetres.
void loot_pump();

// Un message MSG_TEAM kTeamLoot.
void loot_on_team(uint8_t from, const uint8_t* data, size_t size);

// La victoire (h_RunVictory, mm_purse) : avant et apres la fonction du jeu.
void loot_victory_before();
void loot_victory_after();

// La fenetre voulue maintenant ; 0 = aucune.
struct LootPopup {
    uint32_t    layout = 0;
    std::string text;
    const char* yes = nullptr;     // libelle UTF-8 du premier bouton
    const char* no  = nullptr;     // du second ; nullptr = un seul bouton
};
bool loot_popup(LootPopup& out);
// Un bouton de cette fenetre presse par le joueur (pas une fermeture du mod).
void loot_popup_button(uint32_t layout, bool yes);

// Invite : la fenetre « Rentrer » attend (un defile ou il est, ou attendu).
bool loot_holds_home();
// Hote : un defile est en cours ou sa fin n'a pas ete vue.
bool loot_host_busy();

// --- le rangement de la partie chargee, pour les dons aux PNJ (mm_npcgift) ------

struct LootStoredItem {
    uint64_t   id = 0;              // Equipment+0x00
    ItemFields f;
};
// Les objets du rangement (kInv_Storage), dans l'ordre de sa liste. Faux : illisible.
bool loot_storage_items(std::vector<LootStoredItem>& out);
// Un ecran des objets ou une confirmation du jeu est ouvert : pas de retrait.
bool loot_storage_busy();
// Retire du rangement l'objet de cet id, verifie par son empreinte (item_digest).
// Faux (et pourquoi) s'il n'y est plus, a change, ou si le jeu n'a rien retire.
bool loot_storage_take(uint64_t id, uint64_t digest, std::string& why);
// Le nom d'un objet tel que le jeu l'affiche (sa cle GON a defaut), en UTF-8.
std::string loot_item_label(const ItemFields& f);

// --- le sac de l'aventure et un objet en memoire, pour les achats (mm_shop) -----

// Les objets du sac (kInv_Backpack), dans l'ordre de sa liste. Faux : illisible.
bool loot_bag_items(std::vector<LootStoredItem>& out);
// Les champs d'un Equipment en memoire (0x60 octets, son id en tete) -- celui
// d'un objet de boutique. Faux : illisible.
bool loot_read_equipment(const void* eq, ItemFields& f, uint64_t* id);

} // namespace mgmp
