#pragma once
// mm_npc -- Mewgenics Multi : les PNJ en direct.
//
// Le monde est celui de l'hote, et mm_world le recopie dans la save de l'invite
// quand elle se charge. Mais entre deux chargements, l'hote donne des chats a
// ses PNJ : sans ce module, l'invite verrait l'ancienne faveur jusqu'a son
// prochain chargement (le retour d'une aventure).
//
// Hote : deux fois par seconde, partout ou une partie est chargee (maison,
// carte, combat, evenement : la fin d'une aventure donne aussi de la faveur), il
// lit ses 8 PNJ en memoire (format dans mm_npc_state.h) et envoie leur etat
// (MSG_NPC) quand il change, ainsi qu'a chaque joueur qui arrive.
//
// Invite : dans sa propre maison (pas dans la copie d'une aventure), il recopie
// de chaque PNJ les champs simples -- la faveur d'abord -- quand les deux en
// sont a la meme recompense (npc_guest_target). Jamais « recompense prete » :
// c'est l'hote, qui a fait les dons, qui prend la recompense ; tant qu'elle est
// prete chez l'hote, l'invite garde sa faveur d'avant, sous le palier. Un PNJ
// dont l'hote a deja pris la recompense attend le prochain chargement de la
// save de l'invite, ou mm_world recopie tout (il faudrait sinon changer des
// chaines et des listes du jeu). Il recopie aussi a chaque retour dans sa
// maison : l'etat en direct est plus recent que la save de l'hote qu'il vient
// de reprendre. Plus d'hote (connexion perdue) : son dernier etat est oublie --
// le joueur peut alors donner ses chats lui-meme, et un vieil etat ecraserait
// sa faveur.

#include <cstdint>

namespace mgmp {

struct SaveFileMsg;

void npc_init();          // session prete (go_ready)
void npc_shutdown();

// Chaque image, session prete.
void npc_pump();

// Hote : l'etat de ses PNJ a un joueur qui vient d'arriver.
void npc_catchup(uint8_t peer);

// Invite : l'etat des PNJ de l'hote est arrive (MSG_NPC). Copie ce qu'il faut.
void npc_on_message(const SaveFileMsg& m);

} // namespace mgmp
