#pragma once
// mm_purse_core -- Mewgenics Multi : les bourses de l'aventure, le calcul seul
// (sans le jeu), pour les tests. Voir mm_purse.h pour la regle et le jeu.
//
// Une bourse par joueur, en pieces et en nourriture. Ce qu'un evenement donne
// (ou coute) a un chat, ou ce qu'un chat ramasse en combat, va a la bourse de son
// proprietaire ; le reste (provisions du depart, recompenses des combats,
// boutique, boites) est partage a parts egales entre les joueurs qui ont amene
// des chats. La somme des bourses est toujours le compte
// de l'aventure (adventure_coins, adventure_food). Une bourse peut passer sous
// zero (une depense de l'equipe, ou une perte de son chat, plus grosse qu'elle) :
// c'est une dette, que les trouvailles suivantes remboursent ; a la fin, une
// bourse negative ne rapporte rien.

#include <cstdint>
#include <string>

namespace mgmp {

struct MmMsg;

constexpr uint8_t kPurseSlots = 8;

struct PurseEntry {
    int64_t coins = 0;
    int64_t food  = 0;
};

// Ce que chacun rapporte a la fin d'une aventure gagnee.
struct PursePayout {
    int32_t coins[kPurseSlots] = {};
    int32_t food[kPurseSlots]  = {};
    int32_t host_coins = 0;     // ce qui reste pour l'hote (le jeu le met dans sa maison)
    int32_t host_food  = 0;
};

class PurseBook {
public:
    void reset();

    // Les joueurs qui partagent (un bit par numero de joueur ; aucun : l'hote
    // seul). L'hote recoit ce qui va a un numero hors limites, et le reste a la
    // fin, meme s'il ne partage pas.
    void    set_players(uint32_t mask, uint8_t host);
    uint32_t players() const { return players_; }
    uint8_t  host() const { return host_; }

    // Un changement du compte de l'aventure, a la bourse de `owner` (un numero
    // hors limites : l'hote).
    void credit(uint8_t owner, int64_t dcoins, int64_t dfood);
    // ... partage a parts egales entre les joueurs. Les unites qui restent de la
    // division vont une a une : un gain a la bourse la plus maigre, une perte a
    // la plus garnie (a egalite, l'hote, puis les numeros dans l'ordre).
    void share(int64_t dcoins, int64_t dfood);

    const PurseEntry& at(uint8_t p) const { return e_[p < kPurseSlots ? p : 0]; }
    int64_t total_coins() const;
    int64_t total_food() const;

    // La fin : chaque invite de `paid` (hors hote) recoit sa bourse si elle est
    // positive, dans la limite de ce que l'aventure contient encore (run_coins,
    // run_food), dans l'ordre des numeros ; l'hote garde le reste.
    PursePayout settle(uint32_t paid, int64_t run_coins, int64_t run_food) const;

private:
    void split(bool coins, int64_t d);
    PurseEntry e_[kPurseSlots];
    uint32_t   players_ = 1;
    uint8_t    host_    = 0;
};

// --- le message MM_PURSE ----------------------------------------------------------

enum : uint8_t {
    kPurseLedger = 1,    // les bourses : c lignes ; x/y = le compte de l'aventure ; b = le depart
    kPurseFind   = 2,    // un chat a trouve : b = son joueur, c = kFindSpent / kFindBattle,
                         //   x/y = pieces/nourriture, texte = son nom (vide en combat)
    kPursePayout = 3,    // la fin : c lignes (ce que rapporte chaque invite) ; x/y = la part de l'hote
};

// MM_PURSE a = 2, champ c.
constexpr uint8_t kFindSpent  = 0x01;   // le prix d'un choix de l'evenement (« a dépensé »)
constexpr uint8_t kFindBattle = 0x02;   // ramasse en combat par les chats de ce joueur (sans nom)

struct PurseLine {
    uint8_t peer  = 0;
    int32_t coins = 0;
    int32_t food  = 0;
};
constexpr uint8_t kPurseMaxLines = 7;   // 9 octets par ligne dans les 63 du texte

void    purse_encode_lines(MmMsg& m, uint8_t a, uint8_t seq, const PurseLine* lines, uint8_t n,
                           int32_t x, int32_t y);
// Le nombre de lignes lues (0 : message illisible).
uint8_t purse_decode_lines(const MmMsg& m, PurseLine* out, uint8_t max);
void    purse_encode_find(MmMsg& m, uint8_t owner, int32_t coins, int32_t food, const std::string& cat,
                          uint8_t flags = 0);
bool    purse_decode_find(const MmMsg& m, uint8_t& owner, int32_t& coins, int32_t& food, std::string& cat,
                          uint8_t* flags = nullptr);

// « 12 pièces et 3 nourritures », « 1 pièce », « rien » (UTF-8, les mots du jeu :
// WORLDEVENT_GAIN_FOOD dit « nourritures »). Les quantites negatives comptent
// pour zero.
std::string purse_amount_text(int64_t coins, int64_t food);

// Comme purse_amount_text, pour une bourse qui peut etre negative (une dette) :
// « -11 pièces et 13 nourritures » (un tiret : la police du jeu).
std::string purse_balance_text(int64_t coins, int64_t food);

// « de Bob », « d'Ami », « d'Élise » : la bourse d'un joueur, avec l'elision
// devant une voyelle (accentuee ou non), un h, ou un y suivi d'une consonne
// (« d'Yves », mais « de Yannick »).
std::string purse_of(const std::string& name);

// Un nom (joueur, chat) pret a aller dans une fenetre du jeu : ni crochet (les
// balises du texte, [c:red]...) ni retour a la ligne.
std::string purse_safe(const std::string& name);

// Une ligne de la fenetre des trouvailles (UTF-8) :
//   « Frédéric a trouvé 15 pièces : c'est pour ta bourse. »
//   « Frédéric a perdu 5 pièces : c'est pris sur la bourse d'Ami. »
//   « Frédéric a dépensé 10 pièces : c'est pris sur ta bourse. » (`spent`)
//   « Frédéric a trouvé 3 pièces et perdu 2 nourritures : ça compte pour ta bourse. »
// `mine` : la bourse de ce joueur ; sinon celle de `owner` (son nom).
std::string purse_find_line(const std::string& cat, int64_t coins, int64_t food, bool mine,
                            const std::string& owner, bool spent = false);

// Ce que les chats d'un joueur ont ramasse en combat (UTF-8) :
//   « En combat, tes chats ont trouvé 12 pièces : c'est pour ta bourse. »
//   « En combat, les chats d'Ami ont trouvé 3 nourritures : c'est pour sa bourse. »
std::string purse_battle_line(int64_t coins, int64_t food, bool mine, const std::string& owner);

} // namespace mgmp
