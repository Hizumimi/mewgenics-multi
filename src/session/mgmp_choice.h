#pragma once
// mgmp_choice -- replicate the PLAYER'S CHOICE on the two decision screens,
// instead of replicating what the choice does.
//
// This module is the answer to "how do shop, level-up and world events sync",
// and the answer turned out to be that almost none of it needs syncing.
//
// ---------------------------------------------------------------------------
// WHY A CHOICE IS ENOUGH
// ---------------------------------------------------------------------------
//
// The reflex is to replicate EFFECTS: the 119 authored commands
// WorldEvent::spin dispatches write the familiar list at MewDirector+1576, the
// next-fight spawn queue at +1552, the adventure-token vector at +1664, legacy
// counters in the save-cache at +56, and more. Shipping all of that is a
// RUNSTATE message with a field list nobody can prove is complete.
//
// None of it is necessary, because both peers already compute it identically:
//
//   1. THE CLIENT RUNS THE EVENT TOO. mgmp_follow calls the game's own
//      EnterNode on the client for the node the host entered -- every node
//      type, not just battles -- so the client opens its own WorldEvent screen.
//      (docs/NOTES_MGMP.md used to claim the opposite. It is wrong; see
//      core/mgmp_hooks.cpp h_MapUpdate, which has always called o_EnterNode.)
//
//   2. ENTERING A NODE RE-SEEDS THE SIMULATION STREAM, FOR EVERY NODE TYPE.
//      EnterNode copies MapNode+0x118 into TLS+0x178 at 0x1403910C2, before any
//      node-type dispatch -- the `sub eax,5 / cmp eax,1` that picks out combat
//      comes later, and the only branch ahead of the copy is the shift key. So
//      two peers begin the same event from a byte-identical RNG state.
//
//   3. EVERY DRAW ON THE EVENT PATH IS ON THAT STREAM. Checked, not assumed:
//      the item-pool draw at 0x1408DB3FE (`mov r8d,178h; mov rax,gs:58h; add
//      r8,[rax]`) which serves get_item_from_pool, the most common effect in
//      the shipped data at 340 occurrences; random_chance's randfloat at
//      0x14092C62D; reward's RollChance at 0x14092CCBA;
//      party_skip_next_fight_chance's rand2 at 0x140924649. No presentation
//      stream (+0x198) anywhere in the subtree.
//
//   4. THE LEVEL-UP ROLLER IS DETERMINISTIC TOO, DESPITE NOT BEING ON THAT
//      STREAM. It runs a HEAP xoshiro state at object+0xB8, which looks like a
//      problem until you read where it is seeded (0x140379CA5):
//
//          mov  rbx, [r15+0A0h]     ; the CatData*
//          mov  rdx, [rbx]          ; CatData+0x00 -- the cat's persistent id
//          call sub_14094ABB0       ; splitmix64(id)
//          mov  r8d, [rbx+0C30h]    ; a per-cat counter
//          call sub_14094AA50       ; xoshiro jump(), applied that many times
//
//      Both inputs live in CatData and both are round-tripped by
//      SerializeCatData (the id right after the version tag, +0xC30 at line 632
//      of the serializer). So the offered options are a pure function of cat
//      state the client already has. It also explains the design: a given cat
//      always rolls the same options, which makes a reroll save-scum-proof.
//
// So the ONE thing neither peer can derive is which button a person pressed.
// That is all that crosses the wire, and it is why this file is short and
// RUNSTATE does not exist.
//
// A LIMIT WORTH STATING. Three of the lower-frequency commands' RNG sources
// (weather_roll, add_weather, next_fight_from_set, next_event_from_set,
// scramble_*, make_old, upgrade_*) were NOT traced to a primitive -- a two-level
// scan found none and the deeper scan was skipped by choice. They are assumed
// to be on the simulation stream like everything else that was checked. If a
// desync ever shows up on an event node and the choice indices matched, these
// are the first suspects.
//
// ---------------------------------------------------------------------------
// THE TWO BOUNDARIES, WHICH ARE THE SAME SHAPE
// ---------------------------------------------------------------------------
//
// Both screens keep their offered options in a 240-BYTE-STRIDE ARRAY and commit
// through a single function, so each needs exactly one hook that does double
// duty: capture the host's pick, swallow the client's.
//
//              options array        commit                    injected by
//   event      WorldEvent+224..232  sub_140937F30(capture)    calling it
//   level-up   LevelUpScreen+864..872  select_option(opt)     sub_140386810
//
// sub_140937F30 is three statements and the middle one IS the choice:
//
//     *(u8*)(MewDirector + 1812)   = 1;             // a choice was made
//     *(void**)(WorldEvent + 256)  = capture[2];    // the chosen option entry
//     return sub_14091AA00(WorldEvent);             // run it
//
// capture is {vftable, WorldEvent*, entry*}, and `entry` is an element of that
// array -- so the host's pick is an index and injecting one is 24 bytes of
// stack. The level-up side is the mirror image: its button callback
// sub_140386810 takes {vftable, LevelUpScreen*, int index} and does the
// `+ 240*index` itself, so we call THAT rather than hand-building a
// LevelUpOption.
//
// RESOLVE BY INDEX, VALIDATE BY NAME -- the rule the battle layer already
// follows for abilities, and the one that caught the equipment bug. The index
// is stable because both peers build their option lists in authored file order
// from the same resources.gpak, which HELLO hashes. The name rides along and a
// mismatch is shouted about, because it means the two lists were BUILT
// differently, which no index can survive.
//
// ---------------------------------------------------------------------------
// WHAT THIS DOES NOT DO
// ---------------------------------------------------------------------------
//
// There is NO per-node hash yet, and that is the real debt this design takes
// on. State-push sync is self-correcting: a divergence gets overwritten by the
// next CATDATA. Determinism sync is not -- a divergence compounds silently,
// which is exactly why the battle layer carries a per-turn hash and halts. The
// meta layer now needs the same and does not have it. Until it does, the
// existing per-node CATDATA/INVENTORY pushes will paper over a divergence
// rather than report it.
//
// ---------------------------------------------------------------------------
// MEWGENICS MULTI : CHACUN CHOISIT POUR SES CHATS (protocole 44)
// ---------------------------------------------------------------------------
//
// AVANT. L'hote choisissait toutes les montees de niveau, chats d'invites
// compris ; les clics des invites etaient retenus. La relance des options
// (LevelUpScreen::Reroll) et le choix de la capacite a remplacer (AbilityChooser,
// ouvert par la montee de niveau, par un evenement ou par un livre d'objet)
// n'etaient pas repliques du tout : l'hote relancait seul, puis son choix tombait
// sur une liste plus courte chez l'invite ; chacun choisissait sa capacite.
//
// MAINTENANT, en aventure mixte (team_owners_active) :
//   - Trois decisions sont celles du proprietaire du chat (LevelUpScreen+0xA0 ;
//     pour un AbilityChooser, le chat donne a son init) : le choix d'une option de
//     montee de niveau, la relance, et la capacite choisie (ou l'annulation) dans
//     un AbilityChooser de montee de niveau ou d'evenement. L'hote decide pour ses
//     chats, pour une recrue, et pour un chat dont le proprietaire n'est pas dans
//     l'aventure. (Les options d'evenement : voir protocole 45 plus bas.)
//   - Chez celui qui ne decide pas, le clic est retenu, et le titre de l'ecran
//     (« title » du panneau, ou « prompt » de l'AbilityChooser) dit qui choisit.
//   - Un invite proprietaire ne joue pas son clic : il le demande a l'hote
//     (ChoiceMsg, kChoiceFlagRequest, avec le chat), qui verifie que le chat est a
//     lui, attend son propre ecran pour ce chat, y rejoue le choix par le chemin du
//     jeu, puis le publie a tous comme un des siens -- le demandeur l'applique
//     alors comme les autres.
//   - Chez un invite, les choix publies forment une FILE, appliquee dans l'ordre :
//     la tete seule, sur l'ecran de son genre (evenement, montee de niveau,
//     AbilityChooser), quand c'est le bon (meme case, meme chat, ecran pas deja
//     engage). Une tete qui ne va jamais sur l'ecran la (60 s) est laissee.
//   - Un AbilityChooser ouvert par un livre de l'ecran des objets reste local :
//     seul son proprietaire l'a (mm_rungear envoie ce qu'il change).
//   - JAMAIS SUR L'ECRAN D'APRES. Un chat peut monter de deux niveaux de suite : une
//     demande faite sur le premier ecran, arrivee chez l'hote apres qu'il l'a
//     decide, attendrait le second et s'y jouerait. D'ou : (a) un clic sur un
//     ecran deja engage (+0x318) ne publie ni ne demande rien ; (b) chez
//     l'invite, une demande sans reponse retient les clics suivants du meme genre
//     sur le meme ecran (3 s au plus), et une relance derriere une option ; (c)
//     chez l'hote, une option prise laisse les autres demandes d'option et de
//     relance de ce chat, une capacite prise les autres demandes de capacite ;
//     (d) une relance porte l'empreinte de son ecran (le nom de sa premiere
//     option, que les relances ne changent pas) ; (e) une demande de capacite
//     pour un AbilityChooser de montee de niveau qui n'est pas ouvert chez l'hote
//     (le sien s'ouvre toujours avant celui de l'invite) est laissee.
//   - Chez l'hote, l'option demandee juste apres une relance demandee (la liste a
//     grandi entre les deux) est prise si son nom et son type sont a leur place,
//     et publiee avec la liste de l'hote.
//   - Un choix d'evenement ne se joue pas sur la liste ou le precedent vient de
//     l'etre (2 s au plus) : l'etape suivante n'est peut-etre pas encore batie.
//   - Le delai de 60 s d'une tete de file court aussi quand elle retient un choix
//     pour l'ecran qui est la (plus loin dans la file) ; en quittant une case, les
//     choix de cette case qui attendent encore sont laisses.
//
// ---------------------------------------------------------------------------
// MEWGENICS MULTI : LES EVENEMENTS AUSSI (protocole 45)
// ---------------------------------------------------------------------------
//
// AVANT. L'hote choisissait l'option de chaque evenement, meme quand il parlait
// d'un chat d'invite. Le chat d'un evenement « cat_choice: choose » (un joueur le
// choisit, setupCatChoice puis le « ok » du CatSelector) n'etait pas replique :
// chacun envoyait le sien de son cote, et les deux aventures pouvaient diverger.
//
// MAINTENANT :
//   - L'option d'un evenement est au proprietaire du chat de l'evenement
//     (WorldEvent+0x128) -- l'hote pour un evenement sans chat, une recrue, un chat
//     dont le proprietaire a quitte l'aventure, et sans equipe mixte. Meme chemin
//     que les montees de niveau : un invite demande (kChoiceEvent,
//     kChoiceFlagRequest, cat_id le chat, serial les choix d'option deja faits dans
//     la case), l'hote verifie que le chat est le sien et que la demande vise
//     l'etape ou il en est (une demande faite sur une etape deja decidee n'est
//     jamais jouee sur la suivante), la joue par le chemin du jeu, puis la publie.
//   - Un choix d'evenement ne se joue que quand les options sont a l'ecran
//     (WorldEvent+0x19F9, que setupActionChoice pose et que le choix joue remet a
//     zero tout de suite) : la liste existe des WorldEvent::init, et joue a la page
//     du titre il faisait sauter le clip a l'etape d'apres sans montrer les
//     options. Attendre cette page ne compte pas dans le delai de 60 s de la file
//     (Apply::Hold) : un joueur qui lit le texte ne perd pas le choix.
//   - Le chat d'un evenement « choose » (kChoiceEventCat) : chacun ne peut envoyer
//     que ses chats (son « ok » sur le chat d'un autre est retenu, et l'ecran dit
//     « Ce chat est a X » trois secondes) ; un invite le demande a l'hote ; le
//     premier pris chez l'hote l'emporte, et il est rejoue chez tous quand leur
//     choix du chat s'ouvre (CatSelector::select, puis le « ok » du jeu).
//   - Chez celui qui ne decide pas, les options a l'ecran, l'ecran de l'evenement
//     dit « A X de choisir » : dans son titre s'il se lit, sinon une ligne sous son
//     texte (« prompt ») -- en build 23 le titre ne se lisait pas (sans doute pas
//     de champ « title » dans le clip de l'evenement : MenuPanel::set_text n'y
//     ecrit alors rien), et rien n'etait ecrit. Le journal dit pourquoi un champ ne
//     se lit pas. Le texte du jeu revient une fois le choix fait, s'il est encore
//     celui que le mod a modifie (le resultat du choix le remplace). Le champ
//     n'est reecrit que si le jeu l'a change (une autre page : la ligne suit), et
//     un texte qui contient deja la ligne n'est jamais pris pour celui du jeu (un
//     setter qui reformaterait ce qu'il garde ne la ferait pas doubler). « A toi
//     de choisir » quand ce joueur prend la main, tant que les options sont la.
//   - JAMAIS UN ECRAN FIGE. (a) Le choix du chat est suivi par WorldEvent (adresse
//     et panneau), ouvert par setupCatChoice ou par un clic sur son « ok » (le
//     bouton n'existe que pendant ce choix : une session commencee sur cet ecran
//     ne l'a pas vu s'ouvrir), ferme par un « ok » joue ou par les options a
//     l'ecran ; ni le mode « cat_choice » ni le chat pose ne servent a le dire. (b)
//     L'hote publie le chat que le jeu a pose apres son « ok », pas celui que
//     montrait le CatSelector. (c) Un invite qui ne peut pas rejouer le chat choisi
//     ailleurs choisit le sien chez lui (comme avant). (d) Une demande du
//     proprietaire pour l'etape ou en est l'hote qui ne se joue pas chez lui (les
//     listes different) : l'hote prend la main pour cette etape. (e) Attendre la
//     page du choix (Hold) dure 5 min au plus, et garde vivante chez l'hote la
//     demande qui attend. (f) +0x19F9 et le rejeu du chat ne servent que sur le
//     build vise (resolve_is_pinned_build) ; ailleurs, la liste qui change (10 s au
//     plus) et le chat choisi par chacun, comme avant.
#include <cstdint>

