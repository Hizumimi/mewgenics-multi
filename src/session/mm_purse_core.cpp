// mm_purse_core.cpp -- voir mm_purse_core.h.
#include "mm_purse_core.h"
#include "mgmp_proto.h"

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

std::string purse_amount_text(int64_t coins, int64_t food) {
    if (coins < 0) coins = 0;
    if (food < 0) food = 0;
    char c[48] = {}, f[48] = {};
    snprintf(c, sizeof(c), coins > 1 ? u8"%lld pièces" : u8"%lld pièce", (long long)coins);
    snprintf(f, sizeof(f), food > 1 ? u8"%lld nourritures" : u8"%lld nourriture", (long long)food);
    if (coins && food) return std::string(c) + " et " + f;
    if (coins) return c;
    if (food) return f;
    return "rien";
}

std::string purse_balance_text(int64_t coins, int64_t food) {
    auto part = [](int64_t v, const char* one, const char* many) {
        char b[64] = {};
        const long long a = (long long)(v < 0 ? -v : v);
        snprintf(b, sizeof(b), "%s%lld %s", v < 0 ? "-" : "", a, a > 1 ? many : one);
        return std::string(b);
    };
    const std::string c = part(coins, u8"pièce", u8"pièces");
    const std::string f = part(food, u8"nourriture", u8"nourritures");
    if (coins && food) return c + " et " + f;
    if (coins) return c;
    if (food) return f;
    return "rien";
}

std::string purse_safe(const std::string& name) {
    std::string out = name;
    for (char& c : out)
        if (c == '[') c = '(';
        else if (c == ']') c = ')';
        else if (c == '\n' || c == '\r') c = ' ';
    return out;
}

std::string purse_of(const std::string& raw) {
    const std::string name = purse_safe(raw);
    if (name.empty()) return "de ?";
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
    return (elide ? "d'" : "de ") + name;
}

std::string purse_find_line(const std::string& cat, int64_t coins, int64_t food, bool mine,
                            const std::string& owner, bool spent) {
    const bool gain = coins > 0 || food > 0;
    const bool loss = coins < 0 || food < 0;
    const std::string who   = cat.empty() ? std::string(u8"Un chat") : purse_safe(cat);
    const std::string purse = mine ? std::string("ta bourse") : "la bourse " + purse_of(owner);
    const std::string got   = purse_amount_text(coins > 0 ? coins : 0, food > 0 ? food : 0);
    const std::string lost  = purse_amount_text(coins < 0 ? -coins : 0, food < 0 ? -food : 0);
    if (gain && loss)
        return who + u8" a trouvé " + got + u8" et perdu " + lost + u8" : ça compte pour " + purse + ".";
    if (gain) return who + u8" a trouvé " + got + u8" : c'est pour " + purse + ".";
    if (loss) return who + (spent ? u8" a dépensé " : u8" a perdu ") + lost + u8" : c'est pris sur " + purse + ".";
    return who + u8" n'a rien trouvé.";
}

std::string purse_shop_line(const std::string& what, int64_t coins, int64_t food, bool mine,
                            const std::string& owner) {
    (void)food;
    const std::string thing = what.empty() ? std::string(u8"un objet") : purse_safe(what);
    const std::string who   = mine ? std::string(u8"Tu as acheté ") : purse_safe(owner) + u8" a acheté ";
    const std::string purse = mine ? std::string("ta bourse") : std::string("sa bourse");
    if (coins < 0)
        return who + thing + u8" pour " + purse_amount_text(-coins, 0) + u8" : c'est pris sur " + purse + ".";
    if (coins > 0)
        return who + thing + u8" : " + purse_amount_text(coins, 0) + u8" de plus pour " + purse + ".";
    return who + thing + u8" (gratuit).";
}

std::string purse_battle_line(int64_t coins, int64_t food, bool mine, const std::string& owner) {
    const bool gain = coins > 0 || food > 0;
    const bool loss = coins < 0 || food < 0;
    const std::string who   = mine ? std::string(u8"En combat, tes chats") : u8"En combat, les chats " + purse_of(owner);
    const std::string purse = mine ? std::string("ta bourse") : std::string("sa bourse");
    const std::string got   = purse_amount_text(coins > 0 ? coins : 0, food > 0 ? food : 0);
    const std::string lost  = purse_amount_text(coins < 0 ? -coins : 0, food < 0 ? -food : 0);
    if (gain && loss)
        return who + u8" ont trouvé " + got + u8" et perdu " + lost + u8" : ça compte pour " + purse + ".";
    if (gain) return who + u8" ont trouvé " + got + u8" : c'est pour " + purse + ".";
    if (loss) return who + u8" ont perdu " + lost + u8" : c'est pris sur " + purse + ".";
    return who + u8" n'ont rien trouvé.";
}

} // namespace mgmp
