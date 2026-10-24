// mm_gift_core -- Mewgenics Multi : les dons entre joueurs, la partie portable.
// Voir mm_gift_core.h.

#include "mm_gift_core.h"
#include "mm_purse_core.h"   // purse_of, purse_safe
#include "mm_lang.h"

#include <algorithm>
#include <cstdarg>
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

namespace {

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

} // namespace

bool gift_elide(const std::string& name) { return purse_of(name).compare(0, 2, "d'") == 0; }

std::string gift_list_text(int64_t coins, int64_t food, const std::vector<std::string>& labels) {
    std::vector<std::string> parts;
    if (coins > 0) parts.push_back(fmt(coins > 1 ? tr(u8"%lld pièces") : tr(u8"%lld pièce"), (long long)coins));
    if (food > 0)
        parts.push_back(fmt(food > 1 ? tr(u8"%lld nourritures") : tr(u8"%lld nourriture"), (long long)food));
    for (const std::string& l : labels)
        if (!l.empty()) parts.push_back(l);
    if (parts.empty()) return tr(u8"rien");
    std::string s = parts[0];
    for (size_t i = 1; i < parts.size(); ++i)
        s = fmt(i + 1 == parts.size() ? tr(u8"%s et %s") : tr(u8"%s, %s"), s.c_str(), parts[i].c_str());
    return s;
}

