#pragma once
// mm_gift_core -- Mewgenics Multi : les dons entre joueurs (protocole 62), la partie
// portable (bibliotheque standard seulement, verifiee par tests/test_gift.cpp).
//
// LES REGLES (document de conception, « Échanges » ; choisies le 18 octobre :
// a la maison et en aventure, un bouton « Échanger », don direct)
//   - A la maison : des pieces, de la nourriture et des objets du rangement, d'une
//     save a l'autre, a un joueur qui est lui aussi dans sa maison (la sienne, pas
//     une copie de l'aventure de l'hote).
//   - En aventure mixte : des pieces et de la nourriture de sa bourse (mm_purse), a
//     la bourse d'un autre joueur de l'aventure. Les objets passent deja par le sac
//     de l'aventure, commun.
//   - Don direct : celui qui recoit n'a rien a accepter. Mais a la maison, rien ne
//     quitte le donneur avant que celui qui recoit l'ait garde (ajoute a sa maison
//     et sauve) : sans reponse, un doublon plutot qu'une perte, comme les dons aux
//     PNJ et le defile du butin.
//
// LE MESSAGE (MSG_TEAM kTeamGift, voir mm_team_merge) : la tete MSG_TEAM, puis u8 op,
// u32 numero du don (compte chez le donneur), u8 donneur, u8 destinataire (numeros
// de la session), puis selon op :
//   kGiftOffer    donneur -> destinataire (l'hote relaie) : i32 pieces, i32
//                 nourriture, u8 n puis n objets (nom, texte, a, b, c, d (u32), e
//                 (u8), f (u32), puis le nom affiche), puis le nom du donneur.
//   kGiftGot      destinataire -> donneur : tout est garde chez lui (et sauve).
//   kGiftRefused  destinataire -> donneur : u8 raison (kGiftWhy*) ; rien n'est garde.
// Les chaines : u16 n puis les octets (UTF-8 pour les noms affiches).

#include "mm_team_merge.h"   // ItemFields, kTeamMagic, kTeamFormat, kTeamGift

#include <cstdint>
#include <string>
#include <vector>

namespace mgmp {

constexpr size_t  kGiftMaxItems = 16;        // objets dans un don
constexpr int32_t kGiftMaxMoney = 1000000;   // pieces ou nourriture dans un don
constexpr size_t  kGiftMaxName  = 63;        // la cle GON d'un objet
constexpr size_t  kGiftMaxText  = 255;       // le texte d'un objet (un livre : sa capacite)
constexpr size_t  kGiftMaxLabel = 127;       // un nom affiche (objet ou joueur), UTF-8

enum : uint8_t {
    kGiftOffer   = 1,
    kGiftGot     = 2,
    kGiftRefused = 3,
};

// Pourquoi un don est refuse (kGiftRefused).
enum : uint8_t {
    kGiftWhyNotHome = 1,   // il n'est pas dans sa maison (une aventure, le menu...)
    kGiftWhyBusy    = 2,   // un ecran des objets ou une fenetre du jeu, trop longtemps
    kGiftWhyFailed  = 3,   // le jeu n'a pas pu (rangement illisible, objet refuse...)
    kGiftWhyOff     = 4,   // les dons ne marchent pas chez lui (fonction du jeu introuvable)
    kGiftWhyGone    = 5,   // l'hote : il n'est plus dans la partie
    kGiftWhyLast    = kGiftWhyGone,
};

struct GiftItem {
    ItemFields  f;
    std::string label;     // le nom affiche chez le donneur (UTF-8)
};

struct GiftMsg {
    uint8_t  op     = 0;
    uint32_t id     = 0;
    uint8_t  giver  = 0;
    uint8_t  target = 0;
    // kGiftOffer
    int32_t               coins = 0;
    int32_t               food  = 0;
    std::vector<GiftItem> items;
    std::string           giver_name;
    // kGiftRefused
    uint8_t why = 0;
};

// Le message complet (tete MSG_TEAM comprise). Faux au decodage : illisible, pas un
// kTeamGift, ou des quantites hors des bornes.
std::vector<uint8_t> gift_encode(const GiftMsg& m);
bool gift_decode(const uint8_t* p, size_t n, GiftMsg& out);

// Ajoute des objets a un bloc de rangement tel que le jeu l'ecrit (u32 nombre, puis
// les objets, team_item_record) ; un bloc vide ou absent vaut « 0 objet ». Faux (et
// `why`) si le bloc n'a pas la forme attendue : il reste alors tel quel.
bool gift_storage_append(std::vector<uint8_t>& blob, const std::vector<ItemFields>& items, std::string* why);

// Les objets d'un bloc de rangement (pour verifier un ajout) ; faux s'il est illisible.
bool gift_storage_items(const std::vector<uint8_t>& blob, std::vector<ItemFields>& out);

// « 20 pièces, 5 nourritures et Casse-croûte » (UTF-8) ; « rien » si vide.
std::string gift_list_text(int64_t coins, int64_t food, const std::vector<std::string>& labels);

// La raison d'un refus (UTF-8), avec le nom de celui qui devait recevoir :
// « Ami n'est pas dans sa maison », « le jeu d'Ami n'a pas pu les ranger ».
std::string gift_why_text(uint8_t why, const std::string& name);

// Les nouvelles d'un don (UTF-8, les noms passes par purse_safe) :
//   celui qui recoit, une fenetre du jeu : « Cadeau d'Ami\n\nAmi t'a donné 20
//   pièces et Casse-croûte.\nC'est dans ta maison. » (`saved` : « ... et ta save
//   est à jour. ») ;
//   le donneur : « Ami a bien reçu 20 pièces : c'est parti de ta maison. »
//   (`late` : « Finalement, ... » ; `kept` objets restes aussi chez lui : dit) ;
//   « Le don à Ami n'est pas passé : Ami n'est pas dans sa maison. Rien n'a
//   quitté ta maison. »
std::string gift_received_text(const std::string& giver, const std::string& list, bool saved);
std::string gift_done_text(const std::string& target, const std::string& list, size_t kept, bool late);
std::string gift_refused_text(const std::string& target, uint8_t why);

// Celui qui recoit un don, a chaque image : le garder maintenant, attendre (un
// ecran ou une fenetre ouverte, au plus kGiftWaitMs), ou le refuser (et pourquoi).
enum class GiftStep { Apply, Wait, Refuse };
constexpr uint64_t kGiftWaitMs = 20000;
GiftStep gift_receiver_step(bool at_home, bool busy, uint64_t waited_ms, uint8_t& why);

// Ce que le donneur peut donner : `want` ramene entre 0 et `have` (et kGiftMaxMoney).
int32_t gift_clamp(int64_t want, int64_t have);

} // namespace mgmp
