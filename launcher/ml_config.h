#pragma once
// ml_config -- Mewgenics Multi, le launcher : ses reglages dans mgmp.json et le
// contenu de steam_appid.txt. La partie portable (sans Windows), verifiee par
// tests/test_launcher.cpp ; les fichiers eux-memes sont lus et ecrits par ml_env.
//
// Le launcher ne possede pas mgmp.json : le loader y lit "game", la DLL tout le
// reste. Il n'y change donc que ses deux reglages et rend le texte avec tous les
// autres champs, dans leur ordre (ordered_json), indente de 2 espaces comme le
// fichier livre. Un fichier qui n'est pas du JSON n'est jamais reecrit : ce que
// le joueur y a tape serait perdu.

#include <string>

namespace ml {

// Le jeu chez Steam, quand mgmp.json ne dit rien (le meme que le fichier livre).
constexpr const char* kDefaultGame =
    "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Mewgenics\\Mewgenics.exe";

// L'id Steam de Mewgenics, ce que doit contenir steam_appid.txt a cote du jeu.
constexpr const char* kAppId = "686060";

struct Settings {
    std::string game = kDefaultGame;   // chemin du jeu, en UTF-8 comme dans le JSON
    bool        steam = true;          // "net"."steam" : absent du fichier livre, donc vrai
};

// Lit les reglages d'un mgmp.json. Un texte vide (pas de fichier) donne les
// valeurs par defaut ; false si le texte n'est pas un objet JSON (out garde
// alors les valeurs par defaut).
bool settings_read(const std::string& text, Settings* out);

// Le texte de mgmp.json avec les reglages de `s`, tout le reste garde. "steam"
// n'est ajoute que s'il change quelque chose (faux, ou deja present). false si
// `text` n'est pas un objet JSON : on ne reecrit pas un fichier qu'on ne
// comprend pas. false aussi s'il a des commentaires (que le loader tolere, et
// settings_read aussi) : les reecrire les effacerait. Les fins de ligne CRLF
// sont gardees.
bool settings_write(const std::string& text, const Settings& s, std::string* out);

// steam_appid.txt dit-il 686060 ? Espaces et fins de ligne toleres autour.
bool appid_ok(const std::string& content);

} // namespace ml