std::string gift_why_text(uint8_t why, const std::string& raw) {
    const std::string name = purse_safe(raw.empty() ? std::string("?") : raw);
    const char*       n    = name.c_str();
    switch (why) {
    case kGiftWhyNotHome: return fmt(tr(u8"%s n'est pas dans sa maison"), n);
    case kGiftWhyBusy:    return fmt(tr(u8"une fenêtre ou l'écran des objets est resté ouvert chez %s"), n);
    case kGiftWhyFailed:
        return fmt(gift_elide(name) ? tr(u8"le jeu d'%s n'a pas pu les ranger") : tr(u8"le jeu de %s n'a pas pu les ranger"),
                   n);
    case kGiftWhyOff:     return fmt(tr(u8"les dons ne marchent pas chez %s"), n);
    case kGiftWhyGone:    return fmt(tr(u8"%s n'est plus dans la partie"), n);
    case kGiftWhyRun:
        return fmt(tr(u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)"), n);
    default:              return tr(u8"raison inconnue");
    }
}

std::string gift_received_text(const std::string& giver, const std::string& list, bool saved) {
    const std::string name = purse_safe(giver.empty() ? std::string("?") : giver);
    return fmt(gift_elide(name) ? tr(u8"Cadeau d'%s") : tr(u8"Cadeau de %s"), name.c_str()) + "\n\n" +
           fmt(tr(u8"%s t'a donné %s."), name.c_str(), list.c_str()) + "\n" +
           (saved ? tr(u8"C'est dans ta maison, et ta save est à jour.") : tr(u8"C'est dans ta maison."));
}

std::string gift_done_text(const std::string& target, const std::string& list, size_t kept, bool late) {
    const std::string name = purse_safe(target.empty() ? std::string("?") : target);
    std::string s = fmt(late ? tr(u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.")
                             : tr(u8"%s a bien reçu %s : c'est parti de ta maison."),
                        name.c_str(), list.c_str());
    if (kept == 1)
        s += std::string(" ") + tr(u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)");
    else if (kept > 1)
        s += " " + fmt(tr(u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)"), (unsigned)kept);
    return s;
}

std::string gift_refused_text(const std::string& target, uint8_t why) {
    const std::string name = purse_safe(target.empty() ? std::string("?") : target);
    return fmt(tr(u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison."), name.c_str(),
               gift_why_text(why, name).c_str());
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

// --- « Échanger » dans la boite du jeu --------------------------------------------------

int64_t trade_amount_key(int64_t value, int key, int64_t max) {
    if (key == kTradeKeyAll) return gift_clamp(max, max);
    value = gift_clamp(value, kGiftMaxMoney);
    if (key == kTradeKeyBack) value /= 10;
    else if (key >= 0 && key <= 9) value = value * 10 + key;
    return gift_clamp(value, max);
}

std::string trade_fold(const std::string& s) {
    // U+00C0 a U+00FF (C3 80 a C3 BF en UTF-8) : leur lettre de base.
    static const char kLatin1[] = "aaaaaaaceeeeiiiidnooooo*ouuuuyts"
                                  "aaaaaaaceeeeiiiidnooooo*ouuuuyty";
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        const uint8_t c = (uint8_t)s[i];
        const uint8_t d = i + 1 < s.size() ? (uint8_t)s[i + 1] : 0;
        if (c >= 'A' && c <= 'Z') {
            out += (char)(c + 32);
        } else if (c == 0xC3 && (d & 0xC0) == 0x80) {
            out += kLatin1[d - 0x80];
            ++i;
        } else if (c == 0xC5 && (d == 0x92 || d == 0x93)) {   // Œ, œ
            out += "oe";
            ++i;
        } else {
            out += (char)c;
        }
    }
    return out;
}

std::vector<TradeLine> trade_lines(const TradeList& l) {
    std::vector<TradeLine> all;
    std::map<std::string, size_t> at;
    for (const std::string& s : l.labels) {
        const auto it = at.find(s);
        if (it != at.end()) {
            ++all[it->second].have;
            continue;
        }
        at.emplace(s, all.size());
        TradeLine t;
        t.label = s;
        t.have  = 1;
        all.push_back(std::move(t));
    }
    const std::string f = trade_fold(l.filter);
    std::vector<TradeLine> out;
    for (TradeLine& t : all) {
        if (!f.empty() && trade_fold(t.label).find(f) == std::string::npos) continue;
        const auto p = l.picked.find(t.label);
        t.picked = p == l.picked.end() ? 0 : std::min(p->second, t.have);
        out.push_back(std::move(t));
    }
    return out;
}

size_t trade_picked(const TradeList& l) {
    size_t n = 0;
    for (const auto& kv : l.picked) n += kv.second;
    return n;
}

void trade_list_fit(TradeList& l) {
    std::map<std::string, size_t> have;
    for (const std::string& s : l.labels) ++have[s];
    size_t total = 0;
    for (auto it = l.picked.begin(); it != l.picked.end();) {
        const auto h = have.find(it->first);
        const size_t n = std::min(h == have.end() ? 0 : std::min(it->second, h->second), kGiftMaxItems - total);
        total += n;
        if (n) {
            it->second = n;
            ++it;
        } else {
            it = l.picked.erase(it);
        }
    }
    const size_t lines = trade_lines(l).size();
    if (l.cursor >= lines) l.cursor = lines ? lines - 1 : 0;
}

void trade_list_key(TradeList& l, TradeListKey k, size_t page) {
    const std::vector<TradeLine> lines = trade_lines(l);
    if (lines.empty()) return;
    if (!page) page = 1;
    size_t& c = l.cursor;
    if (c >= lines.size()) c = lines.size() - 1;
    const TradeLine& t   = lines[c];
    const bool       one = t.picked < t.have && trade_picked(l) < kGiftMaxItems;   // un de plus possible
    switch (k) {
    case TradeListKey::Up:       if (c) --c; break;
    case TradeListKey::Down:     if (c + 1 < lines.size()) ++c; break;
    case TradeListKey::PageUp:   c = c >= page ? c - page : 0; break;
    case TradeListKey::PageDown: c = std::min(c + page, lines.size() - 1); break;
    case TradeListKey::More:     if (one) l.picked[t.label] = t.picked + 1; break;
    case TradeListKey::Less:
        if (t.picked > 1) l.picked[t.label] = t.picked - 1;
        else l.picked.erase(t.label);
        break;
    case TradeListKey::Toggle:
        if (one) l.picked[t.label] = t.picked + 1;
        else l.picked.erase(t.label);
        break;
    }
}

std::vector<size_t> trade_list_chosen(const TradeList& l) {
    std::map<std::string, size_t> left = l.picked;
    std::vector<size_t> out;
    for (size_t i = 0; i < l.labels.size() && out.size() < kGiftMaxItems; ++i) {
        const auto it = left.find(l.labels[i]);
        if (it == left.end() || !it->second) continue;
        --it->second;
        out.push_back(i);
    }
    return out;
}

std::string trade_list_text(const TradeList& l, size_t page) {
    if (l.labels.empty()) return tr(u8"Rien à donner dans ton rangement.");
    if (!page) page = 1;
    std::string s = l.filter.empty() ? std::string(tr(u8"Tape des lettres pour chercher un objet."))
                                     : fmt(tr(u8"Recherche : %s_"), purse_safe(l.filter).c_str());
    s += "\n\n";
    const std::vector<TradeLine> lines = trade_lines(l);
    if (lines.empty()) return s + tr(u8"Aucun objet ne correspond.");
    const size_t cur   = std::min(l.cursor, lines.size() - 1);
    const size_t first = cur / page * page;
    for (size_t i = first; i < lines.size() && i < first + page; ++i) {
        const TradeLine& t = lines[i];
        // Pas de [ ] : le jeu les prendrait pour des balises de couleur.
        s += i == cur ? "> " : "    ";
        s += !t.picked ? std::string("( ) ") : t.have == 1 ? std::string("(x) ") : "(" + std::to_string(t.picked) + ") ";
        s += purse_safe(t.label);
        if (t.have > 1) s += " x" + std::to_string(t.have);
        s += "\n";
    }
    const size_t pages = (lines.size() + page - 1) / page;
    if (pages > 1) s += fmt(tr(u8"(page %u sur %u)"), (unsigned)(first / page + 1), (unsigned)pages);
    else s.pop_back();
    return s;
}

} // namespace mgmp
