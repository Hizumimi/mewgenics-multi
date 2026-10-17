#pragma once
// mm_loot_core -- Mewgenics Multi : le defile du butin, la partie portable
// (bibliotheque standard seulement, verifiee par tests/test_loot.cpp).
//
// LES REGLES (document de conception, « Butin et or » ; decide le 17 octobre)
//   - A la victoire d'une aventure mixte, les objets du sac que le jeu range dans
//     la maison de l'hote passent un par un. Un objet equipe rentre avec son chat
//     (il n'est pas dans le sac).
//   - Chaque joueur de l'aventure (l'hote, et chaque invite qui y a amene des
//     chats) choisit : Passer (je n'en veux pas), Cupidite (je le prends si
//     personne n'en a besoin) ou Besoin (je le veux).
//   - Un de de 100 entre ceux qui ont choisi Besoin ; sans Besoin, entre ceux qui
//     ont choisi Cupidite ; le plus haut l'emporte, une egalite en tete est
//     relancee entre les ex aequo (on montre le premier lancer et le nombre de
//     relances). Personne : l'objet reste a l'hote. Les des sont lances
//     par l'hote, hors du flux aleatoire du jeu (qui doit rester le meme chez
//     tous).
//   - 30 s par objet ; sans choix a la fin, le joueur passe. Le resultat tombe des
//     que tous les joueurs prets ont choisi ; il reste 4 s a l'ecran, puis l'objet
//     suivant.
//   - Un joueur pas encore pret (sa fenetre du defile n'est pas ouverte : un
//     chargement, une autre fenetre) n'est pas attendu pour un objet : il passe,
//     et entre dans le defile des qu'il est pret. Le premier objet attend que tous
//     soient prets, 3 minutes au plus apres l'hote (qui peut commencer avant), et
//     2,5 s au moins : chacun lit la fenetre d'attente.
//     Un joueur qui quitte la session passe. Quand l'hote ne voit pas le defile,
//     le temps s'arrete pour tous.
//
// LE MESSAGE (MSG_TEAM kTeamLoot, voir mm_team_merge) : la tete MSG_TEAM, puis
// u8 op, u32 numero du defile, puis selon op :
//   kLootStart  hote -> invites : u8 vote du depart de l'aventure, u32 joueurs (un
//               bit par numero de joueur), u8 nombre d'objets (0 : pas de defile),
//               puis chaque objet : nom, texte (u16 n + octets), a, b, c, d (u32),
//               e, f (u8).
//   kLootReady  invite -> hote : u8 1 il voit le defile, 0 il ne le voit plus.
//   kLootChoose invite -> hote : u8 objet, u8 choix (LootChoice).
//   kLootState  hote -> invites : u8 objet, u8 phase (LootPhase), u32 ms restantes,
//               u32 joueurs, u32 prets, u32 ont choisi cet objet, u8 decide,
//               u8 gagnant (kLootNobody : personne, il reste a l'hote), u8 choix
//               gagnant, u8 relances, u8 n, n x { u8 joueur, u8 choix, u8 de }
//               (de 0 : pas lance), puis u8 m, m x u8 le gagnant de chaque objet
//               (kLootUndecided : pas encore).
//   kLootGive   hote -> gagnant : u8 objet (il le prend dans son rangement).
//   kLootGot    gagnant -> hote : u8 objet, u8 1 pris / 0 pas pu (il reste a l'hote).

#include "mm_team_merge.h"   // ItemFields, kTeamMagic, kTeamFormat, kTeamLoot

#include <cstdint>
#include <string>
#include <vector>

