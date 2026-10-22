// mm_collars -- Mewgenics Multi : les colliers de classe, choisis ensemble au
// depart. Voir mm_collars.h.

#include "mm_collars.h"
#include "mm_embark.h"
#include "mm_menu.h"
#include "mm_team.h"

#include "mgmp_addresses.h"
#include "mgmp_catsync.h"
#include "mgmp_log.h"
#include "mgmp_mem.h"
#include "mgmp_net.h"
#include "mgmp_proto.h"
#include "mgmp_resolve.h"

#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- le jeu (1.1.21239), voir mm_collars.h -----------------------------------------

constexpr uintptr_t kCC_Selector  = 0x48;    // CatSelector*
constexpr uintptr_t kCC_BoxCount  = 0x9C;    // u32
constexpr uintptr_t kCC_Boxes     = 0xA0;    // ClassTagBox**
constexpr uintptr_t kCC_CopyBegin = 0xB8;    // copies des CatData de l'equipe
constexpr uintptr_t kCC_CopyEnd   = 0xC0;
constexpr uintptr_t kCC_Current   = 0xD0;    // u64, la cle du chat montre
constexpr uintptr_t kTag_Chooser  = 0x38;    // ClassChooser*
constexpr uintptr_t kTag_Button   = 0x48;    // Button*
constexpr uintptr_t kTag_Class    = 0x58;    // std::string
constexpr uintptr_t kTag_On       = 0x90;    // u64, la cle du chat qui le porte
constexpr uintptr_t kSel_Index    = 0x78;    // i32, le rang du chat montre
constexpr uintptr_t kSel_Key      = 0x88;    // u64, sa cle
constexpr size_t    kCatDataSize  = 0xC58;
constexpr uintptr_t kCat_Id       = 0x00;
constexpr uintptr_t kCat_Class    = 0xC10;   // std::string
constexpr uintptr_t kCat_Key      = 0xC48;

constexpr uint64_t kNone          = ~0ull;   // ClassTagBox+0x90 : collier libre
constexpr size_t   kMaxCats       = 8;
constexpr uint32_t kMaxBoxes      = 64;
constexpr uint32_t kMaxRunKeys    = 16;
constexpr size_t   kMaxRequests   = 32;
constexpr int      kApplyPerFrame = 8;       // clics du jeu faits par image, au plus
constexpr int      kMaxTries      = 3;       // pour un meme changement, chez l'invite
constexpr uint64_t kRequestMs     = 10000;   // une demande d'invite attend 10 s au plus
constexpr uint64_t kOverdueMs     = 5000;    // ecart avec l'hote, choix a l'ecran : 5 s

constexpr char kColorless[]   = "Colorless";
constexpr char kChooser[]     = "ClassChooser";

using FnTagUpdate = void (__fastcall*)(void* box);
using FnTagClick  = void (__fastcall*)(void* box);
using FnSelect    = void (__fastcall*)(void* selector, uint64_t key);

FnTagUpdate o_TagUpdate = nullptr;

struct TeamCat {
    uint64_t key  = kNone;
    uint64_t id   = 0;
    bool     free = false;      // d'origine « Colorless » : il peut prendre un collier
};

struct Box {
    void*    p        = nullptr;
    void*    button   = nullptr;
    uint64_t on       = kNone;
    char     name[64] = {};
};

// Ce que montre le choix des colliers, lu d'un coup.
struct View {
    void*                cc      = nullptr;
    void*                sel     = nullptr;
    uint64_t             current = kNone;
    std::vector<TeamCat> cats;
    std::vector<Box>     boxes;

    const TeamCat* cat(uint64_t key) const {
        for (const TeamCat& t : cats)
            if (t.key == key) return &t;
        return nullptr;
    }
    const Box* worn_by(uint64_t key) const {
        for (const Box& b : boxes)
            if (b.on == key) return &b;
        return nullptr;
    }
    const Box* free_box(const char* name) const {
        for (const Box& b : boxes)
            if (b.on == kNone && strcmp(b.name, name) == 0) return &b;
        return nullptr;
    }
    const Box* by_button(void* button) const {
        for (const Box& b : boxes)
            if (b.button == button) return &b;
        return nullptr;
    }
    std::string collar(uint64_t key) const {
        const Box* b = worn_by(key);
        return b ? b->name : "";
    }
};

struct Request {
    uint8_t     from  = 0;
    bool        equip = false;
    uint64_t    key   = kNone;
    std::string name;
    uint64_t    at    = 0;
};

