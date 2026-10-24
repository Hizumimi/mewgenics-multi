// mm_vote_core.cpp -- voir mm_vote_core.h.
#include "mm_vote_core.h"

#include "mgmp_proto.h"
#include "mm_lang.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <utility>

namespace mgmp {
namespace {

uint32_t bit(uint8_t slot) { return slot < kVoteSlots ? 1u << slot : 0u; }

void put16(uint8_t* p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
uint16_t get16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }

unsigned secs(uint32_t ms) { return (unsigned)((ms + 999) / 1000); }

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

} // namespace

uint32_t vote_cast_mask(const VoteState& s) {
    uint32_t m = 0;
    for (uint8_t i = 0; i < kVoteSlots; ++i)
        if ((s.voters & bit(i)) && s.ballot[i].valid()) m |= bit(i);
    return m;
}

// --- l'hote -----------------------------------------------------------------------

void VoteHost::reset() {
    const uint8_t next = next_round_;
    const uint32_t keep = never_drop_;
    *this = VoteHost{};
    next_round_ = next;
    never_drop_ = keep;
}

bool VoteHost::cast(uint8_t slot, uint16_t node, uint8_t type, uint32_t voters, uint64_t now) {
    if (slot >= kVoteSlots || node == kVoteNoNode) return false;
    if (s_.phase == VotePhase::Result) return false;
    if (s_.phase == VotePhase::Idle) {
        if (!(voters & bit(slot))) return false;
        const uint8_t round = next_round_;
        next_round_ = (uint8_t)(next_round_ + 1);
        if (!next_round_) next_round_ = 1;               // 0 veut dire « aucun tour »
        s_ = VoteState{};
        s_.round        = round;
        s_.phase        = VotePhase::Voting;
        s_.voters       = voters & ((1u << kVoteSlots) - 1);
        s_.present      = bit(slot);                     // il vient de cliquer sur la carte
        s_.remaining_ms = kVoteCountdownMs;
        started_at_     = now;
        last_tick_      = now;
        std::memset(absent_ms_, 0, sizeof(absent_ms_));
    }
    if (!(s_.voters & bit(slot))) {
        // Arrive dans l'aventure pendant le tour : il y vote en cliquant.
        if (!(voters & bit(slot))) return false;
        s_.voters  |= bit(slot);
        s_.present |= bit(slot);
    }
    s_.ballot[slot].node = node;
    s_.ballot[slot].type = type;
    // Un absent ecarte qui revient voter a temps : il compte de nouveau.
    s_.dropped &= ~bit(slot);
    absent_ms_[slot] = 0;
    return true;
}

bool VoteHost::keep_voters(uint32_t still_here) {
    if (s_.phase != VotePhase::Voting) return false;
    const uint32_t gone = s_.voters & ~still_here;
    if (!gone) return false;
    for (uint8_t i = 0; i < kVoteSlots; ++i) {
        if (!(gone & bit(i))) continue;
        s_.ballot[i] = VoteBallot{};
        absent_ms_[i] = 0;
    }
    s_.voters  &= ~gone;
    s_.present &= ~gone;
    s_.dropped &= ~gone;
    return true;
}

void VoteHost::set_present(uint32_t present) {
    s_.present = present & s_.voters;
    if (s_.phase == VotePhase::Voting)
        s_.running = ((s_.voters & ~s_.dropped) & ~s_.present) == 0;
}

bool VoteHost::tick(uint64_t now, Rng rng, void* ctx) {
    if (s_.phase != VotePhase::Voting) {
        last_tick_ = now;
        return false;
    }
    uint64_t step = now > last_tick_ ? now - last_tick_ : 0;
    if (step > kVoteMaxStepMs) step = kVoteMaxStepMs;
    last_tick_ = now;
    const uint32_t dt = (uint32_t)step;

    bool changed = false;
    // Mewgenics Multi : l'absence se compte d'affilee -- un votant revenu sur la
    // carte repart de zero (un menu ouvert plusieurs fois ne l'ecarte pas).
    for (uint8_t i = 0; i < kVoteSlots; ++i)
        if (s_.present & bit(i)) absent_ms_[i] = 0;
    uint32_t eligible = s_.voters & ~s_.dropped;
    const uint32_t absent = eligible & ~s_.present;
    if (!absent) {
        s_.running = true;
        s_.remaining_ms = s_.remaining_ms > dt ? s_.remaining_ms - dt : 0;
    } else {
        if (s_.running) changed = true;
        s_.running = false;
        for (uint8_t i = 0; i < kVoteSlots; ++i) {
            if (!(absent & bit(i))) continue;
            absent_ms_[i] += dt;
            if (absent_ms_[i] >= kVoteAbsentMs && !(never_drop_ & bit(i))) {
                s_.dropped |= bit(i);
                changed = true;
            }
        }
    }

    const uint32_t cast = vote_cast_mask(s_);
    if (!cast) {
        // Les seuls votes sont partis avec leurs joueurs : plus de tour.
        finish();
        return true;
    }
    eligible = s_.voters & ~s_.dropped;
    const bool all_voted = (eligible & ~cast) == 0;
    const bool timed_out = s_.remaining_ms == 0;
    const bool too_long  = now - started_at_ >= kVoteSafetyMs;
    if (all_voted || timed_out || too_long) {
        resolve(rng, ctx);
        return true;
    }
    return changed;
}

void VoteHost::resolve(Rng rng, void* ctx) {
    // Les voix par case, dans l'ordre des index : le meme ordre chez tous.
    uint16_t nodes[kVoteSlots];
    uint8_t  types[kVoteSlots];
    uint8_t  counts[kVoteSlots];
    int n = 0;
    for (uint8_t i = 0; i < kVoteSlots; ++i) {
        if (!(s_.voters & bit(i)) || !s_.ballot[i].valid()) continue;
        int k = 0;
        while (k < n && nodes[k] != s_.ballot[i].node) ++k;
        if (k == n) {
            nodes[n] = s_.ballot[i].node;
            types[n] = s_.ballot[i].type;
            counts[n] = 0;
            ++n;
        }
        ++counts[k];
    }
    // Tri par index de case (insertion : au plus 8).
    for (int a = 1; a < n; ++a)
        for (int b = a; b > 0 && nodes[b - 1] > nodes[b]; --b) {
            std::swap(nodes[b - 1], nodes[b]);
            std::swap(types[b - 1], types[b]);
            std::swap(counts[b - 1], counts[b]);
        }
    uint8_t best = 0;
    for (int k = 0; k < n; ++k) if (counts[k] > best) best = counts[k];
    VoteBallot tied[kVoteSlots];
    int t = 0;
    for (int k = 0; k < n; ++k)
        if (counts[k] == best && best) { tied[t].node = nodes[k]; tied[t].type = types[k]; ++t; }

    s_.phase   = VotePhase::Result;
    s_.running = false;
    s_.drawn   = t > 1;
    s_.tied_count = 0;
    for (int k = 0; k < 4; ++k) s_.tied[k] = VoteBallot{};
    if (!t) {                                   // impossible : tick l'a verifie
        s_.winner = VoteBallot{};
        return;
    }
    uint32_t pick = 0;
    if (t > 1) {
        pick = rng ? rng(ctx, (uint32_t)t) : 0;
        if (pick >= (uint32_t)t) pick = 0;
        for (int k = 0; k < t && k < 4; ++k) s_.tied[k] = tied[k];
        s_.tied_count = (uint8_t)(t < 4 ? t : 4);
    }
    s_.winner = tied[pick];
}

void VoteHost::finish() {
    const uint8_t round = s_.round;
    s_ = VoteState{};
    s_.round = round;
    started_at_ = 0;
    std::memset(absent_ms_, 0, sizeof(absent_ms_));
}

// --- le message ---------------------------------------------------------------

void vote_encode_cast(const VoteCast& c, MmMsg& m) {
    m = MmMsg{};
    m.kind = MM_VOTE;
    m.a    = kVoteCast;
    m.b    = c.round;
    m.c    = c.type;
    m.x    = (uint32_t)c.node | ((uint32_t)c.node_count << 16);
    m.y    = c.seed_lo;
}

bool vote_decode_cast(const MmMsg& m, VoteCast& c) {
    if (m.kind != MM_VOTE || m.a != kVoteCast) return false;
    c = VoteCast{};
    c.round      = m.b;
    c.type       = m.c;
    c.node       = (uint16_t)(m.x & 0xFFFF);
    c.node_count = (uint16_t)(m.x >> 16);
    c.seed_lo    = m.y;
    return c.node != kVoteNoNode && c.node < c.node_count;
}

void vote_encode_here(bool on_map, MmMsg& m) {
    m = MmMsg{};
    m.kind = MM_VOTE;
    m.a    = kVoteHere;
    m.c    = on_map ? 1 : 0;
}

void vote_encode_state(const VoteState& s, MmMsg& m) {
    m = MmMsg{};
    m.kind = MM_VOTE;
    m.a    = kVoteState;
    m.b    = s.round;
    m.c    = (uint8_t)s.phase;
    m.x    = s.remaining_ms;
    m.y    = (s.voters & 0xFF) | ((s.present & 0xFF) << 8) | ((s.dropped & 0xFF) << 16) |
             (s.running ? 1u << 24 : 0u) | (s.drawn ? 1u << 25 : 0u);
    uint8_t* t = (uint8_t*)m.text;
    static_assert(sizeof(m.text) >= 42, "MmMsg::text trop petit pour l'etat du vote");
    for (int i = 0; i < kVoteSlots; ++i) {
        put16(t + 2 * i, s.ballot[i].node);
        t[16 + i] = s.ballot[i].type;
    }
    put16(t + 24, s.winner.node);
    t[26] = s.winner.type;
    t[27] = s.tied_count;
    for (int i = 0; i < 4; ++i) {
        put16(t + 28 + 2 * i, s.tied[i].node);
        t[36 + i] = s.tied[i].type;
    }
    t[40] = s.refused_slot;
    t[41] = s.refused_reason;
}

bool vote_decode_state(const MmMsg& m, VoteState& s) {
    if (m.kind != MM_VOTE || m.a != kVoteState) return false;
    if (m.c > (uint8_t)VotePhase::Result) return false;
    const uint8_t* t = (const uint8_t*)m.text;
    if (t[27] > 4) return false;
    s = VoteState{};
    s.round        = m.b;
    s.phase        = (VotePhase)m.c;
    s.remaining_ms = m.x;
    s.voters       = m.y & 0xFF;
    s.present      = (m.y >> 8) & 0xFF;
    s.dropped      = (m.y >> 16) & 0xFF;
    s.running      = (m.y >> 24) & 1;
    s.drawn        = (m.y >> 25) & 1;
    for (int i = 0; i < kVoteSlots; ++i) {
        s.ballot[i].node = get16(t + 2 * i);
        s.ballot[i].type = t[16 + i];
    }
    s.winner.node = get16(t + 24);
    s.winner.type = t[26];
    s.tied_count  = t[27];
    for (int i = 0; i < 4; ++i) {
        s.tied[i].node = get16(t + 28 + 2 * i);
        s.tied[i].type = t[36 + i];
    }
    s.refused_slot   = t[40];
    s.refused_reason = t[41];
    return true;
}

// --- ce que voit chaque joueur ----------------------------------------------------

namespace {

VoteBallot ballot_of(const VoteState& s, uint8_t i, uint8_t self, VoteBallot own) {
    if (i == self && own.valid() && s.phase == VotePhase::Voting) return own;
    return i < kVoteSlots ? s.ballot[i] : VoteBallot{};
}

} // namespace

VoteBallot vote_leader_excluding(const VoteState& s, uint8_t self) {
    uint16_t nodes[kVoteSlots];
    uint8_t  types[kVoteSlots];
    uint8_t  counts[kVoteSlots];
    int n = 0;
    for (uint8_t i = 0; i < kVoteSlots; ++i) {
        if (i == self || !(s.voters & bit(i)) || !s.ballot[i].valid()) continue;
        int k = 0;
        while (k < n && nodes[k] != s.ballot[i].node) ++k;
        if (k == n) { nodes[n] = s.ballot[i].node; types[n] = s.ballot[i].type; counts[n] = 0; ++n; }
        ++counts[k];
    }
    int best = -1, ties = 0;
    for (int k = 0; k < n; ++k) {
        if (best < 0 || counts[k] > counts[best]) { best = k; ties = 1; }
        else if (counts[k] == counts[best]) ++ties;
    }
    VoteBallot out;
    if (best >= 0 && ties == 1) { out.node = nodes[best]; out.type = types[best]; }
    return out;
}

VoteLayout vote_layout(const VoteState& s, uint8_t self, VoteBallot own) {
    if (self >= kVoteSlots || !(s.voters & bit(self))) return VoteLayout::None;
    switch (s.phase) {
        case VotePhase::Voting:
            if (ballot_of(s, self, self, own).valid()) return VoteLayout::Voted;
            return vote_leader_excluding(s, self).valid() ? VoteLayout::Open : VoteLayout::Choose;
        case VotePhase::Result:
            return VoteLayout::Result;
        default:
            return VoteLayout::None;
    }
}

const char* vote_type_name(uint8_t type) {
    switch (type) {
        case 3:  return trk(u8"Sortie");
        case 4:  return trk(u8"Retour à la maison");
        case 5:  return trk(u8"Combat");
        case 6:  return trk(u8"Combat difficile");
        case 7:  return trk(u8"Mini-boss");
        case 8:  return trk(u8"Boss");
        case 9:  return trk(u8"Événement");
        case 10: return trk(u8"Événement");
        case 11: return trk(u8"Événement spécial");
        case 12: return trk(u8"Boutique");
        case 13: return trk(u8"Trésor");
        case 14: return trk(u8"Meuble");
        case 15: return trk(u8"Nourriture");
        case 16: return trk(u8"Bonus");
        case 17: return trk(u8"Case vide");
        default: return trk(u8"Case");
    }
}

const char* vote_refusal_text(uint8_t reason) {
    switch (reason) {
        case kRefuseOffMap:   return trk(u8"l'hôte n'est pas sur la carte");
        case kRefuseOtherMap: return trk(u8"vos cartes ne sont pas les mêmes");
        case kRefuseBlocked:  return trk(u8"l'hôte ne peut pas aller sur cette case");
        case kRefuseDecided:  return trk(u8"le chemin est déjà choisi");
        case kRefuseStale:    return trk(u8"ce tour est fini");
        default:              return trk(u8"l'hôte ne l'a pas pris");
    }
}

std::string vote_text(const VoteState& s, uint8_t self, const std::string* names,
                      uint32_t remaining_ms, VoteLabelFn label, void* ctx, VoteBallot own) {
    auto lab = [&](const VoteBallot& b) -> std::string {
        if (label) return label(ctx, b.node, b.type);
        return tr(vote_type_name(b.type));
    };
    auto who = [&](uint8_t i) -> std::string {
        if (names && !names[i].empty()) return names[i];
        return fmt(tr(u8"Joueur %u"), (unsigned)i + 1);
    };

    std::string out = tr(u8"Choix du chemin\n\n");
    for (uint8_t i = 0; i < kVoteSlots; ++i) {
        if (!(s.voters & bit(i))) continue;
        const VoteBallot b = ballot_of(s, i, self, own);
        std::string what;
        if (b.valid())                            what = lab(b);
        else if (s.phase == VotePhase::Result)   what = tr(u8"pas de vote");
        else if (s.dropped & bit(i))              what = tr(u8"absent, ne vote pas");
        else if (!(s.present & bit(i)))           what = tr(u8"pas encore sur la carte");
        else                                      what = tr(u8"pas encore voté");
        out += "  " + fmt(i == self ? tr(u8"%s (toi) : %s") : tr(u8"%s : %s"),
                          who(i).c_str(), what.c_str()) + "\n";
    }
    out += "\n";

    if (s.phase == VotePhase::Voting) {
        if (!s.running)
            out += tr(u8"En pause : on attend tout le monde sur la carte.");
        else if (!ballot_of(s, self, self, own).valid())
            out += fmt(tr(u8"À toi de choisir : encore %u s."), secs(remaining_ms));
        else
            out += fmt(tr(u8"En attente des autres : encore %u s."), secs(remaining_ms));
        return out;
    }
    if (s.phase != VotePhase::Result || !s.winner.valid()) return out;

    const std::string w = lab(s.winner);
    if (s.drawn && s.tied_count >= 2) {
        std::string list = lab(s.tied[0]);
        for (uint8_t k = 1; k < s.tied_count; ++k)
            list = fmt(k + 1 == s.tied_count ? tr(u8"%s et %s") : tr(u8"%s, %s"),
                       list.c_str(), lab(s.tied[k]).c_str());
        out += fmt(tr(u8"Égalité entre %s :\nle sort choisit %s."), list.c_str(), w.c_str());
    } else {
        if (vote_unanimous(s)) out += fmt(tr(u8"Tout le monde a choisi %s."), w.c_str());
        else                   out += fmt(tr(u8"%s l'emporte."), w.c_str());
    }
    return out;
}

bool vote_unanimous(const VoteState& s) {
    if (s.phase != VotePhase::Result || s.drawn || !s.winner.valid()) return false;
    int n = 0;
    for (uint8_t i = 0; i < kVoteSlots; ++i) {
        if (!(s.voters & bit(i))) continue;
        if (!s.ballot[i].valid() || s.ballot[i].node != s.winner.node) return false;
        ++n;
    }
    return n >= 2;
}

uint32_t vote_result_show_ms(const VoteState& s) {
    return vote_unanimous(s) ? kVoteResultSameMs : kVoteResultMs;
}

} // namespace mgmp
