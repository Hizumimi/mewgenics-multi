#pragma once
// mm_vote_core -- Mewgenics Multi : le vote du chemin, la partie portable
// (bibliotheque standard seulement, verifiee par tests/test_vote.cpp).
//
// LES REGLES (document de conception, « Votes en aventure »)
//   - Chaque joueur de l'aventure vote pour une case de la carte ; la case qui a
//     le plus de voix l'emporte.
//   - Egalite : un tirage au sort entre les cases a egalite (fait par l'hote,
//     hors du flux aleatoire du jeu, qui doit rester le meme chez tous).
//   - Le premier vote ouvre le tour et lance un compte a rebours de 30 s ; a la
//     fin, un joueur qui n'a pas vote compte comme abstention. Chacun peut
//     changer son vote tant que tout le monde n'a pas vote.
//   - Le compte a rebours n'avance que quand tous les votants sont sur la carte
//     (sortir d'un combat prend plus ou moins de temps a chacun). Un votant
//     absent depuis 60 s d'affilee n'est plus attendu (il s'abstient, sauf s'il revient
//     voter a temps) ; l'hote, qui fait le deplacement, est toujours attendu.
//     Au plus 3 minutes par tour.
//   - Les votants sont ceux de l'aventure a l'ouverture du tour : ceux qui la
//     quittent (ou quittent la session) en sortent, avec leur vote ; un joueur
//     qui arrive pendant le tour n'est pas attendu, mais son clic y compte.
//
// LE MESSAGE (MM_VOTE, champ a = VoteMsgKind)
//   kVoteCast  invite -> hote : b = tour connu (0 aucun), c = type de la case,
//              x = index de la case | (nombre de cases de la carte << 16),
//              y = graine de la case (32 bits du bas : MapNode+0x118).
//   kVoteHere  invite -> hote : c = 1 sur la carte, 0 non (a chaque changement,
//              et toutes les 3 s).
//   kVoteState hote -> tous : b = tour, c = phase, x = ms restantes,
//              y = votants | presents << 8 | absents ecartes << 16 |
//                  compte en cours << 24 | tirage au sort << 25,
//              texte : 8 x u16 case de chaque joueur, 8 x u8 type, u16 case
//              gagnante, u8 son type, u8 nombre de cases a egalite, 4 x u16 et
//              4 x u8 ces cases, puis u8 joueur dont le vote vient d'etre
//              refuse (0xFF aucun) et u8 la raison (VoteRefusal) : 42 octets.

#include <cstdint>
#include <string>

namespace mgmp {

struct MmMsg;

constexpr int      kVoteSlots       = 8;         // ids des joueurs (0 = l'hote)
constexpr uint16_t kVoteNoNode      = 0xFFFF;    // pas de vote, pas de case
constexpr uint32_t kVoteCountdownMs = 30000;     // apres le premier vote
constexpr uint32_t kVoteAbsentMs    = 60000;     // un absent n'est plus attendu
constexpr uint32_t kVoteSafetyMs    = 180000;    // au plus, par tour
// Un pas de temps plus long (une image figee par un chargement) ne compte que
// pour ca : le delai est fait pour les joueurs, pas pour les a-coups du jeu.
constexpr uint32_t kVoteMaxStepMs   = 500;
// Le resultat reste affiche avant que le pion parte ; moins quand tout le monde
// a choisi la meme case (rien a lire).
constexpr uint32_t kVoteResultMs     = 2500;
constexpr uint32_t kVoteResultSameMs = 1000;

enum class VotePhase : uint8_t { Idle = 0, Voting = 1, Result = 2 };

enum VoteMsgKind : uint8_t {
    kVoteCast  = 1,
    kVoteHere  = 2,
    kVoteState = 3,
};

// Pourquoi l'hote n'a pas pris le vote d'un invite (dit une fois, dans l'etat
// publie juste apres).
enum VoteRefusal : uint8_t {
    kRefuseNone     = 0,
    kRefuseOffMap   = 1,   // l'hote n'est pas sur la carte
    kRefuseOtherMap = 2,   // pas la meme carte, pas la meme case
    kRefuseBlocked  = 3,   // case pas cliquable chez l'hote
    kRefuseDecided  = 4,   // le chemin est deja choisi
    kRefuseStale    = 5,   // un vote d'un tour fini
};
constexpr uint8_t kVoteNobody = 0xFF;

struct VoteBallot {
    uint16_t node = kVoteNoNode;   // index dans MapScreen+0x80
    uint8_t  type = 0;             // MapNodeType (MapNode+0x138)
    bool valid() const { return node != kVoteNoNode; }
};

// Ce que l'hote publie, et ce que chaque invite en garde.
struct VoteState {
    uint8_t    round        = 0;   // numero du tour, 1..255 (0 : jamais)
    VotePhase  phase        = VotePhase::Idle;
    uint32_t   voters       = 0;   // masque des joueurs qui votent a ce tour
    uint32_t   present      = 0;   // ... et de ceux qui sont sur la carte
    uint32_t   dropped      = 0;   // absents depuis trop longtemps : plus attendus
    bool       running      = false;   // le compte a rebours avance
    uint32_t   remaining_ms = 0;
    VoteBallot ballot[kVoteSlots];
    // Resultat (phase Result).
    VoteBallot winner;
    bool       drawn        = false;   // egalite tranchee au sort
    uint8_t    tied_count   = 0;
    VoteBallot tied[4];
    // Un vote refuse a l'instant (un seul envoi).
    uint8_t    refused_slot   = kVoteNobody;
    uint8_t    refused_reason = kRefuseNone;
};

// Les joueurs qui ont un vote (parmi les votants).
uint32_t vote_cast_mask(const VoteState& s);

// Le resultat : au moins deux votants, tous pour la case gagnante (sans tirage).
bool vote_unanimous(const VoteState& s);

// Combien de temps le resultat reste affiche : la meme duree chez l'hote (le
// clic rejoue) et chez chaque invite (sa fenetre), d'apres l'etat publie.
uint32_t vote_result_show_ms(const VoteState& s);

// L'hote : le tour en cours et ses regles.
class VoteHost {
public:
    using Rng = uint32_t (*)(void* ctx, uint32_t n);   // entier dans [0, n)

