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

// Le genre d'un message MSG_TEAM, 0 si ce n'en est pas un.
uint8_t team_kind(const uint8_t* p, size_t n);

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
// Faux = rien n'est change.
struct TeamApplySummary {
    std::vector<std::string> back;      // rentres
    std::vector<std::string> lost;      // morts
    std::vector<std::string> gone;      // perdus en route
    std::vector<std::string> changed;   // joues depuis le depart : laisses tels quels
    int                      skipped = 0;
};
bool team_apply_results(sqlite3* own, const std::vector<TeamCat>& results,
                        const std::vector<TeamCat>* departed, TeamApplySummary& sum);

} // namespace mgmp
