#pragma once
// mm_shop_core -- Mewgenics Multi : les achats en boutique avec sa bourse, la
// partie portable (bibliotheque standard seulement, verifiee par
// tests/test_shop.cpp). Le jeu et le reseau : mm_shop.h.
//
// LA REGLE (document de conception, « Butin et or » : « En boutique pendant
// l'aventure, chacun paie avec sa bourse, et l'objet achete va a celui qui
// paie »).
//   - Dans une boutique de l'aventure mixte, un invite achete comme seul, mais
//     l'achat se fait chez l'hote (kShopBuy) : l'hote verifie l'objet (sa place
//     dans la boutique, son empreinte, son stock) et la bourse du joueur, puis
//     achete par le jeu ; le prix est pris dans la bourse de l'acheteur, pas
//     partage. L'hote qui achete paie avec la sienne.
//   - Chacun voit l'achat dans sa copie de la boutique (kShopSold) : sa copie le
//     rejoue par le jeu (l'objet passe vendu, le compte baisse, l'objet arrive
//     dans le sac commun).
//   - L'objet achete est a son acheteur : a la fin d'une aventure gagnee, il ne
//     defile pas, il rentre chez lui (kShopGive -> kShopGot ; son joueur parti :
//     il reste a l'hote). Equipe sur un chat, il rentre avec le chat.
//   - Un invite achete des objets, de la nourriture et des pieces (genres 1, 16
//     et 17) ; un bonbon (genre 12) ouvre le choix du chat, un familier (13)
//     change l'aventure de tous : l'hote seul, avec sa bourse.
//
// LE MESSAGE (MSG_TEAM kTeamShop) : la tete MSG_TEAM, puis u8 op, puis selon op :
//   kShopBuy   invite -> hote : u64 la case (sa graine), u16 l'objet (sa place
//              dans la boutique), u64 son empreinte (shop_item_digest), i32 le
//              prix vu.
//   kShopDone  hote -> acheteur : u64 la case, u16 l'objet, u8 resultat
//              (ShopResult), i32 le prix, i32 sa bourse (pieces) maintenant.
//   kShopSold  hote -> invites : u64 la case, u16 l'objet, u64 son empreinte,
//              i32 le prix, u8 l'acheteur (numero du joueur ; kShopNoBuyer :
//              l'hote sans bourse), i32 x 4 le compte de l'aventure avant et
//              apres (pieces, nourriture), u16 n + le nom de ce qui est vendu.
//   kShopGive  hote -> acheteur : u8 le vote du depart de l'aventure, u64 le
//              numero de l'envoi, u8 n, n x { l'objet (nom, texte, a, b, c, d, e,
//              f), u16 n + son nom affiche }.
//   kShopGot   acheteur -> hote : u64 le numero de l'envoi, u64 un bit par objet
//              garde.
//   kShopHere  invite -> hote : u64 la case, u8 1 sa boutique est ouverte / 0
//              fermee (a chaque changement, et toutes les 3 s ouverte) : l'hote
//              ne sort pas d'une boutique ou un invite achete encore sans le
//              confirmer.

#include "mm_team_merge.h"   // ItemFields, kTeamMagic, kTeamFormat, kTeamShop

#include <cstdint>
#include <string>
#include <vector>

