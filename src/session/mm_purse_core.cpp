// mm_purse_core.cpp -- voir mm_purse_core.h.
#include "mm_purse_core.h"
#include "mgmp_proto.h"
#include "mm_lang.h"

#include <cstdarg>

#include <cstdio>
#include <cstring>

namespace mgmp {

namespace {

int popcount(uint32_t m) {
    int n = 0;
    for (; m; m &= m - 1) ++n;
    return n;
}

int32_t clamp32(int64_t v) {
    if (v > INT32_MAX) return INT32_MAX;
    if (v < INT32_MIN) return INT32_MIN;
    return (int32_t)v;
}

} // namespace

void PurseBook::reset() {
    for (PurseEntry& e : e_) e = PurseEntry{};
    players_ = 1u << host_;
}

void PurseBook::set_players(uint32_t mask, uint8_t host) {
    host_    = host < kPurseSlots ? host : 0;
    players_ = mask & ((1u << kPurseSlots) - 1);
    if (!players_) players_ = 1u << host_;
}

void PurseBook::credit(uint8_t owner, int64_t dcoins, int64_t dfood) {
    if (owner >= kPurseSlots) owner = host_;
    e_[owner].coins += dcoins;
    e_[owner].food  += dfood;
}

void PurseBook::split(bool coins, int64_t d) {
    if (!d) return;
    auto at = [&](uint8_t p) -> int64_t& { return coins ? e_[p].coins : e_[p].food; };
    const int n = popcount(players_);
    if (n <= 1) {
        // Un seul joueur : tout a lui.
        for (uint8_t p = 0; p < kPurseSlots; ++p)
            if (players_ & (1u << p)) { at(p) += d; return; }
        at(host_) += d;
        return;
    }
    // Division vers zero : le reste a le signe du changement, et |reste| < n.
    const int64_t q = d / n;
    int64_t       r = d - q * n;
    for (uint8_t p = 0; p < kPurseSlots; ++p)
        if (players_ & (1u << p)) at(p) += q;
    // Le reste, une unite a la fois : un gain a la bourse la plus maigre, une
    // perte a la plus garnie ; a egalite, l'hote puis les numeros dans l'ordre.
    while (r) {
        const int step = r > 0 ? 1 : -1;
        int best = -1;
        for (int i = -1; i < (int)kPurseSlots; ++i) {
            const uint8_t p = i < 0 ? host_ : (uint8_t)i;
            if (i >= 0 && p == host_) continue;
            if (!(players_ & (1u << p))) continue;
            if (best < 0 || (step > 0 ? at(p) < at((uint8_t)best) : at(p) > at((uint8_t)best))) best = p;
        }
        at((uint8_t)best) += step;
        r -= step;
    }
}

void PurseBook::share(int64_t dcoins, int64_t dfood) {
    split(true, dcoins);
    split(false, dfood);
}

int64_t PurseBook::total_coins() const {
    int64_t t = 0;
    for (const PurseEntry& e : e_) t += e.coins;
    return t;
}

int64_t PurseBook::total_food() const {
    int64_t t = 0;
    for (const PurseEntry& e : e_) t += e.food;
    return t;
}

PursePayout PurseBook::settle(uint32_t paid, int64_t run_coins, int64_t run_food) const {
    PursePayout out;
    int64_t left_c = run_coins > 0 ? run_coins : 0;
    int64_t left_f = run_food > 0 ? run_food : 0;
    for (uint8_t p = 0; p < kPurseSlots; ++p) {
        if (p == host_ || !(paid & (1u << p))) continue;
        int64_t c = e_[p].coins > 0 ? e_[p].coins : 0;
        int64_t f = e_[p].food > 0 ? e_[p].food : 0;
        if (c > left_c) c = left_c;
        if (f > left_f) f = left_f;
        left_c -= c;
        left_f -= f;
        out.coins[p] = clamp32(c);
        out.food[p]  = clamp32(f);
    }
    out.host_coins = clamp32(left_c);
    out.host_food  = clamp32(left_f);
    return out;
}

bool PurseBook::transfer(uint8_t from, uint8_t to, int64_t coins, int64_t food) {
    if (from >= kPurseSlots || to >= kPurseSlots || from == to) return false;
    if (coins < 0 || food < 0 || (!coins && !food)) return false;
    if (coins > e_[from].coins || food > e_[from].food) return false;
    e_[from].coins -= coins;
    e_[from].food  -= food;
    e_[to].coins   += coins;
    e_[to].food    += food;
    return true;
}

std::string purse_give_no_text(uint8_t why) {
    switch (why) {
    case kPurseNoPurse:  return tr(u8"pas de bourse pour ce joueur dans cette aventure");
    case kPurseNoEnough: return tr(u8"ta bourse n'en a pas autant");
    case kPurseNoRun:    return tr(u8"l'aventure n'est plus suivie");
    default:             return tr(u8"raison inconnue");
    }
}

// --- le message ---------------------------------------------------------------------------

namespace {

void put32(uint8_t* p, int32_t v) {
    const uint32_t u = (uint32_t)v;
    p[0] = (uint8_t)u; p[1] = (uint8_t)(u >> 8); p[2] = (uint8_t)(u >> 16); p[3] = (uint8_t)(u >> 24);
}
int32_t get32(const uint8_t* p) {
    return (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

} // namespace

void purse_encode_lines(MmMsg& m, uint8_t a, uint8_t seq, const PurseLine* lines, uint8_t n,
                        int32_t x, int32_t y) {
    m = MmMsg{};
    m.kind = MM_PURSE;
    m.a    = a;
    m.b    = seq;
    if (n > kPurseMaxLines) n = kPurseMaxLines;
    m.c    = n;
    m.x    = (uint32_t)x;
    m.y    = (uint32_t)y;
    uint8_t* t = (uint8_t*)m.text;
    static_assert(9 * kPurseMaxLines <= kMmTextMax, "MmMsg::text trop petit pour les bourses");
    for (uint8_t i = 0; i < n; ++i) {
        t[9 * i] = lines[i].peer;
        put32(t + 9 * i + 1, lines[i].coins);
        put32(t + 9 * i + 5, lines[i].food);
    }
}

uint8_t purse_decode_lines(const MmMsg& m, PurseLine* out, uint8_t max) {
    if (m.kind != MM_PURSE || (m.a != kPurseLedger && m.a != kPursePayout)) return 0;
    if (m.c == 0 || m.c > kPurseMaxLines || m.c > max) return 0;
    const uint8_t* t = (const uint8_t*)m.text;
    for (uint8_t i = 0; i < m.c; ++i) {
        out[i].peer  = t[9 * i];
        out[i].coins = get32(t + 9 * i + 1);
        out[i].food  = get32(t + 9 * i + 5);
        if (out[i].peer >= kPurseSlots) return 0;
    }
    return m.c;
}

void purse_encode_find(MmMsg& m, uint8_t owner, int32_t coins, int32_t food, const std::string& cat,
                       uint8_t flags) {
    m = MmMsg{};
    m.kind = MM_PURSE;
    m.a    = kPurseFind;
    m.b    = owner;
    m.c    = flags;
    m.x    = (uint32_t)coins;
    m.y    = (uint32_t)food;
    // Le nom, coupe sans casser un caractere UTF-8.
    size_t n = cat.size();
    if (n > sizeof(m.text) - 1) {
        n = sizeof(m.text) - 1;
        while (n > 0 && ((uint8_t)cat[n] & 0xC0) == 0x80) --n;
    }
    memcpy(m.text, cat.data(), n);
}

bool purse_decode_find(const MmMsg& m, uint8_t& owner, int32_t& coins, int32_t& food, std::string& cat,
                       uint8_t* flags) {
    if (m.kind != MM_PURSE || m.a != kPurseFind || m.b >= kPurseSlots) return false;
    owner = m.b;
    if (flags) *flags = m.c;
    coins = (int32_t)m.x;
    food  = (int32_t)m.y;
    cat.assign(m.text, strnlen(m.text, sizeof(m.text)));
    return true;
}

// --- les dons entre bourses (protocole 62) ---------------------------------------------------

namespace {

bool give_amounts_ok(int64_t coins, int64_t food) {
    return coins >= 0 && food >= 0 && (coins || food) && coins <= kPurseGiveMax && food <= kPurseGiveMax;
}

} // namespace

void purse_encode_give_req(MmMsg& m, uint8_t to, int32_t coins, int32_t food, uint8_t seq) {
    m = MmMsg{};
    m.kind = MM_PURSE;
    m.a    = kPurseGiveReq;
    m.b    = to;
    m.c    = seq;
    m.x    = (uint32_t)coins;
    m.y    = (uint32_t)food;
}

bool purse_decode_give_req(const MmMsg& m, uint8_t& to, int32_t& coins, int32_t& food, uint8_t& seq) {
    if (m.kind != MM_PURSE || m.a != kPurseGiveReq || m.b >= kPurseSlots) return false;
    const int32_t c = (int32_t)m.x, f = (int32_t)m.y;
    if (!give_amounts_ok(c, f)) return false;
    to    = m.b;
    seq   = m.c;
    coins = c;
    food  = f;
    return true;
}

void purse_encode_given(MmMsg& m, uint8_t giver, uint8_t to, int32_t coins, int32_t food, uint8_t seq) {
    m = MmMsg{};
    m.kind    = MM_PURSE;
    m.a       = kPurseGiven;
    m.b       = giver;
    m.c       = to;
    m.x       = (uint32_t)coins;
    m.y       = (uint32_t)food;
    m.text[0] = (char)seq;
}

bool purse_decode_given(const MmMsg& m, uint8_t& giver, uint8_t& to, int32_t& coins, int32_t& food,
                        uint8_t& seq) {
    if (m.kind != MM_PURSE || m.a != kPurseGiven || m.b >= kPurseSlots || m.c >= kPurseSlots || m.b == m.c)
        return false;
    const int32_t c = (int32_t)m.x, f = (int32_t)m.y;
    if (!give_amounts_ok(c, f)) return false;
    giver = m.b;
    to    = m.c;
    coins = c;
    food  = f;
    seq   = (uint8_t)m.text[0];
    return true;
}

void purse_encode_give_no(MmMsg& m, uint8_t giver, uint8_t why, uint8_t seq) {
    m = MmMsg{};
    m.kind = MM_PURSE;
    m.a    = kPurseGiveNo;
    m.b    = giver;
    m.c    = why;
    m.x    = seq;
}

bool purse_decode_give_no(const MmMsg& m, uint8_t& giver, uint8_t& why, uint8_t& seq) {
    if (m.kind != MM_PURSE || m.a != kPurseGiveNo || m.b >= kPurseSlots) return false;
    if (m.c == 0 || m.c > kPurseNoLast || m.x > 0xFF) return false;
    giver = m.b;
    why   = m.c;
    seq   = (uint8_t)m.x;
    return true;
}

namespace {

// Un texte compose a partir d'un format traduit par tr().
std::string fmt(const char* f, ...) {
    va_list ap, ap2;
    va_start(ap, f);
    va_copy(ap2, ap);
    const int n = vsnprintf(nullptr, 0, f, ap);
    va_end(ap);
    std::string s(n > 0 ? (size_t)n : 0, '\0');
    if (n > 0) vsnprintf(&s[0], (size_t)n + 1, f, ap2);
    va_end(ap2);
    return s;
}

} // namespace

std::string purse_given_line(bool from_me, bool to_me, const std::string& giver, const std::string& to,
                             int64_t coins, int64_t food) {
    const std::string what = purse_amount_text(coins, food);
    if (to_me) return fmt(tr(u8"%s t'a donné %s : c'est dans ta bourse."), purse_safe(giver).c_str(), what.c_str());
    if (from_me) return fmt(tr(u8"Tu as donné %s à %s : c'est dans sa bourse."), what.c_str(), purse_safe(to).c_str());
    return fmt(tr(u8"%s a donné %s à %s."), purse_safe(giver).c_str(), what.c_str(), purse_safe(to).c_str());
}

std::string purse_amount_text(int64_t coins, int64_t food) {
    if (coins < 0) coins = 0;
    if (food < 0) food = 0;
    const std::string c = fmt(coins > 1 ? tr(u8"%lld pièces") : tr(u8"%lld pièce"), (long long)coins);
    const std::string f = fmt(food > 1 ? tr(u8"%lld nourritures") : tr(u8"%lld nourriture"), (long long)food);
    if (coins && food) return fmt(tr(u8"%s et %s"), c.c_str(), f.c_str());
    if (coins) return c;
    if (food) return f;
    return tr(u8"rien");
}

std::string purse_balance_text(int64_t coins, int64_t food) {
    // Le signe vient du nombre (« -11 pièces ») ; le pluriel, de sa valeur absolue.
    auto part = [](int64_t v, bool food) {
        const bool many = v > 1 || v < -1;
        return fmt(food ? (many ? tr(u8"%lld nourritures") : tr(u8"%lld nourriture"))
                        : (many ? tr(u8"%lld pièces") : tr(u8"%lld pièce")),
                   (long long)v);
    };
    const std::string c = part(coins, false);
    const std::string f = part(food, true);
    if (coins && food) return fmt(tr(u8"%s et %s"), c.c_str(), f.c_str());
    if (coins) return c;
    if (food) return f;
    return tr(u8"rien");
}

std::string purse_safe(const std::string& name) {
    std::string out = name;
    for (char& c : out)
        if (c == '[') c = '(';
        else if (c == ']') c = ')';
        else if (c == '\n' || c == '\r') c = ' ';
    return out;
}

bool purse_elide(const std::string& raw) {
    const std::string name = purse_safe(raw);
    if (name.empty()) return false;
    const uint8_t c0 = (uint8_t)name[0];
    bool elide = c0 < 0x80 && strchr("AEIOUHaeiouh", (char)c0) != nullptr;
    // Un y : voyelle devant une consonne (Yves), consonne devant une voyelle (Yannick).
    if ((c0 == 'Y' || c0 == 'y') && name.size() > 1)
        elide = strchr("aeiouyAEIOUY", name[1]) == nullptr && (uint8_t)name[1] < 0x80;
    // Les voyelles accentuees de Latin-1 en UTF-8 : C3 80..9C (majuscules) et
    // C3 A0..BC (minuscules), sauf les consonnes Ç/ç (87/A7), Ñ/ñ (91/B1),
    // × (97), Ø/ø gardees comme voyelles, ÷ (B7).
    if (!elide && c0 == 0xC3 && name.size() > 1) {
        const uint8_t c1 = (uint8_t)name[1] & 0xDF;     // minuscule -> majuscule (A0 -> 80)
        elide = c1 >= 0x80 && c1 <= 0x9C && c1 != 0x87 && c1 != 0x91 && c1 != 0x97 && c1 != 0x90;
    }
    return elide;
}

std::string purse_of(const std::string& raw) {
    const std::string name = purse_safe(raw);
    if (name.empty()) return "de ?";
    return (purse_elide(name) ? "d'" : "de ") + name;
}

namespace {

// Le format d'une phrase selon la bourse : la sienne (mine), celle d'un autre
// (« de %s » / « d'%s » selon purse_elide : le francais elide, les autres langues
// traduisent les deux formats pareil).
const char* pick(bool mine, const std::string& owner, const char* f_mine, const char* f_de, const char* f_elided) {
    return mine ? f_mine : purse_elide(owner) ? f_elided : f_de;
}

} // namespace

std::string purse_find_line(const std::string& cat, int64_t coins, int64_t food, bool mine,
                            const std::string& owner, bool spent) {
    const bool gain = coins > 0 || food > 0;
    const bool loss = coins < 0 || food < 0;
    const std::string who  = cat.empty() ? std::string(tr(u8"Un chat")) : purse_safe(cat);
    const std::string name = owner.empty() ? std::string("?") : purse_safe(owner);
    const std::string got  = purse_amount_text(coins > 0 ? coins : 0, food > 0 ? food : 0);
    const std::string lost = purse_amount_text(coins < 0 ? -coins : 0, food < 0 ? -food : 0);
    // Les formats « ta bourse » ont un %s de moins : le nom du joueur en trop est ignore.
    if (gain && loss)
        return fmt(pick(mine, name, tr(u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse."),
                        tr(u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s."),
                        tr(u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.")),
                   who.c_str(), got.c_str(), lost.c_str(), name.c_str());
    if (gain)
        return fmt(pick(mine, name, tr(u8"%s a trouvé %s : c'est pour ta bourse."),
                        tr(u8"%s a trouvé %s : c'est pour la bourse de %s."),
                        tr(u8"%s a trouvé %s : c'est pour la bourse d'%s.")),
                   who.c_str(), got.c_str(), name.c_str());
    if (loss && spent)
        return fmt(pick(mine, name, tr(u8"%s a dépensé %s : c'est pris sur ta bourse."),
                        tr(u8"%s a dépensé %s : c'est pris sur la bourse de %s."),
                        tr(u8"%s a dépensé %s : c'est pris sur la bourse d'%s.")),
                   who.c_str(), lost.c_str(), name.c_str());
    if (loss)
        return fmt(pick(mine, name, tr(u8"%s a perdu %s : c'est pris sur ta bourse."),
                        tr(u8"%s a perdu %s : c'est pris sur la bourse de %s."),
                        tr(u8"%s a perdu %s : c'est pris sur la bourse d'%s.")),
                   who.c_str(), lost.c_str(), name.c_str());
    return fmt(tr(u8"%s n'a rien trouvé."), who.c_str());
}

std::string purse_shop_line(const std::string& what, int64_t coins, int64_t food, bool mine,
                            const std::string& owner) {
    (void)food;
    const std::string thing = what.empty() ? std::string(tr(u8"un objet")) : purse_safe(what);
    const std::string who   = purse_safe(owner);
    if (coins < 0) {
        const std::string price = purse_amount_text(-coins, 0);
        return mine ? fmt(tr(u8"Tu as acheté %s pour %s : c'est pris sur ta bourse."), thing.c_str(), price.c_str())
                    : fmt(tr(u8"%s a acheté %s pour %s : c'est pris sur sa bourse."), who.c_str(), thing.c_str(),
                          price.c_str());
    }
    if (coins > 0) {
        const std::string more = purse_amount_text(coins, 0);
        return mine ? fmt(tr(u8"Tu as acheté %s : %s de plus pour ta bourse."), thing.c_str(), more.c_str())
                    : fmt(tr(u8"%s a acheté %s : %s de plus pour sa bourse."), who.c_str(), thing.c_str(),
                          more.c_str());
    }
    return mine ? fmt(tr(u8"Tu as acheté %s (gratuit)."), thing.c_str())
                : fmt(tr(u8"%s a acheté %s (gratuit)."), who.c_str(), thing.c_str());
}

std::string purse_battle_line(int64_t coins, int64_t food, bool mine, const std::string& owner) {
    const bool gain = coins > 0 || food > 0;
    const bool loss = coins < 0 || food < 0;
    const std::string name = owner.empty() ? std::string("?") : purse_safe(owner);
    const std::string got  = purse_amount_text(coins > 0 ? coins : 0, food > 0 ? food : 0);
    const std::string lost = purse_amount_text(coins < 0 ? -coins : 0, food < 0 ? -food : 0);
    if (mine) {
        if (gain && loss)
            return fmt(tr(u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse."), got.c_str(),
                       lost.c_str());
        if (gain) return fmt(tr(u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse."), got.c_str());
        if (loss) return fmt(tr(u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse."), lost.c_str());
        return tr(u8"En combat, tes chats n'ont rien trouvé.");
    }
    const bool el = purse_elide(name);
    if (gain && loss)
        return fmt(el ? tr(u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.")
                      : tr(u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse."),
                   name.c_str(), got.c_str(), lost.c_str());
    if (gain)
        return fmt(el ? tr(u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.")
                      : tr(u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse."),
                   name.c_str(), got.c_str());
    if (loss)
        return fmt(el ? tr(u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.")
                      : tr(u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse."),
                   name.c_str(), lost.c_str());
    return fmt(el ? tr(u8"En combat, les chats d'%s n'ont rien trouvé.") : tr(u8"En combat, les chats de %s n'ont rien trouvé."),
               name.c_str());
}

} // namespace mgmp
