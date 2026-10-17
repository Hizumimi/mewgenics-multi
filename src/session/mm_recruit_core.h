#pragma once
// mm_recruit_core -- Mewgenics Multi : a qui va un chat qui rejoint l'equipe en
// cours d'aventure (une recrue). La partie portable, verifiee par
// tests/test_recruit.cpp ; le jeu est dans mm_team.
//
// LES REGLES (document de conception, « Mort d'un chat, mode spectateur et
// nouveaux chats » ; decide le 17 octobre)
//   - Un joueur de l'aventure qui n'a plus de chat vivant dans l'equipe (ou qui
//     est venu sans chat) est spectateur : il voit tout, ne commande rien.
//   - Un nouveau chat qui rejoint l'equipe va a un spectateur ; plusieurs : a
//     celui qui est sans chat depuis le plus longtemps (egalite : au sort).
//   - Aucun spectateur : au joueur qui a le moins de chats vivants (egalite : au
//     sort).
//   - A la fin de l'aventure, la recrue rentre chez le joueur a qui elle a ete
//     attribuee, comme ses propres chats.

#include <cstdint>
#include <vector>

namespace mgmp {

struct RecruitPlayer {
    uint8_t  peer          = 0;
    int      living        = 0;     // ses chats vivants dans l'equipe
    uint64_t catless_since = 0;     // sans chat vivant depuis (ms ; sans objet s'il en a)
};

// Le joueur a qui va la recrue (son numero), -1 sans joueur. `roll(n)` rend un
// entier dans [0, n) : le tirage au sort de l'hote.
int recruit_owner(const std::vector<RecruitPlayer>& players, uint32_t (*roll)(uint32_t n, void* ctx), void* ctx);

} // namespace mgmp
