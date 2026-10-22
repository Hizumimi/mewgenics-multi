// mm_shop_core -- Mewgenics Multi : les achats en boutique avec sa bourse, la
// partie portable. Voir mm_shop_core.h.

#include "mm_shop_core.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace mgmp {
namespace {

constexpr size_t kMaxName = kShopMaxName;
constexpr size_t kMaxText = kShopMaxText;

template <typename T> void put(std::vector<uint8_t>& b, T v) {
    const uint8_t* q = (const uint8_t*)&v;
    b.insert(b.end(), q, q + sizeof(T));
}

// Une chaine, coupee a `max` octets sans casser un caractere UTF-8.
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

const char* coins_word(long long n) { return (n > 1 || n < -1) ? u8"pièces" : u8"pièce"; }

} // namespace

// --- le message ------------------------------------------------------------------------------

std::vector<uint8_t> shop_encode(const ShopMsg& m) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamShop);
    put<uint8_t>(b, m.op);
    switch (m.op) {
    case kShopBuy:
        put<uint64_t>(b, m.node);
        put<uint16_t>(b, m.index);
        put<uint64_t>(b, m.digest);
        put<int32_t>(b, m.price);
        break;
    case kShopDone:
        put<uint64_t>(b, m.node);
        put<uint16_t>(b, m.index);
        put<uint8_t>(b, (uint8_t)m.result);
        put<int32_t>(b, m.price);
        put<int32_t>(b, m.purse);
        break;
    case kShopSold:
        put<uint64_t>(b, m.node);
        put<uint16_t>(b, m.index);
        put<uint64_t>(b, m.digest);
        put<int32_t>(b, m.price);
        put<uint8_t>(b, m.buyer);
        put<int32_t>(b, m.coins_before);
        put<int32_t>(b, m.food_before);
        put<int32_t>(b, m.coins_after);
        put<int32_t>(b, m.food_after);
        put_str(b, m.label, kShopMaxLabel);
        break;
    case kShopGive: {
        put<uint8_t>(b, m.seq);
        put<uint64_t>(b, m.gift);
        const size_t n = std::min(m.items.size(), kShopMaxItems);
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
            put_str(b, m.items[i].label, kShopMaxLabel);
        }
        break;
    }
    case kShopGot:
        put<uint64_t>(b, m.gift);
        put<uint64_t>(b, m.kept);
        break;
    case kShopHere:
        put<uint64_t>(b, m.node);
        put<uint8_t>(b, m.open ? 1 : 0);
        break;
    default:
        break;
    }
    return b;
}

bool shop_decode(const uint8_t* p, size_t n, ShopMsg& out) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat || r.get<uint8_t>() != kTeamShop)
        return false;
    ShopMsg m;
    m.op = r.get<uint8_t>();
    if (!r.ok) return false;
    switch (m.op) {
    case kShopBuy:
        m.node   = r.get<uint64_t>();
        m.index  = r.get<uint16_t>();
        m.digest = r.get<uint64_t>();
        m.price  = r.get<int32_t>();
        if (!r.ok || m.node == 0 || m.price < 0) return false;
        break;
    case kShopDone: {
        m.node  = r.get<uint64_t>();
        m.index = r.get<uint16_t>();
        const uint8_t res = r.get<uint8_t>();
        m.price = r.get<int32_t>();
        m.purse = r.get<int32_t>();
        if (!r.ok || m.node == 0 || res > kShopResultLast) return false;
        m.result = (ShopResult)res;
        break;
    }
    case kShopSold:
        m.node   = r.get<uint64_t>();
        m.index  = r.get<uint16_t>();
        m.digest = r.get<uint64_t>();
        m.price  = r.get<int32_t>();
        m.buyer  = r.get<uint8_t>();
        m.coins_before = r.get<int32_t>();
        m.food_before  = r.get<int32_t>();
        m.coins_after  = r.get<int32_t>();
        m.food_after   = r.get<int32_t>();
        if (!r.ok || m.node == 0 || m.price < 0 || m.coins_before < 0 || m.food_before < 0 ||
            m.coins_after < 0 || m.food_after < 0)
            return false;
        if (!r.str(m.label, kShopMaxLabel)) return false;
        break;
    case kShopGive: {
        m.seq  = r.get<uint8_t>();
        m.gift = r.get<uint64_t>();
        const uint8_t count = r.get<uint8_t>();
        if (!r.ok || m.gift == 0 || count > kShopMaxItems) return false;
        for (uint8_t i = 0; i < count; ++i) {
            ShopGiveItem it;
            if (!r.str(it.f.name, kMaxName) || !r.str(it.f.text, kMaxText)) return false;
            it.f.a = r.get<uint32_t>();
            it.f.b = r.get<uint32_t>();
            it.f.c = r.get<uint32_t>();
            it.f.d = r.get<uint32_t>();
            it.f.e = r.get<uint8_t>();
            it.f.f = r.get<uint32_t>();
            if (!r.ok || it.f.name.empty()) return false;
            if (!r.str(it.label, kShopMaxLabel)) return false;
            m.items.push_back(std::move(it));
        }
        break;
    }
    case kShopGot:
        m.gift = r.get<uint64_t>();
        m.kept = r.get<uint64_t>();
        if (!r.ok || m.gift == 0) return false;
        break;
    case kShopHere:
        m.node = r.get<uint64_t>();
        m.open = r.get<uint8_t>();
        if (!r.ok || m.node == 0 || m.open > 1) return false;
        break;
    default:
        return false;
    }
    if (!r.ok || r.pos != n) return false;
    out = std::move(m);
    return true;
}

