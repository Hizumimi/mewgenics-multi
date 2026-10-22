#pragma once
// mm_gift -- Mewgenics Multi : les dons entre joueurs (protocole 62). Le message et
// les regles : mm_gift_core.h ; « Échanger », dans les fenetres du jeu : mm_trade_ui.
//
// LE JEU (1.1.21239, releve le 18 octobre)
//   - L'or et la nourriture de la maison : House+0xB4 et +0xB0 (int32). La maison
//     est *(MewDirector+0x5A0) : 0x1403C2400 la cree (0x1403C98C0 pose alors le
//     global D_HouseInstance, que son destructeur remet a 0) ; House::save
//     (0x140205E80) ecrit ces deux champs dans la save.
//   - Le rangement : Inventory+0x78 (kInv_Storage), relu et reecrit avec les
//     crochets de invsync (invsync_serialize_storage, invsync_load_storage). Un
//     objet relu recoit un nouvel id (le compteur du processus) : le donneur
//     retrouve ses objets par leur empreinte (item_digest) si leur id a change ; et
//     chez l'hote, rien n'est relu tant que le defile, les achats ou les dons aux
//     PNJ tiennent des objets de son rangement par leur id (loot_storage_refs...).
//   - MewDirector::save (0x1403BDCF0, rcx = MewDirector) : la maison, les chats,
//     l'inventaire, la progression, puis l'ecriture de la save. Le jeu l'appelle
//     d'une quinzaine d'endroits, des rappels de la maison surtout ; le mod
//     l'appelle dans la maison de CE joueur seulement (pas en aventure, pas dans la
//     copie de l'hote), sans fenetre ni ecran des objets ouvert.
//
// A LA MAISON (les deux joueurs dans leur propre maison)
//   Donneur : la fenetre demande un don ; gift_pump verifie l'or, la nourriture et
//   les objets (encore la, ni promis a un autre ni trop longs pour le message),
//   envoie kGiftOffer (par l'hote, qui relaie, pour un invite) et attend. Celui qui
//   recoit : dans sa maison, sans ecran des objets ni fenetre du jeu (20 s au
//   plus), il ajoute les objets a son rangement (marques « nouveaux »), l'or et la
//   nourriture a sa maison, sauve, puis repond kGiftGot (avec son nom) ; sinon
//   kGiftRefused et pourquoi. Donneur, a kGiftGot : dans la save d'ou le don est
//   parti (sinon a son retour dans cette save), l'or et la nourriture quittent sa
//   maison tout de suite (pas sous 0), les objets son rangement une fois l'ecran
//   des objets ferme, et il sauve. Sans reponse en 35 s, rien n'a quitte sa maison
//   (une reponse en retard, dans les 5 minutes, compte encore : 4 dons au plus).
//   Celui qui recoit n'est pas arrete : la fenetre « Cadeau de ... » attend que sa
//   maison soit deja en pause (Echap, le menu pause).
//   Pas d'objets (ni donnes ni recus) pour un invite dont les chats sont encore
//   dans l'aventure de l'hote : son rangement attend leur retour (mm_team).
//   Presence (MM_GIFT, pour la fenetre) : a = 1 joueur -> hote, c = 1 il peut
//   recevoir (dans sa maison), 0 non ; a = 2 hote -> tous, x = les numeros qui le
//   peuvent (un bit chacun, l'hote compris). Redit a chaque changement et toutes
//   les 10 s.
//
// EN AVENTURE MIXTE : les bourses (mm_purse, purse_give) -- des pieces et de la
// nourriture de sa bourse a celle d'un autre joueur ; les objets passent deja par
// le sac de l'aventure, commun.

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace mgmp {

struct MmMsg;

// Chaque image (mm_menu_pump), session ou pas.
void gift_pump();

// MSG_TEAM kTeamGift (mm_team le route ici).
void gift_on_team(uint8_t from, const uint8_t* p, size_t n);

// MM_GIFT (mm_menu le route ici).
void gift_on_mm(uint8_t from, const MmMsg& m);

// mm_purse : une nouvelle d'un don entre bourses -- `mine` : la reponse au don de
// CE joueur (la fenetre la montre ; `kept` : il est passe) ; sinon un don recu.
void gift_purse_note(const std::string& text_utf8, bool mine, bool kept = false);

// Ce joueur peut donner et recevoir a la maison maintenant (en session, dans SA
// maison, sans depart ni defense) : le bouton « Échanger » du menu pause (mm_menu).
bool gift_house_open();

// --- « Échanger » (mm_trade_ui) -------------------------------------------------------
// Tout passe par des copies sous verrou : la suite de fenetres lit un etat publie
// par gift_pump et lui laisse ses demandes.

enum class TradeMode : uint8_t {
    None  = 0,   // rien a echanger maintenant (why_none dit pourquoi)
    House = 1,   // a la maison : or, nourriture et objets du rangement
    Run   = 2,   // en aventure mixte : pieces et nourriture de sa bourse
};

struct TradePeer {
    uint8_t     id  = 0;
    std::string name;            // UTF-8
    bool        can = false;     // peut recevoir maintenant
    std::string why;             // sinon pourquoi (UTF-8)
};

struct TradeItem {
    uint64_t    id = 0;          // son id dans le rangement (change quand il est relu)
    std::string label;           // UTF-8
};

struct TradeToast {
    uint32_t    id     = 0;
    std::string text;            // UTF-8
    bool        result = false;  // la reponse a un don de ce joueur (deja dans `status`)
};

struct TradeView {
    uint32_t    serial = 0;      // change a chaque etat publie
    bool        show   = false;  // a la maison, ou en aventure sur la carte (la touche E)
    TradeMode   mode   = TradeMode::None;
    std::string why_none;        // UTF-8
    std::vector<TradePeer> peers;          // les autres joueurs
    int64_t     coins = 0;       // ce que ce joueur peut donner (maison ou bourse)
    int64_t     food  = 0;
    std::string have_text;       // « Dans ta maison : 12 pièces et 3 nourritures. » (UTF-8)
    bool        items_ok    = false;       // House : ses objets peuvent se donner maintenant
    std::string items_why;                 // sinon pourquoi (UTF-8)
    bool        items_ready = false;       // House, fenetre ouverte : le rangement est lu
    std::vector<TradeItem> items;
    bool        pending = false;           // son don attend sa reponse
    std::string status;                    // la derniere nouvelle de ses dons (UTF-8)
    uint32_t    status_id = 0;
    uint32_t    kept_count  = 0;           // ses dons passes, meme en retard (mm_trade_ui oublie le choix)
    std::vector<TradeToast> toasts;        // les nouvelles de moins de 8 s (des fenetres du jeu)
};

// Copie l'etat publie si son numero n'est pas `have` ; vrai : copie faite.
bool gift_view(TradeView& out, uint32_t have);
// Les objets se choisissent : le rangement est lu (deux fois par seconde).
void gift_window_open(bool open);
// Une demande de « Échanger » (la derniere compte) ; gift_pump la traite.
void gift_request(TradeMode mode, uint8_t to, int64_t coins, int64_t food, const std::vector<uint64_t>& item_ids);

} // namespace mgmp
