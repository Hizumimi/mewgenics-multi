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
//      Depart ensemble (mm_embark) : l'equipe est preparee des que le vote
//      passe (team_embark_prepare), puis la save de l'hote part chez les
//      invites ; le depart du jeu vient quand ils l'ont chargee, et l'equipe
//      (kTeamEmbark) leur est envoyee pour qu'ils le fassent aussi.
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
//     Une fenetre le lui dit, une fois chez lui. Son rangement aussi, s'il a
//     equipe ses chats au depart (mm_gear) : il devient, dans la meme
//     transaction, celui de la photo que l'hote a appliquee -- si tous ses chats
//     sont recopies et si le rangement de la save n'a pas change depuis
//     « J'arrive ». Un chat dont l'etat final
//     attend encore d'etre recopie ne repart pas (une fenetre demande de
//     recharger la save d'abord) ; un chat reparti avant que l'etat final de
//     son depart d'avant n'arrive ne rentre que de son dernier depart ; un chat
//     deja mort dans la save le reste.
//   - aventure gagnee : sa bourse (mm_purse) arrive avant l'etat final de ses
//     chats (MM_PURSE a = 3) ; elle s'ajoute a l'or et a la nourriture de sa
//     save au meme chargement (team_apply_purse), apres la meme copie de
//     securite -- ou au chargement suivant si sa save etait deja ouverte. Le
//     butin qu'il gagne au defile (mm_loot) s'ajoute de meme au rangement de sa
//     save (team_apply_loot), apres le retour de ses chats.
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
struct ItemFields;
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
// En envoie un (mm_gear) : a tous (peer < 0) ou a un joueur.
bool team_send(const std::vector<uint8_t>& body, int peer);

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
// Depart ensemble (mm_embark), le vote `seq` passe : l'equipe est preparee
// tout de suite (les chats des invites ecrits dans la save, avant qu'elle parte
// chez eux) ; le depart du jeu viendra au clic rejoue. Faux sans maison ou
// sans boite.
bool team_embark_prepare(uint8_t seq);

// --- invite : le depart ensemble (mm_embark) --------------------------------------

// Fait dans la copie de la partie de l'hote le depart qu'il vient de faire :
// les memes cles, dans le meme ordre (C_DepartGo sur la boite de la copie).
bool team_copy_depart(const std::vector<uint64_t>& keys);

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
// Le choix des objets (mm_gear). Son rangement tel que le jeu l'avait a
// « J'arrive » pour le vote `seq` (faux : il n'amene pas de chat, ou il n'a pas
// pu etre lu) ; puis celui de la photo que l'hote a appliquee a ses chats, qui
// deviendra celui de sa save a leur retour.
bool team_guest_storage(uint8_t seq, std::vector<uint8_t>& out);
void team_guest_storage_after(uint8_t seq, const std::vector<uint8_t>& post);
// Ses chats sont-ils dans l'aventure en cours de l'hote ? (Faux sans equipe
// mixte.) Pour la fenetre « Rejoindre l'aventure ».
bool team_guest_has_cats_in_run();
// Invite : des chats qu'il a amenes lui-meme sont dans l'equipe (pas seulement
// des recrues qu'on lui a donnees en route) -- il est du defile du butin.
bool team_guest_brought_cats();
// Hote : les joueurs dont des chats sont dans l'aventure mixte en cours (« Lou et
// Max », sans [ ] : balises du jeu), "" sans chat d'invite. Pour la confirmation
// de l'abandon : c'est une defaite pour toute l'equipe.
std::string team_run_guest_names();

// --- la defense de la maison (protocole 54) -------------------------------------
//
// Le boss de la maison est a chaque joueur (sa save : mm_world le laisse a
// l'invite). Quand il attaque, le panneau dit « Défendre ! » et le depart du jeu
// (ButchBox::depart, 0x1400AE3E1) lance la defense au lieu d'une aventure
// (0x1403B1680) : un combat dans la maison, avec les chats de la boite. En
// session, chacun defend la sienne, seul : le depart passe tel quel
// (h_ButchDepart), et la session n'entre pas dans ce combat (mgmp_lockstep).

