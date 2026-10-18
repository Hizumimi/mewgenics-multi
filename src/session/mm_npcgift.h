#pragma once
// mm_npcgift -- Mewgenics Multi : les dons de l'invite aux PNJ (protocole 50).
// Les regles et le message : mm_npcgift_core.h.
//
// LE JEU (1.1.21239, releve le 17 octobre)
//   - Le tuyau de la maison prend un chat (HousePipe::TryPlaceCat, mm_menu),
//     puis la carte des PNJ (NPCMapDrawer) : un clic sur un PNJ joue une
//     animation, puis son rappel (NPCMapDrawer::click_<pnj>, lambda_1,
//     0x14027E640..0x14027ED20) appelle sub_1401C1650(progression, PNJ, CatData,
//     1) -- la faveur --, marque le chat donne (CatData+0xBF8 |= 0x2001), le
//     retire de la maison et l'efface, puis la carte passe au bureau du PNJ
//     (GoToNPCOffice).
//   - sub_1401C1650 (T_NpcFavor) : faveur += quantite (E+0x40), recompense
//     prete (E+0x48) si la faveur atteint le palier « favor » de la recompense
//     visee ; pour Beanies (PNJ 0), les 4 capacites actives du chat
//     (CatData+0x810, pas de 0x20) et ses 2 passives (+0x910, +0x938), sauf
//     « None », vont dans les listes de la progression (+0x580, +0x598) que ses
//     aiguilles piochent. Rien d'autre n'est lu du chat.
//   - La recompense se prend dans le dialogue du bureau (commande « get
//     npc_reward » de NPCDialogController::BeginSequencePiece, 0x1408EFDC2) :
//     GlobalProgressionData::CacheInReward(PNJ) (sub_1401C18E0,
//     T_CacheReward). Si la recompense est prete : le PNJ passe a la suivante
//     (sub_140274910 : prises, visee, faveur et drapeau remis a 0), puis chaque
//     element est donne -- « coins » et « food » a la maison (House+0xB4,
//     +0xB0, la maison vivante : D_HouseInstance, remis a 0 par son
//     destructeur), les objets au rangement (Inventory::insert_item, lieu 1 =
//     kInv_Storage), le reste dans le monde (agrandissements, boutiques,
//     drapeaux de la save, quetes de Beanies).
//
// LE MOD
//   Invite (sa maison, pas une copie, en session) : le tuyau prend ses chats ;
//   la faveur du don n'est pas ajoutee a sa copie du PNJ -- elle part chez
//   l'hote (kNpcDonate), qui l'ajoute au sien, et l'etat en direct (mm_npc) la
//   lui renvoie. Hote : le don applique, si la jauge vient de se remplir, la
//   recompense est au donneur ; elle est prise des que l'hote est dans sa maison
//   sans fenetre ouverte et que le donneur est la (CacheInReward appele par le
//   mod, ou par le jeu si l'hote va voir le PNJ avant). Les pieces, la
//   nourriture et les objets qu'elle a donnes partent chez le donneur
//   (kNpcGift) ; ils ne quittent la maison de l'hote qu'a sa reponse (kNpcGot).
//   Chez le donneur, ils attendent le prochain chargement de sa save, avec son
//   butin (mm_team, fichier « <save>.mgmp-attente »).

#include <cstddef>
#include <cstdint>

#include "mgmp_addresses.h"   // Target

namespace mgmp {

// Chaque image (mm_menu_pump).
void npcgift_pump();

// MSG_TEAM kTeamNpc (mm_team le route ici).
void npcgift_on_team(uint8_t from, const uint8_t* p, size_t n);

// Invite en session, dans sa propre maison : ses chats vont aux PNJ par l'hote
// (le tuyau les prend).
bool npcgift_guest_donates();

// Mewgenics Multi (protocole 62, mm_gift) : hote, des recompenses envoyees (sans
// reponse) ou des objets pas encore retires tiennent des objets de son rangement
// par leur id -- le relire (un don recu) leur donnerait d'autres ids.
bool npcgift_storage_refs();

// Detours et originaux de T_NpcFavor et T_CacheReward (mgmp_hooks.cpp).
bool npcgift_binding(Target t, void** detour, void*** original);

} // namespace mgmp
