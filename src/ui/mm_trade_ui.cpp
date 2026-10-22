// mm_trade_ui -- Mewgenics Multi : « Échanger », dans les fenetres du jeu. Voir mm_trade_ui.h.

#include "mm_trade_ui.h"
#include "mm_gift.h"
#include "mm_gift_core.h"
#include "mm_menu.h"
#include "mm_purse_core.h"   // purse_safe

#include "mgmp_follow.h"     // follow_pending_count, follow_node_live
#include "mgmp_log.h"
#include "mgmp_net.h"
#include "mgmp_savefile.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace mgmp {
namespace {

constexpr size_t kPage      = 8;    // objets par page (la boite tient une quinzaine de lignes)
constexpr size_t kMaxKeys   = 32;
constexpr size_t kMaxNotes  = 8;
constexpr size_t kMaxFilter = 24;   // la recherche, en octets UTF-8

const char* const kTitle = u8"Échanger";

// Les etapes, dans l'ordre de la suite.
enum class Step { Off, Who, Coins, Food, Items, Confirm, Wait, Nobody };

struct Key {
    UINT    vk = 0;
    wchar_t ch = 0;              // le caractere selon la disposition du clavier (0 : aucun)
};

struct Note {
    std::string text;
    bool        quiet = false;   // celui qui recoit : attendre une pause (mm_info_quiet)
};

struct Trade {
    TradeView v;
    bool      have    = false;
    Step      step    = Step::Off;
    bool      box     = false;   // la boite de l'etape est ouverte
    bool      closing = false;   // ... et nous avons appuye sur « oui » (son action suit)
    TradeMode mode    = TradeMode::None;   // la suite en cours (ou la derniere)
    std::vector<TradePeer> who;  // ceux qui peuvent recevoir
    size_t    who_at  = 0;       // le curseur de l'etape « a qui »
    int       target  = -1;
    int64_t   coins   = 0;
    int64_t   food    = 0;
    TradeList list;              // les objets (a la maison)
    uint32_t  kept_seen = 0;     // TradeView::kept_count deja vu
    uint32_t  sent_sid = 0;      // le numero de la nouvelle au moment de « Donner »
    std::vector<Key>  keys;      // recues par trade_ui_key, traitees par trade_ui_pump
    std::vector<Note> notes;     // les nouvelles a montrer (fenetres d'information)
    uint32_t  toast_seen = 0;
    bool      open_house = false;   // les demandes : le menu pause, la touche E
    bool      open_run   = false;
    bool      e_ok     = false;  // la touche E ouvre la suite maintenant
    bool      e_eat    = false;  // ... et la suite de ce E (WM_CHAR, WM_KEYUP) est prise
    bool      run_said = false;  // la touche E annoncee (une fois)
};
Trade t;

bool refresh() {
    TradeView nv;
    if (!gift_view(nv, t.v.serial)) return false;
    t.v    = std::move(nv);
    t.have = true;
    return true;
}

std::string target_name() {
    for (const TradePeer& p : t.v.peers)
        if ((int)p.id == t.target) return purse_safe(p.name);
    return "?";
}

int64_t max_coins() { return gift_clamp(t.v.coins, t.v.coins); }
int64_t max_food() { return gift_clamp(t.v.food, t.v.food); }

// Les objets se donnent dans cette suite : a la maison, rangement pas bloque.
bool items_here() { return t.mode == TradeMode::House && t.v.items_ok; }

// Les objets choisis, en indices de t.v.items (le rangement lu).
std::vector<size_t> chosen() {
    if (!items_here() || !t.v.items_ready) return {};
    return trade_list_chosen(t.list);
}

bool something() { return t.coins > 0 || t.food > 0 || !chosen().empty(); }

void forget_choice() {
    t.coins = 0;
    t.food  = 0;
    t.list.picked.clear();
    t.list.filter.clear();
    t.list.cursor = 0;
}

// L'etape apres `s`, sans celles ou il n'y a rien a choisir.
Step next(Step s) {
    switch (s) {
    case Step::Who:
        if (max_coins() > 0) return Step::Coins;
        [[fallthrough]];
    case Step::Coins:
        if (max_food() > 0) return Step::Food;
        [[fallthrough]];
    case Step::Food:
        if (items_here() && !(t.v.items_ready && t.v.items.empty())) return Step::Items;
        [[fallthrough]];
    default:
        return Step::Confirm;
    }
}

Step first_step() { return t.who.size() > 1 ? Step::Who : next(Step::Who); }

// « 20 pièces, 2 nourritures et Casse-croûte x2 » ; vide : rien.
std::string what_text() {
    std::vector<std::string> labels;
    if (!chosen().empty())
        for (const auto& kv : t.list.picked)
            labels.push_back(purse_safe(kv.first) + (kv.second > 1 ? " x" + std::to_string(kv.second) : ""));
    if (t.coins <= 0 && t.food <= 0 && labels.empty()) return "";
    return gift_list_text(t.coins, t.food, labels);
}

// A la maison, pourquoi l'etape des objets est sautee (« Pas d'objets maintenant : ... »).
std::string items_why_text() {
    return t.mode == TradeMode::House && !t.v.items_ok && !t.v.items_why.empty() ? "\n" + t.v.items_why : "";
}

std::string step_text() {
    const TradeView&  v    = t.v;
    const std::string with = std::string(kTitle) + u8" avec " + target_name();
    switch (t.step) {
    case Step::Who: {
        std::string s = std::string(kTitle) + u8"\n\nDonner à qui ?\n\n";
        for (size_t i = 0; i < t.who.size(); ++i) s += (i == t.who_at ? "> " : "    ") + purse_safe(t.who[i].name) + "\n";
        return s + u8"\nHaut ou Bas pour choisir, puis Entrée.";
    }
    case Step::Coins:
    case Step::Food: {
        const bool coins = t.step == Step::Coins;
        return with + "\n\n" + v.have_text + items_why_text() + "\n\n" + (coins ? u8"Pièces à donner : " : u8"Nourriture à donner : ") +
               std::to_string(coins ? t.coins : t.food) + u8"_\n\nTape un nombre (T : tout), puis Entrée.";
    }
    case Step::Items: {
        const size_t n = trade_picked(t.list);
        return with + u8" : les objets de ton rangement\n16 au plus par don : " + std::to_string(n) +
               (n > 1 ? u8" choisis." : u8" choisi.") + "\n\n" +
               (v.items_ready ? trade_list_text(t.list, kPage) : std::string(u8"Lecture du rangement...")) +
               u8"\n\nHaut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\nDes lettres pour chercher, "
               u8"puis Entrée.";
    }
    case Step::Confirm: {
        const std::string what = what_text();
        if (what.empty())
            return std::string(kTitle) + (first_step() != Step::Confirm ? u8"\n\nTu n'as rien choisi à donner."
                                                                        : u8"\n\nTu n'as rien à donner pour l'instant.") +
                   items_why_text();
        return std::string(kTitle) + u8"\n\nDonner " + what + (t.mode == TradeMode::Run ? u8" de ta bourse" : "") +
               u8"\nà " + target_name() + u8" ?" + items_why_text();
    }
    case Step::Wait:
        if (v.status_id == t.sent_sid) return std::string(kTitle) + u8"\n\nEnvoi du don...";
        return std::string(kTitle) + "\n\n" + v.status;
    case Step::Nobody: {
        std::string s = std::string(kTitle) + u8"\n\nPersonne ne peut recevoir maintenant.";
        for (const TradePeer& p : v.peers)
            if (!p.why.empty()) s += "\n" + p.why;
        if (v.peers.empty()) s += u8"\nPersonne d'autre n'est dans la partie.";
        return s;
    }
    default:
        return kTitle;
    }
}

// La boite de l'etape (ses boutons ne changent pas une fois ouverte).
void open_box() {
    // Des montants gardes d'une autre fois : pas plus que ce qu'il a maintenant.
    t.coins = std::min(t.coins, max_coins());
    t.food  = std::min(t.food, max_food());
    const char* yes = u8"Valider";
    const char* no  = u8"Passer";
    switch (t.step) {
    case Step::Who:
        yes = u8"Choisir";
        no  = u8"Annuler";
        break;
    case Step::Confirm:
        no = u8"Annuler";
        if (something()) {
            yes = u8"Donner";
        } else if (first_step() != Step::Confirm) {
            yes = u8"Recommencer";
        } else {                 // rien a donner du tout
            yes = u8"D'accord";
            no  = nullptr;
        }
        break;
    case Step::Wait:
        yes = u8"Fermer";
        no  = nullptr;
        break;
    case Step::Nobody:
        yes = u8"D'accord";
        no  = nullptr;
        break;
    default:
        break;
    }
    if (mm_trade_box_open(step_text(), yes, no)) {
        t.box     = true;
        t.closing = false;
    }
}

// Fin de la suite. `forget` : le joueur renonce (Echap, « Annuler »), rien de ce
// qu'il avait choisi ne reste ; sinon (le jeu ferme, la reponse lue) tout reste.
void stop(bool forget) {
    if (t.box && !t.closing) mm_trade_box_close();
    t.step    = Step::Off;
    t.box     = false;
    t.closing = false;
    gift_window_open(false);
    if (forget) forget_choice();
}

void start(TradeMode want) {
    if (t.step != Step::Off) return;
    const TradeView& v = t.v;
    if (!t.have || v.mode != want) {
        const std::string why = t.have && v.mode == TradeMode::None ? v.why_none
                                                                    : std::string(u8"Pas d'échange possible ici.");
        t.notes.push_back({std::string(kTitle) + "\n\n" + why, false});
        log_line("DON", "Echanger : pas maintenant (%s)", want == TradeMode::House ? "maison" : "bourse");
        return;
    }
    if (t.mode != want) forget_choice();   // l'autre sorte d'echange : rien de choisi
    t.mode = want;
    t.who.clear();
    t.who_at = 0;
    for (const TradePeer& p : v.peers)
        if (p.can) {
            if ((int)p.id == t.target) t.who_at = t.who.size();
            t.who.push_back(p);
        }
    if (t.who.size() == 1) t.target = t.who[0].id;
    t.step = t.who.empty() ? Step::Nobody : first_step();
    gift_window_open(want == TradeMode::House);
    log_line("DON", "Echanger ouvert (%s ; %zu joueur(s) peuvent recevoir)",
             want == TradeMode::House ? "maison" : "bourse", t.who.size());
}

void send() {
    std::vector<uint64_t> ids;
    for (size_t i : chosen())
        if (i < t.v.items.size()) ids.push_back(t.v.items[i].id);
    t.sent_sid = t.v.status_id;
    gift_request(t.mode, (uint8_t)t.target, t.coins, t.food, ids);
}

void items_key(const Key& k) {
    TradeList& l = t.list;
    switch (k.vk) {
    case VK_UP:       trade_list_key(l, TradeListKey::Up, kPage); return;
    case VK_DOWN:     trade_list_key(l, TradeListKey::Down, kPage); return;
    case VK_PRIOR:    trade_list_key(l, TradeListKey::PageUp, kPage); return;
    case VK_NEXT:     trade_list_key(l, TradeListKey::PageDown, kPage); return;
    case VK_SPACE:    trade_list_key(l, TradeListKey::Toggle, kPage); return;
    case VK_RIGHT:
    case VK_ADD:      trade_list_key(l, TradeListKey::More, kPage); return;
    case VK_LEFT:
    case VK_SUBTRACT: trade_list_key(l, TradeListKey::Less, kPage); return;
    case VK_BACK:     // un caractere UTF-8 de moins
        while (!l.filter.empty() && ((uint8_t)l.filter.back() & 0xC0) == 0x80) l.filter.pop_back();
        if (!l.filter.empty()) l.filter.pop_back();
        break;
    case VK_DELETE:
        l.filter.clear();
        break;
    default: {
        if (k.ch < 0x21 || k.ch == 0x7F) return;          // pas un caractere visible
        char buf[8] = {};
        const int n = WideCharToMultiByte(CP_UTF8, 0, &k.ch, 1, buf, (int)sizeof(buf), nullptr, nullptr);
        if (n <= 0 || l.filter.size() + (size_t)n > kMaxFilter) return;
        l.filter.append(buf, (size_t)n);
        break;
    }
    }
    l.cursor = 0;                // une autre recherche : en haut de la liste
    trade_list_fit(l);
}

void keys_pump() {
    std::vector<Key> keys;
    keys.swap(t.keys);
    if (!t.box || t.closing || !mm_trade_box_alive()) return;
    for (const Key& k : keys) {
        if (k.vk == VK_ESCAPE) {
            log_line("DON", "Echanger : Echap");
            stop(t.step != Step::Wait && t.step != Step::Nobody);
            return;
        }
        if (k.vk == VK_RETURN) {
            if (mm_trade_box_yes()) t.closing = true;
            return;
        }
        switch (t.step) {
        case Step::Who:
            if (k.vk == VK_UP && t.who_at) --t.who_at;
            else if (k.vk == VK_DOWN && t.who_at + 1 < t.who.size()) ++t.who_at;
            break;
        case Step::Coins:
        case Step::Food: {
            int64_t&      v   = t.step == Step::Coins ? t.coins : t.food;
            const int64_t max = t.step == Step::Coins ? max_coins() : max_food();
            // Les chiffres par leur touche : en AZERTY, ceux du haut sans Maj donnent « & é " ».
            if (k.vk >= '0' && k.vk <= '9') v = trade_amount_key(v, (int)(k.vk - '0'), max);
            else if (k.vk >= VK_NUMPAD0 && k.vk <= VK_NUMPAD9) v = trade_amount_key(v, (int)(k.vk - VK_NUMPAD0), max);
            else if (k.vk == VK_BACK) v = trade_amount_key(v, kTradeKeyBack, max);
            else if (k.vk == 'T') v = trade_amount_key(v, kTradeKeyAll, max);
            else if (k.vk == VK_DELETE) v = 0;
            break;
        }
        case Step::Items:
            items_key(k);
            break;
        default:
            break;
        }
    }
}

// Les nouvelles des dons (mm_gift) deviennent des fenetres d'information du jeu.
void take_toasts() {
    for (const TradeToast& n : t.v.toasts) {
        if (n.id <= t.toast_seen) continue;
        t.toast_seen = n.id;
        if (n.result && t.step == Step::Wait) continue;    // la boite d'attente la montre
        if (t.notes.size() >= kMaxNotes) t.notes.erase(t.notes.begin());
        t.notes.push_back({std::string(kTitle) + "\n\n" + n.text, !n.result});
    }
}

// La premiere nouvelle qui peut s'ouvrir : celle d'un don recu, qui attend une
// pause, ne retient pas les autres derriere elle.
void notes_pump() {
    if (t.notes.empty() || t.step != Step::Off) return;
    // Celui qui recoit n'est pas arrete : sa nouvelle attend une pause (Echap).
    const bool quiet_ok = mm_info_quiet();
    const bool loud_ok  = !mm_info_busy() && !mm_game_prompt_open();
    for (auto it = t.notes.begin(); it != t.notes.end(); ++it)
        if (it->quiet ? quiet_ok : loud_ok) {
            mm_show_info(it->text, kInfoYieldToFollow);
            t.notes.erase(it);
            return;
        }
}

} // namespace

