#pragma once
// mm_team_merge -- Mewgenics Multi : l'equipe mixte, la partie portable (sqlite
// et la bibliotheque standard seulement, verifiee par tests/test_team_merge.cpp).
//
// Chacun amene ses chats dans l'aventure de l'hote :
//   - au depart, l'invite envoie les chats de sa boite de depart, tels que le
//     jeu les serialise (SerializeCatData, sub_14022F410), avec le jour de sa
//     save ; l'hote les ecrit dans la table « cats » de sa save (team_plan_keys
//     choisit les cles), par la fonction meme du jeu, puis les met dans
//     l'equipe ; le jeu les charge lui-meme depuis la table (0x1400D7220) ;
//     l'hote dit ensuite a chacun a qui est chaque chat (kTeamOwners) : chaque
//     joueur dirige les siens ;
//   - a la fin, l'etat final de ses chats (rentre, mort, ou pas revenu)
//     revient a l'invite, qui le recopie dans SA save juste avant qu'elle se
//     charge.
//
// FORMATS (releves sur les saves du joueur et dans le code du jeu)
//   Table cats : key INTEGER (la cle du chat, CatData+0xC48), data = u32 taille
//     decompressee, puis un bloc LZ4 brut du chat serialise.
//   Chat serialise, version 19 : u32 version, u64 id (CatData+0x00), le nom
//     (u64 nombre de caracteres + UTF-16), une chaine (u64 n + octets, +0x38),
//     u32 (+0x58), u32 (+0x5C), puis u64 drapeaux (CatData+0xBF8) : 0x40000 =
//     mort (le jeu le pose a la mort, 0x140116451), 0x80000 = en aventure (pose
//     au depart par 0x1403B13F0).
//   house_state (table files) : u32 version (0), u32 nombre de chats, puis pour
//     chacun u64 cle, u64 n + nom de la piece, trois f64. Un chat mort ou parti
//     reste dans la table cats mais n'est plus dans house_state.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct sqlite3;

