// mm_world -- Mewgenics Multi : le monde partage. Voir mm_world.h.

#include "mm_world.h"
#include "mm_world_merge.h"

#include "mgmp_net.h"
#include "mgmp_log.h"
#include "mgmp_savefile.h"

#include "sqlite3.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace mgmp {
namespace {

// --- etat -------------------------------------------------------------------

// Ce qui dit qu'une save a change. Pas la date de modification : Windows ne la
// met pas toujours a jour tant que le jeu garde la save ouverte en ecriture, et
// sqlite reecrit ses pages sur place (la taille ne bouge pas). Le compteur de
// changements de l'en-tete sqlite (octet 24), lui, augmente a chaque
// transaction validee ; avec le nombre de pages (octet 28) et la taille.
struct FileStat {
    uint32_t counter = 0;
    uint32_t pages   = 0;
    uint64_t size    = 0;
    bool operator==(const FileStat& o) const {
        return counter == o.counter && pages == o.pages && size == o.size;
    }
    bool operator!=(const FileStat& o) const { return !(*this == o); }
};

// Le plus gros cadre que le reseau envoie (mgmp_net.cpp, kMaxFrame = 1 Mio),
// moins l'en-tete du message.
constexpr uint64_t kMaxWorldBytes = (1u << 20) - 256;

struct State {
    bool     on        = false;
    bool     is_client = false;
    uint32_t tick      = 0;

    // --- hote : sa save, lue sans verrou puis verifiee ---
    char                 name[64] = {};    // la save suivie
    std::vector<uint8_t> cand;             // derniere lecture, en attente d'une seconde identique
    uint64_t             cand_hash = 0;
    FileStat             cand_stat{};
    std::vector<uint8_t> stable;           // derniere copie verifiee
    uint64_t             stable_hash = 0;
    FileStat             stable_stat{};
    uint64_t             stable_digest = 0; // empreinte de son monde (world_digest)
    uint64_t             sent_digest = 0;  // celle du dernier envoi a tous
    bool                 said_bad = false;
    uint32_t             busy_polls = 0;
    bool                 said_big = false;
    uint32_t             sends = 0;

    // --- invite ---
    std::vector<uint8_t> host_db;          // la save de l'hote, telle que recue
    uint64_t             host_hash = 0;
    char                 host_name[64] = {};
    std::set<std::string> backed_up;       // saves deja copiees cette session
    uint32_t             applied = 0;
};

State g;

uint64_t fnv1a(const uint8_t* p, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ULL; }
    return h;
}

// --- fichiers ---------------------------------------------------------------

bool save_path(const char* name, wchar_t* out, size_t n) {
    wchar_t dir[MAX_PATH];
    if (!name || !name[0] || !savefile_save_dir(dir, MAX_PATH)) return false;
    wchar_t wname[128] = {};
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 128);
    _snwprintf_s(out, n, _TRUNCATE, L"%s\\%s", dir, wname);
    return true;
}

std::string utf8(const wchar_t* w) {
    char buf[MAX_PATH * 3] = {};
    WideCharToMultiByte(CP_UTF8, 0, w, -1, buf, (int)sizeof(buf), nullptr, nullptr);
    return buf;
}

uint32_t be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

// Les 100 octets de l'en-tete sqlite, lus sans verrou comme le reste.
bool stat_file(const wchar_t* path, FileStat& st) {
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz{};
    uint8_t hdr[100] = {};
    DWORD got = 0;
    const bool ok = GetFileSizeEx(h, &sz) && ReadFile(h, hdr, sizeof(hdr), &got, nullptr) &&
                    got == sizeof(hdr);
    CloseHandle(h);
    if (!ok || memcmp(hdr, "SQLite format 3", 16) != 0) return false;
    st.counter = be32(hdr + 24);
    st.pages   = be32(hdr + 28);
    st.size    = (uint64_t)sz.QuadPart;
    return true;
}

