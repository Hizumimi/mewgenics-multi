// mm_embark -- Mewgenics Multi : le depart ensemble. Voir mm_embark.h.

#include "mm_embark.h"
#include "mm_menu.h"
#include "mm_team.h"

#include "mgmp_addresses.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"
#include "mgmp_savefile.h"

#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// Component+0x20 : la scene du composant (voir mm_menu.cpp) ; un bouton est un
// composant (Button::update est son update).
constexpr uintptr_t kComp_Scene  = 0x20;

// Les delais en millisecondes (horloge du systeme) : compter des images ferait
// courir les deux joueurs a des vitesses differentes (30 ou 144 images/s).
constexpr uint64_t kWaitMs   = 120000;     // l'hote attend son equipe 2 minutes au plus
constexpr uint64_t kJoinMs   = 300000;     // l'invite attend la copie 5 minutes
constexpr uint64_t kReadyMs  = 180000;     // pret, l'hote ne part pas : 3 minutes
constexpr uint64_t kClickMs  = 30000;      // un bouton introuvable : 30 s
constexpr uint64_t kIdleMs   = 300000;     // plus aucun clic de l'hote : 5 minutes
constexpr uint64_t kLingerMs = 120000;     // fin du miroir : la copie de l'aventure arrive
constexpr uint64_t kLateMs   = 180000;     // arrive trop tard : la copie de l'aventure arrive
constexpr uint64_t kPrepMs   = 900000;     // un depart ne dure pas 15 minutes
constexpr size_t   kKeyLen      = sizeof(MmMsg::text);   // « scene|nom », tronque comme sur le fil
constexpr size_t   kMaxQueue    = 512;
constexpr size_t   kMaxKeys     = 64;

// Les deux scenes du depart ou l'on clique. (Les transitions n'ont pas de bouton.)
const char* const kPrepScenes[] = { "ClassChooser", "StorageItems" };

enum MmEmbark : uint8_t { kReady = 1, kClick = 2 };

enum class Host  { Idle, Waiting, Going, Prep };
enum class Guest { Idle, Joining, Ready, Departed };

struct Click {
    std::string key;            // "scene|nom"
    uint32_t    ord    = 0;     // rang parmi les boutons de meme cle, dans l'ordre des updates
    uint32_t    serial = 0;
    uint64_t    since  = 0;     // quand on a commence a le chercher (0 : pas encore)
};

struct Count {
    char     key[kKeyLen];
    uint32_t n;
};

struct State {
    bool     on    = false;
    uint32_t frame = 1;
    uint64_t now   = 0;        // GetTickCount64, au debut de l'image

    // Les rangs des boutons, recomptes a chaque image (les deux roles). On ne
    // compte que si c'etait decide au debut de l'image : un rang compte depuis
    // le milieu de l'image serait faux.
    Count    counts[kMaxKeys];
    size_t   n_counts = 0;
    bool     counting = false;

    // hote
    Host     host      = Host::Idle;
    uint8_t  seq       = 0;
    uint32_t joiners   = 0;
    uint32_t ready     = 0;
    uint64_t wait_from = 0;
    uint64_t prep_from = 0;
    bool     no_wait   = false;
    uint32_t serial    = 0;
    uint32_t sent      = 0;

    // invite
    Guest                 guest       = Guest::Idle;
    uint8_t               gseq        = 0;
    uint64_t              join_from   = 0;
    uint64_t              ready_at    = 0;
    uint64_t              departed_at = 0;
    uint64_t              last_click  = 0;     // le dernier clic de l'hote recu (ou le depart)
    uint64_t              linger_until = 0;    // fin du miroir, encore dans la copie : pas de « retour »
    uint64_t              late_until  = 0;     // arrive trop tard : il attend la copie de l'aventure
    bool                  have_go     = false;
    std::vector<uint64_t> keys;
    std::deque<Click>     queue;
    bool                  replaying   = false;
    bool                  mirror_lost = false;
    uint32_t              replay_frame = 0;   // un clic rejoue par image : il change l'ecran
    uint32_t              replayed    = 0;
    uint32_t              blocked     = 0;

    std::string           scenes;     // la derniere liste des scenes, pour le journal
};

State g;

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

uint32_t present_mask() {
    uint8_t ids[8] = {};
    uint32_t mask = 0;
    const uint8_t n = net_peer_count();
    if (n && net_peer_ids(ids, 8))
        for (uint8_t i = 0; i < n && i < 8; ++i)
            if (ids[i] < 8) mask |= 1u << ids[i];
    return mask;
}

