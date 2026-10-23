// ml_swf.h -- Mewgenics Multi (launcher) : les images et les polices d'un SWF.
//
// Le launcher dessine avec l'art du jeu : les illustrations de ui.swf et les
// polices de fonts.swf, lues dans resources.gpak sur la machine du joueur.
// Rien de ceci n'est copie dans le depot.
//
// Ce qui est lu, et seulement ca :
//   - l'en-tete : FWS (brut) ou CWS (zlib) ; ZWS (LZMA) est refuse avec un
//     message, le jeu n'en a pas.
//   - DefineBitsLossless2 (balise 36), format 5 : ARGB premultiplie compresse
//     zlib -> BGRA premultiplie, lignes de w*4 octets, exactement ce que veut
//     Direct2D (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
//     Le format 3 (palette) n'est pas lu : aucune image voulue ne l'utilise.
//   - DefineFont3 (balise 75) : contours, codes, avances, metriques, crenage.
//
// Le fichier vient du disque : chaque lecture est bornee (une balise ne deborde
// pas du fichier, un glyphe pas de sa balise, une image pas de sa taille
// annoncee). Une donnee fausse donne false et un message, jamais une lecture
// hors limites ni une allocation folle.
//
// Unites des polices : DefineFont3 dessine dans un carre EM de 1024 x 20 =
// 20480 unites. Tout est rendu ici en EM = 1.0 : multiplier par la taille en
// pixels (ou en DIP) de la police. Repere : x vers la droite, y VERS LE BAS
// (celui de Direct2D), origine sur la ligne de base au debut du glyphe ; le
// haut d'une majuscule est donc a y ~ -0.7.
//
// Pas de Windows ici : teste sous Linux (tests/test_swf.cpp).
#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace ml {

// BGRA premultiplie, w*h*4 octets, ligne par ligne de haut en bas. Vide
// (w == h == 0) quand l'image manque : l'interface dessine alors son repli.
struct Image {
    uint32_t w = 0, h = 0;
    std::vector<uint8_t> bgra;
    bool empty() const { return bgra.empty(); }
};

// Une commande de trace en EM = 1.0. Move ouvre une figure, Line et Quad la
// prolongent jusqu'a (x, y) ; Quad a son point de controle en (cx, cy).
struct PathCmd {
    enum Op : uint8_t { Move, Line, Quad } op;
    float cx, cy, x, y;
};

// Chaque figure (de Move au Move suivant) est FERMEE : elle revient a son
// point de depart. Avec Direct2D : BeginFigure(..., D2D1_FIGURE_BEGIN_FILLED)
// sur Move, AddLine / AddQuadraticBezier, EndFigure(D2D1_FIGURE_END_CLOSED).
//
// REGLE DE REMPLISSAGE : pair-impair (D2D1_FILL_MODE_ALTERNATE). Les figures
// sont de plus orientees de facon coherente (un trou tourne a l'inverse de ce
// qui l'entoure), donc l'enroulement non nul (D2D1_FILL_MODE_WINDING) donne la
// meme image ; pair-impair est la regle convenue.
struct Glyph {
    std::vector<PathCmd> path;   // vide pour l'espace
    float advance = 0;           // EM ; 0 si la police n'a pas de table d'avances
};

struct Font {
    std::string name;            // tel que dans le SWF, sans le zero final
    bool bold = false, italic = false;
    float ascent = 0, descent = 0, leading = 0;   // EM, ascent et descent positifs
    std::vector<Glyph> glyphs;
    std::unordered_map<char16_t, uint16_t> glyph_of;   // code UCS-2 -> glyphe

    // Glyphe d'un caractere, ou nullptr s'il n'est pas dans la police (a
    // l'interface de choisir un repli : sans accent, '?', autre police).
    const Glyph* find(char16_t c) const {
        auto it = glyph_of.find(c);
        return it == glyph_of.end() ? nullptr : &glyphs[it->second];
    }
    // Crenage entre deux caracteres, en EM, a ajouter a l'avance de `a`.
    float kern(char16_t a, char16_t b) const {
        auto it = kerning.find((uint32_t)a << 16 | b);
        return it == kerning.end() ? 0.f : it->second;
    }
    std::map<uint32_t, float> kerning;   // (a << 16 | b) -> EM
};

// Un SWF decompresse en memoire. `data` garde les 8 octets d'en-tete pour que
// les offsets restent ceux du fichier ; `first_tag` est la premiere balise.
struct Swf {
    std::vector<uint8_t> data;
    size_t first_tag = 0;
};

struct SwfTag {
    uint16_t code;
    const uint8_t* p;   // corps de la balise, dans Swf::data
    uint32_t len;
};

// Prend le fichier (deplace, pas copie : ui.swf fait 45 Mo). Faux si ce n'est
// pas un SWF lisible ; `err` dit pourquoi, en francais.
bool swf_open(std::vector<uint8_t> file, Swf& out, std::string& err);

// Appelle fn pour chaque balise jusqu'a End (0) ou jusqu'a ce que fn rende
// false. Rend false si une balise deborde du fichier.
bool swf_each_tag(const Swf& swf, const std::function<bool(const SwfTag&)>& fn);

// Identifiant de caractere d'une balise DefineBitsLossless2 / DefineFont3
// (les deux premiers octets). Faux si la balise est trop courte.
bool swf_char_id(const SwfTag& tag, uint16_t& id);

// DefineBitsLossless2 format 5 -> BGRA premultiplie. Faux (et `out` vide) pour
// un autre format ou une donnee qui ne se decompresse pas a w*h*4 octets.
bool swf_decode_bitmap(const SwfTag& tag, Image& out, std::string& err);

// Nom d'une police DefineFont3 sans tout decoder (pour chercher par prefixe).
bool swf_font_name(const SwfTag& tag, std::string& name);

// DefineFont3 -> Font. Faux si la balise est mal formee.
bool swf_decode_font(const SwfTag& tag, Font& out, std::string& err);

} // namespace ml
