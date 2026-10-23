// ml_loader.cpp -- voir ml_loader.h. Les textes cherches sont ceux de
// loader/mgmp_loader.cpp : un message change la-bas se change ici aussi
// (tests/test_launcher.cpp lit cette source et casse sinon).
#include "ml_loader.h"

#include <cstdio>
#include <cstring>

namespace ml {
namespace {

const char* const kSteamClosed =
    u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus "
    u8"fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit "
    u8"connecté, puis relance.";

const char* const kRelaunch = u8"Steam relance le jeu (ça peut prendre une minute ou deux)…";

struct Rule {
    const char* marker;   // debut de la ligne
    const char* text;     // cherche apres le marqueur
    LineKind    kind;
    const char* phrase;
};

// Dans l'ordre : la premiere regle qui va gagne. Les echecs de la relance Steam
// passent avant "inject", qu'ils contiennent aussi ("n'est PAS injecte").
const Rule kRules[] = {
    {"[*] ", "jeu lance en pause", LineKind::Progress, u8"Ouverture du jeu…"},
    {"[*] ", "demarrage du jeu", LineKind::Progress, u8"Le jeu démarre…"},
    {"[*] ", "injection de", LineKind::Progress, u8"Injection du mod dans le jeu…"},
    {"[*] ", "crochets prets", LineKind::Progress, u8"Le mod est en place, le jeu reprend…"},
    {"[*] ", "ferme proprement pendant son demarrage", LineKind::Progress, kRelaunch},
    {"[*] ", "attente du jeu relance", LineKind::Progress, kRelaunch},
    {"[*] ", "on attend toujours la relance", LineKind::Progress,
     u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, "
     u8"il te pose peut-être une question."},
    {"[*] ", "suivi de la relance", LineKind::Progress, u8"Jeu relancé trouvé : injection du mod…"},

    {"[+] ", "", LineKind::Success, u8"Le jeu est lancé avec le mod."},

    {"[!] ", "reprise quand meme", LineKind::Warning,
     u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même."},
    {"[!] ", "injection quand meme", LineKind::Warning,
     u8"Le jeu relancé tarde à démarrer ; injection quand même."},
    {"[!] ", "mgmp.dll introuvable", LineKind::Failure,
     u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier "
     u8"du mod depuis le zip."},
    {"[!] ", "exe du jeu introuvable", LineKind::Failure,
     u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis "
     u8"Mewgenics.exe."},
    {"[!] ", "mgmp.json n'est pas du JSON valide", LineKind::Failure,
     u8"mgmp.json est illisible : corrige-le, ou remets celui du zip."},
    {"[!] ", "le jeu s'est ferme pendant son demarrage", LineKind::Failure, kSteamClosed},
    {"[!] ", "aucun jeu relance", LineKind::Failure,
     u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de "
     u8"Mewgenics.exe (bouton Réparer), puis relance."},
    {"[!] ", "ne correspond pas a la version du jeu", LineKind::Failure,
     u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?"},
    {"[!] ", "a disparu", LineKind::Failure,
     u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod."},
    {"[!] ", "delai depasse", LineKind::Failure,
     u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance."},
    {"[!] ", "LoadLibraryW", LineKind::Failure,
     u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, "
     u8"envoie loader.txt."},
    {"[!] ", "inject", LineKind::Failure,
     u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, "
     u8"envoie loader.txt."},
    {"usage : ", "", LineKind::Failure,
     u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et "
     u8"choisis Mewgenics.exe."},
};

bool starts_with(const std::string& s, const char* p) {
    return s.compare(0, strlen(p), p) == 0;
}

bool valid_utf8(const std::string& s) {
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        int n = c < 0x80 ? 0 : (c >> 5) == 0x6 ? 1 : (c >> 4) == 0xE ? 2 : (c >> 3) == 0x1E ? 3 : -1;
        if (n < 0 || (n == 1 && c < 0xC2) || (n && i + n >= s.size())) return false;
        for (int k = 1; k <= n; ++k)
            if (((unsigned char)s[i + k] >> 6) != 0x2) return false;
        i += n + 1;
    }
    return true;
}

} // namespace

std::vector<std::string> loader_texts() {
    std::vector<std::string> out;
    for (const Rule& r : kRules) out.push_back(*r.text ? r.text : r.marker);
    return out;
}

LineInfo classify_line(const std::string& line) {
    for (const Rule& r : kRules) {
        if (!starts_with(line, r.marker)) continue;
        if (line.find(r.text, strlen(r.marker)) == std::string::npos) continue;
        return {r.kind, r.phrase};
    }
    // Un "[!]" qu'aucune regle ne connait : un echec, avec la raison du loader.
    if (starts_with(line, "[!] "))
        return {LineKind::Failure, u8"Le mod n'a pas pu être chargé : " + line.substr(4)};
    return {};
}

void LoaderReport::feed(const std::string& line) {
    LineInfo li = classify_line(line);
    if (li.kind == LineKind::Info || state == Failed) return;
    // Le premier echec est le plus precis ; les suivants le resument.
    if (li.kind == LineKind::Failure) state = Failed;
    else if (li.kind == LineKind::Success) state = Loaded;
    else if (state == Loaded) return;
    phrase = li.phrase;
}

void LoaderReport::finish(int exit_code) {
    if (state != Running) return;
    state = Failed;
    if (exit_code == 0) {
        phrase = u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, "
                 u8"envoie loader.txt.";
        return;
    }
    // Un code non nul sans "[!]" : le loader n'a rien pu dire, et au demarrage
    // c'est presque toujours Steam (0xC0000409, vu le 20 octobre).
    char code[48];
    if (exit_code < 0 || exit_code > 255) snprintf(code, sizeof(code), "0x%08X", (unsigned)exit_code);
    else snprintf(code, sizeof(code), "%d", exit_code);
    phrase = std::string(u8"mgmp_loader.exe s'est arrêté (code ") + code +
             u8") sans charger le mod. Cause la plus probable : Steam fermé ou "
             u8"déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.";
}

std::string loader_bytes_to_utf8(const std::string& bytes) {
    if (valid_utf8(bytes)) return bytes;
    std::string out;
    out.reserve(bytes.size() * 2);
    for (unsigned char c : bytes) {
        if (c < 0x80) {
            out += (char)c;
        } else {
            out += (char)(0xC0 | (c >> 6));
            out += (char)(0x80 | (c & 0x3F));
        }
    }
    return out;
}

} // namespace ml