struct State {
    CollarRole role      = CollarRole::Off;
    uint8_t    seq       = 0;
    uint32_t   frame     = 1;

    void*      cc        = nullptr;     // le ClassChooser de ces boites
    uint32_t   cc_frame  = 0;           // l'image ou une boite l'a montre
    void*      opened    = nullptr;     // celui dont l'ouverture est faite

    // « ce joueur peut-il toucher au chat montre ? », une fois par image
    uint32_t   act_frame = 0;
    uint64_t   act_key   = kNone;
    bool       act_ok    = true;

    bool       broken    = false;       // le clic du jeu a leve une exception

    // hote
    std::map<uint64_t, std::string> sent;        // cle -> collier envoye ("" : aucun)
    bool                            resend  = false;
    bool                            closing = false;   // il a clique sur « Valider »
    std::deque<Request>             requests;
    uint32_t                        done    = 0;       // les invites qui ont valide leurs colliers
    bool                            done_warned = false;   // « X choisit encore », une fois

    // invite
    std::map<uint64_t, std::string> want;        // cle -> collier chez l'hote
    std::map<std::string, int>      tries;       // « cle|classe|etape » -> essais
    bool                            synced    = false;
    void*                           synced_cc = nullptr;
    uint64_t                        unsynced_ms = 0;   // ecart avec l'hote, choix a l'ecran
    uint64_t                        pump_ms     = 0;
    uint32_t                        pump_frame  = 0;
    // choix a l'ecran sans aucune boite vue (aucun collier, aucun chat classe) :
    // rien a rendre pareil, « Valider » ne doit pas attendre jusqu'a perdre le miroir
    uint64_t                        nobox_ms    = 0;
    uint64_t                        nobox_at    = 0;
    uint32_t                        nobox_frame = 0;
    bool                            close_info = false;
    std::set<std::string>           warned;

    uint32_t applied = 0, asked = 0, refused = 0, blocked = 0, sent_n = 0, dropped = 0;
};

State c;

std::string fmt(const char* f, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof(buf), f, ap);
    va_end(ap);
    return buf;
}

bool is_collar_button(const char* name) {
    return strncmp(name, "InventoryButton_", 16) == 0 || strncmp(name, "EquippedButton_", 15) == 0;
}

// L'equipe de l'aventure (MewDirector+1472), dans l'ordre du selecteur.
bool run_keys(std::vector<uint64_t>& out) {
    out.clear();
    const void* const* slot = (const void* const*)addr_of_data(D_MewDirectorPtr);
    const void* dir = nullptr;
    if (!slot || !mem_read(slot, &dir, sizeof(dir)) || !dir) return false;
    uint32_t n = 0;
    const uint64_t* data = nullptr;
    if (!mem_read((const char*)dir + kDir_CatIdCount, &n, sizeof(n)) ||
        !mem_read((const char*)dir + kDir_CatIdData, &data, sizeof(data)))
        return false;
    if (n == 0 || n > kMaxRunKeys || !data) return false;
    out.resize(n);
    if (!mem_read(data, out.data(), n * sizeof(uint64_t))) { out.clear(); return false; }
    return true;
}

int run_index(uint64_t key) {
    std::vector<uint64_t> keys;
    if (!run_keys(keys)) return -1;
    for (size_t i = 0; i < keys.size(); ++i)
        if (keys[i] == key) return (int)i;
    return -1;
}

bool read_view(void* cc, View& v) {
    v = View{};
    if (!cc) return false;
    v.cc = cc;
    if (!mem_read((const char*)cc + kCC_Selector, &v.sel, sizeof(v.sel)) ||
        !mem_read((const char*)cc + kCC_Current, &v.current, sizeof(v.current)))
        return false;
    uintptr_t b = 0, e = 0;
    if (!mem_read((const char*)cc + kCC_CopyBegin, &b, sizeof(b)) ||
        !mem_read((const char*)cc + kCC_CopyEnd, &e, sizeof(e)))
        return false;
    if (!b || e < b || (e - b) % kCatDataSize != 0 || (e - b) / kCatDataSize > kMaxCats) return false;
    for (uintptr_t p = b; p < e; p += kCatDataSize) {
        TeamCat t;
        char cls[64];
        if (!mem_read((const void*)(p + kCat_Key), &t.key, sizeof(t.key)) ||
            !mem_read((const void*)(p + kCat_Id), &t.id, sizeof(t.id)))
            return false;
        t.free = mem_read_std_string((const void*)(p + kCat_Class), cls, sizeof(cls)) &&
                 strcmp(cls, kColorless) == 0;
        v.cats.push_back(t);
    }
    uint32_t n = 0;
    void* const* arr = nullptr;
    if (!mem_read((const char*)cc + kCC_BoxCount, &n, sizeof(n)) ||
        !mem_read((const char*)cc + kCC_Boxes, &arr, sizeof(arr)))
        return false;
    if (n > kMaxBoxes || (n && !arr)) return false;
    for (uint32_t i = 0; i < n; ++i) {
        Box x;
        if (!mem_read(arr + i, &x.p, sizeof(x.p)) || !x.p) return false;
        if (!mem_read((const char*)x.p + kTag_Button, &x.button, sizeof(x.button)) ||
            !mem_read((const char*)x.p + kTag_On, &x.on, sizeof(x.on)) ||
            !mem_read_std_string((const char*)x.p + kTag_Class, x.name, sizeof(x.name)))
            return false;
        v.boxes.push_back(x);
    }
    return true;
}

