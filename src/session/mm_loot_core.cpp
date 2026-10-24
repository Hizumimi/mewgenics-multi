// mm_loot_core -- Mewgenics Multi : le defile du butin, la partie portable. Voir
// mm_loot_core.h.

#include "mm_loot_core.h"
#include "mm_lang.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace mgmp {
namespace {

constexpr size_t kMaxName = 63;     // une cle GON d'objet
constexpr size_t kMaxText = 255;    // le texte d'un objet (un livre : sa capacite)

template <typename T> void put(std::vector<uint8_t>& b, T v) {
    const uint8_t* q = (const uint8_t*)&v;
    b.insert(b.end(), q, q + sizeof(T));
}

void put_str(std::vector<uint8_t>& b, const std::string& s, size_t max) {
    const size_t n = std::min(s.size(), max);
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

int popcount32(uint32_t v) {
    int n = 0;
    for (; v; v &= v - 1) ++n;
    return n;
}

bool valid_choice(uint8_t c) { return c == kLootPass || c == kLootGreed || c == kLootNeed; }

} // namespace

// --- le message ------------------------------------------------------------------------------

std::vector<uint8_t> loot_encode(const LootMsg& m) {
    std::vector<uint8_t> b;
    put<uint32_t>(b, kTeamMagic);
    put<uint8_t>(b, kTeamFormat);
    put<uint8_t>(b, kTeamLoot);
    put<uint8_t>(b, m.op);
    put<uint32_t>(b, m.parade);
    switch (m.op) {
    case kLootStart: {
        put<uint8_t>(b, m.seq);
        put<uint32_t>(b, m.players);
        const size_t n = std::min(m.items.size(), kLootMaxItems);
        put<uint8_t>(b, (uint8_t)n);
        for (size_t i = 0; i < n; ++i) {
            const ItemFields& f = m.items[i];
            put_str(b, f.name, kMaxName);
            put_str(b, f.text, kMaxText);
            put<uint32_t>(b, f.a);
            put<uint32_t>(b, f.b);
            put<uint32_t>(b, f.c);
            put<uint32_t>(b, f.d);
            put<uint8_t>(b, f.e);
            put<uint8_t>(b, (uint8_t)f.f);
        }
        break;
    }
    case kLootReady:
        put<uint8_t>(b, m.ready > kLootReadyQuit ? 1 : m.ready);
        break;
    case kLootChoose:
        put<uint8_t>(b, m.index);
        put<uint8_t>(b, m.choice);
        break;
    case kLootState: {
        const LootState& s = m.state;
        put<uint8_t>(b, s.index);
        put<uint8_t>(b, (uint8_t)s.phase);
        put<uint32_t>(b, s.ms_left);
        put<uint32_t>(b, s.players);
        put<uint32_t>(b, s.ready);
        put<uint32_t>(b, s.chosen);
        put<uint8_t>(b, s.result.decided ? 1 : 0);
        put<uint8_t>(b, s.result.winner);
        put<uint8_t>(b, s.result.category);
        put<uint8_t>(b, s.result.rerolls);
        const size_t n = std::min(s.result.rolls.size(), (size_t)kLootSlots);
        put<uint8_t>(b, (uint8_t)n);
        for (size_t i = 0; i < n; ++i) {
            put<uint8_t>(b, s.result.rolls[i].peer);
            put<uint8_t>(b, s.result.rolls[i].choice);
            put<uint8_t>(b, s.result.rolls[i].roll);
        }
        const size_t w = std::min(s.winners.size(), kLootMaxItems);
        put<uint8_t>(b, (uint8_t)w);
        for (size_t i = 0; i < w; ++i) put<uint8_t>(b, s.winners[i]);
        break;
    }
    case kLootGive:
        put<uint8_t>(b, m.index);
        break;
    case kLootGot:
        put<uint8_t>(b, m.index);
        put<uint8_t>(b, m.ok);
        break;
    default:
        break;
    }
    return b;
}

bool loot_decode(const uint8_t* p, size_t n, LootMsg& out) {
    if (!p) return false;
    Reader r{p, n};
    if (r.get<uint32_t>() != kTeamMagic || r.get<uint8_t>() != kTeamFormat || r.get<uint8_t>() != kTeamLoot)
        return false;
    LootMsg m;
    m.op     = r.get<uint8_t>();
    m.parade = r.get<uint32_t>();
    if (!r.ok) return false;
    switch (m.op) {
    case kLootStart: {
        m.seq     = r.get<uint8_t>();
        m.players = r.get<uint32_t>();
        const uint8_t count = r.get<uint8_t>();
        if (!r.ok || count > kLootMaxItems) return false;
        for (uint8_t i = 0; i < count; ++i) {
            ItemFields f;
            if (!r.str(f.name, kMaxName) || !r.str(f.text, kMaxText)) return false;
            f.a = r.get<uint32_t>();
            f.b = r.get<uint32_t>();
            f.c = r.get<uint32_t>();
            f.d = r.get<uint32_t>();
            f.e = r.get<uint8_t>();
            f.f = r.get<uint8_t>();
            if (!r.ok || f.name.empty()) return false;
            m.items.push_back(std::move(f));
        }
        break;
    }
    case kLootReady:
        m.ready = r.get<uint8_t>();
        if (r.ok && m.ready > kLootReadyQuit) return false;
        break;
    case kLootChoose:
        m.index  = r.get<uint8_t>();
        m.choice = r.get<uint8_t>();
        if (r.ok && !valid_choice(m.choice)) return false;
        break;
    case kLootState: {
        LootState& s = m.state;
        s.parade  = m.parade;
        s.index   = r.get<uint8_t>();
        const uint8_t phase = r.get<uint8_t>();
        if (phase > (uint8_t)LootPhase::Done) return false;
        s.phase   = (LootPhase)phase;
        s.ms_left = r.get<uint32_t>();
        s.players = r.get<uint32_t>();
        s.ready   = r.get<uint32_t>();
        s.chosen  = r.get<uint32_t>();
        s.result.decided  = r.get<uint8_t>() != 0;
        s.result.winner   = r.get<uint8_t>();
        s.result.category = r.get<uint8_t>();
        s.result.rerolls  = r.get<uint8_t>();
        const uint8_t count = r.get<uint8_t>();
        if (!r.ok || count > kLootSlots) return false;
        for (uint8_t i = 0; i < count; ++i) {
            LootRoll x;
            x.peer   = r.get<uint8_t>();
            x.choice = r.get<uint8_t>();
            x.roll   = r.get<uint8_t>();
            if (x.peer >= kLootSlots || x.roll > 100) return false;
            s.result.rolls.push_back(x);
        }
        const uint8_t w = r.get<uint8_t>();
        if (!r.ok || w > kLootMaxItems) return false;
        for (uint8_t i = 0; i < w; ++i) s.winners.push_back(r.get<uint8_t>());
        break;
    }
    case kLootGive:
        m.index = r.get<uint8_t>();
        break;
    case kLootGot:
        m.index = r.get<uint8_t>();
        m.ok    = r.get<uint8_t>();
        break;
    default:
        return false;
    }
    if (!r.ok || r.pos != n) return false;
    out = std::move(m);
    return true;
}

// --- les des ---------------------------------------------------------------------------------

uint32_t LootRng::next() {
    // xorshift64* : assez bon pour des des, et a lui seul.
    s ^= s >> 12;
    s ^= s << 25;
    s ^= s >> 27;
    return (uint32_t)((s * 0x2545F4914F6CDD1Dull) >> 32);
}

uint8_t LootRng::d100() { return (uint8_t)(1 + next() % 100); }

LootResult loot_resolve(const uint8_t choices[kLootSlots], uint32_t players, LootRng& rng) {
    LootResult r;
    r.decided = true;
    uint32_t need = 0, greed = 0;
    for (int i = 0; i < kLootSlots; ++i) {
        if (!(players & (1u << i))) continue;
        LootRoll x;
        x.peer   = (uint8_t)i;
        x.choice = valid_choice(choices[i]) ? choices[i] : (uint8_t)kLootPass;
        if (x.choice == kLootNeed) need |= 1u << i;
        if (x.choice == kLootGreed) greed |= 1u << i;
        r.rolls.push_back(x);
    }
    const uint32_t pool = need ? need : greed;
    r.category = need ? kLootNeed : greed ? kLootGreed : kLootNone;
    if (!pool) return r;                     // personne : il reste a l'hote
    // Le premier lancer, celui que tous voient.
    uint8_t  best = 0;
    uint32_t at   = 0;
    for (LootRoll& x : r.rolls) {
        if (!(pool & (1u << x.peer))) continue;
        x.roll = rng.d100();
        if (x.roll > best) { best = x.roll; at = 1u << x.peer; }
        else if (x.roll == best) at |= 1u << x.peer;
    }
    // Des ex aequo en tete : on relance entre eux seuls. Les relances departagent
    // sans remplacer le premier lancer (un perdant du premier tour garderait
    // sinon un de plus haut que celui du gagnant) ; on les compte.
    while (popcount32(at) > 1 && r.rerolls < kLootMaxRerolls) {
        ++r.rerolls;
        uint8_t  b2 = 0;
        uint32_t a2 = 0;
        for (int i = 0; i < kLootSlots; ++i) {
            if (!(at & (1u << i))) continue;
            const uint8_t d = rng.d100();
            if (d > b2) { b2 = d; a2 = 1u << i; }
            else if (d == b2) a2 |= 1u << i;
        }
        at = a2;
    }
    for (int i = 0; i < kLootSlots; ++i)
        if (at & (1u << i)) { r.winner = (uint8_t)i; break; }
    return r;
}

// --- l'hote ----------------------------------------------------------------------------------

void LootHost::start(uint32_t parade, uint8_t seq, std::vector<ItemFields> items, uint32_t players, uint8_t host,
                     uint64_t now) {
    stop();
    if (items.size() > kLootMaxItems) items.resize(kLootMaxItems);
    if (items.empty() || host >= kLootSlots) return;
    parade_  = parade;
    seq_     = seq;
    host_    = host;
    players_ = (players | (1u << host)) & ((1u << kLootSlots) - 1);
    items_   = std::move(items);
    results_.assign(items_.size(), LootResult{});
    phase_   = LootPhase::Waiting;
    since_   = now;
    last_    = now;
}

void LootHost::stop() {
    phase_   = LootPhase::Idle;
    parade_  = 0;
    seq_     = 0;
    players_ = 0;
    ready_   = 0;
    index_   = 0;
    since_   = 0;
    last_    = 0;
    host_ready_seen = false;
    memset(choices_, 0, sizeof(choices_));
    items_.clear();
    results_.clear();
}

void LootHost::set_ready(uint8_t peer, bool ready, uint64_t now) {
    if (peer >= kLootSlots || !(players_ & (1u << peer))) return;
    if (ready) ready_ |= 1u << peer;
    else ready_ &= ~(1u << peer);
    if (peer == host_ && ready && !host_ready_seen && phase_ == LootPhase::Waiting) {
        host_ready_seen = true;
        since_ = now;                        // l'attente des autres commence
    }
}

bool LootHost::start_now(uint64_t now) {
    if (phase_ != LootPhase::Waiting) return false;
    ready_ |= 1u << host_;
    host_ready_seen = true;
    last_ = now;
    begin_item(now);
    return true;
}

bool LootHost::choose(uint8_t peer, uint8_t index, uint8_t choice) {
    if (phase_ != LootPhase::Choosing || index != index_ || peer >= kLootSlots) return false;
    if (!(players_ & (1u << peer)) || choices_[peer] != kLootNone || !valid_choice(choice)) return false;
    choices_[peer] = choice;
    ready_ |= 1u << peer;                    // il voit le defile
    return true;
}

void LootHost::gone(uint8_t peer) {
    if (peer >= kLootSlots || peer == host_) return;
    players_ &= ~(1u << peer);
    ready_   &= ~(1u << peer);
    choices_[peer] = kLootNone;
}

void LootHost::begin_item(uint64_t now) {
    phase_ = LootPhase::Choosing;
    since_ = now;
    memset(choices_, 0, sizeof(choices_));
}

void LootHost::resolve_item(uint64_t now, LootRng& rng) {
    results_[index_] = loot_resolve(choices_, players_, rng);
    phase_ = LootPhase::Result;
    since_ = now;
}

bool LootHost::tick(uint64_t now, LootRng& rng) {
    const uint64_t step = now >= last_ ? now - last_ : 0;
    last_ = now;
    const bool host_ready = (ready_ & (1u << host_)) != 0;
    switch (phase_) {
    case LootPhase::Waiting:
        if (!host_ready || !host_ready_seen) return false;
        if (((ready_ & players_) == players_ && now - since_ >= kLootIntroMs) || now - since_ >= kLootReadyWaitMs) {
            begin_item(now);
            return true;
        }
        return false;
    case LootPhase::Choosing: {
        // L'hote ne voit plus le defile (une autre fenetre, un chargement) : le
        // temps s'arrete pour tous.
        if (!host_ready) { since_ += step; return false; }
        uint32_t chosen = 0;
        for (int i = 0; i < kLootSlots; ++i)
            if (choices_[i] != kLootNone) chosen |= 1u << i;
        const uint32_t waiting = players_ & ready_ & ~chosen;
        if (waiting == 0 || now - since_ >= kLootChoiceMs) {
            resolve_item(now, rng);
            return true;
        }
        return false;
    }
    case LootPhase::Result:
        if (!host_ready) { since_ += step; return false; }
        if (now - since_ < result_ms()) return false;
        if ((size_t)index_ + 1 < items_.size()) {
            ++index_;
            begin_item(now);
        } else {
            phase_ = LootPhase::Done;
        }
        return true;
    default:
        return false;
    }
}

LootState LootHost::state(uint64_t now) const {
    LootState s;
    s.parade  = parade_;
    s.index   = index_;
    s.phase   = phase_;
    s.players = players_;
    s.ready   = ready_;
    for (int i = 0; i < kLootSlots; ++i)
        if (choices_[i] != kLootNone && phase_ == LootPhase::Choosing) s.chosen |= 1u << i;
    const uint64_t el = now >= since_ ? now - since_ : 0;
    switch (phase_) {
    case LootPhase::Waiting:
        s.ms_left = host_ready_seen && el < kLootReadyWaitMs ? (uint32_t)(kLootReadyWaitMs - el) : 0;
        break;
    case LootPhase::Choosing:
        s.ms_left = el < kLootChoiceMs ? (uint32_t)(kLootChoiceMs - el) : 0;
        break;
    case LootPhase::Result:
        s.ms_left = el < result_ms() ? (uint32_t)(result_ms() - el) : 0;
        s.result  = results_[index_];
        break;
    case LootPhase::Done:
        if (!results_.empty()) s.result = results_.back();
        break;
    default:
        break;
    }
    s.winners.reserve(results_.size());
    for (const LootResult& r : results_) s.winners.push_back(r.decided ? r.winner : kLootUndecided);
    return s;
}

// --- les textes ------------------------------------------------------------------------------

namespace {

// Un texte compose (format traduit par tr(), longueur libre : une liste d'objets).
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

const char* loot_choice_word(uint8_t choice) {
    switch (choice) {
    case kLootNeed:  return tr(u8"Besoin");
    case kLootGreed: return tr(u8"Cupidité");
    default:         return tr(u8"Passer");
    }
}

std::string loot_rolls_text(const LootResult& r, std::string (*name)(uint8_t)) {
    std::vector<LootRoll> rolled;
    for (const LootRoll& x : r.rolls)
        if (x.roll) rolled.push_back(x);
    std::stable_sort(rolled.begin(), rolled.end(), [](const LootRoll& a, const LootRoll& b) { return a.roll > b.roll; });
    std::string s;
    for (const LootRoll& x : rolled) {
        if (!s.empty()) s += ", ";
        s += (name ? name(x.peer) : std::to_string(x.peer)) + " " + std::to_string(x.roll);
    }
    return s;
}

std::string loot_time_text(uint32_t ms) {
    const uint32_t s = (ms + 999) / 1000;
    char buf[32];
    if (s >= 60) snprintf(buf, sizeof(buf), "%u:%02u", s / 60, s % 60);
    else snprintf(buf, sizeof(buf), tr(u8"%u s"), s);
    return buf;
}

std::string loot_join(const std::vector<std::string>& v) {
    if (v.empty()) return std::string();
    std::string head;
    for (size_t i = 0; i + 1 < v.size(); ++i) head += (i ? ", " : "") + v[i];
    return v.size() == 1 ? v[0] : fmt(tr(u8"%s et %s"), head.c_str(), v.back().c_str());
}

// --- les fenetres ----------------------------------------------------------------------------

namespace {

std::string who(const LootView& v, uint8_t peer) {
    if (peer == v.self) return tr(u8"toi");
    if (v.name) return v.name(peer);
    return fmt(tr(u8"Joueur %u"), (unsigned)peer);
}

std::vector<std::string> names_of(const LootView& v, uint32_t mask, bool skip_self) {
    std::vector<std::string> out;
    for (int i = 0; i < kLootSlots; ++i)
        if ((mask & (1u << i)) && !(skip_self && i == v.self)) out.push_back(who(v, (uint8_t)i));
    return out;
}

std::string item_label(const LootView& v, size_t i) {
    return i < v.item_names.size() && !v.item_names[i].empty() ? v.item_names[i] : std::string(tr(u8"Un objet"));
}

std::string header(const LootView& v) {
    return fmt(tr(u8"Partage du butin : objet %u sur %u"), (unsigned)v.st.index + 1, (unsigned)v.item_names.size());
}

// « En attente de l'hôte » quand il ne voit pas le defile (chez un invite : le
// temps est arrete).
bool host_away(const LootView& v) { return !v.is_host && !(v.st.ready & (1u << v.host)); }

// Les choix d'un objet, groupes : « Besoin : Ami et Lou ; Passer : toi ».
std::string choices_text(const LootView& v, const LootResult& r) {
    std::string out;
    for (uint8_t c : { (uint8_t)kLootNeed, (uint8_t)kLootGreed, (uint8_t)kLootPass }) {
        std::vector<std::string> who_chose;
        for (const LootRoll& x : r.rolls)
            if (x.choice == c) who_chose.push_back(who(v, x.peer));
        if (who_chose.empty()) continue;
        if (!out.empty()) out += " ; ";
        const char* f = c == kLootNeed ? tr(u8"Besoin : %s") : c == kLootGreed ? tr(u8"Cupidité : %s") : tr(u8"Passer : %s");
        out += fmt(f, loot_join(who_chose).c_str());
    }
    return out;
}

std::string rolls_text(const LootView& v, const LootResult& r) {
    std::vector<LootRoll> rolled;
    for (const LootRoll& x : r.rolls)
        if (x.roll) rolled.push_back(x);
    std::stable_sort(rolled.begin(), rolled.end(), [](const LootRoll& a, const LootRoll& b) { return a.roll > b.roll; });
    std::string s;
    for (const LootRoll& x : rolled) {
        if (!s.empty()) s += ", ";
        s += who(v, x.peer) + " " + std::to_string(x.roll);
    }
    return s;
}

} // namespace

LootScreen loot_screen(const LootView& v) {
    LootScreen s;
    const LootState& st = v.st;
    const size_t n = v.item_names.size();
    if (!n || v.self >= kLootSlots || !(st.players & (1u << v.self))) return s;   // pas du defile
    const uint32_t me = 1u << v.self;
    switch (st.phase) {
    case LootPhase::Waiting: {
        s.layout = kLayWait;
        s.text   = std::string(tr(u8"Partage du butin")) + "\n\n";
        s.text  += fmt(n > 1 ? tr(u8"%u objets du sac à partager entre %s.") : tr(u8"%u objet du sac à partager entre %s."),
                       (unsigned)n, loot_join(names_of(v, st.players, false)).c_str()) + "\n";
        s.text  += std::string(tr(u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
                                  u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.")) + "\n";
        const uint32_t missing = st.players & ~st.ready & ~me;
        if (host_away(v))
            s.text += "\n" + fmt(tr(u8"En attente de %s..."), who(v, v.host).c_str());
        else if (missing)
            s.text += "\n" + fmt(tr(u8"En attente de %s... (%s)"), loot_join(names_of(v, missing, true)).c_str(),
                                 loot_time_text(st.ms_left).c_str());
        else
            s.text += "\n" + std::string(tr(u8"Tout le monde est là : ça commence !"));
        s.yes = v.is_host ? tr(u8"Commencer") : tr(u8"D'accord");
        return s;
    }
    case LootPhase::Choosing: {
        const std::string item = item_label(v, st.index);
        const bool chosen = (st.chosen & me) || v.my_choice != kLootNone;
        if (!chosen && !v.step2) {
            s.layout = kLayChoose;
            s.text   = header(v) + "\n\n" + item;
            if (st.index < v.item_descs.size() && !v.item_descs[st.index].empty())
                s.text += "\n" + v.item_descs[st.index];
            s.text  += "\n\n" + fmt(tr(u8"Le veux-tu ? (%s)"), loot_time_text(st.ms_left).c_str());
            s.yes    = tr(u8"Je le veux");
            s.no     = tr(u8"Passer");
            return s;
        }
        if (!chosen) {
            s.layout = kLayNeedGreed;
            s.text   = header(v) + "\n\n" + item + "\n\n" +
                       fmt(tr(u8"Besoin : il te sert, tu passes avant les autres.\n"
                              u8"Cupidité : tu le prends si personne n'en a besoin.\n(%s)"),
                           loot_time_text(st.ms_left).c_str());
            s.yes    = tr(u8"Besoin");
            s.no     = tr(u8"Cupidité");
            return s;
        }
        s.layout = kLayChosen;
        s.text   = header(v) + "\n\n" + item + "\n";
        s.text  += v.my_choice != kLootNone ? fmt(tr(u8"Ton choix : %s."), loot_choice_word(v.my_choice))
                                            : std::string(tr(u8"Ton choix est fait."));
        const uint32_t waiting = st.players & st.ready & ~st.chosen & ~me;
        const uint32_t absent  = st.players & ~st.ready & ~st.chosen & ~me;
        if (host_away(v))
            s.text += "\n" + fmt(tr(u8"En attente de %s..."), who(v, v.host).c_str());
        else if (waiting)
            s.text += "\n" + fmt(tr(u8"En attente de %s (%s)."), loot_join(names_of(v, waiting, true)).c_str(),
                                 loot_time_text(st.ms_left).c_str());
        else
            s.text += "\n" + std::string(tr(u8"Le résultat arrive..."));
        if (absent) {
            const std::vector<std::string> a = names_of(v, absent, true);
            s.text += "\n" + fmt(a.size() > 1 ? tr(u8"%s ne sont pas là : leur tour passe.")
                                              : tr(u8"%s n'est pas là : son tour passe."),
                                 loot_join(a).c_str());
        }
        s.yes = tr(u8"D'accord");
        return s;
    }
    case LootPhase::Result: {
        const LootResult& r = st.result;
        s.layout = kLayResult;
        s.text   = header(v) + "\n\n" + item_label(v, st.index) + "\n";
        if (r.winner == kLootNobody)
            s.text += v.is_host ? std::string(tr(u8"Personne n'en veut : il reste chez toi."))
                                : fmt(tr(u8"Personne n'en veut : il reste chez %s."), who(v, v.host).c_str());
        else if (r.winner == v.self)
            s.text += fmt(tr(u8"Tu l'emportes (%s) !"), loot_choice_word(r.category));
        else
            s.text += fmt(tr(u8"%s l'emporte (%s)."), who(v, r.winner).c_str(), loot_choice_word(r.category));
        const std::string rolls = rolls_text(v, r);
        if (!rolls.empty())
            s.text += "\n" + fmt(r.rerolls ? tr(u8"Dés : %s (égalité départagée).") : tr(u8"Dés : %s."), rolls.c_str());
        const std::string ch = choices_text(v, r);
        if (!ch.empty()) s.text += "\n" + ch + ".";
        s.yes = tr(u8"D'accord");
        return s;
    }
    case LootPhase::Done: {
        s.layout = kLaySummary;
        s.text   = std::string(tr(u8"Partage du butin terminé")) + "\n";
        std::vector<std::string> per[kLootSlots];
        std::vector<std::string> nobody, kept;
        for (size_t i = 0; i < st.winners.size() && i < n; ++i) {
            const uint8_t w = st.winners[i];
            if (w == kLootNobody) nobody.push_back(item_label(v, i));
            else if (w == v.self && v.track_received && !(i < 64 && (v.received >> i) & 1)) kept.push_back(item_label(v, i));
            else if (w < kLootSlots) per[w].push_back(item_label(v, i));
        }
        bool any = !nobody.empty();
        if (!per[v.self].empty()) s.text += "\n" + fmt(tr(u8"Tu remportes : %s."), loot_join(per[v.self]).c_str());
        bool others = false;
        for (int p = 0; p < kLootSlots; ++p) {
            if (p == v.self || per[p].empty()) continue;
            s.text += "\n" + fmt(tr(u8"%s remporte : %s."), who(v, (uint8_t)p).c_str(), loot_join(per[p]).c_str());
            others = true;
        }
        any = any || others || !per[v.self].empty() || !kept.empty();
        if (!kept.empty())
            s.text += "\n" + fmt(tr(u8"Remporté, mais resté chez %s : %s."), who(v, v.host).c_str(),
                                 loot_join(kept).c_str());
        if (!nobody.empty())
            s.text += "\n" + (v.is_host ? fmt(tr(u8"Personne n'en a voulu (restent chez toi) : %s."),
                                              loot_join(nobody).c_str())
                                        : fmt(tr(u8"Personne n'en a voulu (restent chez %s) : %s."),
                                              who(v, v.host).c_str(), loot_join(nobody).c_str()));
        if (!any) s.text += "\n" + std::string(tr(u8"Aucun objet n'a été partagé."));
        if (!v.is_host && !per[v.self].empty())
            s.text += "\n\n" + std::string(v.at_home ? tr(u8"Ta save est déjà ouverte : la fenêtre suivante\n"
                                                          u8"te dit comment la recharger avec ton butin.")
                                                     : tr(u8"Ton butin t'attend dans ton rangement, chez toi."));
        if (v.is_host && others) s.text += "\n\n" + std::string(tr(u8"Ce que les autres remportent quitte ton rangement."));
        s.yes = tr(u8"D'accord");
        return s;
    }
    default:
        return s;
    }
}

} // namespace mgmp
