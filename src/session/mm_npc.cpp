// mm_npc -- Mewgenics Multi : les PNJ en direct. Voir mm_npc.h.

#include "mm_npc.h"
#include "mm_npc_state.h"

#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"   // savefile_in_coop
#include "mgmp_session.h"    // session_ready
#include "mm_menu.h"         // mm_in_house, mm_in_game

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// L'objet de progression : MewDirector+0x5A8, rempli depuis la save par
// sub_1401D2A40 (appele par le chargement du Director, 0x1403AEFD0) ; c'est
// aussi l'objet du boss de la maison (mm_menu.cpp). Ses 8 PNJ commencent a
// +0x48, 0x78 octets chacun ; leurs champs sont decrits dans mm_npc_state.h.
constexpr uintptr_t kDir_Progress = 0x5A8;
constexpr uintptr_t kProg_Npcs    = 0x48;
constexpr uintptr_t kNpcStride    = 0x78;
constexpr uintptr_t kNpc_Taken    = 0x00;   // vector : debut, fin a +0x08
constexpr uintptr_t kNpc_Cur      = 0x18;   // std::string
constexpr uintptr_t kNpc_Simple   = 0x38;   // les champs simples, de 0x38 a 0x78
constexpr uintptr_t kTakenElem    = 0x28;   // une recompense prise (sub_1401D84D0 : pas de 0x28)

constexpr uint32_t kHostEvery   = 30;       // l'hote relit ses PNJ deux fois par seconde
constexpr uint32_t kClientEvery = 10;       // l'invite regarde six fois par seconde s'il est chez lui
constexpr uint32_t kRetry       = 120;      // apres une lecture ratee : nouvel essai dans 2 s
constexpr uint32_t kSayEvery    = 60 * 60;  // une ligne d'erreur par minute au plus

struct State {
    bool     on        = false;
    bool     is_client = false;
    uint32_t tick      = 0;
    uint32_t said_at   = 0;                  // image de la derniere ligne d'erreur

    // --- hote ---
    std::vector<uint8_t> latest;            // le dernier etat lu, encode
    NpcState             latest_s[kNpcCount];
    bool                 has_latest = false;
    uint32_t             sends = 0;

    // --- invite ---
    NpcState host[kNpcCount];
    uint64_t host_hash = 0;
    bool     has_host  = false;
    bool     due       = false;              // a recopier des que possible
    uint32_t retry_at  = 0;                  // pas avant cette image (lecture ratee)
    bool     was_home  = false;
    uint8_t  said_step[kNpcCount]  = {};     // « l'hote a pris la recompense » deja dit
    uint8_t  said_ready[kNpcCount] = {};     // « recompense prete chez l'hote » deja dit
    uint32_t copied    = 0;                  // PNJ mis a jour depuis le debut de la session
};

State g;

bool others_here() {
    return session_ready() && net_active() && net_peer_count() >= 2;
}

// Le Director du jeu, s'il est vivant. Son emplacement global (D_MewDirectorPtr)
// n'est jamais remis a zero : un nouveau Director est cree a chaque chargement
// d'une save, et le jeu verifie lui-meme, avant de s'en servir, que l'etiquette
// placee juste avant l'objet (ptr-8) est celle notee a cote de l'emplacement
// (+8) -- la carte des PNJ le fait en 0x140277A68, le constructeur les pose en
// 0x1403A6BD4.
void* progress() {
    const void** slot = (const void**)addr_of_data(D_MewDirectorPtr);
    void*    dir  = nullptr;
    uint64_t tag  = 0, mark = 0;
    void*    prog = nullptr;
    if (!slot || !mem_read(slot, &dir, sizeof(dir)) || !dir) return nullptr;
    if (!mem_read(slot + 1, &tag, sizeof(tag)) ||
        !mem_read((const char*)dir - 8, &mark, sizeof(mark)) || mark != tag)
        return nullptr;
    if (!mem_read((const char*)dir + kDir_Progress, &prog, sizeof(prog))) return nullptr;
    return prog;
}

