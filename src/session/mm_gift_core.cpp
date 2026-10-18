// mm_gift_core -- Mewgenics Multi : les dons entre joueurs, la partie portable.
// Voir mm_gift_core.h.

#include "mm_gift_core.h"
#include "mm_purse_core.h"   // purse_of, purse_safe

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace mgmp {
namespace {

constexpr uint32_t kMaxStorageCount = 100000;      // comme team_apply_loot
constexpr size_t   kMaxStorageBytes = 8u << 20;

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

} // namespace

// --- le message -----------------------------------------------------------------------------

std::vector<uint8_t> gift_encode(const GiftMsg& m) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamGift);
    put<uint8_t>(b, m.op);
    put<uint32_t>(b, m.id);
    put<uint8_t>(b, m.giver);
    put<uint8_t>(b, m.target);
    switch (m.op) {
    case kGiftOffer: {
        put<int32_t>(b, std::max<int32_t>(0, std::min(m.coins, kGiftMaxMoney)));
        put<int32_t>(b, std::max<int32_t>(0, std::min(m.food, kGiftMaxMoney)));
        const size_t n = std::min(m.items.size(), kGiftMaxItems);
        put<uint8_t>(b, (uint8_t)n);
        for (size_t i = 0; i < n; ++i) {
            const ItemFields& f = m.items[i].f;
            put_str(b, f.name, kGiftMaxName);
            put_str(b, f.text, kGiftMaxText);
            put<uint32_t>(b, f.a);
            put<uint32_t>(b, f.b);
            put<uint32_t>(b, f.c);
            put<uint32_t>(b, f.d);
            put<uint8_t>(b, f.e);
            put<uint32_t>(b, f.f);
            put_str(b, m.items[i].label, kGiftMaxLabel);
        }
        put_str(b, m.giver_name, kGiftMaxLabel);
        break;
    }
    case kGiftGot:
        put_str(b, m.receiver_name, kGiftMaxLabel);
        break;
    case kGiftRefused:
        put<uint8_t>(b, m.why);
        break;
    default:
        break;
    }
    return b;
}

bool gift_decode(const uint8_t* p, size_t n, GiftMsg& out) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat || r.get<uint8_t>() != kTeamGift)
        return false;
    GiftMsg m;
    m.op     = r.get<uint8_t>();
    m.id     = r.get<uint32_t>();
    m.giver  = r.get<uint8_t>();
    m.target = r.get<uint8_t>();
    if (!r.ok || m.giver == m.target) return false;
    switch (m.op) {
    case kGiftOffer: {
        m.coins = r.get<int32_t>();
        m.food  = r.get<int32_t>();
        if (!r.ok || m.coins < 0 || m.food < 0 || m.coins > kGiftMaxMoney || m.food > kGiftMaxMoney) return false;
        const uint8_t count = r.get<uint8_t>();
        if (!r.ok || count > kGiftMaxItems) return false;
        for (uint8_t i = 0; i < count; ++i) {
            GiftItem it;
            ItemFields& f = it.f;
            if (!r.str(f.name, kGiftMaxName) || !r.str(f.text, kGiftMaxText)) return false;
            f.a = r.get<uint32_t>();
            f.b = r.get<uint32_t>();
            f.c = r.get<uint32_t>();
            f.d = r.get<uint32_t>();
            f.e = r.get<uint8_t>();
            f.f = r.get<uint32_t>();
            if (!r.str(it.label, kGiftMaxLabel)) return false;
            if (!r.ok || f.name.empty()) return false;
            m.items.push_back(std::move(it));
        }
        if (!r.str(m.giver_name, kGiftMaxLabel)) return false;
        // Un don vide ne part pas (la fenetre ne le permet pas) : refuse ici aussi.
        if (!m.coins && !m.food && m.items.empty()) return false;
        break;
    }
    case kGiftGot:
        if (!r.str(m.receiver_name, kGiftMaxLabel)) return false;
        break;
    case kGiftRefused:
        m.why = r.get<uint8_t>();
        if (!r.ok || m.why == 0 || m.why > kGiftWhyLast) return false;
        break;
    default:
        return false;
    }
    if (!r.ok || r.pos != n) return false;
    out = std::move(m);
    return true;
}

// --- le rangement -----------------------------------------------------------------------------