namespace mgmp {

enum : uint8_t {
    kShopBuy  = 1,
    kShopDone = 2,
    kShopSold = 3,
    kShopGive = 4,
    kShopGot  = 5,
    kShopHere = 6,
};

constexpr uint8_t kShopNoBuyer    = 0xFF;
constexpr size_t  kShopMaxItems   = 64;     // kShopGive : objets par envoi (kept : un bit chacun)
constexpr size_t  kShopMaxLabel   = 127;    // un nom affiche (UTF-8)
constexpr size_t  kShopMaxName    = 63;     // la cle GON d'un objet ; plus long : il n'est pas envoye
constexpr size_t  kShopMaxText    = 255;    // son texte (un livre : sa capacite) ; idem

// Les genres d'objets de boutique (ShopItem+0x00).
enum : int32_t {
    kShopTypeItem     = 1,
    kShopTypeLevelUp  = 12,   // un bonbon : le choix du chat, puis la montee de niveau
    kShopTypeFamiliar = 13,
    kShopTypeFurniture = 15,
    kShopTypeFood     = 16,
    kShopTypeCoins    = 17,
    kShopTypeCollar   = 18,
};

enum class ShopResult : uint8_t {
    Bought    = 0,
    NoShop    = 1,   // l'hote n'est plus dans la boutique de cette case
    NoItem    = 2,   // pas d'objet a cette place, ou pas le meme
    SoldOut   = 3,
    Poor      = 4,   // la bourse de l'acheteur ne suffit pas
    NotItem   = 5,   // un bonbon, un familier... : pas pour un invite
    Failed    = 6,   // le jeu n'a pas fait l'achat
    NotRun    = 7,   // pas d'aventure mixte (pas de bourses)
    NoPurse   = 8,   // ce joueur n'a pas de bourse dans l'aventure
    Busy      = 9,   // la boutique de l'hote est en pause trop longtemps (une fenetre chez lui)
};
constexpr uint8_t kShopResultLast = (uint8_t)ShopResult::Busy;

struct ShopGiveItem {
    ItemFields  f;
    std::string label;      // le nom affiche (UTF-8)
};

struct ShopMsg {
    uint8_t  op     = 0;
    // kShopBuy, kShopDone, kShopSold
    uint64_t node   = 0;
    uint16_t index  = 0;
    uint64_t digest = 0;    // kShopBuy, kShopSold
    int32_t  price  = 0;    // kShopBuy (vu), kShopDone, kShopSold (paye)
    // kShopDone
    ShopResult result = ShopResult::Bought;
    int32_t  purse  = 0;    // sa bourse (pieces) apres l'achat, ou au refus
    // kShopSold
    uint8_t  buyer  = kShopNoBuyer;
    int32_t  coins_before = 0, food_before = 0;
    int32_t  coins_after  = 0, food_after  = 0;
    std::string label;
    // kShopGive, kShopGot
    uint8_t  seq    = 0;
    uint64_t gift   = 0;
    std::vector<ShopGiveItem> items;
    uint64_t kept   = 0;
    // kShopHere
    uint8_t  open   = 0;
};

// Le message complet (tete MSG_TEAM comprise). Faux : illisible ou pas un kTeamShop.
std::vector<uint8_t> shop_encode(const ShopMsg& m);
bool shop_decode(const uint8_t* p, size_t n, ShopMsg& out);

// L'empreinte d'un objet de boutique : son genre, son prix, sa quantite (pieces
// ou nourriture) et, pour un objet, celle de l'objet (item_digest).
uint64_t shop_item_digest(int32_t type, int32_t price, int32_t amount, uint64_t equip_digest);

// Un invite peut-il acheter ce genre ?
bool shop_guest_type(int32_t type);

// Ce que l'hote voit de l'objet demande, dans sa boutique.
struct ShopLook {
    bool     shop_here = false;   // sa boutique vivante est celle de cette case
    bool     present   = false;   // un objet a cette place
    uint64_t digest    = 0;
    int32_t  type      = 0;
    int32_t  price     = 0;
    int32_t  stock     = 0;
};

// L'hote peut-il faire cet achat pour ce joueur ? `mixed` : une aventure mixte
// avec des bourses ; `has_purse` : ce joueur en a une ; `purse` : ses pieces.
ShopResult shop_check(const ShopMsg& req, const ShopLook& look, bool mixed, bool has_purse, int64_t purse);

// L'hote achete lui-meme : sa bourse suffit-elle ? (Le jeu verifie le reste.)
ShopResult shop_host_check(bool has_purse, int64_t purse, int32_t price);

// Hote, sur « Sortir » : la fenetre qui dit qui n'a pas fini dans la boutique
// (`names` : « Ami », « Ami et Lou ») ; un second clic sort quand meme.
std::string shop_wait_text(const std::string& names, bool several);

// Invite : sa demande n'a pas pu partir chez l'hote (rien n'a ete achete).
std::string shop_local_failure_text();

// La fenetre d'un refus (UTF-8, en francais, avec son titre) ; `host` : la
// fenetre de l'hote qui achete lui-meme. `price` et `purse` pour Poor.
std::string shop_refusal_text(ShopResult r, bool host, int32_t price, int64_t purse);

// Le nom de ce qui se vend, pour la nourriture et les pieces (« 5 nourritures »,
// « 10 pièces ») ; "" pour les autres genres.
std::string shop_amount_label(int32_t type, int32_t amount);

// La place de l'objet dans la boutique, de son adresse : le vecteur
// [begin, end) d'elements de `stride` octets. Faux hors du vecteur, ou pas au
// debut d'un element.
bool shop_index_of(uintptr_t begin, uintptr_t end, uintptr_t item, size_t stride, uint16_t& index);

// --- l'hote : les achats et a qui ils sont ----------------------------------------------------

// Un objet achete, dans le sac de l'hote : son id (Equipment+0x00, propre au
// processus de l'hote) et son acheteur (numero de place de l'equipe, celui des
// bourses). Les exemplaires d'un objet en stock ont le meme id (insert_item copie
// l'objet de la boutique) : un achat par exemplaire, dans l'ordre. Son empreinte
// (item_digest) le retrouve apres un rechargement de l'aventure, qui donne a
// chaque objet un nouvel id.
struct ShopOwned {
    uint64_t    id     = 0;
    uint64_t    digest = 0;
    uint8_t     slot   = 0;
    std::string label;
};

class ShopLedger {
public:
    void clear() { owned_.clear(); }
    // Un achat de plus (un exemplaire de plus de cet id).
    void add(uint64_t id, uint64_t digest, uint8_t slot, const std::string& label);
    // L'acheteur du premier exemplaire de cet objet, ou -1.
    int  owner(uint64_t id) const;
    size_t size() const { return owned_.size(); }
    const std::vector<ShopOwned>& all() const { return owned_; }
private:
    std::vector<ShopOwned> owned_;
};

// Le nombre d'exemplaires de l'objet `id` dans un sac (ses ids).
size_t shop_count(const std::vector<uint64_t>& ids, uint64_t id);

// La victoire : a chaque achat (dans l'ordre), un objet du rangement de meme id
// pas encore pris -- l'indice dans `stored`, ou -1 (il n'y est plus : equipe sur un
// chat, utilise, devenu autre chose).
std::vector<int> shop_match_owned(const std::vector<ShopOwned>& owned, const std::vector<uint64_t>& stored);

// La victoire, avant que le sac rentre : les achats dont l'id n'est plus dans le
// sac (l'hote a recharge l'aventure : nouveaux ids) prennent l'id d'un objet du
// sac de meme empreinte, pas deja a un achat. Le sac seulement : jamais un objet
// que l'hote avait deja chez lui. Rend le nombre d'achats recales.
size_t shop_rekey_owned(std::vector<ShopOwned>& owned, const std::vector<uint64_t>& bag_ids,
                        const std::vector<uint64_t>& bag_digests);

} // namespace mgmp
