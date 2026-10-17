// mm_npcgift_core -- Mewgenics Multi : les dons de l'invite aux PNJ, la partie
// portable. Voir mm_npcgift_core.h.

#include "mm_npcgift_core.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace mgmp {
namespace {

constexpr size_t kMaxName    = kNpcGiftMaxName;
constexpr size_t kMaxText    = kNpcGiftMaxText;
constexpr size_t kMaxLabel   = 127;    // un nom affiche (UTF-8)
constexpr size_t kMaxCatName = 95;     // le nom d'un chat (UTF-8)
constexpr size_t kMaxAbility = 63;     // le nom d'une capacite

template <typename T> void put(std::vector<uint8_t>& b, T v) {
    const uint8_t* q = (const uint8_t*)&v;
    b.insert(b.end(), q, q + sizeof(T));
}

// Coupee a `max` octets sans casser un caractere UTF-8.
void put_str(std::vector<uint8_t>& b, const std::string& s, size_t max) {
    size_t n = std::min(s.size(), max);
    if (n < s.size())
        while (n > 0 && ((uint8_t)s[n] & 0xC0) == 0x80) --n;
    put<uint16_t>(b, (uint16_t)n);
    b.insert(b.end(), s.begin(), s.begin() + (ptrdiff_t)n);
}

struct Reader {
    const uint8_t* p;
    size_t         n;
    size_t         pos = 0;
    bool           ok  = true;
    bool need(size_t k) {
        if (!ok || k > n || pos > n - k) ok = false;
        return ok;
    }
    template <typename T> T get() {
        T v{};
        if (need(sizeof(T))) { memcpy(&v, p + pos, sizeof(T)); pos += sizeof(T); }
        return v;
    }
    bool str(std::string& out, size_t max) {
        const uint16_t len = get<uint16_t>();
        if (!ok || len > max || !need(len)) { ok = false; return false; }
        out.assign((const char*)p + pos, len);
        pos += len;
        return true;
    }
};

// Un nom de capacite tel que le jeu en ecrit : lettres, chiffres, _ (pas de
// place vide : « None »).
bool ability_ok(const std::string& s) {
    if (s.empty() || s.size() > kMaxAbility) return false;
    for (char c : s)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
            return false;
    return true;
}

} // namespace

// --- le message ------------------------------------------------------------------------------

std::vector<uint8_t> npcgift_encode(const NpcGiftMsg& m) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamNpc);
    put<uint8_t>(b, m.op);
    put<uint8_t>(b, m.npc);
    switch (m.op) {
    case kNpcDonate: {
        put<uint32_t>(b, m.favor);
        put_str(b, m.cat, kMaxCatName);
        const size_t n = std::min(m.abilities.size(), kNpcGiftAbilities);
        put<uint8_t>(b, (uint8_t)n);
        for (size_t i = 0; i < n; ++i) put_str(b, m.abilities[i], kMaxAbility);
        break;
    }
    case kNpcReady:
        break;
    case kNpcGift: {
        put<uint64_t>(b, m.gift);
        put<int32_t>(b, m.coins);
        put<int32_t>(b, m.food);
        const size_t n = std::min(m.items.size(), kNpcGiftMaxItems);
        put<uint8_t>(b, (uint8_t)n);
        for (size_t i = 0; i < n; ++i) {
            const ItemFields& f = m.items[i].f;
            put_str(b, f.name, kMaxName);
            put_str(b, f.text, kMaxText);
            put<uint32_t>(b, f.a);
            put<uint32_t>(b, f.b);
            put<uint32_t>(b, f.c);
            put<uint32_t>(b, f.d);
            put<uint8_t>(b, f.e);
            put<uint32_t>(b, f.f);
            put_str(b, m.items[i].label, kMaxLabel);
        }
        break;
    }
    case kNpcGot:
        put<uint64_t>(b, m.gift);
        put<uint8_t>(b, m.money ? 1 : 0);
        put<uint64_t>(b, m.kept);
        break;
    default:
        break;
    }
    return b;
}