bool gift_storage_items(const std::vector<uint8_t>& blob, std::vector<ItemFields>& out) {
    out.clear();
    if (blob.empty()) return true;
    if (blob.size() < 4 || blob.size() > kMaxStorageBytes) return false;
    uint32_t count = 0;
    memcpy(&count, blob.data(), 4);
    if (count > kMaxStorageCount) return false;
    size_t pos = 4;
    for (uint32_t i = 0; i < count; ++i) {
        ItemFields f;
        if (!team_item_parse(blob.data(), blob.size(), pos, f)) return false;
        out.push_back(std::move(f));
    }
    return pos == blob.size();
}

bool gift_storage_append(std::vector<uint8_t>& blob, const std::vector<ItemFields>& items, std::string* why) {
    if (items.empty()) return true;
    std::vector<ItemFields> have;
    if (!gift_storage_items(blob, have)) {
        if (why) *why = "rangement illisible";
        return false;
    }
    if (have.size() + items.size() > kMaxStorageCount) {
        if (why) *why = "rangement trop plein";
        return false;
    }
    std::vector<uint8_t> out = blob.size() >= 4 ? blob : std::vector<uint8_t>(4, 0);
    for (const ItemFields& it : items) {
        if (it.name.empty()) {
            if (why) *why = "objet sans nom";
            return false;
        }
        const std::vector<uint8_t> rec = team_item_record(it);
        out.insert(out.end(), rec.begin(), rec.end());
    }
    const uint32_t count = (uint32_t)(have.size() + items.size());
    memcpy(out.data(), &count, 4);
    if (out.size() > kMaxStorageBytes) {
        if (why) *why = "rangement trop gros";
        return false;
    }
    blob.swap(out);
    return true;
}

// --- les textes -------------------------------------------------------------------------------

std::string gift_list_text(int64_t coins, int64_t food, const std::vector<std::string>& labels) {
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

std::string gift_why_text(uint8_t why, const std::string& raw) {
    const std::string name = purse_safe(raw.empty() ? std::string("?") : raw);
    switch (why) {
    case kGiftWhyNotHome: return name + u8" n'est pas dans sa maison";
    case kGiftWhyBusy:    return u8"une fenêtre ou l'écran des objets est resté ouvert chez " + name;
    case kGiftWhyFailed:  return u8"le jeu " + purse_of(name) + u8" n'a pas pu les ranger";
    case kGiftWhyOff:     return u8"les dons ne marchent pas chez " + name;
    case kGiftWhyGone:    return name + u8" n'est plus dans la partie";
    case kGiftWhyRun:     return name + u8" a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)";
    default:              return u8"raison inconnue";
    }
}

std::string gift_received_text(const std::string& giver, const std::string& list, bool saved) {
    const std::string name = purse_safe(giver.empty() ? std::string("?") : giver);
    return u8"Cadeau " + purse_of(name) + u8"\n\n" + name + u8" t'a donné " + list + u8".\n" +
           (saved ? u8"C'est dans ta maison, et ta save est à jour." : u8"C'est dans ta maison.");
}

std::string gift_done_text(const std::string& target, const std::string& list, size_t kept, bool late) {
    const std::string name = purse_safe(target.empty() ? std::string("?") : target);
    std::string s = (late ? u8"Finalement, " : "") + name + u8" a bien reçu " + list + u8" : c'est parti de ta maison.";
    if (kept == 1)
        s += u8" (1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)";
    else if (kept > 1)
        s += u8" (" + std::to_string(kept) + u8" objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)";
    return s;
}

std::string gift_refused_text(const std::string& target, uint8_t why) {
    const std::string name = purse_safe(target.empty() ? std::string("?") : target);
    return u8"Le don à " + name + u8" n'est pas passé : " + gift_why_text(why, name) + u8". Rien n'a quitté ta maison.";
}

GiftStep gift_receiver_step(bool at_home, bool busy, uint64_t waited_ms, uint8_t& why) {
    why = 0;
    if (!at_home) {
        why = kGiftWhyNotHome;
        return GiftStep::Refuse;
    }
    if (!busy) return GiftStep::Apply;
    if (waited_ms < kGiftWaitMs) return GiftStep::Wait;
    why = kGiftWhyBusy;
    return GiftStep::Refuse;
}

int32_t gift_clamp(int64_t want, int64_t have) {
    if (want <= 0 || have <= 0) return 0;
    int64_t v = std::min(want, have);
    if (v > kGiftMaxMoney) v = kGiftMaxMoney;
    return (int32_t)v;
}

} // namespace mgmp