bool is_prep_scene(const char* scene) {
    for (const char* s : kPrepScenes)
        if (strcmp(s, scene) == 0) return true;
    return false;
}

// La scene et le nom d'un bouton.
bool button_ident(void* b, char* scene, size_t ns, char* name, size_t nn) {
    void* sc = nullptr;
    scene[0] = name[0] = 0;
    if (!b || !mem_read((const char*)b + kComp_Scene, &sc, sizeof(sc)) || !sc) return false;
    if (!mem_read_std_string((const char*)sc + kScene_Name, scene, ns)) return false;
    return mem_read_std_string((const char*)b + kBtn_Name, name, nn);
}

// « scene|nom », tronque a la taille du message (les deux roles comparent la
// meme chose).
void make_key(char (&key)[kKeyLen], const char* scene, const char* name) {
    snprintf(key, sizeof(key), "%s|%s", scene, name);
}

// Le rang de ce bouton parmi ceux de meme cle deja vus dans l'image : appele
// avant son propre comptage, c'est son rang a lui. `bump` le compte.
uint32_t rank_of(const char* key, bool bump) {
    for (size_t i = 0; i < g.n_counts; ++i)
        if (strcmp(g.counts[i].key, key) == 0) {
            const uint32_t r = g.counts[i].n;
            if (bump) ++g.counts[i].n;
            return r;
        }
    if (bump && g.n_counts < kMaxKeys) {
        Count& c = g.counts[g.n_counts++];
        strncpy_s(c.key, key, _TRUNCATE);
        c.n = 1;
    }
    return 0;
}

bool host_counting()  { return g.host == Host::Prep; }
bool guest_counting() { return g.guest == Guest::Departed && !g.queue.empty() && !g.mirror_lost; }

// Une scene du depart est ouverte.
bool prep_scene_alive() {
    for (const char* s : kPrepScenes)
        if (mm_scene_alive(s)) return true;
    return false;
}

void send_ready() {
    MmMsg m{};
    m.kind = MM_EMBARK;
    m.a    = kReady;
    m.b    = g.gseq;
    if (net_send_mm(m))
        log_line("EMBARK", "-> dans la maison de l'hote : pret a partir avec lui (vote %u)", (unsigned)g.gseq);
}

// `linger` : il reste dans la copie en attendant celle de l'aventure (publiee
// par l'hote sur la carte) -- pas de « Retour a la maison » pendant ce temps.
void guest_reset(const char* why, bool linger = false) {
    if (g.guest != Guest::Idle)
        log_line("EMBARK", "fin du depart ensemble chez ce joueur (%s) : %u clic(s) de l'hote rejoue(s), "
                           "%u clic(s) retenu(s)%s", why, g.replayed, g.blocked,
                 g.mirror_lost ? ", miroir perdu en route" : "");
    g.linger_until = (linger && g.guest != Guest::Idle && savefile_in_coop()) ? g.now + kLingerMs : 0;
    g.late_until   = 0;
    g.guest       = Guest::Idle;
    g.have_go     = false;
    g.keys.clear();
    g.queue.clear();
    g.replaying   = false;
    g.mirror_lost = false;
    g.replayed    = 0;
    g.blocked     = 0;
}

void host_reset(const char* why) {
    if (g.host != Host::Idle)
        log_line("EMBARK", "fin du depart ensemble (%s) : %u clic(s) envoye(s)", why, g.sent);
    g.host    = Host::Idle;
    g.joiners = 0;
    g.ready   = 0;
    g.no_wait = false;
    g.sent    = 0;
}

void lose_mirror(const char* why) {
    if (g.mirror_lost) return;
    g.mirror_lost = true;
    g.queue.clear();
    log_line("EMBARK", "!! miroir du depart perdu (%s) : ce joueur finit les ecrans lui-meme", why);
    mm_show_info(fmt(u8"Départ en aventure\n\n"
                          u8"Le départ de %s ne s'affiche plus ici.\n"
                          u8"Termine ces écrans toi-même : tu le rejoindras\n"
                          u8"sur la carte.", mm_peer_name(kHostPeer).c_str()));
}

// La liste des scenes, au journal quand elle change (pendant un depart).
void log_scenes() {
    const std::string now = mm_scene_list();
    if (now == g.scenes) return;
    g.scenes = now;
    log_line("EMBARK", "scenes : %s", now.c_str());
}

} // namespace

// ---------------------------------------------------------------------------

