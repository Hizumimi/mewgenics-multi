#pragma once
// mm_world -- Mewgenics Multi : le monde partage (PNJ, histoire, deblocages).
//
// Chaque joueur garde sa maison et ses chats ; le monde, lui, est celui de
// l'hote. L'hote envoie sa save entiere (MSG_WORLD) a chaque invite qui arrive,
// puis a nouveau chaque fois qu'elle change sur le disque (fin de journee, fin
// d'aventure...). L'invite garde la derniere, et juste avant que le jeu n'ouvre
// SA save -- son choix sur l'ecran des sauvegardes, ou le retour d'une
// aventure -- il y recopie le monde de l'hote :
//
//   - npc_progress (la faveur et les quetes des PNJ, leurs boutiques, les
//     intros et drapeaux de l'histoire) : celui de l'hote, sauf ses
//     « recompenses pretes » (c'est l'hote, qui a fait les dons, qui les
//     prend) ; entre deux chargements, mm_npc recopie la faveur en direct ;
//   - unlocks (classes, capacites, boss...) et tutorial_tokens (tutoriels vus) :
//     ceux de l'hote EN PLUS des siens ;
//   - house_unlocks : les agrandissements de maison de l'hote en plus des siens,
//     et la plus grande des deux maisons -- « déblocages à tous » ;
//   - les proprietes de l'histoire (zones, compteurs de deblocage, suivi des
//     dialogues, infos sur les chats donnees par Tink, rangement de Butch...) :
//     pour chacune, la plus grande des deux valeurs.
//
// Tout le reste -- chats, meubles, or, nourriture, inventaire, jour, meteo,
// boss de la maison -- reste a l'invite. Une copie de sa save est faite avant
// la premiere modification de la session (saves\mgmp_backups).
//
// LES SAVES SONT DES BASES SQLITE, et le mod a son propre sqlite
// (third_party/sqlite). L'hote ne pose aucun verrou sur sa save, que le jeu
// tient ouverte : il en lit les octets, deux fois a une seconde d'ecart, et
// n'envoie qu'une copie identique les deux fois, sans journal a cote, et qui
// passe le controle d'integrite de sqlite. L'invite ouvre ces octets en
// memoire (sqlite3_deserialize) et ecrit dans sa propre save, que le jeu n'a
// pas encore ouverte, en une seule transaction : en cas d'erreur, rien n'est
// change.

#include <cstdint>

namespace mgmp {

struct SaveFileMsg;

void world_init();          // session prete (go_ready)
void world_shutdown();

// Chaque image, session prete. Hote : surveille sa save et la renvoie quand
// elle a change.
void world_pump();

// Hote : envoie le monde actuel a un joueur qui vient d'arriver (s'il y en a un).
void world_catchup(uint8_t peer);

// Invite : la save de l'hote est arrivee (MSG_WORLD). Copie m.data.
void world_on_message(const SaveFileMsg& m);

// Invite : le monde de l'hote est la. Hote : il a une save a envoyer.
bool world_have_host();

// Invite, juste avant que le jeu n'ouvre sa propre save `name`
// (« steamcampaign03.sav ») : y recopie le monde de l'hote. Ne fait rien hors
// session, sur l'hote, ou si le monde de l'hote n'est pas encore arrive.
void world_prepare_own_save(const char* name);

} // namespace mgmp
