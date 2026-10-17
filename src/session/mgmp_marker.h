#pragma once
// mgmp_marker -- Mewgenics Multi : le pion de la carte arrive sur une case sans
// y avoir marche.
//
// POURQUOI. Un invite suit l'hote dans une case par un EnterNode direct
// (mgmp_follow) : son pion ne marche pas. Or EnterNode ne touche pas au pion,
// et la fin d'une case (0x140392FC0, appelee au retour sur la carte par
// 0x1403B3D60, et dans EnterNode meme pour les boites de meubles et de
// nourriture) travaille sur la case OU EST LE PION (MapScreen+0xA0 -> +0x50) :
// elle la marque visitee (0x140228240), compte son type (RunHistory), fait
// avancer la Nemesis (types 5 a 9) et le boss (type 8), puis MapNode::refresh
// (0x1402262A0) rend cliquables les cases atteignables DEPUIS LE PION. Sans
// pion deplace, la carte de l'invite n'avance jamais : il garde les cases du
// debut, et ses votes du chemin (mm_vote) ne valent plus rien des le second.
//
// CE QUE FAIT LA MARCHE A L'ARRIVEE (MapMarker::update, 0x1402222BD-0x1402222F8) :
//   pion+0x50 = la case, pion+0x58 (etape suivante) = 0, pion+0x68 (avancement,
//   double) -= 1, l'entite du pion (pion+0x48) a +0x80/+0x88 = position de la
//   case (image de la case +0x160, floats +0x70/+0x74, en double), +0x90 = 0 ;
//   puis le rappel d'arrivee (0x14039B180), qui entre dans la case et vide la
//   destination (pion+0x60). On fait la meme chose, avancement a 0 (au repos),
//   juste avant l'EnterNode du suivi.

#include "mgmp_log.h"
#include "mgmp_mem.h"

#include <cstdint>

namespace mgmp {

constexpr uintptr_t kMk_MapMarker  = 0xA0;    // MapScreen -> MapMarker*
constexpr uintptr_t kMk_Entity     = 0x48;    // MapMarker -> son entite (position)
constexpr uintptr_t kMk_At         = 0x50;    // la case ou est le pion
constexpr uintptr_t kMk_Next       = 0x58;    // l'etape suivante de sa marche
constexpr uintptr_t kMk_Target     = 0x60;    // sa destination (le clic)
constexpr uintptr_t kMk_Progress   = 0x68;    // double, avancement vers l'etape
constexpr uintptr_t kMk_EntityPos  = 0x80;    // Vec3D (3 doubles)
constexpr uintptr_t kMk_NodeVisual = 0x160;   // MapNode -> son image
constexpr uintptr_t kMk_VisualXY   = 0x70;    // 2 floats

// Ce pion (MapMarker : celui du joueur, ou celui de la Nemesis -- meme classe)
// est sur `node`, au repos. Vrai = ecrit.
inline bool marker_place(void* marker, void* node) {
    if (!marker || !node) return false;
    void* const  none = nullptr;
    const double rest = 0.0;
    bool ok = mem_write((uint8_t*)marker + kMk_At, &node, sizeof(node)) &&
              mem_write((uint8_t*)marker + kMk_Next, &none, sizeof(none)) &&
              mem_write((uint8_t*)marker + kMk_Target, &none, sizeof(none)) &&
              mem_write((uint8_t*)marker + kMk_Progress, &rest, sizeof(rest));
    // L'image du pion sur la case (sinon il resterait dessine ou il etait).
    void* visual = nullptr;
    void* entity = nullptr;
    float xy[2] = {};
    if (mem_read((const uint8_t*)node + kMk_NodeVisual, &visual, sizeof(visual)) && visual &&
        mem_read((const uint8_t*)visual + kMk_VisualXY, xy, sizeof(xy)) &&
        mem_read((const uint8_t*)marker + kMk_Entity, &entity, sizeof(entity)) && entity) {
        const double pos[3] = { (double)xy[0], (double)xy[1], 0.0 };
        mem_write((uint8_t*)entity + kMk_EntityPos, pos, sizeof(pos));
    }
    return ok;
}

// Le pion de `map` est sur `node`, au repos. Vrai = ecrit.
inline bool marker_arrive(void* map, void* node) {
    void* marker = nullptr;
    if (!map || !node ||
        !mem_read((const uint8_t*)map + kMk_MapMarker, &marker, sizeof(marker)) || !marker) {
        log_line("FOLLOW", "!! pion de la carte introuvable : il reste ou il est");
        return false;
    }
    const bool ok = marker_place(marker, node);
    if (!ok) log_line("FOLLOW", "!! pion de la carte : ecriture impossible");
    return ok;
}

} // namespace mgmp