// Un journal a cote de la save : le jeu est en train de l'ecrire.
bool being_written(const wchar_t* path) {
    for (const wchar_t* ext : { L"-journal", L"-wal" }) {
        wchar_t j[MAX_PATH];
        _snwprintf_s(j, _TRUNCATE, L"%s%s", path, ext);
        if (GetFileAttributesW(j) != INVALID_FILE_ATTRIBUTES) return true;
    }
    return false;
}

// Lecture sans verrou : le jeu peut tenir la save ouverte, et ne doit jamais
// voir une ecriture echouer a cause de nous.
bool read_file(const wchar_t* path, std::vector<uint8_t>& out) {
    out.clear();
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart < 100 || (uint64_t)sz.QuadPart > kMaxWorldBytes) {
        CloseHandle(h);
        return false;
    }
    out.resize((size_t)sz.QuadPart);
    DWORD got = 0;
    const bool ok = ReadFile(h, out.data(), (DWORD)out.size(), &got, nullptr) && got == out.size();
    CloseHandle(h);
    if (!ok) out.clear();
    return ok;
}

bool back_up(const wchar_t* path, const char* name) {
    wchar_t dir[MAX_PATH];
    if (!savefile_save_dir(dir, MAX_PATH)) return false;
    wchar_t bdir[MAX_PATH];
    _snwprintf_s(bdir, _TRUNCATE, L"%s\\mgmp_backups", dir);
    CreateDirectoryW(bdir, nullptr);
    wchar_t wname[128] = {};
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 128);
    SYSTEMTIME t{};
    GetLocalTime(&t);
    wchar_t dst[MAX_PATH];
    _snwprintf_s(dst, _TRUNCATE, L"%s\\%s.avant-monde-hote.%04u%02u%02u-%02u%02u%02u.bak",
                 bdir, wname, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    if (!CopyFileW(path, dst, FALSE)) {
        log_line("WORLD", "!! copie de securite de %s impossible (win32 %lu) -- la save "
                          "n'est pas modifiee", name, GetLastError());
        return false;
    }
    log_line("WORLD", "copie de securite : mgmp_backups\\%ls", wcsrchr(dst, L'\\') + 1);
    return true;
}

void broadcast_stable(const char* why) {
    SaveFileMsg m{};
    m.size  = (uint32_t)g.stable.size();
    m.hash  = g.stable_hash;
    m.data  = g.stable.data();
    m.fresh = 0;
    strncpy_s(m.name, g.name, _TRUNCATE);
    if (net_send_world(m)) {
        g.sent_digest = g.stable_digest;
        ++g.sends;
        log_line("WORLD", "-> monde de l'hote envoye : %s, %u octets (%s)", g.name, m.size, why);
    } else {
        log_line("WORLD", "!! envoi du monde de l'hote impossible (%u octets)", m.size);
    }
}