char* npc_at(void* prog, int i) {
    return (char*)prog + kProg_Npcs + (uintptr_t)i * kNpcStride;
}

// Une recompense visee a la forme d'un identifiant du jeu (« jack_shopupgrade2 »),
// ou est vide (nouvelle partie : le jeu la remplit plus tard ; ou toutes prises).
bool name_ok(const std::string& s) {
    if (s.size() > 64) return false;
    for (char c : s)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
            return false;
    return true;
}

// nullptr si lu, sinon ce qui n'allait pas.
const char* read_npc(void* prog, int i, NpcState& s) {
    const char* e = npc_at(prog, i);
    uintptr_t first = 0, last = 0;
    char      cur[96] = {};
    uint8_t   b[0x40] = {};
    if (!mem_read(e + kNpc_Taken, &first, sizeof(first)) ||
        !mem_read(e + kNpc_Taken + 8, &last, sizeof(last)))
        return "liste illisible";
    if (last < first || last - first > 1000 * kTakenElem || (last - first) % kTakenElem != 0)
        return "liste incoherente";
    if (!mem_read_std_string(e + kNpc_Cur, cur, sizeof(cur))) return "recompense visee illisible";
    if (!mem_read(e + kNpc_Simple, b, sizeof(b))) return "champs illisibles";
    s = NpcState{};
    s.cur = cur;
    if (!name_ok(s.cur)) return "recompense visee inattendue";
    s.taken = last - first;
    memcpy(&s.a38,   b + 0x00, 4);   // E+0x38
    memcpy(&s.favor, b + 0x08, 4);   // E+0x40
    memcpy(&s.c44,   b + 0x0C, 4);   // E+0x44
    s.ready = b[0x10];               // E+0x48
    memcpy(&s.e4c,   b + 0x14, 4);   // E+0x4C
    s.f50 = b[0x18];                 // E+0x50
    s.g51 = b[0x19];                 // E+0x51
    memcpy(&s.i54,   b + 0x1C, 4);   // E+0x54
    memcpy(s.h58,    b + 0x20, 32);  // E+0x58
    return nullptr;
}

// Les 8 PNJ du Director vivant. Faux (et une ligne au journal, une par minute
// au plus) si l'un d'eux ne ressemble pas a ce qu'on attend : on n'y touche pas.
bool read_all(void* prog, NpcState (&s)[kNpcCount], const char* who) {
    const char* why = nullptr;
    int bad = -1;
    if (!prog) why = "pas de partie chargee";
    for (int i = 0; !why && i < kNpcCount; ++i)
        if ((why = read_npc(prog, i, s[i])) != nullptr) bad = i;
    if (!why) return true;
    const uint32_t now = g.tick ? g.tick : 1;
    if (!g.said_at || now - g.said_at > kSayEvery) {
        g.said_at = now;
        if (bad >= 0) log_line("NPC", "!! %s : PNJ %s, %s -- PNJ en direct en pause", who,
                               npc_name(bad), why);
        else          log_line("NPC", "!! %s : %s -- PNJ en direct en pause", who, why);
    }
    return false;
}

// Seulement les champs qui different ; jamais « recompense prete » (0x48), ni
// ce que le serialiseur ignore (0x3C, le bourrage).
bool write_npc(void* prog, int i, const NpcState& t, const NpcState& mine) {
    char* e = npc_at(prog, i);
    bool ok = true;
    auto put = [&](uintptr_t off, const void* v, size_t n, bool differs) {
        if (ok && differs) ok = mem_write(e + off, v, n);
    };
    put(0x38, &t.a38,   4,  t.a38   != mine.a38);
    put(0x40, &t.favor, 4,  t.favor != mine.favor);
    put(0x44, &t.c44,   4,  t.c44   != mine.c44);
    put(0x4C, &t.e4c,   4,  t.e4c   != mine.e4c);
    put(0x50, &t.f50,   1,  t.f50   != mine.f50);
    put(0x51, &t.g51,   1,  t.g51   != mine.g51);
    put(0x54, &t.i54,   4,  t.i54   != mine.i54);
    put(0x58, t.h58,    32, memcmp(t.h58, mine.h58, sizeof(t.h58)) != 0);
    return ok;
}