bool npcgift_decode(const uint8_t* p, size_t n, NpcGiftMsg& out) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat || r.get<uint8_t>() != kTeamNpc)
        return false;
    NpcGiftMsg m;
    m.op  = r.get<uint8_t>();
    m.npc = r.get<uint8_t>();
    if (!r.ok || m.npc >= kNpcGiftNpcs) return false;
    switch (m.op) {
    case kNpcDonate: {
        m.favor = r.get<uint32_t>();
        if (!r.ok || m.favor == 0 || m.favor > kNpcGiftMaxFavor) return false;
        if (!r.str(m.cat, kMaxCatName)) return false;
        const uint8_t count = r.get<uint8_t>();
        if (!r.ok || count > kNpcGiftAbilities) return false;
        for (uint8_t i = 0; i < count; ++i) {
            std::string a;
            if (!r.str(a, kMaxAbility)) return false;
            m.abilities.push_back(std::move(a));
        }
        break;
    }
    case kNpcReady:
        break;
    case kNpcGift: {
        m.gift  = r.get<uint64_t>();
        m.coins = r.get<int32_t>();
        m.food  = r.get<int32_t>();
        if (!r.ok || m.coins < 0 || m.food < 0) return false;
        const uint8_t count = r.get<uint8_t>();
        if (!r.ok || count > kNpcGiftMaxItems) return false;
        for (uint8_t i = 0; i < count; ++i) {
            NpcGiftItem it;
            ItemFields& f = it.f;
            if (!r.str(f.name, kMaxName) || !r.str(f.text, kMaxText)) return false;
            f.a = r.get<uint32_t>();
            f.b = r.get<uint32_t>();
            f.c = r.get<uint32_t>();
            f.d = r.get<uint32_t>();
            f.e = r.get<uint8_t>();
            f.f = r.get<uint32_t>();
            if (!r.str(it.label, kMaxLabel)) return false;
            if (!r.ok || f.name.empty()) return false;
            m.items.push_back(std::move(it));
        }
        break;
    }
    case kNpcGot:
        m.gift  = r.get<uint64_t>();
        m.money = r.get<uint8_t>();
        m.kept  = r.get<uint64_t>();
        if (r.ok && m.money > 1) return false;
        break;
    default:
        return false;
    }
    if (!r.ok || r.pos != n) return false;
    out = std::move(m);
    return true;
}

// --- ce que l'hote donne au jeu ------------------------------------------------------------

std::vector<std::string> npcgift_abilities(const std::vector<std::string>& in) {
    std::vector<std::string> out(kNpcGiftAbilities, "None");
    for (size_t i = 0; i < kNpcGiftAbilities && i < in.size(); ++i)
        if (ability_ok(in[i])) out[i] = in[i];
    return out;
}

std::vector<size_t> npcgift_new_items(const std::vector<NpcStored>& before, const std::vector<NpcStored>& after) {
    std::vector<size_t> out;
    for (size_t i = 0; i < after.size(); ++i) {
        bool had = false;
        for (const NpcStored& b : before)
            if (b.id == after[i].id) { had = true; break; }
        if (!had) out.push_back(i);
    }
    return out;
}

std::string npcgift_list_text(int64_t coins, int64_t food, const std::vector<std::string>& labels) {
    std::vector<std::string> parts;
    char b[64] = {};
    if (coins > 0) {
        snprintf(b, sizeof(b), coins > 1 ? u8"%lld pièces" : u8"%lld pièce", (long long)coins);
        parts.push_back(b);
    }
    if (food > 0) {
        snprintf(b, sizeof(b), food > 1 ? u8"%lld nourritures" : u8"%lld nourriture", (long long)food);
        parts.push_back(b);
    }
    for (const std::string& l : labels)
        if (!l.empty()) parts.push_back(l);
    if (parts.empty()) return "rien";
    std::string s = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) s += (i + 1 == parts.size() ? " et " : ", ") + parts[i];
    return s;
}

// --- a qui est la recompense ----------------------------------------------------------------

bool npcgift_after_donation(NpcDonor& d, bool was_ready, bool now_ready, uint8_t from, const std::string& name,
                            const std::string& reward) {
    if (was_ready || !now_ready) return false;     // pas ce don-ci qui l'a remplie
    NpcDonor fresh;
    if (from != kNpcNoDonor) {
        fresh.peer   = from;
        fresh.name   = name;
        fresh.reward = reward;
    }
    const bool changed = fresh.peer != d.peer || fresh.name != d.name || fresh.reward != d.reward;
    d = fresh;
    return changed;
}

uint8_t npcgift_reward_owner(const NpcDonor& d, const std::string& reward_now) {
    if (d.peer == kNpcNoDonor) return kNpcNoDonor;
    if (d.reward != reward_now) return kNpcNoDonor;   // ce n'est plus la recompense de son don
    return d.peer;
}

} // namespace mgmp