void host_pump() {
    if (++g.tick % 60) return;                     // une fois par seconde environ

    char name[64] = {};
    if (!savefile_host_save_name(name, sizeof(name))) return;   // pas encore de save choisie
    if (strcmp(name, g.name) != 0) {               // une autre save : on repart de zero
        strncpy_s(g.name, name, _TRUNCATE);
        g.cand.clear();   g.cand_hash = 0;   g.cand_stat = {};
        g.stable.clear(); g.stable_hash = 0; g.stable_stat = {};
        g.said_bad = false; g.said_big = false; g.busy_polls = 0;
        log_line("WORLD", "l'hote joue %s : son monde sera envoye aux invites", g.name);
    }

    wchar_t path[MAX_PATH];
    if (!save_path(g.name, path, MAX_PATH)) return;
    FileStat st{};
    if (!stat_file(path, st)) return;              // nouvelle partie : pas encore ecrite
    if (st.size > kMaxWorldBytes) {
        if (!g.said_big) log_line("WORLD", "!! la save de l'hote fait %llu octets, trop pour un "
                                           "envoi (%llu au plus) : les invites gardent leur monde",
                                  (unsigned long long)st.size, (unsigned long long)kMaxWorldBytes);
        g.said_big = true;
        return;
    }
    // Un journal a cote : le jeu ecrit. On attend -- mais pas indefiniment, au
    // cas ou un journal resterait la apres un plantage : la double lecture et
    // le controle de sqlite suffisent alors.
    if (being_written(path)) {
        if (g.busy_polls < 10) { ++g.busy_polls; g.cand.clear(); g.cand_hash = 0; return; }
    } else {
        g.busy_polls = 0;
    }
    if (!g.stable.empty() && st == g.stable_stat) return;        // rien de neuf

    std::vector<uint8_t> bytes;
    if (!read_file(path, bytes)) return;
    const uint64_t h = fnv1a(bytes.data(), bytes.size());
    if (h != g.cand_hash || st != g.cand_stat) {   // attendre une seconde lecture identique
        g.cand      = std::move(bytes);
        g.cand_hash = h;
        g.cand_stat = st;
        return;
    }

    // Deux lectures identiques a une seconde d'ecart, sans journal : au repos.
    sqlite3* db = world_open_bytes(g.cand.data(), g.cand.size());
    const bool good = world_looks_like_save(db);
    const uint64_t digest = good ? world_digest(db) : 0;
    sqlite3_close(db);
    if (!good) {
        if (!g.said_bad) log_line("WORLD", "!! la save de l'hote ne passe pas le controle "
                                           "de sqlite -- nouvel essai a sa prochaine ecriture");
        g.said_bad = true;
        g.stable_stat = st;                        // pas de nouvel essai avant un changement
        g.cand.clear(); g.cand_hash = 0;
        return;
    }
    g.said_bad      = false;
    g.stable        = std::move(g.cand);
    g.stable_hash   = h;
    g.stable_stat   = st;
    g.stable_digest = digest;
    g.cand.clear(); g.cand_hash = 0;
    // Aux invites deja la, si le monde a change (une save ecrite a chaque case
    // d'une aventure ne change pas le monde) ; un invite qui arrive plus tard
    // recoit la derniere copie a son arrivee (world_catchup).
    if (g.stable_digest != g.sent_digest && net_active() && net_peer_count() >= 2)
        broadcast_stable("son monde a change");
}

} // namespace

// ---------------------------------------------------------------------------

void world_init() {
    g = State{};
    g.on        = true;
    g.is_client = net_role() == NetRole::Client;
    log_line("WORLD", g.is_client
        ? "arme -- le monde de l'hote (PNJ, histoire, deblocages) sera repris dans la "
          "save de ce joueur juste avant qu'elle se charge"
        : "arme -- la save de l'hote sera envoyee aux invites comme reference du monde");
}

void world_shutdown() {
    if (!g.on) return;
    log_line("WORLD", "fin : %u envoi(s) du monde, %u save(s) mise(s) au monde de l'hote",
             g.sends, g.applied);
    g = State{};
}

void world_pump() {
    if (!g.on || g.is_client) return;
    host_pump();
}

void world_catchup(uint8_t peer) {
    if (!g.on || g.is_client || g.stable.empty()) return;
    SaveFileMsg m{};
    m.size  = (uint32_t)g.stable.size();
    m.hash  = g.stable_hash;
    m.data  = g.stable.data();
    m.fresh = 0;
    strncpy_s(m.name, g.name, _TRUNCATE);
    if (net_send_world_to(peer, m))
        log_line("WORLD", "-> monde de l'hote envoye au joueur %u : %s, %u octets",
                 (unsigned)peer, g.name, m.size);
}

void world_on_message(const SaveFileMsg& m) {
    if (!g.on) return;
    if (!g.is_client) {
        log_line("WORLD", "!! monde recu alors que ce joueur heberge -- ignore");
        return;
    }
    if (!m.data || !world_is_sqlite(m.data, m.size) || fnv1a(m.data, m.size) != m.hash) {
        log_line("WORLD", "!! monde de l'hote illisible (%u octets) -- ignore", m.size);
        return;
    }
    if (m.hash == g.host_hash) return;
    g.host_db.assign(m.data, m.data + m.size);
    g.host_hash = m.hash;
    strncpy_s(g.host_name, m.name, _TRUNCATE);
    log_line("WORLD", "<- monde de l'hote recu : %s, %u octets -- il sera repris au "
                      "prochain chargement de la save de ce joueur", g.host_name, m.size);
}

