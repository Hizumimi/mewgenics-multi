#pragma once
// mgmp_catsync -- the host's cats are pushed to the client, as bytes the GAME
// produced, through the game's own serializer.
//
// THE BUG THIS CLOSES, measured 2026-08-24. Both peers loaded byte-identical
// saves (hash 78887bfc13cd5b04 on both). The host then equipped a trinket during
// the adventure. Four turns into the next battle:
//
//   !! HALT at turn 4: cat 19 slot 4:4 (gon 'tk_GlowingCoin') is empty on this peer
//
// A save file syncs the run at the moment it is written and nothing after it, so
// every meta-layer change since then is invisible. Worse, it is invisible to the
// detector too: `state_hash` covers HP, shield, max HP, tile, facing and dead --
// not abilities and not equipment -- so the peers agreed on every hash right up
// to the halt. What caught it was the ability cross-check (resolve by slot,
// validate by GON name), which is the one thing in the battle layer that looks
// at what a cat can DO rather than at what has happened to it.
//
// WHY BYTES AND NOT A REPLAYED ACTION. Replicating the click was the obvious
// plan, and the map layer's success makes it look easy. It is a dead end:
//
//   - glaiel::InventoryItemBox::click() @ 0x14034DF60 IS a clean boundary by the
//     usual test -- exactly one code caller, only data xref is .pdata, so not
//     virtual. But it is `void click(void)`: everything about what was clicked
//     lives in `this`, a UI box that exists only while the inventory screen is
//     open, and the client is not in that screen.
//   - it only STARTS a flow. Confirmation popup, then an AbilityChooser -- a
//     second decision point -- then lambdas whose actual mutation fans out over
//     ~8 CatData helpers. The bottom of that stack, sub_1400B3920, has 40 call
//     sites: a shared container primitive, the same trap as
//     CustomVector<T*>::push_back.
//
// So replaying means driving a UI flow on a peer that is not in it, and
// reimplementing means reproducing eight mutations exactly, where a single miss
// diverges SILENTLY. Both are worse than the halt they would replace.
//
// WHAT MAKES THE THIRD OPTION WORK:
//
//   void glaiel::SerializeCatData(CatData&, ByteStream&, bool)  @ 0x14022E9A0
//
// is BIDIRECTIONAL. Every field branches on the stream mode at ByteStream+0x00
// (0 = read, 1 = write), so the same function that saves a cat on the host loads
// one on the client. We reimplement nothing; we move bytes the game wrote into
// the game's own reader. The format even carries its own version tag (19) as its
// first field, and both peers run the same pinned build.
//
// Identity is free here, unlike everywhere else in this project. CatData+0x00 is
// a u64 id, serialized right after that version tag, and the run holds a
// registry that maps it to a CatData*. No index scheme, no name cross-check, no
// pointer that dies between runs.
//
// This is RUNSTATE from docs/NOTES_MGMP.md's protocol table, arriving early and scoped to
// cats.
//
// WHAT IT DOES NOT COVER, stated plainly: equipping MOVES an item from the run
// inventory onto the cat. Syncing CatData alone leaves the client with the item
// on the cat AND still in its inventory -- a duplicate. The run inventory is
// separate state and needs the same treatment; nothing here or in `state_hash`
// will notice. See "the inventory half" in the .cpp.

#include <cstdint>
#include <vector>

