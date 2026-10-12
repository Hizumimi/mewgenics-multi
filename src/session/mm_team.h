#pragma once
// mm_team -- Mewgenics Multi : l'equipe mixte. Chaque joueur amene ses propres
// chats dans l'aventure de l'hote, les dirige, et les retrouve chez lui au
// retour avec leur etat final.
//
// L'EQUIPE A TOUJOURS 4 PLACES. A deux : 2 chats chacun ; a trois : 1 chacun,
// et l'hote prend la place libre ; a quatre : 1 chacun (team_quota). Chacun met
// les siens dans sa boite de depart (chez Butch). Un invite peut aussi venir
// sans chat : il joue alors avec ceux des autres.
//
// LE DEPART
//   1. L'hote clique sur « À l'aventure ! » : mm_menu refuse s'il a plus que sa
//      part dans sa boite, puis lance le vote (numerote : une reponse a un vote
//      d'avant ne compte jamais).
//   2. L'invite repond « J'arrive » : ses chats (sa part au plus) sont
//      serialises par le jeu lui-meme et partent chez l'hote (MSG_TEAM,
//      kTeamDepart, avec le jour de sa save), juste avant sa reponse
//      (MM_DEPART_READY) sur le meme canal. Trop de chats dans sa boite : une
//      fenetre le lui dit, et il repond « pas encore ».
//   3. Le vote passe : mm_menu rejoue le clic du panneau, et le jeu appelle
//      ButchBox::depart -- que ce module detourne (T_ButchBoxDepart) : les
//      chats des invites, marques kCatGuest, sont ecrits dans la table cats de
//      la save de l'hote par la fonction meme du jeu (C_SqlUpsert, celle de
//      Registry::Save), relus par le jeu (0x1400D7220), recales sur le jour de
//      l'hote (naissance, mort) et sans parents chez lui, puis ajoutes a la
//      liste des chats de la boite, et le depart du jeu est lance
//      (C_DepartGo). Un chat deja venu reprend sa ligne ; les autres prennent
//      les cles que le jeu aurait donnees a ses prochains chats (compteur
//      D_CatKeyCounter, avance d'autant). Le vote valait confirmation : pas de
//      « moins de 4 chats ? ».
//   4. L'hote envoie a tous la liste des proprietaires (kTeamOwners). Dans
//      chaque combat, chacun dirige les siens (mgmp_lockstep) ; un chat inconnu
//      (recrue) ou dont le joueur est parti est a l'hote. Les proprietaires
//      sont des joueurs (leur nom), pas des numeros : un joueur qui revient
//      retrouve ses chats, un autre qui prend son numero ne les a pas. Au
//      moindre probleme avant le depart, l'hote part avec ses seuls chats, la
//      liste envoyee est vide, le partage reste celui de mgmp (par place dans
//      l'equipe), et chacun est prevenu. Si sa boite est vide aussi (videe
//      pendant le vote), personne ne part : le depart du jeu finirait la
//      journee hors du vote de la nuit ; les invites qui attendaient en sont
//      liberes (mm_depart_called_off).
//      Un chat d'invite du meme id qu'un chat de l'hote (un clone) reste chez
//      lui : le meme chat deux fois dans l'equipe, le jeu ne le prevoit pas.
//
// LE RETOUR (fins d'aventure : victoire 0x1403B5430, defaite ou abandon
// 0x1403B4750, dans une transaction du jeu)
//   - House::AddReturningCat (T_HouseReturnCat) saute les chats des invites :
//     ils ne s'installent jamais dans la maison de l'hote ;
//   - juste avant ClearRunState (T_RunCleared), chaque chat d'invite retrouve
//     son jour et ses parents, et son etat final part chez son proprietaire
//     (kTeamResult) : mort, rentre si le jeu le ramenait a la maison (il
//     appelle AddReturningCat pour lui, comme pour ceux de l'hote), sinon pas
//     revenu -- une aventure abandonnee ou perdue ne ramene personne
//     (0x1403B4F03) ; puis une liste de proprietaires vide pour tous ;
//   - l'invite calcule aussi cet etat dans SA copie de l'aventure, qui finit en
//     meme temps (lockstep) : il peut rentrer avant que l'hote ait fini ;
//     l'etat de l'hote, s'il arrive avant, est celui qui compte ;
//   - l'invite le recopie dans sa save juste avant que le jeu ne l'ouvre
//     (team_prepare_own_save, appele par mgmp_savefile a cote du monde), apres
//     une copie de securite. Arrive trop tard (sa save a ete chargee depuis le
//     depart), un chat n'est recopie que s'il n'a pas change depuis le depart.
//     Une fenetre le lui dit, une fois chez lui. Un chat dont l'etat final
//     attend encore d'etre recopie ne repart pas (une fenetre demande de
//     recharger la save d'abord) ; un chat reparti avant que l'etat final de
//     son depart d'avant n'arrive ne rentre que de son dernier depart ; un chat
//     deja mort dans la save le reste.
//
// LIMITES. Les chats des invites restent dans la save de l'hote (comme les
// chats donnes ou partis), hors de sa maison, avec leur marque : meme apres un
// redemarrage de l'hote en pleine aventure, ils ne s'installent pas chez lui --
// mais l'aventure et ses proprietaires ne vivent qu'en memoire : apres ce
// redemarrage, le partage redevient celui de mgmp et aucun etat final n'est
// envoye (l'invite garde ses chats tels qu'au depart, ou tels que sa copie les
// a vus). De meme si l'hote charge une autre save en pleine aventure mixte puis
// revient a celle-ci : l'autre save ne doit pas prendre ses chats marques pour
// des clones, l'aventure est donc oubliee des que l'autre se charge. Si l'invite ferme son jeu avant de rentrer chez lui, l'etat final
// est perdu : ses chats restent tels qu'au depart. Une recrue va a l'hote. Les
// joueurs sont reconnus par leur nom : deux invites du meme nom ne s'echangent
// pas leurs chats tant que chacun garde son numero. Un chat d'invite repris
// d'une aventure d'avant n'a pas son post-chargement visuel (0x1407393E0) :
// sans effet tant que son apparence n'a pas change. Une erreur sqlite dans la
// fonction du jeu (C_SqlUpsert) ouvre la boite d'erreur du jeu et le ferme,
// comme pour ses propres ecritures : rien ne peut l'attraper.