// --- les regles ------------------------------------------------------------------------------

uint64_t shop_item_digest(int32_t type, int32_t price, int32_t amount, uint64_t equip_digest) {
    uint64_t h = 0xcbf29ce484222325ull;
    auto mix = [&](uint64_t v) {
        for (int i = 0; i < 8; ++i) {
            h ^= (v >> (8 * i)) & 0xFF;
            h *= 0x100000001b3ull;
        }
    };
    mix((uint32_t)type);
    mix((uint32_t)price);
    mix((uint32_t)amount);
    mix(equip_digest);
    return h ? h : 1;
}

bool shop_guest_type(int32_t type) {
    return type == kShopTypeItem || type == kShopTypeFood || type == kShopTypeCoins;
}

ShopResult shop_check(const ShopMsg& req, const ShopLook& look, bool mixed, bool has_purse, int64_t purse) {
    if (!mixed) return ShopResult::NotRun;
    if (!has_purse) return ShopResult::NoPurse;
    if (!look.shop_here) return ShopResult::NoShop;
    if (!look.present || look.digest != req.digest) return ShopResult::NoItem;
    if (!shop_guest_type(look.type)) return ShopResult::NotItem;
    if (look.stock <= 0) return ShopResult::SoldOut;
    if (purse < look.price) return ShopResult::Poor;
    return ShopResult::Bought;
}

ShopResult shop_host_check(bool has_purse, int64_t purse, int32_t price) {
    if (price <= 0) return ShopResult::Bought;        // gratuit : rien a payer
    if (!has_purse) return ShopResult::NoPurse;
    if (purse < price) return ShopResult::Poor;
    return ShopResult::Bought;
}

std::string shop_wait_text(const std::string& names, bool several) {
    if (several)
        return u8"Boutique\n\n" + names + u8" n'ont pas fini dans cette\n"
               u8"boutique : attends-les (une fois sorti, tu ne\n"
               u8"pourrais plus acheter pour eux).\n"
               u8"Clique encore pour sortir quand même.";
    return u8"Boutique\n\n" + names + u8" n'a pas fini dans cette\n"
           u8"boutique : attends-le (une fois sorti, tu ne\n"
           u8"pourrais plus acheter pour lui).\n"
           u8"Clique encore pour sortir quand même.";
}

std::string shop_local_failure_text() {
    return u8"Boutique\n\nTon achat n'a pas pu être demandé à l'hôte :\nrien n'a été acheté. Réessaie dans un instant.";
}