// Cette boite est-elle bien l'une de ce ClassChooser ?
bool box_in_chooser(void* cc, void* box) {
    uint32_t n = 0;
    void* const* arr = nullptr;
    if (!mem_read((const char*)cc + kCC_BoxCount, &n, sizeof(n)) ||
        !mem_read((const char*)cc + kCC_Boxes, &arr, sizeof(arr)) || n > kMaxBoxes || !arr)
        return false;
    for (uint32_t i = 0; i < n; ++i) {
        void* p = nullptr;
        if (mem_read(arr + i, &p, sizeof(p)) && p == box) return true;
    }
    return false;
}

// --- les joueurs -----------------------------------------------------------------------

// Le proprietaire d'un chat : celui de l'equipe mixte, l'hote a defaut.
int owner_of(const TeamCat& t) {
    const int p = team_owner_of(t.id);
    return p < 0 ? (int)kHostPeer : p;
}

bool follower_active(int peer) {
    return peer > 0 && peer < 32 && embark_follower_active((uint8_t)peer);
}

// Ce joueur peut-il poser ou retirer les colliers de ce chat ? L'hote : les
// siens, et ceux d'un invite qui ne suit pas le depart (tous, si le choix
// partage s'est arrete). Un invite : les siens.
bool can_act(const TeamCat* t) {
    if (!t) return c.role == CollarRole::Host;
    const int o = owner_of(*t);
    if (c.role == CollarRole::Host) return o == (int)kHostPeer || c.broken || !follower_active(o);
    return o != (int)kHostPeer && o == (int)net_self();
}

std::string owner_name(const TeamCat* t) {
    return t ? mm_peer_name((uint8_t)owner_of(*t)) : std::string("?");
}

bool send_collar(uint8_t a, uint8_t flag, uint64_t key, const std::string& name) {
    MmMsg m{};
    m.kind = MM_EMBARK;
    m.a    = a;
    m.b    = c.seq;
    m.c    = flag;
    m.x    = (uint32_t)(key & 0xFFFFFFFFull);
    m.y    = (uint32_t)(key >> 32);
    strncpy_s(m.text, name.c_str(), _TRUNCATE);
    return net_send_mm(m);
}

// --- le clic du jeu --------------------------------------------------------------------