// Un boss attaque la maison de ce joueur : la progression (MewDirector+0x5A8),
// +0x468 non nul et +0x478 nul -- le test du depart du jeu (0x1400AE3E1).
bool team_house_defense_pending();
// Le combat de la defense de la maison est en cours chez ce joueur :
// MewDirector+0x70C (un octet), pose par le jeu au debut de ce combat
// (0x1403B1A37 ; 0x1403191C3 dans le lanceur de deboguage) ou a la reprise d'une defense sauvee
// (ContinueAdventure, 0x1403BCEF5), remis a zero par ClearRunState (0x1403B6493) ;
// ClearRunState et les fins d'aventure le lisent (0x1403B477E : une defense
// ramene son equipe meme perdue).
bool team_house_defense_running();
// Protocole 55. Au menu, et au chargement d'une partie (MewDirector::init, avant
// ContinueAdventure qui repose le drapeau si la save reprend une defense) : une
// defense quittee par « Quitter vers le menu » laisse MewDirector+0x70C pose --
// le jeu ne le remet a zero qu'a la fin d'une aventure, et le MewDirector vit
// autant que le processus. On le remet a zero (vrai si c'etait le cas), sinon la
// save chargee ensuite se croirait en defense : la session ne suivrait plus ses
// combats, et le jeu les jouerait autrement que chez les autres
// (Character::BeginTurn le lit).
bool team_house_defense_forget_stale();

// --- mgmp_lockstep ------------------------------------------------------------

// Une equipe mixte est en cours : les chats se partagent par proprietaire.
bool team_owners_active();
// Le joueur qui dirige ce chat (id CatData+0x00), -1 si l'equipe ne le connait pas.
int team_owner_of(uint64_t cat_id);

// --- mm_purse : les bourses de l'aventure ---------------------------------------

// Hote : l'aventure mixte en cours. `gen` change a chaque nouvelle equipe mixte ;
// `slots` = les joueurs qui y ont amene des chats, un bit par numero de joueur
// AU DEPART (stable meme si un joueur revient sous un autre numero), l'hote
// compris s'il en a amene. Faux sans aventure mixte.
bool team_run_slots(uint32_t& gen, uint32_t& slots);
// Hote : le vote du depart de l'aventure mixte en cours (MM_PURSE a = 3, champ b).
uint8_t team_run_seq();
// Hote : le joueur (numero au depart) a qui est ce chat (id CatData+0x00) ;
// l'hote pour les siens et pour un chat que l'aventure ne connait pas (une recrue).
uint8_t team_slot_of_cat(uint64_t cat_id);
// Hote : le numero maintenant du joueur de ce numero au depart (-1 : il n'est
// pas la), et son nom.
int         team_slot_peer(uint8_t slot);
std::string team_slot_name(uint8_t slot);
// Invite : ce qu'il rapporte de l'aventure du vote `seq` (MM_PURSE a = 3). Ajoute
// a l'or et a la nourriture de sa save quand elle se charge (team_prepare_own_save),
// et dit dans la fenetre du retour.
void team_note_payout(uint8_t seq, int32_t coins, int32_t food);
// Invite : un objet gagne au defile du butin (mm_loot) de l'aventure du vote
// `seq`. Ajoute au rangement de sa save quand elle se charge
// (team_prepare_own_save, apres le retour de ses chats), et dit dans la fenetre
// du retour. `key` (le defile et l'objet) : note une seule fois. Faux s'il n'a
// pas pu etre pris (trop d'objets en attente) : il reste chez l'hote.
bool team_note_loot(uint8_t seq, uint64_t key, const ItemFields& item, const std::string& name_utf8);
// Invite : les pieces et la nourriture qu'un PNJ lui donne pour son don
// (mm_npcgift). Ajoutees a sa save a son prochain chargement, comme sa bourse
// (et dites a part). Faux si elles n'ont pas pu etre gardees : elles restent chez
// l'hote.
bool team_note_gift(int32_t coins, int32_t food);

// --- invite : retour chez lui (mgmp_savefile.cpp) -----------------------------

// Juste avant que le jeu n'ouvre sa save `name` : y recopie l'etat final des
// chats qu'il avait emmenes depuis cette save, et ajoute a sa maison ce qu'il
// a rapporte (mm_purse).
void team_prepare_own_save(const char* name);

// --- crochets (mgmp_hooks.cpp) -------------------------------------------------

// Detour et original de T_ButchBoxDepart, T_HouseReturnCat et T_RunCleared.
bool team_binding(Target t, void** detour, void*** original);

} // namespace mgmp