namespace mgmp {

struct CatDataMsg;

// Resolves and prologue-checks the game functions this module calls. Called
// from hooks_verify_module alongside rng_set_base, because a bad address here
// is a refusal at startup rather than a crash three screens later.
void catsync_set_base(uintptr_t base);

void catsync_init();
void catsync_shutdown();

// HOST: serialize every cat in the run, send the ones whose bytes changed since
// the last push. Called where the host enters a map node -- the point both peers
// already synchronize on, and the last moment before a battle can be built out
// of a stale cat. Cheap and idempotent: unchanged cats cost a serialize and a
// hash compare, and send nothing. Mewgenics Multi : returns how many were sent.
uint32_t catsync_publish(const char* why);

// Mewgenics Multi (protocole 58) : L'ETAT D'ENTREE DE LA CASE. La poussee faite a
// l'entree d'une case (follow_on_enter_node, le combat de la Nemesis) garde ici
// une copie de TOUS les chats de la course, envoyes ou non (ceux qui n'avaient pas
// change aussi). Un joueur qui arrive en pleine case la recoit avant la case
// (catsync_send_entry_to) : il construit le combat depuis les chats d'avant la
// case, comme l'hote, et pas depuis ceux du milieu du combat -- puis les decisions
// deja prises le menent ou en est l'hote.
uint32_t catsync_publish_entry(const char* why);
// Envoie a `peer` seul la copie de l'entree. Faux s'il n'y en a pas.
bool catsync_send_entry_to(uint8_t peer);

// HOST: forget what was already sent, so the next catsync_publish is a FULL
// push. For a reconnecting peer -- see the comment on the definition.
void catsync_forget();

// CLIENT: resolve m.id in this peer's run and deserialize over it.
//
// A cat that arrives while this peer is in a battle is HELD, not applied and
// not dropped -- writing it would edit state the fight derives from outside the
// hashed stream, and dropping it loses it for good, because the host dedupes
// against the last hash it sent.
void catsync_on_message(const CatDataMsg& m);

// CLIENT: apply every held cat, if any. Called from the map-follow tick,
// immediately before this peer enters the node the host entered -- the same
// point invsync_apply_pending uses, and for the same two reasons: it is off the
// battle screen, and it is still in time for whatever the node opens.
void catsync_apply_pending(const char* why);

// Mewgenics Multi (invite) : seulement les chats arrives avant l'ENTERNODE de
// rang `bound` (follow_epoch) ; les autres restent gardes, dans l'ordre.
// catsync_apply_pending prend la borne de follow_apply_bound.
void catsync_apply_pending_before(uint32_t bound, const char* why);


// Mewgenics Multi : jette les poussees gardees (une save a jour de l'aventure de
// l'hote vient d'arriver chez un invite qui la rejoint ; elle les contient).
void catsync_drop_pending();

// Mewgenics Multi (mm_rungear) : les chats recus de l'hote et pas encore
// appliques chez cet invite.
uint32_t catsync_pending_count();

// Mewgenics Multi (mm_rungear, invite) : le plus grand numero d'arrivee
// (follow_note_arrival) des poussees appliquees ou laissees parce qu'identiques.
uint32_t catsync_last_applied_arrival();
// Mewgenics Multi (mm_rungear, invite) : seulement les poussees arrivees avant le
// numero `arrival_limit` (et d'avant la prochaine case a suivre).
void catsync_apply_pending_before_arrival(uint32_t arrival_limit, const char* why);
// Mewgenics Multi (mm_rungear, invite) : laisse les poussees gardees qui
// s'appliqueraient maintenant et qui sont identiques au chat de ce joueur (rien a
// relire, meme sous un ecran des objets ouvert). Renvoie celles qui restent.
uint32_t catsync_drop_identical(const char* why);

// Mewgenics Multi (protocole 60) : l'empreinte des chats de la course de ce
// joueur -- l'id et SerializeCatData de chacun, dans l'ordre de la liste (la meme
// serialisation que les poussees, comparee octet par octet d'un joueur a l'autre
// par catsync_drop_identical). Faux si la liste ou un chat ne se lit pas.
bool catsync_run_hash(uint64_t& hash, uint32_t& count);

// Mewgenics Multi (protocole 60) : les poussees gardees arrivees avant le numero
// `limit` (follow_note_arrival) -- combien, ou les jeter (la save qui arrive les
// contient ; celles d'apres restent).
uint32_t catsync_pending_before_arrival(uint32_t limit);
void     catsync_drop_pending_before_arrival(uint32_t limit);

// Mewgenics Multi (equipe mixte). Le chat du registre du jeu a cette cle de save
// (CatData*, charge depuis la table cats s'il ne l'est pas encore ; nul s'il
// n'existe pas), et ses octets tels que SerializeCatData les ecrit -- ceux de la
// table cats une fois decompresses. Sur le thread du jeu seulement.
void* catsync_cat_by_key(uint64_t key);
bool  catsync_serialize_cat(void* cat, std::vector<uint8_t>& out);
// Et l'inverse : ces octets relus dans un chat deja la (comme CATDATA chez un
// invite) -- pour rafraichir un chat du registre que la table vient de changer.
bool  catsync_deserialize_cat(void* cat, const std::vector<uint8_t>& bytes);

} // namespace mgmp