bool world_have_host() {
    if (!g.on) return false;
    return g.is_client ? !g.host_db.empty() : !g.stable.empty();
}

void world_prepare_own_save(const char* name) {
    if (!g.on || !g.is_client || !name || !name[0]) return;
    if (!net_active()) return;
    if (g.host_db.empty()) {
        log_line("WORLD", "!! le monde de l'hote n'est pas encore arrive : %s garde le sien "
                          "pour cette fois", name);
        return;
    }
    wchar_t path[MAX_PATH];
    if (!save_path(name, path, MAX_PATH)) {
        log_line("WORLD", "!! dossier des saves introuvable : %s garde son monde", name);
        return;
    }
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) {
        log_line("WORLD", "%s est une nouvelle partie : le monde de l'hote y sera repris au "
                          "prochain chargement", name);
        return;
    }

    sqlite3* host = world_open_bytes(g.host_db.data(), g.host_db.size());
    if (!host) {
        log_line("WORLD", "!! le monde de l'hote ne s'ouvre pas : %s garde le sien", name);
        return;
    }
    sqlite3* own = nullptr;
    const std::string upath = utf8(path);
    if (sqlite3_open_v2(upath.c_str(), &own, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
        log_line("WORLD", "!! %s ne s'ouvre pas (%s) : elle garde son monde", name,
                 own ? sqlite3_errmsg(own) : "?");
        sqlite3_close(own);
        sqlite3_close(host);
        return;
    }
    sqlite3_busy_timeout(own, 500);

    // Deux instances du jeu sur un meme PC partagent le dossier des saves : si
    // ce joueur a pris la meme save que l'hote, c'est le meme fichier, que le
    // jeu de l'hote tient ouvert. Surtout ne pas y ecrire. Meme nom et meme
    // proprietaire (ou proprietaire inconnu des deux cotes) : on n'y touche pas.
    if (_stricmp(name, g.host_name) == 0) {
        const std::string mine = world_owner(own), theirs = world_owner(host);
        if (mine == theirs) {
            log_line("WORLD", "%s porte le nom et le proprietaire de la save de l'hote : sans "
                              "doute la meme (meme PC) -- rien a reprendre", name);
            sqlite3_close(own);
            sqlite3_close(host);
            return;
        }
    }

    // Une copie avant la premiere modification de la session.
    if (!g.backed_up.count(name)) {
        if (!back_up(path, name)) {
            sqlite3_close(own);
            sqlite3_close(host);
            return;
        }
        g.backed_up.insert(name);
    }

    WorldSummary sum{};
    const bool ok = world_apply(host, own, sum);
    sqlite3_close(own);
    sqlite3_close(host);
    if (!ok) {
        log_line("WORLD", "!! le monde de l'hote n'a pas pu etre repris dans %s : rien n'est "
                          "change", name);
        return;
    }
    ++g.applied;
    char house[96] = "";
    if (!sum.house_to.empty() && sum.house_to != sum.house_from)
        _snprintf_s(house, _TRUNCATE, ", maison %s -> %s", sum.house_from.c_str(), sum.house_to.c_str());
    char ready[64] = "";
    if (sum.npc_ready_cleared > 0)
        _snprintf_s(ready, _TRUNCATE, " (%d recompense(s) prete(s) laissee(s) a l'hote)",
                    sum.npc_ready_cleared);
    log_line("WORLD", "monde de l'hote (%s) repris dans %s : PNJ %s%s, %d deblocage(s), "
                      "%d tutoriel(s), %d agrandissement(s)%s, %d propriete(s)",
             g.host_name, name, sum.npc ? "ceux de l'hote" : "inchanges", ready, sum.unlocks,
             sum.tutorials, sum.house_ups, house, sum.props);
}

} // namespace mgmp
