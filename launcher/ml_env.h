#pragma once
// ml_env -- Mewgenics Multi, le launcher : ce qu'il trouve sur la machine. Les
// chemins, mgmp.json sur le disque, les verifications d'avant le lancement, la
// reparation de steam_appid.txt et les journaux du mod. Ce que faisait le .bat
// (detection de Steam) et ce que LISEZ-MOI.txt demandait de faire a la main
// (etapes 2 a 4).
//
// Tous les chemins sont en wchar_t (un dossier avec des accents marche) ; les
// textes pour le joueur sont en UTF-8, avec accents. Rien ici ne retient d'etat :
// l'interface rappelle check_all() quand elle veut un etat a jour.

#include "ml_config.h"
#include "mm_lang.h"

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace ml {

std::wstring utf8_to_wide(const std::string& s);
std::string  wide_to_utf8(const std::wstring& s);

// Les fichiers du mod, tous a cote de l'exe du launcher.
struct Paths {
    std::wstring dir;      // dossier du launcher, sans \ final
    std::wstring config;   // mgmp.json
    std::wstring loader;   // mgmp_loader.exe
    std::wstring dll;      // mgmp.dll
    std::wstring appid;    // steam_appid.txt fourni dans le zip
};
Paths mod_paths();

// mgmp.json du dossier. false si le fichier existe mais n'est pas du JSON (les
// reglages par defaut sont alors rendus) ; un fichier absent donne les defauts.
bool config_load(const Paths& p, Settings* out);
// Ecrit les reglages en gardant le reste du fichier (fichier temporaire puis
// remplacement). Faux avec une phrase pour le joueur dans *why.
bool config_save(const Paths& p, const Settings& s, std::string* why);

// Le jeu d'apres les reglages.
std::wstring game_exe(const Settings& s);
std::wstring game_dir(const Settings& s);   // sans \ final

// La langue du lanceur (tr(), src/i18n/mm_lang.h) : `setting` ("ui"."lang" de
// mgmp.json) s'il nomme une langue du jeu ; sinon ("auto") celle du jeu, lue dans
// son settings.txt le plus recent ; sinon celle de Windows ; sinon le francais.
// `from` dit laquelle a servi.
enum class LangFrom { Setting, Game, Windows, Default };
mm::Lang ui_lang(const std::string& setting, LangFrom* from);

// --- verifications ---------------------------------------------------------

enum class Status { Ok, Problem, Warning };

struct Check {
    const char* id;            // "steam", "config", "game", "appid", "files", "version"
    Status      status;
    std::string text;          // UTF-8, une phrase pour le joueur, sous le titre de la
                               // ligne : sans repeter son sujet ("Steam ferme" -> "Sans lui...")
    bool        repairable;    // vrai : repair_appid() le corrige
};

// Dans l'ordre d'affichage. Un Problem empeche probablement le mod de se
// charger ; l'interface peut quand meme laisser lancer.
std::vector<Check> check_all();

bool steam_running();               // steam.exe en cours, comme le .bat
const char* build_id();             // MM_BUILD_ID, "local" hors GitHub Actions
uint32_t    proto_version();        // kProtoVersion de src/net/mgmp_proto.h

// --- reparation ------------------------------------------------------------

enum class Repair { Done, Refused, Failed };

// Ecrit 686060 dans steam_appid.txt a cote du jeu. Program Files refuse
// l'ecriture a un compte normal : le launcher se relance alors lui-meme en
// administrateur (UAC, fenetre `owner`) pour cette seule copie. Refused : le
// joueur a dit non a Windows. Bloque pendant la question de l'UAC.
Repair repair_appid(HWND owner, std::string* why);

// A appeler au tout debut de wWinMain : si la ligne de commande est celle de la
// copie en administrateur, la fait et rend le code de sortie ; -1 sinon.
int handle_command_line();

// --- journaux --------------------------------------------------------------

struct Logs {
    int          count = 0;     // mgmp_trace_*.log du dossier
    std::wstring latest;        // chemin complet du plus recent, vide sans journal
    std::string  when;          // UTF-8, "24/10/2026 00:28" dans l'ordre de la langue (heure locale)
};
Logs logs_find(const Paths& p);
bool logs_open_folder(const Paths& p);
// Copie le dernier journal dans le presse-papiers comme un fichier (CF_HDROP) :
// Ctrl+V dans Discord ou l'Explorateur colle le fichier lui-meme.
bool logs_copy_latest(HWND owner, const Paths& p);

} // namespace ml
