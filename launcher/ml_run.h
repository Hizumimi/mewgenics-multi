#pragma once
// ml_run -- Mewgenics Multi, le launcher : lancer mgmp_loader.exe comme le .bat
// (`mgmp_loader.exe > loader.txt 2>&1`), mais cache, et sans console.
//
// Le loader tourne sans fenetre (CREATE_NO_WINDOW) ; sa sortie et ses erreurs
// arrivent par un seul tuyau, lu sur un fil a part, recopiees telles quelles
// dans loader.txt et lues par ml_loader pour l'etat et la phrase du joueur.
// L'interface lit tout cela par snapshot(), depuis n'importe quel fil.
//
// Le loader et le jeu qu'il demarre sont dans un job : si le launcher se ferme
// (ou cancel()) pendant le lancement, les deux sont arretes, au lieu de laisser
// un jeu fige sur la pause que le loader y avait posee. Une fois le loader
// sorti, le job lache le jeu, qui vit sa vie.

#include "ml_loader.h"

#include <windows.h>

#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ml {

class Launch {
public:
    enum class State { Idle, Running, Loaded, Failed };

    struct Snapshot {
        State                    state = State::Idle;
        std::string              phrase;   // UTF-8, pour le joueur
        bool                     cancelled = false;   // Failed parce que cancel() l'a arrete
        std::vector<std::string> lines;    // les dernieres lignes du loader, en UTF-8
        unsigned                 seq = 0;  // change a chaque nouvelle ligne ou etat
    };

    Launch() = default;
    Launch(const Launch&) = delete;
    Launch& operator=(const Launch&) = delete;
    ~Launch();   // cancel() si le lancement n'est pas fini

    // Demarre le loader de `dir` (dossier du launcher) ; faux s'il tourne deja.
    // Un echec au demarrage (loader absent...) passe l'etat a Failed avec sa
    // phrase : l'interface n'a qu'un endroit a lire. `loader` remplace
    // dir\mgmp_loader.exe (le faux loader du programme de controle).
    bool start(const std::wstring& dir, const std::wstring& loader = L"");

    // Arrete le loader et le jeu qu'il a demarre, et attend le fil. Un jeu deja
    // repris avec le mod n'est pas touche. start() et cancel() : depuis le fil
    // de l'interface seulement.
    void cancel();

    Snapshot snapshot() const;

private:
    void read_loop(HANDLE out_read, HANDLE txt);
    void push_line(const std::string& raw);
    void set_failed(const std::string& phrase);

    mutable std::mutex m_;
    Snapshot           snap_;
    LoaderReport       report_;
    std::thread        thread_;
    HANDLE             job_ = nullptr;    // remis a nul par le fil avant de les fermer
    HANDLE             proc_ = nullptr;
    bool               cancelled_ = false;
};

} // namespace ml
