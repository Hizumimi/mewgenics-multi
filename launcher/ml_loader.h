#pragma once
// ml_loader -- Mewgenics Multi, le launcher : ce que disent les lignes de
// mgmp_loader.exe, en une phrase pour le joueur. La partie portable, verifiee
// par tests/test_launcher.cpp ; le loader est lance par ml_run.
//
// Le loader ecrit pour une console (francais sans accents, voir
// loader/mgmp_loader.cpp) : "[*]" une etape, "[+]" la reussite, "[!]" un echec
// ou un avertissement, et des lignes en retrait qui continuent la precedente.
// Le launcher n'affiche pas ces lignes telles quelles : il en tire l'etat et une
// phrase avec accents ; les lignes brutes restent dans loader.txt et le detail.

#include <string>
#include <vector>

namespace ml {

enum class LineKind {
    Info,       // rien a dire au joueur (adresses, octets, suite d'une ligne)
    Progress,   // une etape du lancement
    Warning,    // un "[!]" apres lequel le loader continue
    Success,    // le mod est charge
    Failure,    // le mod ne sera pas charge
};

struct LineInfo {
    LineKind    kind = LineKind::Info;
    std::string phrase;   // UTF-8, vide pour Info
};

// Une ligne du loader, sans sa fin de ligne.
LineInfo classify_line(const std::string& line);

// Les bouts de lignes que classify_line reconnait (le texte cherche, ou le
// marqueur seul). Le test verifie qu'ils sont tous dans loader/mgmp_loader.cpp.
std::vector<std::string> loader_texts();

// L'etat d'un lancement, ligne apres ligne, puis au code de sortie du loader.
struct LoaderReport {
    enum State { Running, Loaded, Failed };
    State       state = Running;
    std::string phrase;   // UTF-8, la derniere chose a dire au joueur

    void feed(const std::string& line);
    void finish(int exit_code);
};

// Les octets d'une ligne du loader en UTF-8. En locale "C", wprintf ecrit un
// caractere accentue d'un chemin comme son octet latin-1 ; de l'UTF-8 valide
// est garde tel quel.
std::string loader_bytes_to_utf8(const std::string& bytes);

} // namespace ml