// --- hote -------------------------------------------------------------------

bool send_latest(bool all, uint8_t peer) {
    if (g.latest.empty()) return false;
    SaveFileMsg m{};
    m.size = (uint32_t)g.latest.size();
    m.hash = savefile_hash(g.latest.data(), m.size);
    m.data = g.latest.data();
    strncpy_s(m.name, "npc", _TRUNCATE);
    const bool ok = all ? net_send_npc(m) : net_send_npc_to(peer, m);
    if (ok) ++g.sends;
    else    log_line("NPC", "!! envoi de l'etat des PNJ impossible");
    return ok;
}

void log_host_changes(const NpcState (&s)[kNpcCount]) {
    if (!g.has_latest) {
        char line[256] = {};
        int  off = 0;
        for (int i = 0; i < kNpcCount && off < (int)sizeof(line) - 24; ++i)
            off += _snprintf_s(line + off, sizeof(line) - off, _TRUNCATE, "%s%s %u",
                               i ? ", " : "", npc_name(i), s[i].favor);
        log_line("NPC", "PNJ de l'hote (faveur) : %s", line);
        return;
    }
    for (int i = 0; i < kNpcCount; ++i) {
        const NpcState& was = g.latest_s[i];
        if (!npc_same_step(s[i], was))
            log_line("NPC", "PNJ %s : recompense prise (%s -> %s)", npc_name(i), was.cur.c_str(),
                     s[i].cur.c_str());
        else if (s[i].favor != was.favor || s[i].ready != was.ready)
            log_line("NPC", "PNJ %s : faveur %u -> %u%s", npc_name(i), was.favor, s[i].favor,
                     s[i].ready ? " (recompense prete)" : "");
    }
}

// Partout ou une partie est chargee, pas seulement a la maison : la fin d'une
// aventure donne de la faveur (l'orgue), et l'invite qui rentre chez lui doit
// trouver l'etat d'apres, pas celui d'avant le depart.
void host_pump() {
    if (++g.tick % kHostEvery) return;
    if (!mm_in_game()) return;
    NpcState s[kNpcCount];
    if (!read_all(progress(), s, "hote")) return;
    std::vector<uint8_t> enc = npc_state_encode(s);
    if (g.has_latest && enc == g.latest) return;
    log_host_changes(s);
    g.latest = std::move(enc);
    for (int i = 0; i < kNpcCount; ++i) g.latest_s[i] = s[i];
    g.has_latest = true;
    if (others_here()) send_latest(true, 0);
}

// --- invite -----------------------------------------------------------------

// Vrai si fini (meme sans rien a changer), faux s'il faut reessayer plus tard.
bool client_apply() {
    void* prog = progress();
    NpcState mine[kNpcCount];
    if (!read_all(prog, mine, "invite")) return false;
    for (int i = 0; i < kNpcCount; ++i) {
        const NpcState& h = g.host[i];
        if (!npc_same_step(h, mine[i])) {
            if (!g.said_step[i])
                log_line("NPC", "PNJ %s : l'hote en est a « %s » (ici « %s ») -- mis a jour au "
                                "prochain chargement de cette save", npc_name(i), h.cur.c_str(),
                         mine[i].cur.c_str());
            g.said_step[i] = 1;
            continue;
        }
        if (h.ready && !g.said_ready[i]) {
            log_line("NPC", "PNJ %s : recompense prete chez l'hote, c'est lui qui la prend "
                            "(faveur gardee a %u ici)", npc_name(i), mine[i].favor);
            g.said_ready[i] = 1;
        }
        const NpcState t = npc_guest_target(h, mine[i]);
        if (!npc_simple_fields_differ(t, mine[i])) continue;
        if (!write_npc(prog, i, t, mine[i])) {
            log_line("NPC", "!! PNJ %s : ecriture impossible", npc_name(i));
            continue;
        }
        ++g.copied;
        if (t.favor != mine[i].favor)
            log_line("NPC", "PNJ %s : faveur %u -> %u, comme chez l'hote", npc_name(i),
                     mine[i].favor, t.favor);
        else
            log_line("NPC", "PNJ %s : mis a jour comme chez l'hote", npc_name(i));
    }
    return true;
}

