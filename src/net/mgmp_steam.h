#pragma once
// mgmp_steam -- Mewgenics Multi (protocole 59) : jouer ensemble par Steam, sans
// adresse a taper ni port a ouvrir.
//
// CE QUE CA FAIT POUR LES JOUEURS
//   - L'hote ouvre son salon (« Héberger ») : ses amis Steam voient
//     « Rejoindre la partie » sur son nom dans leur liste d'amis, et il peut les
//     inviter (Maj+Tab, ou la liste d'amis : « Inviter à jouer »).
//   - Un ami qui a le jeu lance avec le mod clique sur « Rejoindre la partie »
//     (ou accepte l'invitation) : son jeu se connecte tout seul au salon de
//     l'hote, par le reseau de Steam (relais compris : ni VPN, ni port ouvert).
//   - La connexion par adresse (mgmp, TCP direct) reste la, inchangee.
//
// COMMENT
//   Le transport du mod reste le TCP de mgmp_net : ce module est un TUYAU entre
//   une connexion Steam (ISteamNetworkingSockets, P2P) et une connexion TCP
//   locale (127.0.0.1). Chez l'hote, chaque connexion Steam acceptee ouvre une
//   connexion TCP vers son propre salon (le port du salon) ; chez l'invite, le
//   salon est rejoint a 127.0.0.1 sur un port local que ce module ecoute, et ce
//   qui y entre part par Steam. Les octets passent tels quels, dans l'ordre
//   (messages Steam fiables) : rien d'autre dans le mod ne sait que c'est Steam.
//
//   steam_api64.dll est celle du jeu (il l'importe : SteamAPI_RunCallbacks a
//   chaque image). Ce module n'en lit que des fonctions exportees de l'API
//   « flat » de Steamworks ; s'il en manque une (une autre version de la DLL),
//   tout Steam est coupe et le dit au journal -- la connexion par adresse marche
//   toujours.
//
//   Les rappels de Steam arrivent sur le fil du jeu, pendant
//   SteamAPI_RunCallbacks : « Rejoindre la partie » (GameRichPresenceJoinRequested_t,
//   337) par un CCallbackBase enregistre aupres de SteamAPI_RegisterCallback, et
//   les changements d'etat des connexions par un pointeur de fonction
//   (k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged) et par le rappel
//   1221. Les octets passent sur un fil a part (ISteamNetworkingSockets est sur
//   pour plusieurs fils).

#include <cstdint>
#include <string>

namespace mgmp {

// Chaque image, sur le fil du jeu (mm_menu_pump) : Steam pret ou pas, le rappel
// de « Rejoindre la partie » enregistre, la presence de l'hote tenue a jour.
void steam_pump();

// Steam est utilisable (les fonctions de la DLL trouvees, Steam lance).
bool steam_available();

// Le nom Steam de ce joueur ("" si inconnu).
std::string steam_self_name();

// --- hote ---------------------------------------------------------------------

// Le salon s'ouvre sur `tcp_port` : les amis Steam peuvent le rejoindre. Appele a
// chaque image tant que le salon est ouvert (sans effet une fois fait).
void steam_host_open(uint16_t tcp_port, int players);
// Le salon se ferme : plus de « Rejoindre la partie », connexions Steam fermees.
void steam_host_close();
// Combien de joueurs sont arrives par Steam (pour le texte du salon).
int steam_host_peer_count();

// --- invite ---------------------------------------------------------------------

// Un ami a clique sur « Rejoindre la partie » / accepte une invitation : vrai une
// fois par demande, avec l'identifiant Steam et le nom de l'hote.
bool steam_take_join_request(uint64_t& host_id, std::string& host_name);
// Commence la connexion Steam a l'hote ; le port local ou rejoindre le salon
// (127.0.0.1), 0 si impossible.
uint16_t steam_client_begin(uint64_t host_id);
// La connexion est finie (salon quitte, ou coupee).
void steam_client_end();
// Une connexion Steam a un hote est ouverte (en cours, etablie ou tombee).
bool steam_client_on();
// Etat de la connexion Steam, pour le texte du salon : 0 rien, 1 en cours, 2
// etablie, 3 echouee (`why` : la raison, en francais).
int steam_client_state(std::string* why);

// Tout fermer (pas a la fin du processus : Steam s'arrete avec le jeu, et ses
// fonctions ne s'appellent plus sous le verrou du chargeur).
void steam_shutdown();

} // namespace mgmp