#include <cstdint>
#include <string>
#include <vector>

namespace mgmp {

struct SaveFileMsg;
enum Target : int;

void team_init();          // session prete (go_ready)
void team_shutdown();

// Chaque image, session prete.
void team_pump();

// Chaque image, session ou pas (mm_menu_pump) : les fenetres de ce module.
void team_ui_pump();

// Hote : un joueur arrive -- les proprietaires de l'aventure en cours.
void team_catchup(uint8_t peer);

// Un message MSG_TEAM.
void team_on_message(uint8_t from, const SaveFileMsg& m);

// Le nombre de chats qu'un joueur emmene, pour `players` joueurs en session.
int team_quota(bool host, int players);

// --- hote : le vote de depart (mm_menu.cpp) -------------------------------------

// Les chats de sa boite de depart (-1 : maison ou boite introuvable).
int team_host_box_count();
// Le texte pour l'hote qui a trop de chats dans sa boite.
std::string team_host_too_many_text(int players, int have);
// Un nouveau vote commence (son numero) : les chats d'un vote d'avant ne
// comptent plus.
void team_vote_begin(uint8_t seq);
// Le vote est annule : oublier les chats recus.
void team_vote_reset(const char* why);
// Ce joueur ne vient plus (« pas encore » apres « J'arrive »).
void team_forget_peer(uint8_t peer);

// --- invite : « J'arrive » (mm_menu.cpp) ----------------------------------------

// La ligne de la fenetre de depart qui parle de ses chats : ceux qu'il emmene,
// ou comment en emmener.
std::string team_guest_offer_line();
// Envoie ses chats a l'hote pour le vote `seq` (aucun s'il n'en a pas mis).
// Faux, avec le texte d'une fenetre pour le joueur, s'il ne peut pas partir
// ainsi (trop de chats, chats illisibles, envoi impossible).
bool team_guest_send(uint8_t seq, std::string& problem_utf8);
// Il ne vient plus au vote `seq` (« pas encore », annulation, vote annule) :
// pas de fenetre « tes chats n'ont pas pu partir » pour ce depart.
void team_guest_withdraw(uint8_t seq);
// Ses chats sont-ils dans l'aventure en cours de l'hote ? (Faux sans equipe
// mixte.) Pour la fenetre « Rejoindre l'aventure ».
bool team_guest_has_cats_in_run();

// --- mgmp_lockstep ------------------------------------------------------------

// Une equipe mixte est en cours : les chats se partagent par proprietaire.
bool team_owners_active();
// Le joueur qui dirige ce chat (id CatData+0x00), -1 si l'equipe ne le connait pas.
int team_owner_of(uint64_t cat_id);

// --- invite : retour chez lui (mgmp_savefile.cpp) -----------------------------

// Juste avant que le jeu n'ouvre sa save `name` : y recopie l'etat final des
// chats qu'il avait emmenes depuis cette save.
void team_prepare_own_save(const char* name);

// --- crochets (mgmp_hooks.cpp) -------------------------------------------------

// Detour et original de T_ButchBoxDepart, T_HouseReturnCat et T_RunCleared.
bool team_binding(Target t, void** detour, void*** original);

} // namespace mgmp