void trade_ui_pump() {
    if (refresh() && t.v.items_ready) {
        // Le choix est garde par nom : le rangement relu le retrouve.
        t.list.labels.clear();
        for (const TradeItem& it : t.v.items) t.list.labels.push_back(it.label);
        trade_list_fit(t.list);
    }
    if (!t.have) return;
    const TradeView& v = t.v;
    take_toasts();

    // Un don passe, meme en retard (apres « Pas de réponse ») : ce qui etait choisi
    // est oublie ; refuse ou sans reponse, tout reste pour la fois suivante.
    if (v.kept_count != t.kept_seen) {
        t.kept_seen = v.kept_count;
        t.coins     = 0;
        t.food      = 0;
        t.list.picked.clear();
        t.list.filter.clear();
    }

    // Un invite que l'hote emmene dans une case : rien ne doit arreter sa carte.
    const bool follow = net_role() == NetRole::Client && savefile_in_coop() &&
                        (follow_pending_count() != 0 || follow_node_live());
    if (t.step != Step::Off) {
        if (v.mode != t.mode) {
            log_line("DON", "Echanger ferme : plus d'echange possible ici");
            stop(false);
        } else if (t.mode == TradeMode::Run && follow) {
            log_line("DON", "Echanger ferme : l'hote entre dans une case");
            t.notes.push_back({std::string(kTitle) + u8"\n\nLa fenêtre s'est fermée : l'hôte est entré dans une "
                                                     u8"case.\nCe que tu avais choisi est gardé (E pour reprendre).",
                               false});
            stop(false);
        } else if (t.box && !mm_trade_box_alive()) {
            // Fermee sans passer par ses boutons (une transition qui emporte tout).
            log_line("DON", "Echanger ferme : la fenetre a disparu");
            t.box = false;
            stop(false);
        }
    }

    // Les demandes : le bouton du menu pause, la touche E.
    if (t.open_house) {
        t.open_house = false;
        start(TradeMode::House);
    }
    if (t.open_run) {
        t.open_run = false;
        start(TradeMode::Run);
    }

    keys_pump();
    if (t.box && !t.closing) mm_trade_box_text(step_text());
    if (t.step != Step::Off && !t.box) open_box();
    // Une boite que nous avons quittee et que le jeu n'a pas encore refermee.
    if (t.step == Step::Off && mm_trade_box_alive()) mm_trade_box_close();

    // En aventure, la touche E : sur la carte qui tourne, rien d'autre d'ouvert.
    t.e_ok = t.step == Step::Off && v.mode == TradeMode::Run && v.show && !follow && mm_scene_running("Map") &&
             !mm_other_popup_open() && !mm_loot_popup_open();
    if (!t.run_said && v.mode == TradeMode::Run && v.show && !v.peers.empty()) {
        t.run_said = true;
        t.notes.push_back({std::string(kTitle) + u8" en aventure\n\nSur la carte, appuie sur E pour donner des "
                                                 u8"pièces\nou de la nourriture de ta bourse à un autre joueur.",
                           false});
    }
    notes_pump();
}