namespace mgmp {

constexpr uint64_t kCatDead      = 0x40000;
constexpr uint64_t kCatAway      = 0x80000;
// Mewgenics Multi : la marque d'un chat d'invite dans la save de l'hote, posee
// par l'hote avant d'ecrire le chat. Les drapeaux sont un u64 (serialises sur 8
// octets, 0x14022F6DA) dont le jeu n'utilise que le mot du bas, et tous ses
// acces sont 64 bits (or/and/copie) : la marque suit le chat partout -- en
// memoire, dans la table, apres un redemarrage. L'invite l'efface au retour.
constexpr uint64_t kCatGuest     = 1ull << 56;
constexpr uint32_t kCatVersion   = 19;

// La tete de tout message MSG_TEAM : u32 magie, u8 format, u8 genre.
constexpr uint32_t kTeamMagic  = 0x4D41454D;   // « MEAM » (Mewgenics multi, equipe)
constexpr uint8_t  kTeamFormat = 2;            // 2 : vote, jour de l'invite, sort de chaque chat, cle hote des proprietaires

// Ce que devient un chat (kTeamResult).
enum : uint8_t {
    kFateHome = 0,      // il rentre (vivant, ou mort : ses drapeaux le disent)
    kFateGone = 1,      // vivant, mais le jeu ne le ramene pas (aventure abandonnee ou
                        // perdue, ou il a quitte l'equipe en route) : il ne rentre pas
};

struct TeamCat {
    uint64_t             own_key  = 0;   // sa cle dans la save de son proprietaire
    uint64_t             host_key = 0;   // sa cle dans la save de l'hote (0 au depart)
    uint8_t              fate     = kFateHome;
    std::vector<uint8_t> data;           // le chat serialise, non compresse
};

enum : uint8_t {
    kTeamDepart = 1,    // invite -> hote : les chats qu'il amene
    kTeamResult = 2,    // hote -> invite : ce qu'ils sont devenus
    kTeamOwners = 3,    // hote -> invites : a qui est chaque chat de l'equipe
    kTeamEmbark = 4,    // hote -> invites : il part, voici l'equipe (mm_embark)
    kTeamGear   = 5,    // invite -> hote : l'equipement de ses chats et son rangement (mm_gear)
    kTeamGearAck = 6,   // hote -> invite : la photo de son equipement que l'hote a prise
    kTeamRunGear = 7,   // (protocole 42-43, plus envoye) invite -> hote : sa photo d'objets en aventure
    kTeamRunGearAck = 8,// (protocole 42-43, plus envoye) hote -> invite : prise ou refusee
    kTeamItemOp  = 9,   // les deux sens : un mouvement d'objet en aventure (mm_rungear)
    kTeamItemAck = 10,  // hote -> invite : son mouvement pris ou refuse
    kTeamItemSync = 11, // hote -> invite : le sac, la poubelle et les chats de l'aventure, pour repartir juste
    kTeamItemSyncReq = 12, // invite -> hote : demande kTeamItemSync (apres ses mouvements numerotes jusqu'a seq)
    kTeamLoot    = 13,  // les deux sens : le defile du butin (mm_loot, protocole 47 ; corps : mm_loot_core)
    kTeamRecruit = 14,  // hote -> tous : une recrue rejoint l'equipe en route, et a qui elle est (protocole 49)
    kTeamNewCats = 15,  // hote -> un invite : a la fin, les recrues qui rentrent chez lui (protocole 49)
    kTeamNpc     = 16,  // les deux sens : les dons de l'invite aux PNJ (mm_npcgift, protocole 50)
    kTeamShop    = 17,  // les deux sens : les achats en boutique avec sa bourse (mm_shop, protocole 51)
    kTeamGift    = 18,  // les deux sens : les dons entre joueurs a la maison (mm_gift, protocole 62)
    kTeamLast    = kTeamGift,
};

// Le message MSG_TEAM : des chats (kTeamDepart, kTeamResult)...
struct TeamMsg {
    uint8_t              kind = 0;
    uint8_t              seq  = 0;       // kTeamDepart : le vote de depart auquel il repond
    int64_t              day  = 0;       // kTeamDepart : le jour de la save de l'invite
    std::vector<TeamCat> cats;
};
std::vector<uint8_t> team_encode(const TeamMsg& m);
bool team_decode(const uint8_t* p, size_t n, TeamMsg& m);

// ... ou les proprietaires (kTeamOwners) : le vote du depart de cette aventure,
// puis pour chaque chat de l'equipe son id (CatData+0x00), sa cle dans la save
// de l'hote, et le numero du joueur qui le dirige. Vide = pas d'equipe mixte
// (le partage des chats reste celui de mgmp, par place dans l'equipe).
struct TeamOwner {
    uint64_t cat_id   = 0;
    uint64_t host_key = 0;
    uint8_t  peer     = 0;
};
std::vector<uint8_t> team_owners_encode(uint8_t seq, const std::vector<TeamOwner>& owners);
bool team_owners_decode(const uint8_t* p, size_t n, uint8_t& seq, std::vector<TeamOwner>& owners);

// ... ou le depart ensemble (kTeamEmbark) : le vote du depart, puis les cles des
// chats de l'equipe dans la save de l'hote, dans l'ordre ou le jeu les a mis
// (ceux de l'hote, puis ceux des invites). Chaque invite fait le meme depart
// dans sa copie de la partie de l'hote.
std::vector<uint8_t> team_embark_encode(uint8_t seq, const std::vector<uint64_t>& keys);
bool team_embark_decode(const uint8_t* p, size_t n, uint8_t& seq, std::vector<uint64_t>& keys);

// ... ou l'equipement d'un invite au choix des objets (kTeamGear) : une photo
// numerotee (1, 2... ; 0 = aucune) de ses chats tels que sa copie les a (cle
// dans la save de l'hote -- la meme dans la copie --, chat serialise) et de son
// rangement (le bloc inventory_storage). `final` : il ne changera plus rien.
// Sans `has_data`, seulement « finie, rien de neuf depuis la photo `snap` ».
struct GearCat {
    uint64_t             key = 0;
    std::vector<uint8_t> data;
};
struct GearMsg {
    uint8_t              vote     = 0;
    uint32_t             snap     = 0;
    bool                 final    = false;
    bool                 has_data = false;
    std::vector<uint8_t> storage;
    std::vector<GearCat> cats;
};
std::vector<uint8_t> team_gear_encode(const GearMsg& m);
bool team_gear_decode(const uint8_t* p, size_t n, GearMsg& m);
// ... et la reponse de l'hote (kTeamGearAck) : la photo qu'il a appliquee a
// ses chats dans l'aventure (0 : aucune, ils restent tels qu'au depart), avec
// le rangement de cette photo -- celui que l'invite retrouvera chez lui.
struct GearAck {
    uint8_t              vote        = 0;
    uint32_t             snap        = 0;
    bool                 has_storage = false;
    std::vector<uint8_t> storage;
};
std::vector<uint8_t> team_gear_ack_encode(const GearAck& a);
bool team_gear_ack_decode(const uint8_t* p, size_t n, GearAck& a);

// ... ou, pendant l'aventure, ce qu'un invite a change en equipant ses chats
// (kTeamRunGear) : sa photo numerotee, la revision de l'aventure de l'hote sur
// laquelle il l'a faite (MM_RUNREV), le sac et la poubelle de l'aventure (blocs
// inventory_backpack et inventory_trash, tels que le jeu les ecrit) et ses
// chats (cle, chat serialise).
struct RunGearMsg {
    uint32_t             seq  = 0;
    uint32_t             base = 0;
    std::vector<uint8_t> backpack;
    std::vector<uint8_t> trash;
    std::vector<GearCat> cats;
};
std::vector<uint8_t> team_rungear_encode(const RunGearMsg& m);
bool team_rungear_decode(const uint8_t* p, size_t n, RunGearMsg& m);
// ... et la reponse de l'hote (kTeamRunGearAck) : cette photo prise ou non, et
// la revision de son aventure apres.
enum : uint8_t {
    kRunGearApplied = 1,    // prise : l'aventure de l'hote est celle de la photo
    kRunGearStale   = 2,    // refusee : l'aventure a change depuis la revision de la photo
    kRunGearInvalid = 3,    // refusee : un chat qui n'est pas a lui, illisible...
    kRunGearNode    = 4,    // refusee : l'hote est entre dans une case avant de la prendre
};
struct RunGearAck {
    uint32_t seq    = 0;
    uint8_t  result = 0;
    uint32_t rev    = 0;
};
std::vector<uint8_t> team_rungear_ack_encode(const RunGearAck& a);
bool team_rungear_ack_decode(const uint8_t* p, size_t n, RunGearAck& a);

// ... ou, en aventure (protocole 44), un mouvement d'objet (kTeamItemOp), fait
// par `origin` -- l'invite qui l'a fait, ou l'hote -- et numerote par lui (seq) :
// equiper un objet du sac (son empreinte `item`, l'emplacement `slot` ou il va,
// l'empreinte `prev` de ce que l'emplacement tenait, 0 : rien), rendre au sac
// l'objet porte a `slot` (empreinte `item`), ou un livre lu (le chat tel qu'il
// est apres, serialise par le jeu, et l'empreinte du livre retire du sac, 0 :
// aucun). Le chat : sa cle dans l'aventure (MewDirector+1472) et son id
// (CatData+0x00). `name` : le nom GON de l'objet, pour le journal.
enum : uint8_t { kItemEquip = 1, kItemUnequip = 2, kItemLearn = 3 };
constexpr uint8_t kItemSlots = 5;
struct ItemOp {
    uint32_t             seq     = 0;
    uint8_t              origin  = 0;
    uint8_t              op      = 0;
    uint8_t              slot    = 0;
    uint64_t             cat_key = 0;
    uint64_t             cat_id  = 0;
    uint64_t             item    = 0;
    uint64_t             prev    = 0;
    std::string          name;
    std::vector<uint8_t> cat;
    std::string          detail;     // chez qui le fait, pour son journal : jamais sur le fil
};
std::vector<uint8_t> team_item_op_encode(const ItemOp& m);
bool team_item_op_decode(const uint8_t* p, size_t n, ItemOp& m);
// ... la reponse de l'hote a l'invite qui l'a fait (kTeamItemAck).
enum : uint8_t {
    kItemOk       = 1,   // pris, et parti chez les autres
    kItemConflict = 2,   // refuse : l'objet n'est plus la (un autre l'a pris avant)
    kItemInvalid  = 3,   // refuse : pas son chat, illisible, refuse par le jeu
    kItemNode     = 4,   // refuse : l'hote est dans une case (sa poussee d'entree le remet)
};
struct ItemAck {
    uint32_t seq    = 0;
    uint8_t  result = 0;
};
std::vector<uint8_t> team_item_ack_encode(const ItemAck& a);
bool team_item_ack_decode(const uint8_t* p, size_t n, ItemAck& a);
// ... la demande d'un invite qui doit repartir juste (kTeamItemSyncReq) : ses
// mouvements jusqu'a `seq` sont partis avant elle -- l'hote les a traites quand il
// la lit ; et la reponse (kTeamItemSync) : seq, la revision de l'hote (base), le
// sac, la poubelle et les chats de l'aventure.
std::vector<uint8_t> team_item_sync_req_encode(uint32_t seq);
bool team_item_sync_req_decode(const uint8_t* p, size_t n, uint32_t& seq);
std::vector<uint8_t> team_item_sync_encode(const RunGearMsg& m);
bool team_item_sync_decode(const uint8_t* p, size_t n, RunGearMsg& m);

// L'empreinte d'un objet d'un joueur a l'autre (protocole 46) : ce qu'est l'objet,
// pas ce que ce joueur en a vu. Les champs que le jeu sauve (serialiseur
// 0x14022D2A0, version 5), SAUF :
//   - son id (Equipment+0x00) : un compteur propre a chaque processus ;
//   - d (+0x54) : sa place a l'ecran, 1000 + son rang quand l'ecran des objets
//     du sac se ferme (0x14034D9B0 -> Inventory::set_order 0x1402E2380), 4000000
//     + son rang a la fin d'une aventure (0x1402E2460) ;
//   - les bits 0x2 et 0x4 de c (+0x50) : « nouvel objet », poses a la creation ;
//     0x2 s'efface quand la souris passe sur l'objet a l'ecran des objets
//     (InventoryItemBox::update, 0x14034EA08), les deux au depart en aventure
//     (0x1403B1143).
// Chacun change chez un joueur sans changer chez l'autre : avec eux, un objet
// pris a la souris n'avait plus l'empreinte de sa copie chez l'autre (« objet
// absent du sac »). f (+0x5C, l'usure : 0, 1, 3, 5 -- 0x1402DF420) compte pour
// l'octet que la save en garde (le serialiseur l'y ramene). Jamais 0.
struct ItemFields {
    std::string name;            // Equipment+0x08, la cle GON
    std::string text;            // +0x28 (un livre : la capacite qu'il apprend)
    uint32_t    a = 0, b = 0, c = 0, d = 0;   // +0x48 (charges, -1 sans fin), +0x4C, +0x50 (drapeaux), +0x54 (place)
    uint8_t     e = 0;           // +0x58
    uint32_t    f = 0;           // +0x5C (usure)
};
constexpr uint32_t kItemSeenBits = 0x6;   // c : « nouvel objet » (voir plus haut)
uint64_t item_digest(const ItemFields& it);

// Le butin (mm_loot) : un objet tel que le serialiseur du jeu l'ecrit dans un
// seau de l'inventaire (version 5, sans table de chaines), et des objets ajoutes
// au rangement d'une save (files, cle inventory_storage : u32 nombre, puis les
// objets) : 1 ajoutes, 0 rangement illisible (rien ecrit), -1 erreur sqlite.
std::vector<uint8_t> team_item_record(const ItemFields& f);
int team_apply_loot(sqlite3* own, const std::vector<ItemFields>& items, uint32_t* count_before = nullptr,
                    uint32_t* count_after = nullptr);
// Un objet ecrit par team_item_record, a partir de `pos` (avance) : faux s'il
// n'en est pas un (version 5, present, chaines raisonnables).
bool team_item_parse(const uint8_t* p, size_t n, size_t& pos, ItemFields& out);

// Ce qu'un invite attend d'ajouter a une de ses saves -- ses bourses (mm_purse)
// et son butin (mm_loot) --, garde dans un fichier a cote d'elle
// (« <save>.mgmp-attente », mm_team) : rien n'est perdu s'il ferme le jeu avant
// de la recharger. u32 « MATT », u8 version (1), u32 n, n x { u8 vote, u64 cle,
// u16 n + nom affiche, u32 n + objet (team_item_record) }, u32 m, m x { u8 vote,
// i32 pieces, i32 nourriture }.
struct PendingLoot {
    uint8_t     seq = 0;
    uint64_t    key = 0;             // le defile et l'objet
    ItemFields  item;
    std::string name;
};
struct PendingPayout {
    uint8_t seq   = 0;
    int32_t coins = 0;
    int32_t food  = 0;
};
// Version 2 (protocole 49) : puis u32 c, c x { u8 vote, i64 jour de l'hote a la
// fin, i64 naissance, i64 mort, u32 n + le chat serialise } -- les recrues de
// l'aventure qui lui reviennent (team_add_new_cats). Ecrit en version 1 sans
// recrue.
struct PendingNewCat {
    uint8_t              seq      = 0;
    int64_t              host_day = 0;    // pour recaler ses jours (cat_shift_days)
    int64_t              born     = 0;    // ... tels que la serialisation les porte
    int64_t              died     = -1;
    std::vector<uint8_t> cat;             // serialise, non compresse
};
std::vector<uint8_t> team_pending_encode(const std::vector<PendingLoot>& loot, const std::vector<PendingPayout>& pay,
                                         const std::vector<PendingNewCat>* cats = nullptr);
bool team_pending_decode(const uint8_t* p, size_t n, std::vector<PendingLoot>& loot, std::vector<PendingPayout>& pay,
                         std::vector<PendingNewCat>* cats = nullptr);

// Une recrue (kTeamRecruit) : le vote du depart de l'aventure, son id
// (CatData+0x00), sa cle chez l'hote, le joueur a qui elle va et son nom (UTF-8,
// pour la fenetre de chacun).
struct RecruitNote {
    uint8_t     seq      = 0;
    uint64_t    cat_id   = 0;
    uint64_t    host_key = 0;
    uint8_t     owner    = 0;
    std::string name;
};
std::vector<uint8_t> team_recruit_encode(const RecruitNote& n);
bool team_recruit_decode(const uint8_t* p, size_t n, RecruitNote& out);

// Les recrues qui rentrent chez un invite (kTeamNewCats) : le vote du depart, le
// jour de l'hote a la fin, puis pour chacune sa cle chez l'hote, ses jours de
// naissance et de mort tels que la serialisation les porte (pour les recaler
// sur le calendrier de l'invite : cat_shift_days) et le chat serialise (sans
// liens : ils seraient des chats de l'hote).
struct NewCatEntry {
    uint64_t             host_key = 0;
    int64_t              born     = 0;
    int64_t              died     = -1;
    std::vector<uint8_t> data;
};
struct NewCatsMsg {
    uint8_t                  seq      = 0;
    int64_t                  host_day = 0;
    std::vector<NewCatEntry> cats;
};
std::vector<uint8_t> team_newcats_encode(const NewCatsMsg& m);
bool team_newcats_decode(const uint8_t* p, size_t n, NewCatsMsg& out);

// Recale les jours d'un chat serialise sur un autre calendrier : naissance et
// mort (mort = -1 : vivant, reste -1) moins `shift`. Ils sont ecrits l'un apres
// l'autre (CatData+0xC38 puis +0xC40) apres une partie de longueur variable, et
// suivis d'au moins 79 octets (0x14022FE35..0x14022FFD9) : on cherche, apres les
// drapeaux, la seule place ou ils sont avec leurs valeurs connues. Faux (rien
// n'est change) s'ils n'y sont pas exactement une fois.
bool cat_shift_days(std::vector<uint8_t>& raw, int64_t born, int64_t died, int64_t shift);

// Le genre d'un message MSG_TEAM, 0 si ce n'en est pas un.
uint8_t team_kind(const uint8_t* p, size_t n);

// Le rangement d'une save (table files, cle inventory_storage) : 1 lu, 0 pas
// de ligne (`out` vide), -1 erreur.
int team_read_storage(sqlite3* db, std::vector<uint8_t>& out);

// Un chat serialise : son id, son nom (UTF-8), ses drapeaux et leur position.
struct CatInfo {
    uint64_t    id       = 0;
    std::string name;
    uint64_t    flags    = 0;
    size_t      flags_at = 0;
};
bool cat_info(const std::vector<uint8_t>& raw, CatInfo& out);
bool cat_set_flags(std::vector<uint8_t>& raw, uint64_t flags);

// Le bloc d'un chat dans la table cats. L'ecriture fait un bloc LZ4 de
// litteraux seulement (valide pour tout decodeur LZ4) ; la lecture decode
// n'importe quel bloc.
std::vector<uint8_t> cat_blob_pack(const std::vector<uint8_t>& raw);
bool cat_blob_unpack(const uint8_t* p, size_t n, std::vector<uint8_t>& raw);

// house_state : les cles des chats de la maison ; en retirer (renvoie le nombre
// retire, -1 si le format n'est pas celui attendu -- le bloc reste alors tel quel).
bool house_state_keys(const std::vector<uint8_t>& hs, std::vector<uint64_t>& keys);
int  house_state_remove(std::vector<uint8_t>& hs, const std::vector<uint64_t>& keys);
// Ajoute un chat (cle `key`) a la maison : a la piece et a la place du premier
// chat qui y est (les chats s'y promenent ensuite), sinon dans Floor1_Large ; sa
// troisieme valeur (un entier par chat, House+0xF8) est la plus grande + 1. Faux
// si le format n'est pas celui attendu (le bloc reste tel quel).
bool house_state_add(std::vector<uint8_t>& hs, uint64_t key);

// Hote, avant d'ecrire les chats de ses invites dans sa save (une copie en
// memoire, lue seulement) : la cle de chacun, dans l'ordre de `cats`.
//   - Un chat deja venu dans une aventure de cette save y a laisse sa ligne,
//     marquee kCatGuest : il la reprend (reuse) -- la plus haute s'il y en a
//     plusieurs ; une ligne du meme id sans la marque (un clone reste chez
//     l'hote) n'empeche alors rien.
//   - Les autres prennent des cles qui se suivent, a partir de la plus grande
//     de la table + 1 et jamais sous `min_key` (le compteur du jeu + 1 : ses
//     prochains chats ne doivent pas les rencontrer).
//   - Un chat est refuse (refused, et pourquoi) s'il n'a pas le format attendu,
//     s'il est deux fois dans la liste, ou si la save de l'hote l'a deja pour
//     elle seulement (meme id, sans aucune ligne marquee : une save copiee de
//     l'autre) -- le meme chat deux fois dans une aventure, le jeu ne le
//     prevoit pas. Il reste chez lui ; les autres partent.
// Faux (et `why`) seulement si la table ne se lit pas.
struct KeyPlan {
    uint64_t    key     = 0;
    bool        reuse   = false;
    bool        refused = false;
    std::string why;
};
bool team_plan_keys(sqlite3* host, const std::vector<TeamCat>& cats, uint64_t min_key,
                    std::vector<KeyPlan>& plan, std::string& why);

// Invite : recopie dans sa save l'etat final de ses chats, en une transaction.
// Un chat n'est recopie que si la save a, a sa cle, le meme chat (meme id), et
// vivant (deja mort, il le reste : compte dans `changed`) ; ce qui est ecrit
// perd « en aventure » et la marque kCatGuest. Un chat mort, ou pas revenu
// (kFateGone), quitte aussi house_state.
// `departed` (facultatif) : les chats tels qu'ils etaient au depart (cle et
// octets). Donne quand l'etat final arrive en retard -- la save a pu etre jouee
// depuis : un chat dont la ligne a change depuis le depart n'est pas touche.
// `storage` (facultatif) : le rangement de l'invite au depart et ce qu'il en
// restait apres le choix des objets (mm_gear). `post` remplace celui de la save,
// dans la meme transaction, si chacun des chats partis (`cats`, leurs cles chez
// lui) est recopie -- sinon les objets partis avec un chat laisse tel quel, ou
// absent de `results`, seraient perdus -- et si la save a encore le rangement du
// depart : `pre_disk` (lu dans le fichier) ou `pre_mem` (tel que le jeu l'avait
// en memoire, pas encore ecrit). Pas de ligne au depart (`pre_disk_found` faux)
// ni maintenant : la save n'avait pas de rangement.
// Faux = rien n'est change.
struct TeamStorageChange {
    std::vector<uint64_t> cats;
    std::vector<uint8_t>  pre_disk;
    bool                  pre_disk_found = false;
    std::vector<uint8_t>  pre_mem;
    std::vector<uint8_t>  post;
};
enum TeamStorageResult : int {
    kStorageNone      = 0,    // rien a recopier (pas de `storage`)
    kStorageWritten   = 1,
    kStorageSame      = 2,    // la save a deja `post`
    kStorageChanged   = 3,    // le rangement de la save a change depuis le depart
    kStorageCatsKept  = 4,    // des chats ne sont pas recopies : le rangement non plus
};
struct TeamApplySummary {
    std::vector<std::string> back;      // rentres
    std::vector<std::string> lost;      // morts
    std::vector<std::string> gone;      // perdus en route
    std::vector<std::string> changed;   // joues depuis le depart : laisses tels quels
    int                      skipped = 0;
    int                      storage = kStorageNone;
};
bool team_apply_results(sqlite3* own, const std::vector<TeamCat>& results,
                        const std::vector<TeamCat>* departed, TeamApplySummary& sum,
                        const TeamStorageChange* storage = nullptr);

// Invite : ajoute ce qu'il rapporte d'une aventure (sa bourse, mm_purse) a l'or
// et a la nourriture de sa maison -- les lignes house_gold et house_food de la
// table properties --, en une transaction. Une ligne absente, ou qui n'est pas un
// entier, vaut ce que le jeu en lit alors (0 pour l'or, 25 pour la nourriture :
// get_int a 0x14020684E et 0x1402067FC) ; elle est ecrite en entier. Les
// quantites negatives comptent pour zero ; le total reste dans un int32
// (House+0xB4 et +0xB0). Faux = rien n'est change (erreur sqlite).
struct TeamPurseApply {
    int64_t gold_before = 0, gold_after = 0;
    int64_t food_before = 0, food_after = 0;
};
bool team_apply_purse(sqlite3* own, int64_t coins, int64_t food, TeamPurseApply* out = nullptr);

// Invite : les chats recrutes en route qui lui reviennent (le resultat d'une
// aventure, own_key 0), ajoutes a sa save en une transaction : chacun une cle
// neuve (au-dessus de la plus grande de la table cats et du compteur du jeu, la
// propriete catid_counter, que Registry::Load compare), sa ligne dans cats (sans
// « en aventure » ni la marque kCatGuest) et sa place dans la maison
// (house_state_add ; sans house_state, il est compte dans `unplaced`). Un chat
// dont l'id est deja dans la save n'est pas ajoute une deuxieme fois (`already`).
// Faux = rien n'est change.
struct TeamNewCats {
    std::vector<uint64_t>    keys;          // dans l'ordre de `cats` ; 0 : pas ajoute
    std::vector<std::string> added;
    std::vector<std::string> already;
    int                      skipped  = 0;  // format inattendu
    int                      unplaced = 0;  // ajoute, mais pas dans house_state
};
bool team_add_new_cats(sqlite3* own, const std::vector<TeamCat>& cats, TeamNewCats& out);

} // namespace mgmp
