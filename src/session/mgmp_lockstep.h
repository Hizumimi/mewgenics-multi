// mgmp_lockstep.h -- deterministic lockstep over the battle. Phase 4, layer 3.
//
// THE SEAM IS THE ONE THE REPLAYER ALREADY PROVED. Phase 2B injected recorded
// decisions at Brain::GetChoice and got a byte-identical battle back -- 17 of
// 29 decisions injected, the other 12 left for the AI to re-derive, all 12
// matched. This module swaps the recorded FIFO for a socket. Same hook, same
// injection point, same validation; the decisions now arrive from a peer.
//
// WHY WAITING IS FREE. GetChoice is a *poll*, not a decision point: it returns
// type=1 ("nothing decided yet") on every frame while waiting on a human, and
// did so for 1695 of 1711 calls in one tutorial battle. So a remote decision
// that has not arrived needs no timeout, no blocking recv and no frame budget.
// Return type=1 and the game waits exactly as it already waits for a person.
//
// CAT IDENTITY, AND WHY IT IS A SNAPSHOT.
//
// Pointers are unusable across peers, and turn order is unusable as an index:
// NextTurn shuffles the turn list every round with an inlined Fisher-Yates
// (sub_140085F80) drawing from the simulation stream TLS+0x178. Deterministic,
// so lockstep-safe -- but a cat's position in it changes every round.
//
// The pre-shuffle source is the battle's character list:
//
//     TurnControl+0x18 -> +0x08 -> +0x20 -> +0x1F90
//         { u32 refcount@0; u32 pad@4; u32 cap@8; u32 count@12; Character** data@16 }
//
// (the same list Ability::TargetIsValid and Character::BeginTurn/EndTurn read).
// Its ORDER is spawn order -- authored, identical on both peers given the same
// save and RNG state.
//
// We snapshot it once at battle start and index into the snapshot. That is
// deliberately not a live lookup: summons append to the live list and deaths
// may remove from it, either of which would renumber every cat mid-battle. A
// snapshot pins the numbering for the whole battle, and Character pointers are
// stable *within* one battle (they are only unstable across runs, which is what
// runs C/D measured -- 0 of 21 matched).
//
// CONTROL IS A SPLIT OF INPUT, NOT OF STATE. Both peers simulate every cat
// identically. `net_control` only says which cats local input may decide for.
// For every other cat this module overwrites GetChoice unconditionally, which
// is also what suppresses a stray click on a cat you do not own.
#pragma once

#include <cstdint>
#include "mgmp_proto.h"

