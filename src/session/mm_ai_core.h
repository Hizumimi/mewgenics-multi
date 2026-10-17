#pragma once
// mm_ai_core -- Mewgenics Multi : le rythme et les garde-fous de l'IA qui joue les
// chats d'un absent (mm_ai.h). Pur, sans Windows ni jeu : teste par
// tests/test_ai.cpp.
#include <cstdint>

#include "mgmp_ability.h"   // AbilitySlotKind (un enum seul)

namespace mgmp {

// Avant chaque decision de l'IA : les joueurs voient a qui c'est le tour, puis le
// chat agir -- sans cela ses actions s'enchainent d'un coup, sans un regard.
constexpr uint32_t kAiThinkMs = 450;

// Decisions par tour au plus. Une IA qui reprend sans fin une action sans effet
// (une cible qui ne bouge pas, un pouvoir refuse) finirait sinon par bloquer le
// combat : au-dela, le tour finit.
constexpr uint32_t kAiMaxActions = 20;

enum class AiStep : uint8_t {
    Wait,      // pas encore : « rien de decide » (TurnAction type 1)
    Decide,    // l'IA choisit maintenant
    EndTurn,   // trop de decisions dans ce tour : fin du tour
};

// Le rythme d'un tour. Une demande du jeu (Brain::GetChoice, chaque image tant que
// rien n'est decide) passe par poll() ; une decision partie, par took().
struct AiPacer {
    uint64_t battle = 0;
    uint32_t turn   = 0;
    uint8_t  cat    = 0xFF;
    uint32_t taken  = 0;    // decisions prises dans ce tour
    uint64_t since  = 0;    // premiere demande de la decision en cours (ms) ; 0 = aucune

    AiStep poll(uint64_t now, uint64_t b, uint32_t t, uint8_t c) {
        if (b != battle || t != turn || c != cat) {
            battle = b; turn = t; cat = c;
            taken = 0;
            since = 0;
        }
        if (!since) since = now ? now : 1;
        const uint64_t elapsed = now > since ? now - since : 0;
        if (elapsed < kAiThinkMs) return AiStep::Wait;
        return taken >= kAiMaxActions ? AiStep::EndTurn : AiStep::Decide;
    }

    // La decision est partie : la suivante attend de nouveau.
    void took() {
        ++taken;
        since = 0;
    }
};

// Ce que l'IA a rendu, et ce qui part : une action (type 2) ne part que si son
// pouvoir est dans une case du chat -- c'est par la case que les autres joueurs
// le retrouvent (mgmp_ability.h) ; une fin de tour (type 3) part telle quelle.
// Tout le reste -- rien de choisi (type 0, un cerveau qui n'est pas un
// PlayerBrain), un genre qui ne voyage pas -- devient une fin de tour : le combat
// ne doit jamais attendre l'IA.
enum class AiVerdict : uint8_t { Use, EndTurn };

inline AiVerdict ai_verdict(uint32_t type, uint8_t slot_kind, const char** why) {
    if (why) *why = nullptr;
    if (type == 3) return AiVerdict::Use;
    if (type == 2) {
        if (slot_kind >= SLOT_MOVE && slot_kind <= SLOT_SPELL) return AiVerdict::Use;
        if (why) *why = "son pouvoir n'est dans aucune case du chat";
        return AiVerdict::EndTurn;
    }
    if (why) *why = type == 0 ? "l'IA n'a rien choisi" : "une action d'un genre qui ne voyage pas";
    return AiVerdict::EndTurn;
}

} // namespace mgmp