void embark_init() {
    const State keep = g;
    g = State{};
    g.on    = true;
    g.frame = keep.frame;
    g.now   = GetTickCount64();
    log_line("EMBARK", net_role() == NetRole::Host
        ? "arme -- depart ensemble : l'equipe charge la partie de l'hote et suit son depart"
        : "arme -- depart ensemble : apres « J'arrive », ce joueur suit le depart de l'hote");
}

void embark_shutdown() {
    host_reset("fin de session");
    guest_reset("fin de session");
    g.on = false;
}

void embark_frame_begin() {
    ++g.frame;
    g.now = GetTickCount64();
    g.n_counts = 0;
    g.counting = g.on && (host_counting() || guest_counting());
}

void embark_pump() {
    if (!g.on) return;

    // --- hote ---
    if (g.host == Host::Prep) {
        log_scenes();
        if (savefile_run_published())
            host_reset("l'hote est sur la carte, son aventure est publiee");
        else if (g.now - g.prep_from > kPrepMs)
            host_reset("trop long");
    }

    // --- invite ---
    if (g.linger_until && (!savefile_in_coop() || g.now >= g.linger_until)) g.linger_until = 0;
    // Arrive trop tard : jusqu'a la copie de l'aventure (ou le delai).
    if (g.late_until && (g.now >= g.late_until || (savefile_in_coop() && savefile_adventure_is_loaded())))
        g.late_until = 0;
    switch (g.guest) {
        case Guest::Idle:
            break;
        case Guest::Joining:
            // Une copie de l'AVENTURE est arrivee (l'hote est parti sans
            // l'attendre) : il le rejoint sur la carte, comme avant.
            if (savefile_in_coop() && savefile_adventure_is_loaded()) {
                guest_reset("l'hote est deja parti : il le rejoint sur la carte");
                break;
            }
            // La copie de la maison de l'hote est chargee : il le dit.
            if (savefile_in_coop() && mm_house()) {
                g.guest    = Guest::Ready;
                g.ready_at = g.now;
                log_scenes();
                send_ready();
            } else if (g.now - g.join_from > kJoinMs) {
                guest_reset("la partie de l'hote n'est pas arrivee");
            }
            break;
        case Guest::Ready:
            if (!savefile_in_coop()) { guest_reset("sorti de la copie"); break; }
            if (g.have_go && mm_house()) {
                if (team_copy_depart(g.keys)) {
                    g.guest       = Guest::Departed;
                    g.departed_at = g.now;
                    g.last_click  = g.now;
                    log_line("EMBARK", "depart dans la copie avec l'equipe de l'hote (%zu chat(s))", g.keys.size());
                } else {
                    guest_reset("le depart dans la copie a echoue", /*linger=*/true);
                }
            } else if (!g.have_go && g.now - g.ready_at > kReadyMs) {
                // L'hote n'est jamais parti (ou est parti sans le depart
                // ensemble) : il rentre chez lui -- s'il est en aventure, sa copie
                // arrivera de toute facon.
                guest_reset("l'hote ne part pas");
            }
            break;
        case Guest::Departed:
            log_scenes();
            if (!savefile_in_coop()) { guest_reset("l'aventure de l'hote se recharge"); break; }
            if (mm_scene_alive("Map") && !prep_scene_alive()) {
                guest_reset("arrive sur la carte", /*linger=*/true);
                break;
            }
            if (g.now - g.departed_at > kPrepMs) { guest_reset("trop long", /*linger=*/true); break; }
            // Plus aucun clic de l'hote depuis longtemps, un ecran du depart ouvert :
            // il ne viendra peut-etre plus (l'hote a quitte la partie...). Le
            // joueur reprend la main.
            if (!g.mirror_lost && g.queue.empty() && prep_scene_alive() &&
                g.now - g.last_click > kIdleMs)
                lose_mirror("plus aucun clic de l'hote");
            if (!g.queue.empty() && !g.mirror_lost) {
                Click& c = g.queue.front();
                const size_t bar = c.key.find('|');
                const std::string scene = bar == std::string::npos ? c.key : c.key.substr(0, bar);
                // On ne compte le temps que la scene de ce clic ouverte : la copie
                // part un peu apres l'hote, sa transition peut durer.
                if (mm_scene_alive(scene.c_str())) {
                    if (!c.since) c.since = g.now;
                    else if (g.now - c.since > kClickMs)
                        lose_mirror(("bouton introuvable : " + c.key).c_str());
                }
            }
            break;
    }
}

// --- hote ------------------------------------------------------------------------

