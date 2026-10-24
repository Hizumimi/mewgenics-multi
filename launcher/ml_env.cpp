// ml_env.cpp -- voir ml_env.h.
#include "ml_env.h"

#include "mgmp_proto.h"   // kProtoVersion : la version que le launcher annonce
#include "mm_lang_win.h"  // la langue du jeu (ui_lang)
#include "ml_text.h"     // fmt

#include <shellapi.h>
#include <shlobj.h>
#include <tlhelp32.h>

#include <cstdio>
#include <fstream>
#include <sstream>

// Fixe par la compilation (CMakeLists.txt, comme pour mgmp.dll).
#ifndef MM_BUILD_ID
#define MM_BUILD_ID "local"
#endif

namespace ml {
namespace {

// La ligne de commande de la copie en administrateur (repair_appid).
const wchar_t kAppIdCmd[] = L"--ml-ecrire-appid";

bool is_file(const std::wstring& path) {
    DWORD a = GetFileAttributesW(path.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring parent(const std::wstring& path) {
    size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? std::wstring() : path.substr(0, slash);
}

// Faux si le fichier existe mais ne se lit pas : ne pas le prendre pour vide,
// config_save le remplacerait par un fichier sans les reglages du joueur.
bool read_file(const std::wstring& path, std::string* out, bool* exists) {
    out->clear();
    *exists = GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
    if (!*exists) return true;
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    *out = ss.str();
    return !in.bad();
}

// 0, ou l'erreur Windows (ERROR_ACCESS_DENIED dans Program Files).
DWORD write_appid(const std::wstring& exe) {
    std::wstring path = parent(exe) + L"\\steam_appid.txt";
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return GetLastError();
    DWORD n = 0, len = (DWORD)strlen(kAppId);
    BOOL ok = WriteFile(h, kAppId, len, &n, nullptr) && n == len;
    DWORD e = ok ? 0 : GetLastError();
    CloseHandle(h);
    return ok ? 0 : (e ? e : ERROR_WRITE_FAULT);
}

bool appid_in_place(const std::wstring& exe) {
    std::string text;
    bool exists = false;
    return read_file(parent(exe) + L"\\steam_appid.txt", &text, &exists) && exists && appid_ok(text);
}

std::wstring self_exe() {
    std::wstring buf(32768, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, &buf[0], (DWORD)buf.size());
    buf.resize(n < buf.size() ? n : 0);
    return buf;
}

} // namespace

std::wstring utf8_to_wide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}

std::string wide_to_utf8(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}

Paths mod_paths() {
    Paths p;
    p.dir    = parent(self_exe());
    p.config = p.dir + L"\\mgmp.json";
    p.loader = p.dir + L"\\mgmp_loader.exe";
    p.dll    = p.dir + L"\\mgmp.dll";
    p.appid  = p.dir + L"\\steam_appid.txt";
    return p;
}

bool config_load(const Paths& p, Settings* out) {
    std::string text;
    bool exists = false;
    if (!read_file(p.config, &text, &exists)) {
        *out = Settings();
        return false;
    }
    return settings_read(text, out);
}

bool config_save(const Paths& p, const Settings& s, std::string* why) {
    std::string text, next;
    bool exists = false;
    if (!read_file(p.config, &text, &exists)) {
        *why = tr(u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.");
        return false;
    }
    if (!settings_write(text, s, &next)) {
        Settings ignored;   // lisible quand meme : refuse pour ses commentaires
        *why = settings_read(text, &ignored)
                   ? tr(u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement "
                        u8"effacerait : retire-les, ou change le réglage à la main dans le fichier.")
                   : tr(u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) "
                        u8"avant de changer les réglages.");
        return false;
    }

    // Le fichier temporaire puis le remplacement : une coupure en pleine
    // ecriture laisse l'ancien mgmp.json entier.
    std::wstring tmp = p.config + L".tmp";
    {
        std::ofstream o(tmp.c_str(), std::ios::binary | std::ios::trunc);
        o << next;
        o.close();
        if (!o) {
            DeleteFileW(tmp.c_str());
            *why = tr(u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.");
            return false;
        }
    }
    if (!MoveFileExW(tmp.c_str(), p.config.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(tmp.c_str());
        *why = tr(u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.");
        return false;
    }
    return true;
}

std::wstring game_exe(const Settings& s) { return utf8_to_wide(s.game); }
std::wstring game_dir(const Settings& s) { return parent(game_exe(s)); }

mm::Lang ui_lang(const std::string& setting, LangFrom* from) {
    mm::Lang l = mm::Lang::fr;
    *from = LangFrom::Setting;
    if (setting != "auto" && mm::lang_from_code(setting, &l)) return l;
    *from = LangFrom::Game;
    const std::wstring game = mm::game_settings_newest();
    if (!game.empty() && mm::game_settings_lang(game, &l)) return l;
    *from = LangFrom::Windows;
    if (mm::lang_from_windows(GetUserDefaultUILanguage(), &l)) return l;
    *from = LangFrom::Default;
    return mm::Lang::fr;
}

bool steam_running() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W pe = { sizeof(pe) };
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"steam.exe") == 0) { found = true; break; }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

const char* build_id() { return MM_BUILD_ID; }
uint32_t proto_version() { return mgmp::kProtoVersion; }

std::vector<Check> check_all() {
    std::vector<Check> out;
    Paths p = mod_paths();

    if (steam_running())
        out.push_back({"steam", Status::Ok, tr(u8"Steam est ouvert."), false});
    else
        out.push_back({"steam", Status::Problem,
                       tr(u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends "
                          u8"qu'il soit connecté."), false});

    Settings s;
    std::string text;
    bool exists = false;
    bool readable = read_file(p.config, &text, &exists);
    if (!exists)
        out.push_back({"config", Status::Problem,
                       tr(u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages "
                          u8"pour le recréer."), false});
    else if (!readable || !settings_read(text, &s))
        out.push_back({"config", Status::Problem,
                       tr(u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip."),
                       false});
    else
        out.push_back({"config", Status::Ok, tr(u8"Réglages lus dans mgmp.json."), false});

    std::wstring exe = game_exe(s), dir = game_dir(s);
    bool game_ok = is_file(exe);
    if (!game_ok)
        out.push_back({"game", Status::Problem,
                       fmt(tr(u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur "
                              u8"Parcourir… et choisis Mewgenics.exe."),
                           s.game.c_str()),
                       false});
    else if (!is_file(dir + L"\\resources.gpak"))
        out.push_back({"game", Status::Problem,
                       tr(u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du "
                          u8"jeu dans Steam."), false});
    else
        out.push_back({"game", Status::Ok, fmt(tr(u8"Jeu trouvé : %s."), wide_to_utf8(dir).c_str()), false});

    std::string appid;
    bool appid_exists = false;
    if (!game_ok)
        out.push_back({"appid", Status::Problem,
                       tr(u8"steam_appid.txt : à vérifier une fois le jeu trouvé."), false});
    else if (!read_file(dir + L"\\steam_appid.txt", &appid, &appid_exists) || !appid_exists)
        out.push_back({"appid", Status::Problem,
                       tr(u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le "
                          u8"mod."), true});
    else if (!appid_ok(appid))
        out.push_back({"appid", Status::Problem,
                       tr(u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le "
                          u8"jeu sans le mod."), true});
    else
        out.push_back({"appid", Status::Ok, tr(u8"steam_appid.txt est en place à côté du jeu."), false});

    const bool dll = is_file(p.dll), loader = is_file(p.loader);
    if (dll && loader)
        out.push_back({"files", Status::Ok, tr(u8"Fichiers du mod présents."), false});
    else if (!dll && !loader)
        out.push_back({"files", Status::Problem,
                       tr(u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : "
                          u8"extrais de nouveau tout le dossier du zip."), false});
    else
        out.push_back({"files", Status::Problem,
                       fmt(tr(u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau "
                              u8"tout le dossier du zip."),
                           dll ? "mgmp_loader.exe" : "mgmp.dll"),
                       false});

    out.push_back({"version", Status::Ok,
                   fmt(tr(u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même."),
                       build_id(), (unsigned)proto_version()),
                   false});
    return out;
}

Repair repair_appid(HWND owner, std::string* why) {
    Settings s;
    config_load(mod_paths(), &s);
    std::wstring exe = game_exe(s);
    if (!is_file(exe)) {
        *why = tr(u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.");
        return Repair::Failed;
    }

    DWORD e = write_appid(exe);
    if (e == 0 && appid_in_place(exe)) return Repair::Done;
    if (e != ERROR_ACCESS_DENIED) {
        *why = tr(u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main "
                  u8"(LISEZ-MOI, étape 2).");
        return Repair::Failed;
    }

    // Program Files : le launcher se relance en administrateur pour cette seule
    // copie (handle_command_line), et attend qu'elle soit faite.
    std::wstring self = self_exe();
    std::wstring args = std::wstring(kAppIdCmd) + L" \"" + exe + L"\"";
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.fMask        = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    sei.hwnd         = owner;
    sei.lpVerb       = L"runas";
    sei.lpFile       = self.c_str();
    sei.lpParameters = args.c_str();
    sei.nShow        = SW_HIDE;
    if (!ShellExecuteExW(&sei)) {
        if (GetLastError() == ERROR_CANCELLED) {
            *why = tr(u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux "
                      u8"réessayer, ou le copier à la main (LISEZ-MOI, étape 2).");
            return Repair::Refused;
        }
        *why = tr(u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie "
                  u8"steam_appid.txt à la main (LISEZ-MOI, étape 2).");
        return Repair::Failed;
    }
    if (sei.hProcess) {
        WaitForSingleObject(sei.hProcess, 30000);
        CloseHandle(sei.hProcess);
    }
    if (appid_in_place(exe)) return Repair::Done;
    *why = tr(u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main "
              u8"(LISEZ-MOI, étape 2).");
    return Repair::Failed;
}

int handle_command_line() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return -1;
    int rc = -1;
    if (argc == 3 && wcscmp(argv[1], kAppIdCmd) == 0) {
        // Seulement a cote d'un .exe qui existe : en administrateur, ne pas
        // ecrire n'importe ou ce qu'une ligne de commande demande.
        std::wstring exe = argv[2];
        bool exe_ok = exe.size() > 4 && _wcsicmp(exe.c_str() + exe.size() - 4, L".exe") == 0 &&
                      is_file(exe);
        rc = exe_ok && write_appid(exe) == 0 ? 0 : 1;
    }
    LocalFree(argv);
    return rc;
}

Logs logs_find(const Paths& p) {
    Logs l;
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((p.dir + L"\\mgmp_trace_*.log").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return l;
    FILETIME best = {};
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        ++l.count;
        if (CompareFileTime(&fd.ftLastWriteTime, &best) > 0) {
            best = fd.ftLastWriteTime;
            l.latest = p.dir + L"\\" + fd.cFileName;
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);

    // La date dans l'ordre de la langue du lanceur (jour/mois, mois/jour, annee d'abord) : un
    // format traduit ne pourrait pas changer l'ordre de ses %u.
    static const wchar_t* const kLocale[mm::kLangCount] = { L"fr-FR", L"en-US", L"es-ES", L"de-DE", L"it-IT",
                                                            L"pt-BR", L"ru-RU", L"ko-KR", L"ja-JP", L"zh-CN" };
    SYSTEMTIME utc, loc;
    wchar_t day[80], hour[80];
    const wchar_t* locale = kLocale[(int)mm::lang_current()];
    if (!l.latest.empty() && FileTimeToSystemTime(&best, &utc) &&
        SystemTimeToTzSpecificLocalTime(nullptr, &utc, &loc) &&
        GetDateFormatEx(locale, DATE_SHORTDATE, &loc, nullptr, day, 80, nullptr) &&
        GetTimeFormatEx(locale, TIME_NOSECONDS, &loc, nullptr, hour, 80)) {
        l.when = wide_to_utf8(day) + " " + wide_to_utf8(hour);
    }
    return l;
}

bool logs_open_folder(const Paths& p) {
    return (INT_PTR)ShellExecuteW(nullptr, L"open", p.dir.c_str(), nullptr, nullptr, SW_SHOWNORMAL) > 32;
}

bool logs_copy_latest(HWND owner, const Paths& p) {
    Logs l = logs_find(p);
    if (l.latest.empty()) return false;

    // DROPFILES, puis la liste des chemins (un seul) terminee par deux zeros.
    size_t bytes = sizeof(DROPFILES) + (l.latest.size() + 2) * sizeof(wchar_t);
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
    if (!mem) return false;
    auto* df = (DROPFILES*)GlobalLock(mem);
    if (!df) { GlobalFree(mem); return false; }
    df->pFiles = sizeof(DROPFILES);
    df->fWide  = TRUE;
    memcpy(df + 1, l.latest.c_str(), l.latest.size() * sizeof(wchar_t));
    GlobalUnlock(mem);

    if (!OpenClipboard(owner)) { GlobalFree(mem); return false; }
    EmptyClipboard();
    bool ok = SetClipboardData(CF_HDROP, mem) != nullptr;
    CloseClipboard();
    if (!ok) GlobalFree(mem);   // sinon le presse-papiers en est proprietaire
    return ok;
}

} // namespace ml