namespace mgmp {

enum Target : int;
struct ChoiceMsg;

// Mewgenics Multi : detours et originaux de T_LevelReroll, T_AbilityInit,
// T_AbilityPick, T_AbilityUpdate, T_EventCatChoice et T_EventCatPick.
bool choice_binding(Target t, void** detour, void*** original);

void choice_set_base(uintptr_t base);
void choice_init();
void choice_shutdown();

// The hooks call these. Each returns true if the ORIGINAL should run.
//
// `cap` is the {vftable, WorldEvent*, entry*} capture block sub_140937F30 takes.
bool choice_on_event_commit(void* cap);
// `screen` / `opt` are select_option's two arguments.
bool choice_on_level_select(void* screen, void* opt);

// Per-frame ticks. Both cache the screen pointer and apply a held choice.
void choice_on_event_update(void* world_event);
void choice_on_level_update(void* level_screen);

// `from` : l'expediteur (l'hote recoit les demandes des invites).
void choice_on_message(uint8_t from, const ChoiceMsg& m);

// This peer has ENTERED the node with this seed. Called by mgmp_follow from
// remember_node, on both peers, whenever the node actually changes.
//
// It does NOT drop held choices, and that restraint is load-bearing. A choice
// can legitimately arrive for a node this peer has not reached yet -- the host
// finishes node 17's battle and picks a level-up while the client is still
// walking into node 6 -- so "the seed is not the one I am standing in" says
// nothing about staleness. See apply_pending.
//
// What it does do is forget the cached WorldEvent*/LevelUpScreen*. Those
// screens belong to the node just left and may already be destroyed, and
// choice_on_message applies straight into whichever pointer is held.
void choice_on_node_entered(uint64_t node_seed);

// mgmp_follow has DISCARDED a node it was told about without entering it --
// queue overflow, or a validation failure (map size, index range, seed). That
// is the only way a node is passed over now that the pending queue exists, and
// it is therefore the only moment at which a held choice can be known stale.
//
// This is what stops a choice outliving its node, which is the fault that
// corrupted a run on 2026-08-25: node 6's 'int' was applied to node 3's event
// two nodes later, because nothing anywhere knew node 6 had been skipped. See
// ChoiceMsg::node_seed.
void choice_on_node_skipped(uint64_t node_seed);


// Mewgenics Multi (hote) : un choix (evenement, montee de niveau) a deja ete
// publie dans la case actuelle. Un invite qui arrive maintenant recevrait une
// save d'avant ce choix, et ce choix ne lui serait jamais renvoye.
bool choice_made_in_node();

// Mewgenics Multi : oublie les ecrans retenus (une autre partie se charge).
void choice_forget_screens();

// Mewgenics Multi (mm_rungear) : un ecran d'evenement ou de montee de niveau a
// tourne chez ce joueur dans les `within_ms` dernieres millisecondes. Tant qu'il
// est la, un choix de l'hote peut encore s'y appliquer : on ne relit pas les
// chats par-dessus.
bool choice_screen_recent(uint64_t within_ms);
// Mewgenics Multi : oublie aussi les choix retenus et la case actuelle (une
// copie a jour de l'aventure de l'hote vient d'arriver, elle les contient).
void choice_forget();

} // namespace mgmp