bool safe_tag_click(FnTagClick f, void* box) {
    __try { f(box); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool safe_select(FnSelect f, void* sel, uint64_t key) {
    __try { f(sel, key); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// Une exception dans le jeu : son etat est peut-etre a moitie change. Plus
// aucun clic du jeu d'ici la fin du choix. L'hote choisit alors pour toute
// l'equipe ; l'invite laisse le miroir (mm_embark le voit).
void mark_broken(const char* where) {
    if (c.broken) return;
    c.broken = true;
    log_line("COLLIERS", "!! exception dans le jeu (%s) : le choix partage des colliers s'arrete ici", where);
    if (c.role == CollarRole::Host) {
        c.act_frame = 0;
        mm_show_info(u8"Choix des colliers\n\n"
                     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
                     u8"choisis les colliers pour toute l'équipe.");
    }
}

// Montre ce chat, comme les fleches du selecteur.
bool show_cat(const View& v, uint64_t key) {
    auto select = (FnSelect)addr_of_call(C_CatSelectorSelect);
    const int idx = run_index(key);
    if (c.broken || !select || !v.sel || idx < 0) return false;
    const int32_t i = idx;
    if (!mem_write((char*)v.sel + kSel_Index, &i, sizeof(i))) return false;
    if (safe_select(select, v.sel, key)) return true;
    mark_broken("selection du chat");
    return false;
}

// Le clic du jeu sur la boite `box`, le chat `key` montre le temps de l'appel,
// puis le chat que ce joueur regardait, remis par la selection du jeu.
bool apply(const View& v, void* box, uint64_t key, const char* what, const char* name) {
    if (c.broken) return false;
    auto click  = (FnTagClick)addr_of_call(C_ClassTagClick);
    auto select = (FnSelect)addr_of_call(C_CatSelectorSelect);
    const int idx = run_index(key);
    if (!click || !select || !v.sel || idx < 0) {
        log_line("COLLIERS", "!! %s %s (chat %llu) impossible : %s", what, name, (unsigned long long)key,
                 !click || !select ? "fonction du jeu introuvable" : "chat absent de l'equipe");
        return false;
    }
    uint64_t shown = 0, sel_key = 0;
    int32_t  sel_idx = 0;
    if (!mem_read((const char*)v.cc + kCC_Current, &shown, sizeof(shown)) ||
        !mem_read((const char*)v.sel + kSel_Index, &sel_idx, sizeof(sel_idx)) ||
        !mem_read((const char*)v.sel + kSel_Key, &sel_key, sizeof(sel_key)))
        return false;
    const int32_t want_idx = idx;
    bool ok = mem_write((char*)v.cc + kCC_Current, &key, sizeof(key)) &&
              mem_write((char*)v.sel + kSel_Index, &want_idx, sizeof(want_idx));
    if (ok && !safe_tag_click(click, box)) { ok = false; mark_broken("clic d'un collier"); }
    mem_write((char*)v.cc + kCC_Current, &shown, sizeof(shown));
    mem_write((char*)v.sel + kSel_Index, &sel_idx, sizeof(sel_idx));
    if (!safe_select(select, v.sel, sel_key)) { ok = false; mark_broken("selection du chat"); }
    if (ok) ++c.applied;
    log_line("COLLIERS", "%s%s %s (chat %llu)", ok ? "" : "!! echec : ", what, name, (unsigned long long)key);
    return ok;
}

// --- l'ouverture ---------------------------------------------------------------------

void open_chooser(const View& v) {
    c.opened = v.cc;
    size_t mine = 0;
    for (const TeamCat& t : v.cats)
        if (t.free && can_act(&t)) ++mine;
    log_line("COLLIERS", "choix des colliers ouvert : %zu chat(s), %zu collier(s), %zu chat(s) a choisir ici",
             v.cats.size(), v.boxes.size(), mine);
    if (c.role == CollarRole::Guest) {
        MmMsg m{};
        m.kind = MM_EMBARK;
        m.a    = kEmbarkChooserOpen;
        m.b    = c.seq;
        net_send_mm(m);
    }
    // D'abord un de ses chats (un qui peut prendre un collier, de preference).
    if (can_act(v.cat(v.current))) return;
    const TeamCat* pick = nullptr;
    for (const TeamCat& t : v.cats)
        if (can_act(&t) && (!pick || (t.free && !pick->free))) pick = &t;
    if (pick && show_cat(v, pick->key))
        log_line("COLLIERS", "ce joueur voit d'abord son chat %llu", (unsigned long long)pick->key);
}

// --- hote ------------------------------------------------------------------------------

void host_handle(View& v, const Request& r) {
    const std::string who = mm_peer_name(r.from);
    if (!follower_active(r.from)) {
        log_line("COLLIERS", "demande de %s ignoree : il ne suit plus le depart", who.c_str());
        return;
    }
    const TeamCat* t = v.cat(r.key);
    if (!t || !t->free) {
        log_line("COLLIERS", "demande de %s ignoree : chat %llu sans choix de collier", who.c_str(),
                 (unsigned long long)r.key);
        return;
    }
    if (owner_of(*t) != (int)r.from) {
        ++c.refused;
        log_line("COLLIERS", "!! %s demande un collier pour le chat %llu, qui n'est pas a lui : refuse",
                 who.c_str(), (unsigned long long)r.key);
        return;
    }
    const std::string have = v.collar(r.key);
    if (r.equip) {
        if (have == r.name) return;
        const Box* b = v.free_box(r.name.c_str());
        if (!b) {
            ++c.refused;
            log_line("COLLIERS", "%s voulait %s pour le chat %llu : plus de collier libre", who.c_str(),
                     r.name.c_str(), (unsigned long long)r.key);
            return;
        }
        apply(v, b->p, r.key, fmt("pose, demande de %s :", who.c_str()).c_str(), r.name.c_str());
    } else {
        if (have.empty() || have != r.name) return;
        if (const Box* b = v.worn_by(r.key))
            apply(v, b->p, r.key, fmt("retire, demande de %s :", who.c_str()).c_str(), r.name.c_str());
    }
}

void host_send_changes(const View& v) {
    bool all = true;
    for (const TeamCat& t : v.cats) {
        if (!t.free) continue;
        const std::string have = v.collar(t.key);
        auto it = c.sent.find(t.key);
        if (!c.resend && it != c.sent.end() && it->second == have) continue;
        if (!send_collar(kEmbarkCollarSet, have.empty() ? 0 : 1, t.key, have)) { all = false; continue; }
        c.sent[t.key] = have;
        ++c.sent_n;
        log_line("COLLIERS", "-> chat %llu (%s) : %s", (unsigned long long)t.key, owner_name(&t).c_str(),
                 have.empty() ? "sans collier" : have.c_str());
    }
    if (all) c.resend = false;
}

void drop_stale_requests() {
    const uint64_t now = GetTickCount64();
    while (!c.requests.empty() && now - c.requests.front().at > kRequestMs) {
        log_line("COLLIERS", "demande de %s trop ancienne, oubliee (%s)",
                 mm_peer_name(c.requests.front().from).c_str(), c.requests.front().name.c_str());
        c.requests.pop_front();
    }
}

void host_pump(View& v) {
    drop_stale_requests();
    // Apres son « Valider », plus de changement : les colliers sont ceux qu'il a
    // valides (et que la confirmation du jeu a vus).
    while (c.closing && !c.requests.empty()) {
        ++c.dropped;
        log_line("COLLIERS", "demande de %s arrivee apres « Valider », ignoree (%s)",
                 mm_peer_name(c.requests.front().from).c_str(), c.requests.front().name.c_str());
        c.requests.pop_front();
    }
    int budget = kApplyPerFrame;
    while (!c.requests.empty() && budget-- > 0) {
        const Request r = c.requests.front();
        c.requests.pop_front();
        host_handle(v, r);
        if (!read_view(v.cc, v)) return;
    }
    host_send_changes(v);
}

// --- invite ----------------------------------------------------------------------------

// Un essai de plus pour ce changement ; faux au-dela de kMaxTries (une fois dit).
bool try_once(uint64_t key, const std::string& name, const char* step) {
    int& n = c.tries[fmt("%llu|%s|%s", (unsigned long long)key, name.c_str(), step)];
    if (n >= kMaxTries) return false;
    if (++n == kMaxTries)
        log_line("COLLIERS", "!! chat %llu : « %s » (%s) ne prend pas ici, dernier essai", (unsigned long long)key,
                 name.c_str(), step);
    return true;
}

void guest_pump(View& v) {
    int budget = kApplyPerFrame;
    std::vector<uint64_t> keys;
    for (const auto& kv : c.want) keys.push_back(kv.first);

    // 1. Retirer les colliers que l'hote ne voit pas (ou pas celui-la).
    for (uint64_t key : keys) {
        if (budget <= 0) break;
        const TeamCat* t = v.cat(key);
        const Box* b = v.worn_by(key);
        if (!t || !t->free || !b || c.want[key] == b->name) continue;
        const std::string name = b->name;
        void* p = b->p;
        if (!try_once(key, name, "retire")) continue;
        --budget;
        apply(v, p, key, "retire, comme chez l'hote :", name.c_str());
        if (!read_view(v.cc, v)) return;
    }
    // 2. Poser ceux qui manquent.
    for (uint64_t key : keys) {
        if (budget <= 0) break;
        const std::string want = c.want[key];
        const TeamCat* t = v.cat(key);
        if (want.empty() || !t || !t->free || v.worn_by(key)) continue;
        const Box* b = v.free_box(want.c_str());
        if (!b) {
            if (c.warned.insert(fmt("%llu|%s", (unsigned long long)key, want.c_str())).second)
                log_line("COLLIERS", "!! chat %llu : pas de collier « %s » libre ici", (unsigned long long)key,
                         want.c_str());
            continue;
        }
        void* p = b->p;
        if (!try_once(key, want, "pose")) continue;
        --budget;
        apply(v, p, key, "pose, comme chez l'hote :", want.c_str());
        if (!read_view(v.cc, v)) return;
    }
    // 3. A jour ?
    bool ok = true;
    for (const auto& kv : c.want) {
        const TeamCat* t = v.cat(kv.first);
        if (t && t->free && v.collar(kv.first) != kv.second) { ok = false; break; }
    }
    if (ok && !(c.synced && c.synced_cc == v.cc) && !c.want.empty())
        log_line("COLLIERS", "colliers a jour avec l'hote (%zu chat(s))", c.want.size());
    // Le temps d'ecart ne compte que d'une image du choix a la suivante : une
    // fenetre par-dessus le met en pause, et ce temps-la ne compte pas.
    const uint64_t now = GetTickCount64();
    if (ok)                                  c.unsynced_ms = 0;
    else if (c.pump_frame + 1 == c.frame)    c.unsynced_ms += now - c.pump_ms;
    c.pump_ms    = now;
    c.pump_frame = c.frame;
    c.synced    = ok;
    c.synced_cc = v.cc;
}

bool guest_click(void* button, const char* name) {
    if (strcmp(name, "CloseButton") == 0) {
        // Retenu (l'hote valide pour tous), mais il le sait : ce joueur a fini.
        if (embark_guest_close_queued()) return true;
        const bool sent = send_collar(kEmbarkCollarsDone, 1, kNone, "");
        log_line("COLLIERS", "« Valider » de ce joueur retenu : c'est l'hote qui valide -- %s",
                 sent ? "-> il sait que ce joueur a fini" : "!! (non envoye)");
        if (!c.close_info) {
            c.close_info = true;
            mm_show_info(fmt(sent ? u8"Choix des colliers\n\n"
                                    u8"Prêt ! %s sait que tu as fini :\n"
                                    u8"il validera le départ pour toute l'équipe."
                                  : u8"Choix des colliers\n\n"
                                    u8"C'est %s qui valide le départ :\n"
                                    u8"dis-lui quand tes chats ont leurs colliers.",
                             mm_peer_name(kHostPeer).c_str()));
        }
        return true;
    }
    if (!is_collar_button(name)) {
        if (++c.blocked <= 3 || c.blocked % 20 == 0)
            log_line("COLLIERS", "clic de ce joueur retenu (%s)", name);
        return true;
    }
    View v;
    if (!c.cc || !read_view(c.cc, v)) {
        log_line("COLLIERS", "!! clic sur un collier, choix des colliers illisible : retenu");
        return true;
    }
    const Box* b = v.by_button(button);
    const TeamCat* t = v.cat(v.current);
    if (!b || !t) return true;
    if (!can_act(t)) {
        if (++c.blocked <= 3 || c.blocked % 20 == 0)
            log_line("COLLIERS", "clic retenu : le chat %llu est a %s", (unsigned long long)t->key,
                     owner_name(t).c_str());
        return true;
    }
    if (!t->free) return true;           // il a deja sa classe : le jeu ne ferait rien
    bool equip;
    if (b->on == kNone)           equip = true;
    else if (b->on == v.current)  equip = false;
    else                          return true;
    const bool sent = send_collar(kEmbarkCollarReq, equip ? 1 : 0, v.current, b->name);
    if (sent) ++c.asked;
    log_line("COLLIERS", "%s demande a l'hote : %s %s (chat %llu)", sent ? "->" : "!! (non envoyee)",
             equip ? "poser" : "retirer", b->name, (unsigned long long)v.current);
    return true;
}

// Hote, « Valider » : un invite qui suit le depart n'a pas encore valide les
// colliers de ses chats (il en a un a choisir) -- le clic est retenu une fois,
// avec une fenetre ; le suivant passe.
bool host_close_early() {
    if (c.done_warned || c.broken || !c.cc) return false;
    View v;
    if (!read_view(c.cc, v)) return false;
    std::vector<std::string> names;
    uint32_t seen = 0;
    for (const TeamCat& t : v.cats) {
        const int o = owner_of(t);
        if (!t.free || !follower_active(o) || ((c.done | seen) >> o) & 1) continue;
        seen |= 1u << o;
        names.push_back(mm_peer_name((uint8_t)o));
    }
    if (names.empty()) return false;
    c.done_warned = true;
    std::string who = names[0];
    for (size_t i = 1; i < names.size(); ++i) who += (i + 1 == names.size() ? " et " : ", ") + names[i];
    log_line("COLLIERS", "« Valider » de l'hote retenu une fois : %s n'a pas encore valide ses colliers",
             who.c_str());
    mm_show_info(fmt(u8"Choix des colliers\n\n"
                     u8"%s %s encore les colliers de %s chats.\n"
                     u8"Clique de nouveau sur « Valider » pour partir\n"
                     u8"quand même.", who.c_str(), names.size() > 1 ? u8"choisissent" : u8"choisit",
                     names.size() > 1 ? u8"leurs" : u8"ses"));
    return true;
}

bool host_click(void* button, const char* name) {
    if (strcmp(name, "CloseButton") == 0) return host_close_early();
    if (!is_collar_button(name)) return false;
    View v;
    if (!c.cc || !read_view(c.cc, v)) return false;      // illisible : le jeu decide
    const TeamCat* t = v.cat(v.current);
    if (!v.by_button(button) || can_act(t)) return false;
    if (++c.blocked <= 3 || c.blocked % 20 == 0)
        log_line("COLLIERS", "clic de l'hote retenu : le chat %llu est a %s, qui choisit lui-meme",
                 (unsigned long long)v.current, owner_name(t).c_str());
    return true;
}

// --- le detour --------------------------------------------------------------------------

void on_box_update(void* box) {
    if (c.role == CollarRole::Off || !box) return;
    void* cc = nullptr;
    if (!mem_read((const char*)box + kTag_Chooser, &cc, sizeof(cc)) || !cc) return;
    if (cc != c.cc) {
        if (!box_in_chooser(cc, box)) return;
        c.cc = cc;
    }
    c.cc_frame = c.frame;
    uint64_t cur = kNone;
    if (!mem_read((const char*)cc + kCC_Current, &cur, sizeof(cur))) return;
    if (c.act_frame != c.frame || c.act_key != cur) {
        c.act_frame = c.frame;
        c.act_key   = cur;
        View v;
        c.act_ok = !read_view(cc, v) || can_act(v.cat(cur));
    }
    if (c.act_ok) return;
    // Pas son chat : ses colliers grises, comme le jeu grise ceux des autres
    // chats (et Button::update n'envoie aucun clic dans cet etat).
    void* button = nullptr;
    int32_t state = 0;
    if (!mem_read((const char*)box + kTag_Button, &button, sizeof(button)) || !button ||
        !mem_read((const char*)button + kBtn_State, &state, sizeof(state)) || state == kBtnState_Disabled)
        return;
    const int32_t disabled = kBtnState_Disabled;
    mem_write((char*)button + kBtn_State, &disabled, sizeof(disabled));
}

void __fastcall h_TagUpdate(void* box) {
    o_TagUpdate(box);
    on_box_update(box);
}

} // namespace

// ---------------------------------------------------------------------------------------

void collars_begin(CollarRole role, uint8_t seq) {
    const uint32_t frame = c.frame;
    c = State{};
    c.frame = frame;
    c.role  = role;
    c.seq   = seq;
    log_line("COLLIERS", role == CollarRole::Host
        ? "choix des colliers partage (vote %u) : chacun choisit pour ses chats, l'hote tranche"
        : "choix des colliers partage (vote %u) : ce joueur choisit pour ses chats", (unsigned)seq);
}

void collars_end(const char* why) {
    if (c.role == CollarRole::Off) return;
    log_line("COLLIERS", "fin du choix partage (%s) : %u clic(s) du jeu faits ici, %u demande(s) envoyee(s), "
                         "%u changement(s) envoye(s), %u refus, %u clic(s) retenu(s)", why ? why : "?",
             c.applied, c.asked, c.sent_n, c.refused, c.blocked);
    const uint32_t frame = c.frame;
    c = State{};
    c.frame = frame;
}

void collars_pump() {
    if (c.role == CollarRole::Off) return;
    const bool alive = mm_scene_alive(kChooser);
    const bool seen  = alive && c.cc && c.cc_frame == c.frame;
    ++c.frame;
    if (!alive && c.cc) {
        log_line("COLLIERS", "choix des colliers ferme");
        c.cc = nullptr;
    }
    if (c.role == CollarRole::Host) drop_stale_requests();
    // Invite, aucune boite vue : compte comme unsynced_ms, d'une image du choix
    // (pas en pause) a la suivante seulement.
    if (c.role == CollarRole::Guest) {
        if (c.cc || !alive) {
            c.nobox_ms = 0;
        } else if (mm_scene_running(kChooser)) {
            const uint64_t now = GetTickCount64();
            if (c.nobox_frame + 1 == c.frame) c.nobox_ms += now - c.nobox_at;
            c.nobox_at    = now;
            c.nobox_frame = c.frame;
        }
    }
    if (!seen) return;
    View v;
    if (!read_view(c.cc, v)) return;
    if (v.cc != c.opened) open_chooser(v);
    if (c.role == CollarRole::Host) host_pump(v);
    else                            guest_pump(v);
}

bool collars_on_click(void* button, const char* name) {
    if (c.role == CollarRole::Off || !button || !name) return false;
    if (strcmp(name, "CatSelector_Left") == 0 || strcmp(name, "CatSelector_Right") == 0) return false;
    return c.role == CollarRole::Host ? host_click(button, name) : guest_click(button, name);
}

bool collars_synced() {
    if (c.role != CollarRole::Guest || !mm_scene_alive(kChooser)) return true;
    return c.synced && c.cc && c.synced_cc == c.cc;
}

bool collars_sync_overdue() {
    if (c.role != CollarRole::Guest) return true;
    if (c.broken) return true;
    if (!c.cc && c.nobox_ms > kOverdueMs) return true;     // aucune boite : rien a synchroniser
    return c.cc && c.synced_cc == c.cc && !c.synced && c.unsynced_ms > kOverdueMs;
}

void collars_flush() {
    if (c.role != CollarRole::Host) return;
    if (!c.closing) log_line("COLLIERS", "« Valider » de l'hote : plus aucune demande ne sera appliquee");
    c.closing = true;
    if (!c.cc || !mm_scene_alive(kChooser)) return;
    View v;
    if (read_view(c.cc, v)) host_send_changes(v);
}

void collars_cancel_close() {
    if (c.role != CollarRole::Host || !c.closing) return;
    c.closing = false;
    log_line("COLLIERS", "l'hote reste au choix des colliers (« non » a la confirmation) : les demandes reprennent");
}

int collars_host_prompt_expected() {
    std::vector<uint64_t> keys;
    if (!run_keys(keys)) return -1;
    bool unknown = false;
    for (uint64_t key : keys) {
        void* cat = catsync_cat_by_key(key);
        char cls[64];
        if (!cat || !mem_read_std_string((const char*)cat + kCat_Class, cls, sizeof(cls))) {
            unknown = true;
            continue;
        }
        if (strcmp(cls, kColorless) == 0) return 1;
    }
    return unknown ? -1 : 0;
}

bool collars_broken() { return c.broken; }

void collars_on_mm(uint8_t from, const MmMsg& m) {
    char text[sizeof(m.text) + 1] = {};
    memcpy(text, m.text, sizeof(m.text));
    const uint64_t key = (uint64_t)m.x | ((uint64_t)m.y << 32);
    switch (m.a) {
        case kEmbarkCollarReq:
            if (c.role != CollarRole::Host || m.b != c.seq || from == kHostPeer || from >= 32) return;
            if (c.requests.size() >= kMaxRequests) {
                log_line("COLLIERS", "!! trop de demandes en attente : celle de %s est oubliee",
                         mm_peer_name(from).c_str());
                return;
            }
            c.requests.push_back(Request{ from, m.c != 0, key, text, GetTickCount64() });
            c.done &= ~(1u << from);          // il change encore : plus « fini »
            log_line("COLLIERS", "<- %s demande : %s %s (chat %llu)", mm_peer_name(from).c_str(),
                     m.c ? "poser" : "retirer", text, (unsigned long long)key);
            return;
        case kEmbarkCollarSet: {
            if (c.role != CollarRole::Guest || from != kHostPeer || m.b != c.seq) return;
            const std::string w = m.c ? text : "";
            auto it = c.want.find(key);
            if (it != c.want.end() && it->second == w) return;
            c.want[key] = w;
            c.tries.clear();              // un nouvel etat : les essais repartent de zero
            c.synced_cc   = nullptr;      // a refaire avant le prochain « Valider »
            c.unsynced_ms = 0;
            log_line("COLLIERS", "<- chez l'hote, chat %llu : %s", (unsigned long long)key,
                     w.empty() ? "sans collier" : w.c_str());
            return;
        }
        case kEmbarkChooserOpen:
            if (c.role != CollarRole::Host || m.b != c.seq) return;
            c.resend = true;
            log_line("COLLIERS", "<- %s voit le choix des colliers : tout l'etat lui sera renvoye",
                     mm_peer_name(from).c_str());
            return;
        case kEmbarkCollarsDone:
            if (c.role != CollarRole::Host || m.b != c.seq || from == kHostPeer || from >= 32) return;
            if (!((c.done >> from) & 1))
                log_line("COLLIERS", "<- %s a valide ses colliers : pret", mm_peer_name(from).c_str());
            c.done |= 1u << from;
            return;
        default:
            return;
    }
}

bool collars_binding(Target t, void** detour, void*** original) {
    if (t != T_ClassTagUpdate) return false;
    *detour   = (void*)&h_TagUpdate;
    *original = (void**)&o_TagUpdate;
    return true;
}

} // namespace mgmp