void client_pump() {
    ++g.tick;
    // Plus d'hote : son dernier etat ne vaut plus rien. Ce joueur peut de nouveau
    // donner ses chats lui-meme (le tuyau ne refuse qu'en session), et un vieil
    // etat ecraserait sa faveur au prochain retour dans sa maison.
    if (!others_here()) {
        if (g.has_host) log_line("NPC", "plus d'hote : ses PNJ ne sont plus recopies");
        g.has_host = false;
        g.due      = false;
        return;
    }
    if (g.tick % kClientEvery) return;
    // Sa propre maison : ni le menu, ni la copie de l'aventure de l'hote.
    const bool home = mm_in_house() && !savefile_in_coop();
    if (home && !g.was_home) g.due = true;    // sa save vient de se charger
    g.was_home = home;
    if (!home || !g.due || !g.has_host) return;
    if (g.retry_at && (int32_t)(g.tick - g.retry_at) < 0) return;
    g.retry_at = 0;
    if (client_apply()) g.due = false;
    else                g.retry_at = g.tick + kRetry;
}

} // namespace

// ---------------------------------------------------------------------------

void npc_init() {
    g = State{};
    g.on        = true;
    g.is_client = net_role() == NetRole::Client;
    log_line("NPC", g.is_client
        ? "arme -- la faveur des PNJ de l'hote sera recopiee en direct dans la maison de ce joueur"
        : "arme -- l'etat des PNJ de l'hote sera envoye aux invites a chaque changement");
}

void npc_shutdown() {
    if (!g.on) return;
    log_line("NPC", "fin : %u envoi(s), %u PNJ mis a jour", g.sends, g.copied);
    g = State{};
}

void npc_pump() {
    if (!g.on) return;
    if (g.is_client) client_pump();
    else             host_pump();
}

void npc_catchup(uint8_t peer) {
    if (!g.on || g.is_client || !g.has_latest) return;
    if (send_latest(false, peer))
        log_line("NPC", "-> etat des PNJ envoye au joueur %u", (unsigned)peer);
}

void npc_on_message(const SaveFileMsg& m) {
    if (!g.on) return;
    if (!g.is_client) {
        log_line("NPC", "!! etat des PNJ recu alors que ce joueur heberge -- ignore");
        return;
    }
    NpcState s[kNpcCount];
    if (!m.data || savefile_hash(m.data, m.size) != m.hash || !npc_state_decode(m.data, m.size, s)) {
        log_line("NPC", "!! etat des PNJ de l'hote illisible (%u octets) -- ignore", m.size);
        return;
    }
    if (g.has_host && m.hash == g.host_hash) return;
    if (!g.has_host) log_line("NPC", "<- etat des PNJ de l'hote recu");
    for (int i = 0; i < kNpcCount; ++i) {
        if (!npc_same_step(s[i], g.host[i])) g.said_step[i] = 0;   // une nouvelle etape a signaler
        if (!s[i].ready) g.said_ready[i] = 0;
        g.host[i] = s[i];
    }
    g.host_hash = m.hash;
    g.has_host  = true;
    g.due       = true;
    g.retry_at  = 0;
}

} // namespace mgmp