bool embark_begin(uint8_t seq, uint32_t joiners) {
    if (!g.on || net_role() != NetRole::Host) return false;
    joiners &= present_mask() & ~1u;
    if (!joiners) return false;
    if (!team_embark_prepare(seq)) {
        log_line("EMBARK", "!! equipe pas preparee (maison ou boite introuvable) -- depart tout de suite");
        return false;
    }
    if (!savefile_publish_embark()) {
        log_line("EMBARK", "!! la save de l'hote n'a pas pu partir -- depart tout de suite, l'equipe "
                           "le rejoindra sur la carte");
        return false;
    }
    g.host      = Host::Waiting;
    g.seq       = seq;
    g.joiners   = joiners;
    g.ready     = 0;
    g.no_wait   = false;
    g.wait_from = g.now;
    g.sent      = 0;
    log_line("EMBARK", "vote %u passe : la partie de l'hote part chez son equipe, il attend qu'elle "
                       "l'ait chargee", (unsigned)seq);
    return true;
}

bool embark_waiting() { return g.host == Host::Waiting; }

bool embark_should_go() {
    if (g.host != Host::Waiting) return false;
    const uint32_t still = g.joiners & present_mask();
    const char* why = nullptr;
    if (g.no_wait)                         why = "l'hote n'attend plus";
    else if (!still)                       why = "plus personne a attendre";
    else if ((still & ~g.ready) == 0)      why = "toute l'equipe est la";
    else if (g.now - g.wait_from > kWaitMs)
        why = "attente trop longue, les retardataires le rejoindront sur la carte";
    if (!why) return false;
    g.host = Host::Going;
    log_line("EMBARK", "l'hote part (%s)", why);
    return true;
}

void embark_no_wait() {
    if (g.host != Host::Waiting) return;
    g.no_wait = true;
    log_line("EMBARK", "l'hote n'attend plus son equipe");
}

std::string embark_wait_text() {
    std::string lines;
    for (uint8_t id = 1; id < 8; ++id) {
        if (!(g.joiners & (1u << id))) continue;
        const bool here = (present_mask() >> id) & 1;
        lines += "  " + mm_peer_name(id) + " : " +
                 (!here ? u8"parti" : ((g.ready >> id) & 1) ? u8"prêt !" : u8"charge ta partie...") + "\n";
    }
    return u8"Départ en aventure\n\n"
           u8"Ton équipe charge ta partie pour partir avec toi :\n\n" + lines;
}

void embark_host_departed(uint8_t seq) {
    if (g.host != Host::Going && g.host != Host::Waiting) return;   // pas de depart ensemble
    g.host      = Host::Prep;
    g.seq       = seq;
    g.prep_from = g.now;
    g.serial    = 0;
    g.scenes.clear();
    log_line("EMBARK", "l'hote est parti : ses clics dans le choix des colliers et des objets partent "
                       "chez l'equipe");
}

void embark_cancel(const char* why) { host_reset(why ? why : "annule"); }

// --- invite ---------------------------------------------------------------------

void embark_guest_joining(uint8_t seq) {
    if (!g.on || net_role() != NetRole::Client) return;
    guest_reset("nouveau depart");
    g.guest     = Guest::Joining;
    g.gseq      = seq;
    g.join_from = g.now;
    log_line("EMBARK", "ce joueur rejoint le depart de l'hote (vote %u) : il chargera sa partie", (unsigned)seq);
}

void embark_guest_cancel(const char* why) { guest_reset(why ? why : "annule"); }

void embark_on_go(uint8_t seq, const std::vector<uint64_t>& keys) {
    if (g.guest == Guest::Idle || seq != g.gseq) {
        log_line("EMBARK", "depart de l'hote (vote %u) : ce joueur ne le suit pas", (unsigned)seq);
        return;
    }
    if (g.guest == Guest::Joining) {
        // Arrive trop tard dans la maison de l'hote : il le rejoindra sur la carte.
        log_line("EMBARK", "!! l'hote part et ce joueur n'a pas encore sa partie : il le rejoindra sur la carte");
        guest_reset("trop tard pour le depart");
        // La copie de la maison ne sert plus : si elle attend encore, on la jette
        // (il attendra celle de l'aventure) ; deja en train de se charger, il
        // y attendra la suivante, sans « Retour a la maison ».
        savefile_drop_pending();
        g.late_until = g.now + kLateMs;
        return;
    }
    if (g.guest == Guest::Departed) return;
    g.keys    = keys;
    g.have_go = true;
    log_line("EMBARK", "<- l'hote part (vote %u, %zu chat(s)) : ce joueur fait le meme depart", (unsigned)seq,
             keys.size());
}