namespace mgmp {

constexpr int      kLootSlots       = 8;        // numeros des joueurs (0 = l'hote)
constexpr uint32_t kLootChoiceMs    = 30000;    // par objet
constexpr uint32_t kLootResultMs    = 4000;     // le resultat a l'ecran
constexpr uint32_t kLootReadyWaitMs = 180000;   // au plus, pour que tous soient prets
constexpr uint32_t kLootIntroMs     = 2500;     // au moins, la fenetre d'attente (tous prets d'un coup)
constexpr size_t   kLootMaxItems    = 64;
constexpr uint8_t  kLootNobody      = 0xFF;
constexpr uint8_t  kLootUndecided   = 0xFE;
constexpr int      kLootMaxRerolls  = 20;

enum LootChoice : uint8_t { kLootNone = 0, kLootPass = 1, kLootGreed = 2, kLootNeed = 3 };
enum class LootPhase : uint8_t { Idle = 0, Waiting = 1, Choosing = 2, Result = 3, Done = 4 };
enum LootOp : uint8_t { kLootStart = 1, kLootReady = 2, kLootChoose = 3, kLootState = 4, kLootGive = 5, kLootGot = 6 };

struct LootRoll {
    uint8_t peer   = 0;
    uint8_t choice = kLootNone;
    uint8_t roll   = 0;           // le premier lancer (0 : pas lance)
};

// Le resultat d'un objet.
struct LootResult {
    bool     decided  = false;
    uint8_t  winner   = kLootNobody;   // personne : il reste a l'hote
    uint8_t  category = kLootNone;     // Besoin ou Cupidite (kLootNone : personne)
    uint8_t  rerolls  = 0;             // relances entre ex aequo en tete (non montrees)
    std::vector<LootRoll> rolls;       // chaque joueur : son choix, son de
};

// Ce que l'hote publie, et ce que chaque invite en garde.
struct LootState {
    uint32_t  parade   = 0;
    uint8_t   index    = 0;
    LootPhase phase    = LootPhase::Idle;
    uint32_t  ms_left  = 0;
    uint32_t  players  = 0;
    uint32_t  ready    = 0;
    uint32_t  chosen   = 0;
    LootResult result;                 // en phase Result (et Done : le dernier)
    std::vector<uint8_t> winners;      // chaque objet : son gagnant, ou kLootUndecided
};

struct LootMsg {
    uint8_t  op     = 0;
    uint32_t parade = 0;
    // kLootStart
    uint8_t  seq     = 0;
    uint32_t players = 0;
    std::vector<ItemFields> items;
    // kLootReady
    uint8_t  ready  = 0;
    // kLootChoose, kLootGive, kLootGot
    uint8_t  index  = 0;
    uint8_t  choice = kLootNone;
    uint8_t  ok     = 0;
    // kLootState
    LootState state;
};

// Le message complet (tete MSG_TEAM comprise). Faux : illisible ou pas un kTeamLoot.
std::vector<uint8_t> loot_encode(const LootMsg& m);
bool loot_decode(const uint8_t* p, size_t n, LootMsg& m);

// Les des de l'hote : un generateur a lui.
struct LootRng {
    uint64_t s;
    explicit LootRng(uint64_t seed) : s(seed ? seed : 0x9E3779B97F4A7C15ull) {}
    uint32_t next();
    uint8_t  d100();                   // 1..100
};

// Le gagnant d'un objet : choix[i] le choix du joueur i (kLootNone : pas de
// choix, il passe), seulement pour les joueurs de `players`.
LootResult loot_resolve(const uint8_t choices[kLootSlots], uint32_t players, LootRng& rng);

// L'hote : le defile. Le temps est en millisecondes (GetTickCount64).
class LootHost {
public:
    void start(uint32_t parade, uint8_t seq, std::vector<ItemFields> items, uint32_t players, uint8_t host,
               uint64_t now);
    void stop();
    bool running() const { return phase_ != LootPhase::Idle && phase_ != LootPhase::Done; }
    bool finished() const { return phase_ == LootPhase::Done; }
    LootPhase phase() const { return phase_; }
    uint8_t index() const { return index_; }
    // Un joueur peut (ou ne peut plus) voir le defile.
    void set_ready(uint8_t peer, bool ready, uint64_t now);
    // L'hote commence sans attendre les autres (le premier objet) : vrai si
    // c'etait l'attente.
    bool start_now(uint64_t now);
    // Son choix pour cet objet : vrai si pris (objet en cours, joueur du defile,
    // pas deja choisi).
    bool choose(uint8_t peer, uint8_t index, uint8_t choice);
    // Il quitte la session : il passe, et n'est plus attendu.
    void gone(uint8_t peer);
    // Avance ; vrai si l'etat a change (a publier).
    bool tick(uint64_t now, LootRng& rng);
    LootState state(uint64_t now) const;
    uint32_t parade() const { return parade_; }
    uint8_t  seq() const { return seq_; }
    uint32_t players() const { return players_; }
    uint32_t ready() const { return ready_; }
    const std::vector<ItemFields>& items() const { return items_; }
    const std::vector<LootResult>& results() const { return results_; }
    uint8_t choice_of(uint8_t peer) const { return peer < kLootSlots ? choices_[peer] : (uint8_t)kLootNone; }

private:
    void begin_item(uint64_t now);
    void resolve_item(uint64_t now, LootRng& rng);