    // Tout oublier (fin d'aventure, session coupee). Le numero du prochain tour
    // continue : un invite ne confond pas un nouveau tour avec l'ancien.
    void reset();

    // Un vote. Ouvre un tour s'il n'y en a pas. `voters` = les joueurs dans
    // l'aventure en ce moment, `slot` doit en etre : ce sont les votants d'un
    // nouveau tour, et un arrivant entre dans le tour en cours par son vote.
    // Refuse (faux) pendant le resultat, ou d'un joueur hors de l'aventure.
    bool cast(uint8_t slot, uint16_t node, uint8_t type, uint32_t voters, uint64_t now);

    // Les votants encore la (connectes, dans l'aventure) : les autres sortent
    // du tour, leur vote avec eux. Vrai = le tour a change.
    bool keep_voters(uint32_t still_here);

    // Qui est sur la carte en ce moment.
    void set_present(uint32_t present);

    // Le joueur que l'absence n'ecarte jamais (l'hote : c'est lui qui deplace
    // le pion).
    void set_never_drop(uint32_t mask) { never_drop_ = mask; }

    // Fait avancer le tour. Vrai = un changement a publier (un absent ecarte,
    // le resultat, un tour abandonne faute de votes).
    bool tick(uint64_t now, Rng rng, void* ctx);

    // Resultat -> plus de tour (le deplacement est fait, ou abandonne) ; ou
    // abandon du tour en cours.
    void finish();

    const VoteState& state() const { return s_; }

private:
    void resolve(Rng rng, void* ctx);

    VoteState s_;
    uint64_t  started_at_ = 0;
    uint64_t  last_tick_  = 0;
    uint32_t  absent_ms_[kVoteSlots] = {};
    uint32_t  never_drop_ = 0;
    uint8_t   next_round_ = 1;
};

// --- le message ---------------------------------------------------------------

struct VoteCast {
    uint8_t  round      = 0;
    uint16_t node       = kVoteNoNode;
    uint16_t node_count = 0;
    uint32_t seed_lo    = 0;
    uint8_t  type       = 0;
};

void vote_encode_cast(const VoteCast& c, MmMsg& m);
bool vote_decode_cast(const MmMsg& m, VoteCast& c);
void vote_encode_here(bool on_map, MmMsg& m);
void vote_encode_state(const VoteState& s, MmMsg& m);
bool vote_decode_state(const MmMsg& m, VoteState& s);

// --- ce que voit chaque joueur ----------------------------------------------------

// La fenetre du vote chez un joueur, d'apres l'etat (sans compter s'il l'a
// fermee ni s'il est sur la carte).
enum class VoteLayout : uint8_t {
    None   = 0,
    Open   = 1,   // il n'a pas vote, et un chemin mene : « Même chemin » / « Choisir »
    Choose = 2,   // il n'a pas vote : « Choisir »
    Voted  = 3,   // il a vote : « Changer »
    Result = 4,   // le resultat : « D'accord »
};

// `own` remplace le vote de `self` dans l'etat (le sien, envoye et pas encore
// revenu de l'hote) quand il est valide.
VoteLayout vote_layout(const VoteState& s, uint8_t self, VoteBallot own = {});

// La case qui a le plus de voix parmi les AUTRES votants, si elle est seule en
// tete ; sinon une case invalide. C'est ce que « Même chemin » choisit.
VoteBallot vote_leader_excluding(const VoteState& s, uint8_t self);

// Le nom d'un type de case (« Combat », « Boutique »...).
const char* vote_type_name(uint8_t type);

// Pourquoi un vote n'a pas ete pris, pour la fenetre (« l'hôte n'est pas sur
// la carte »...).
const char* vote_refusal_text(uint8_t reason);

// Le nom affiche d'une case : le type, et sa position quand plusieurs cases du
// meme type sont proposees (fourni par le jeu).
using VoteLabelFn = std::string (*)(void* ctx, uint16_t node, uint8_t type);

// Le texte de la fenetre. `names` : un nom par joueur (kVoteSlots).
// `remaining_ms` : le compte a rebours a afficher.
std::string vote_text(const VoteState& s, uint8_t self, const std::string* names,
                      uint32_t remaining_ms, VoteLabelFn label, void* ctx,
                      VoteBallot own = {});

} // namespace mgmp
