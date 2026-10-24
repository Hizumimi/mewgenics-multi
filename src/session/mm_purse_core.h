#pragma once
// mm_purse_core -- Mewgenics Multi : les bourses de l'aventure, le calcul seul
// (sans le jeu), pour les tests. Voir mm_purse.h pour la regle et le jeu.
//
// Une bourse par joueur, en pieces et en nourriture. Ce qu'un evenement donne
// (ou coute) a un chat, ou ce qu'un chat ramasse en combat, va a la bourse de son
// proprietaire, un achat en boutique a celle de l'acheteur (mm_shop) ; le reste
// (provisions du depart, recompenses des combats, boites) est partage a parts
// egales entre les joueurs qui ont amene des chats. La somme des bourses est toujours le compte
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

    // Mewgenics Multi (protocole 62, mm_gift) : un don de la bourse de `from` a
    // celle de `to` -- `coins` pieces et `food` nourritures. Le compte de
    // l'aventure ne change pas. Faux (rien ne change) si un numero est hors
    // limites, si from == to, si un montant est negatif ou les deux nuls, ou si
    // la bourse de `from` n'en a pas autant (on ne donne pas une dette).
    bool transfer(uint8_t from, uint8_t to, int64_t coins, int64_t food);

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
    // Mewgenics Multi (protocole 62, mm_gift) : les dons entre bourses.
    kPurseGiveReq = 4,   // invite -> hote : donner a b (numero de maintenant) x pieces, y nourritures
    kPurseGiven   = 5,   // hote -> tous : b a donne a c x pieces, y nourritures (les bourses suivent)
    kPurseGiveNo  = 6,   // hote -> tous : le don de b n'est pas passe ; c = raison (kPurseNo*)
};

// MM_PURSE a = 6, champ c.
enum : uint8_t {
    kPurseNoPurse  = 1,   // pas de bourse (ce joueur, ou celui a qui il donne)
    kPurseNoEnough = 2,   // sa bourse n'en a pas autant
    kPurseNoRun    = 3,   // plus d'aventure mixte suivie
    kPurseNoLast   = kPurseNoRun,
};
// La raison d'un refus, pour la fenetre de celui qui donnait (UTF-8).
std::string purse_give_no_text(uint8_t why);

// Les dons entre bourses, sur le fil (protocole 62) :
//   a = 4  invite -> hote : b = a qui (numero de maintenant), c = numero de la
//          demande, x/y = pieces/nourriture (0 a kPurseGiveMax, pas les deux nuls) ;
//   a = 5  hote -> tous : b = qui donne, c = a qui, x/y, texte[0] = le numero de
//          sa demande (0 : un don de l'hote) ;
//   a = 6  hote -> tous : b = qui donnait, c = raison (kPurseNo*), x = le numero
//          de sa demande.
// Faux au decodage : un autre message, un numero hors limites, des quantites
// hors des bornes, ou un don a soi-meme.
constexpr int32_t kPurseGiveMax = 1000000;
void purse_encode_give_req(MmMsg& m, uint8_t to, int32_t coins, int32_t food, uint8_t seq);
bool purse_decode_give_req(const MmMsg& m, uint8_t& to, int32_t& coins, int32_t& food, uint8_t& seq);
void purse_encode_given(MmMsg& m, uint8_t giver, uint8_t to, int32_t coins, int32_t food, uint8_t seq);
bool purse_decode_given(const MmMsg& m, uint8_t& giver, uint8_t& to, int32_t& coins, int32_t& food,
                        uint8_t& seq);
void purse_encode_give_no(MmMsg& m, uint8_t giver, uint8_t why, uint8_t seq);
bool purse_decode_give_no(const MmMsg& m, uint8_t& giver, uint8_t& why, uint8_t& seq);

// La nouvelle d'un don entre bourses (UTF-8) :
//   « Ami t'a donné 20 pièces : c'est dans ta bourse. »         (to_me)
//   « Tu as donné 20 pièces à Ami : c'est dans sa bourse. »     (from_me)
//   « Bob a donné 20 pièces à Ami. »                            (les autres)
std::string purse_given_line(bool from_me, bool to_me, const std::string& giver, const std::string& to,
                             int64_t coins, int64_t food);

// MM_PURSE a = 2, champ c.
constexpr uint8_t kFindSpent  = 0x01;   // le prix d'un choix de l'evenement (« a dépensé »)
constexpr uint8_t kFindBattle = 0x02;   // ramasse en combat par les chats de ce joueur (sans nom)
constexpr uint8_t kFindShop   = 0x04;   // un achat en boutique (mm_shop) : texte = ce qui est achete

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
// La regle d'elision de purse_of : vrai pour « d'Ami ». Pour choisir entre deux
// phrases traduites (« ... de %s » / « ... d'%s »), que les autres langues
// traduisent pareil.
bool purse_elide(const std::string& name);

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

// Un achat en boutique (UTF-8) :
//   « Tu as acheté Épée pour 12 pièces : c'est pris sur ta bourse. »
//   « Ami a acheté 5 nourritures pour 8 pièces : c'est pris sur sa bourse. »
// coins/food : ce que l'achat a change a la bourse (le prix en negatif).
std::string purse_shop_line(const std::string& what, int64_t coins, int64_t food, bool mine,
                            const std::string& owner);

} // namespace mgmp