    LootPhase phase_    = LootPhase::Idle;
    uint32_t  parade_   = 0;
    uint8_t   seq_      = 0;
    uint8_t   host_     = 0;
    uint32_t  players_  = 0;
    uint32_t  ready_    = 0;
    uint8_t   index_    = 0;
    uint64_t  since_    = 0;      // Waiting : l'hote pret depuis ; Choosing/Result : debut de la phase
    uint64_t  last_     = 0;      // le dernier tick (le temps s'arrete quand l'hote ne voit pas le defile)
    bool      host_ready_seen = false;
    uint8_t   choices_[kLootSlots] = {};
    std::vector<ItemFields> items_;
    std::vector<LootResult> results_;
};

// --- les fenetres -------------------------------------------------------------------------
//
// Une seule fenetre du jeu a la fois, a deux boutons au plus : le choix se fait
// en deux temps (« Je le veux » / « Passer », puis « Besoin » / « Cupidité »).

enum LootLayout : uint8_t {
    kLayNone      = 0,
    kLayWait      = 1,   // le defile attend ses joueurs ; hote : « Commencer », invite : « D'accord »
    kLayChoose    = 2,   // « Je le veux » / « Passer »
    kLayNeedGreed = 3,   // « Besoin » / « Cupidité »
    kLayChosen    = 4,   // son choix est fait, les autres choisissent : « D'accord »
    kLayResult    = 5,   // « D'accord »
    kLaySummary   = 6,   // la fin du defile : « D'accord »
};

// Ce que voit un joueur.
struct LootView {
    LootState st;                           // ms_left deja ramene a maintenant
    uint8_t   self    = 0;
    uint8_t   host    = 0;
    bool      is_host = false;
    std::vector<std::string> item_names;    // UTF-8 (balises du jeu permises)
    std::vector<std::string> item_descs;
    uint8_t   my_choice = kLootNone;        // pour l'objet en cours (chez un invite : le sien, pas encore su de l'hote)
    bool      step2     = false;            // « Je le veux » appuye pour l'objet en cours
    bool      at_home   = false;            // invite : dans sa propre save, deja ouverte
    // Invite : les objets que l'hote lui a vraiment donnes (kLootGive), un bit par
    // objet. Un objet gagne mais pas donne (l'hote ne l'avait plus) le dit a la fin.
    bool      track_received = false;
    uint64_t  received  = 0;
    std::string (*name)(uint8_t) = nullptr; // le nom d'un joueur
};

struct LootScreen {
    uint8_t     layout = kLayNone;
    std::string text;
    const char* yes = nullptr;              // UTF-8
    const char* no  = nullptr;              // nullptr : un seul bouton
};

// La fenetre de ce joueur (kLayNone : aucune -- il n'est pas du defile).
LootScreen loot_screen(const LootView& v);

// Les textes (UTF-8), ici pour etre testes.
const char* loot_choice_word(uint8_t choice);       // « Besoin », « Cupidité », « Passer »
// « Ami 87, Lou 42 » : les des du resultat, du plus haut au plus bas ; `name`
// donne le nom d'un joueur.
std::string loot_rolls_text(const LootResult& r, std::string (*name)(uint8_t));
// « 25 s », « 2:40 »
std::string loot_time_text(uint32_t ms);
// « A », « A et B », « A, B et C »
std::string loot_join(const std::vector<std::string>& v);

} // namespace mgmp