bool embark_guest_active() {
    if (!g.on) return false;
    if (g.linger_until && savefile_in_coop()) return true;
    if (g.late_until && savefile_in_coop() && !savefile_adventure_is_loaded()) return true;
    return g.guest == Guest::Ready || g.guest == Guest::Departed ||
           (g.guest == Guest::Joining && savefile_in_coop());
}

// --- les deux --------------------------------------------------------------------

void embark_on_mm(uint8_t from, const MmMsg& m) {
    if (!g.on) return;
    if (m.a == kReady) {
        if (net_role() != NetRole::Host || from >= 8 || g.host != Host::Waiting || m.b != g.seq) return;
        if (!(g.ready & (1u << from)))
            log_line("EMBARK", "<- %s a charge la partie, pret a partir", mm_peer_name(from).c_str());
        g.ready |= 1u << from;
        return;
    }
    if (m.a == kClick) {
        if (net_role() != NetRole::Client || m.b != g.gseq) return;
        if (g.guest != Guest::Ready && g.guest != Guest::Departed) return;
        if (g.mirror_lost) return;
        if (g.queue.size() >= kMaxQueue) { lose_mirror("trop de clics en attente"); return; }
        Click c;
        char text[kKeyLen + 1] = {};
        memcpy(text, m.text, kKeyLen);
        text[kKeyLen - 1] = 0;
        c.key    = text;
        g.last_click = g.now;
        c.ord    = m.x;
        c.serial = m.y;
        g.queue.push_back(std::move(c));
        return;
    }
}

bool embark_on_button_click(void* button) {
    if (!g.on) return false;
    if (g.replaying) return false;                // notre propre clic, rejoue
    const bool host  = host_counting();
    const bool guest = g.guest == Guest::Departed && !g.mirror_lost && savefile_in_coop();
    if (!host && !guest) return false;
    char scene[48], name[64];
    if (!button_ident(button, scene, sizeof(scene), name, sizeof(name))) return false;
    if (host) {
        if (!is_prep_scene(scene)) {
            log_line("EMBARK", "clic de l'hote hors des ecrans du depart : %s / %s", scene, name);
            return false;
        }
        char key[kKeyLen];
        make_key(key, scene, name);
        const uint32_t ord = rank_of(key, false);
        if (!g.counting)
            log_line("EMBARK", "!! clic de l'hote avant le comptage des boutons : rang incertain");
        MmMsg m{};
        m.kind = MM_EMBARK;
        m.a    = kClick;
        m.b    = g.seq;
        m.x    = ord;
        m.y    = ++g.serial;
        strncpy_s(m.text, key, _TRUNCATE);
        const bool ok = net_send_mm(m);
        if (ok) ++g.sent;
        log_line("EMBARK", "%s clic de l'hote %u : %s #%u", ok ? "->" : "!! (non envoye)", g.serial, key, ord);
        return false;
    }
    // Invite : il regarde. Ses clics dans ces ecrans sont retenus.
    if (!is_prep_scene(scene)) return false;
    if (++g.blocked <= 3 || g.blocked % 20 == 0)
        log_line("EMBARK", "clic de ce joueur retenu (%s / %s) : l'hote fait le depart", scene, name);
    return true;
}

void embark_on_button_update(void* button) {
    if (!g.on || !g.counting) return;
    const bool host  = host_counting();
    const bool guest = guest_counting() && g.replay_frame != g.frame;
    if (!host && !guest_counting()) return;
    char scene[48], name[64];
    if (!button_ident(button, scene, sizeof(scene), name, sizeof(name)) || !is_prep_scene(scene)) return;
    char key[kKeyLen];
    make_key(key, scene, name);
    const uint32_t ord = rank_of(key, true);
    if (!guest) return;
    Click& c = g.queue.front();
    if (c.key != key || c.ord != ord) return;
    // Grise (une animation d'entree, par exemple) : Button::update n'enverrait
    // pas ce clic. On attend qu'il soit cliquable, dans les images suivantes.
    int32_t state = 0;
    if (mem_read((const char*)button + kBtn_State, &state, sizeof(state)) && state == kBtnState_Disabled)
        return;
    using FnButtonClick = void (*)(void* button, bool force);
    auto click = (FnButtonClick)addr_of_call(C_ButtonClick);
    if (!click) { lose_mirror("Button::Click introuvable"); return; }
    log_line("EMBARK", "<- clic de l'hote %u rejoue : %s #%u", c.serial, key, ord);
    g.queue.pop_front();
    ++g.replayed;
    g.replay_frame = g.frame;
    g.replaying    = true;
    click(button, false);
    g.replaying    = false;
}

} // namespace mgmp