namespace mgmp {

// Called once the transport reports Ready. Reads net_control from the config.
// Verifies and resolves the one game function this module CALLS
// (Character::get_affecting_elements). Called from hooks_install alongside
// catsync_set_base. Failure is non-fatal and only turns off element hashing.
void lockstep_set_base(uintptr_t base);

void lockstep_init();
void lockstep_shutdown();
bool lockstep_active();

// Drains the network queue and dispatches. Safe to call every frame; it is
// called from the frame hook so messages arrive even while no brain is polled.
void lockstep_pump();

// --- the seam ---------------------------------------------------------------

// Mirror of replay_fill_choice. Returns true if `out` was overwritten.
//
//   locally-controlled cat -> false, but the brain's own decision is captured
//                             and sent if it is a real one (type 2 or 3);
//   remote cat             -> true, always: either the peer's decision, or
//                             type=1 to keep waiting.
bool lockstep_fill_choice(void* brain, void* out);

// Called from the ApplyTurnAction hook, before the original runs. Clears the
// outstanding-send guard and validates that what landed is what we expected.
void lockstep_on_applied(const void* action, const void* actor);

// Called from the NextTurn hook. Snapshots the cat list on the first call of a
// battle, then exchanges and compares the turn hash.
void lockstep_turn_boundary(void* turn_control);

// --- what the cursor overlay needs ------------------------------------------

// This peer's battle counter, so a cosmetic message crossing a battle boundary
// can be dropped rather than drawn on the wrong board. 0 outside a battle.
// Which battle this peer is in: the node seed both peers read out of
// MapNode+0x118. kNoBattle when not in one. See mgmp_battleid.h.
uint64_t lockstep_battle_id();

// From the map layer, on BOTH peers, as a node is entered. Establishes battle
// identity without any negotiation -- both peers pass the same seed because
// they entered the same node.
void lockstep_enter_battle(uint64_t seed0);

// Mewgenics Multi (protocole 48) : les membres d'un combat, un bit par numero de
// joueur (mgmp_members.h). L'hote les note en entrant dans la case (lui et les
// invites de son aventure) et les met dans ENTERNODE ; un invite note ceux de
// l'ENTERNODE qu'il suit. Le partage des chats de ce combat se fait sur eux.
void    lockstep_note_members(uint64_t battle_id, uint8_t mask);
// Les membres de ce combat maintenant (le combat en cours : apres les departs),
// sinon ceux notes ; 0 si inconnus.
uint8_t lockstep_members_of(uint64_t battle_id);
// Mewgenics Multi : un invite sort de l'aventure de l'hote (il rentre chez lui,
// « Quitter l'aventure ») : son etat du combat en cours (file, empreintes,
// photo, partage, barriere) est oublie sans retirer ce combat -- s'il y revient,
// la reprise de l'hote le refait du debut. Les combats de l'hote ne le
// concernent plus jusqu'a sa prochaine case suivie (lockstep_enter_battle).
void    lockstep_leave_run();
// ... et il y revient (il demande a la rejoindre, ou il y est arrive) : les
// combats de l'hote le concernent de nouveau.
void    lockstep_rejoin_run();
// Invite : arrete (desynchronise) dans un combat qu'il ne faisait que regarder --
// le combat des autres continue, le sien est fige (mm_menu le fait rentrer).
bool    lockstep_halted_watching();
// Hote : les invites arretes en regardant un combat, parmi `still` (ceux encore
// dans son aventure et connectes ; les autres sont oublies). Ils ne sont pas
// membres des combats suivants avant d'etre revenus (lockstep_catchup).
uint8_t lockstep_halted_peers(uint8_t still);
// Hote : ce joueur revient (une reprise lui est faite, meme sur la carte) -- un
// arret dans un combat d'avant ne compte plus pour lui.
void    lockstep_peer_back(uint8_t peer);

// A peer joined or reconnected: replay this battle's decisions to it so it can
// fast-forward into a fight already under way. To that peer only.
void lockstep_catchup(uint8_t peer);

// True when the cat the game is currently asking for a decision is one this
// peer controls -- the "it is your turn" bit, and the thing peer cursors fade
// on. Refreshed at every GetChoice poll, which happens on essentially every
// frame of a battle, so it is never more than a frame stale.
//
// Deliberately NOT derived from the turn order: a cat's position in it is
// reshuffled every round by NextTurn's inlined Fisher-Yates, so the only stable
// answer comes from the roster index of whoever is actually being polled.
bool lockstep_local_actor();

// Does a peer -- specifically NOT this one -- own the input for this exact
// Character? Asked by the combat-menu lock, which has a Character* in hand
// (CombatMenu+280) and must not depend on who happens to be being polled.
//
// lockstep_local_actor answers "is the cat currently being asked for a decision
// mine", which is refreshed at a GetChoice poll and therefore holds its last
// value for as long as a decision stays cached -- fine for fading a cursor,
// wrong for a menu that is on screen across that whole window.
//
// Answers false for everything it cannot PROVE: no session, no snapshot, a cat
// that is not in the roster (a summon), or an AI cat. The caller is cosmetic,
// and a menu greyed out when it should not be reads as a bug in the game.
// Mewgenics Multi (protocole 52) : vrai aussi, chez l'hote, pour le chat d'un
// absent que joue l'IA du jeu -- ce n'est pas son joueur qui le joue.
bool lockstep_peer_owns_character(const void* character);

// Mewgenics Multi (protocole 52) : chez l'hote, ce chat est celui d'un absent, et
// l'IA du jeu le joue (mm_ai.h). Pour le journal de la barre grisee.
bool lockstep_ai_character(const void* character);

// --- diagnostics ------------------------------------------------------------

struct LockstepStats {
    uint32_t sent = 0, applied = 0, pending = 0;
    uint32_t desyncs = 0;
    uint32_t cats = 0, local_cats = 0;
    bool     halted = false;
};
LockstepStats lockstep_stats();

// True once a HASH mismatch or a HALT from the peer has stopped the battle.
bool lockstep_halted();

// Whether this peer is standing in a battle it has already snapshotted -- i.e.
// a fight is on screen and lockstep is driving it.
//
// The distinction that matters is against a peer which is merely CONNECTED
// while the other one fights: that peer has no snapshot, so it returns false
// and is safe to push state at. A peer for which this returns true has live
// Character objects whose state belongs to the deterministic stream, and
// writing to them out of band changes the battle underneath the hashes.
//
// mgmp_catsync and mgmp_invsync gate their apply paths on it. See the note on
// catsync_on_message for the shield that got through before they did.
bool lockstep_in_battle();

// The aim preview writes Character+0x388, and the backstab test reads it, so a
// wall-clock-gated presentation write decides damage. Call `begin` on entry to
// Brain::UpdateDecision and `end` on the way out: whatever the preview turned
// the cat to is put back, leaving facing written only by committed actions.
// Both are inert outside a net session. See the long note on the definitions.
// Everything mgmp_aim needs about one character, answered once: is it a HUMAN
// cat in this battle's roster at all, which index, and does the PEER own it.
// False for a summon, for an AI cat and outside a battle -- an aim preview is
// meaningless in all three cases.
bool lockstep_aim_subject(const void* character, uint8_t& cat, bool& peer_owns);

void     lockstep_preview_facing_begin(void* brain);
void     lockstep_preview_facing_end();
uint32_t lockstep_preview_facing_count();

// --- the state fence -------------------------------------------------------
//
// Snapshot every snapped cat's simulation state, run something, then check that
// none of it moved. Built for mgmp_aim, which calls a function INTO the game on
// the one peer that does not own the cat -- the shape where any state write is
// a divergence by construction.
//
// `end` puts FACING back, because that is the field the aim path is known to
// write and the one whose correct value is unambiguous (the last action's).
// Everything else it can only report: a moved HP or tile is already a
// divergence and inventing a value for it would hide that. Returns the number
// of cats that changed, and names the first few in the log.
//
// Cheap enough for a per-frame caller: one pass over the roster, no allocation,
// and it returns immediately when lockstep is not in a battle.
void     lockstep_state_fence_begin();
uint32_t lockstep_state_fence_end(const char* what);
uint32_t lockstep_state_fence_hits();

// --- Mewgenics Multi (protocole 52) : GetChoice d'un chat humain -------------
//
// Autour de l'original de Brain::GetChoice (h_GetChoice), pour un chat humain en
// combat commun : le flux de la simulation est remis comme avant s'il a bouge.
// Le jeu y fait choisir son IA pour un chat charme ou incontrolable, et chaque
// joueur qui attend la decision de ce chat la referait a chaque image : autant
// de tirages que d'images d'attente. La decision, elle, voyage. Sans effet pour
// un chat que son joueur dirige lui-meme (aucun tirage) ; rien pour un ennemi.
// Le jeton vit sur la pile de h_GetChoice : un GetChoice imbrique dans
// l'original aurait le sien, sans desarmer celui du dessus.
struct ChoicePin {
    uint64_t* s = nullptr;
    uint64_t  before[4] = {};
    uint32_t  flight = 0;
    uint8_t   cat = 0xFF;
    bool      armed = false;
};
void lockstep_choice_pin(void* brain, ChoicePin& pin);
void lockstep_choice_unpin(ChoicePin& pin);

} // namespace mgmp