void trade_ui_button(bool yes) {
    if (!t.box) return;          // une boite que nous avons fermee sans action
    t.box     = false;
    t.closing = false;
    switch (t.step) {
    case Step::Who:
        if (!yes || t.who.empty()) {
            stop(true);
            return;
        }
        t.target = t.who[std::min(t.who_at, t.who.size() - 1)].id;
        t.step   = next(Step::Who);
        break;
    case Step::Coins:
        if (!yes) t.coins = 0;   // « Passer »
        t.step = next(Step::Coins);
        break;
    case Step::Food:
        if (!yes) t.food = 0;
        t.step = next(Step::Food);
        break;
    case Step::Items:
        if (!yes) t.list.picked.clear();
        t.step = Step::Confirm;
        break;
    case Step::Confirm:
        if (!yes) {
            stop(true);
            return;
        }
        if (something()) {
            send();
            t.step = Step::Wait;
        } else if (first_step() != Step::Confirm) {
            t.step = first_step();
        } else {
            stop(false);
        }
        break;
    default:                     // l'attente, « personne » : la boite est lue
        stop(false);
        break;
    }
}

void trade_ui_open_house() { t.open_house = true; }

bool trade_ui_key(UINT msg, WPARAM wp, LPARAM lp, bool box) {
    if (box) {
        switch (msg) {
        case WM_KEYDOWN: {
            Key k;
            k.vk = (UINT)wp;
            // Le caractere selon la disposition du clavier (AZERTY) ; drapeau 4 : sans
            // toucher a l'etat du clavier (touches mortes) de la fenetre.
            BYTE keys[256] = {};
            if (GetKeyboardState(keys)) {
                wchar_t out[4] = {};
                if (ToUnicodeEx(k.vk, (UINT)((lp >> 16) & 0xFF), keys, out, 4, 4, GetKeyboardLayout(0)) == 1)
                    k.ch = out[0];
            }
            if (t.keys.size() < kMaxKeys) t.keys.push_back(k);
            return true;         // le jeu ne voit pas ces touches
        }
        case WM_KEYUP:
            if (wp == 'E') t.e_eat = false;
            return true;
        case WM_CHAR:
        case WM_DEADCHAR:
            return true;
        default:
            return false;
        }
    }
    // Sur la carte : E ouvre la suite ; tenue, elle ne la rouvre pas.
    if (msg == WM_KEYDOWN && wp == 'E' && (t.e_ok || t.e_eat) && !(GetKeyState(VK_CONTROL) & 0x8000)) {
        if (!t.e_eat) t.open_run = true;
        t.e_eat = true;
        return true;
    }
    if (t.e_eat && msg == WM_CHAR && (wp == 'e' || wp == 'E')) return true;
    if (t.e_eat && msg == WM_KEYUP && wp == 'E') {
        t.e_eat = false;
        return true;
    }
    return false;
}

} // namespace mgmp