std::string shop_refusal_text(ShopResult r, bool host, int32_t price, int64_t purse) {
    const std::string title = u8"Boutique\n\n";
    switch (r) {
    case ShopResult::Bought:
        return "";
    case ShopResult::NoShop:
        return title + u8"L'hôte n'est plus dans cette boutique :\nl'achat ne s'est pas fait.";
    case ShopResult::NoItem:
        return title + u8"Cet objet n'est plus à vendre chez l'hôte :\nl'achat ne s'est pas fait.";
    case ShopResult::SoldOut:
        return title + u8"Cet objet vient d'être vendu :\nl'achat ne s'est pas fait.";
    case ShopResult::Poor: {
        const long long have = purse < 0 ? 0 : (long long)purse;
        char line[160];
        snprintf(line, sizeof(line), u8"Il coûte %lld %s ; ta bourse en a %lld.", (long long)price,
                 coins_word(price), have);
        return title + u8"Ta bourse ne suffit pas pour cet objet.\n" + line +
               (host ? u8"\n(Chacun paie avec sa bourse, toi aussi.)" : u8"\n(Chacun paie avec sa bourse.)");
    }
    case ShopResult::NotItem:
        return title + u8"Pour l'instant, seul l'hôte achète les\nbonbons et les familiers (avec sa bourse).";
    case ShopResult::Failed:
        return title + u8"Le jeu n'a pas pu faire l'achat chez l'hôte.";
    case ShopResult::NotRun:
        return title + u8"Pas de bourses dans cette aventure :\nseul l'hôte achète.";
    case ShopResult::NoPurse:
        return host ? title + u8"Tu n'as pas amené de chats dans cette\naventure : tu n'as pas de bourse pour\nacheter."
                    : title + u8"Tu n'as pas de bourse dans cette aventure\n(tu n'y as pas amené de chats) : pas d'achat.";
    case ShopResult::Busy:
        return title + u8"L'hôte ne peut pas acheter pour toi en ce\nmoment (une fenêtre est ouverte chez lui) :\nrien n'a été acheté. Réessaie dans un instant.";
    }
    return "";
}

std::string shop_amount_label(int32_t type, int32_t amount) {
    const long long a = amount < 0 ? 0 : amount;
    char buf[64];
    if (type == kShopTypeFood) {
        snprintf(buf, sizeof(buf), a > 1 ? u8"%lld nourritures" : u8"%lld nourriture", a);
        return buf;
    }
    if (type == kShopTypeCoins) {
        snprintf(buf, sizeof(buf), u8"%lld %s", a, coins_word(a));
        return buf;
    }
    return "";
}

bool shop_index_of(uintptr_t begin, uintptr_t end, uintptr_t item, size_t stride, uint16_t& index) {
    if (!stride || begin > end || item < begin || item >= end) return false;
    const uintptr_t off = item - begin;
    if (off % stride) return false;
    const uintptr_t i = off / stride;
    if (i > 0xFFFF) return false;
    index = (uint16_t)i;
    return true;
}

// --- l'hote : les achats et a qui ils sont ----------------------------------------------------

void ShopLedger::add(uint64_t id, uint64_t digest, uint8_t slot, const std::string& label) {
    ShopOwned o;
    o.id     = id;
    o.digest = digest;
    o.slot  = slot;
    o.label = label;
    owned_.push_back(std::move(o));
}

int ShopLedger::owner(uint64_t id) const {
    for (const ShopOwned& o : owned_)
        if (o.id == id) return o.slot;
    return -1;
}

size_t shop_count(const std::vector<uint64_t>& ids, uint64_t id) {
    return (size_t)std::count(ids.begin(), ids.end(), id);
}

std::vector<int> shop_match_owned(const std::vector<ShopOwned>& owned, const std::vector<uint64_t>& stored) {
    std::vector<int>  out(owned.size(), -1);
    std::vector<bool> taken(stored.size(), false);
    for (size_t i = 0; i < owned.size(); ++i)
        for (size_t k = 0; k < stored.size(); ++k)
            if (!taken[k] && stored[k] == owned[i].id) {
                taken[k] = true;
                out[i]   = (int)k;
                break;
            }
    return out;
}

size_t shop_rekey_owned(std::vector<ShopOwned>& owned, const std::vector<uint64_t>& bag_ids,
                        const std::vector<uint64_t>& bag_digests) {
    // D'abord tous les achats par id : un recalage ne prend jamais l'exemplaire
    // dont un autre achat a l'id exact.
    const std::vector<int> by_id = shop_match_owned(owned, bag_ids);
    std::vector<bool> taken(bag_ids.size(), false);
    for (int k : by_id)
        if (k >= 0) taken[(size_t)k] = true;
    // shortcut: un achat disparu (equipe, utilise) peut prendre un objet identique
    // trouve ailleurs dans l'aventure ; reperer le rechargement lui-meme si ca arrive en partie.
    size_t n = 0;
    for (size_t i = 0; i < owned.size(); ++i) {
        if (by_id[i] >= 0 || !owned[i].digest) continue;
        for (size_t k = 0; k < bag_ids.size() && k < bag_digests.size(); ++k)
            if (!taken[k] && bag_digests[k] == owned[i].digest) {
                taken[k]    = true;
                owned[i].id = bag_ids[k];
                ++n;
                break;
            }
    }
    return n;
}

} // namespace mgmp
