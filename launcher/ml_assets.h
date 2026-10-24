// ml_assets.h -- Mewgenics Multi (launcher) : tout l'art du jeu dont la fenetre
// a besoin, charge d'un coup depuis resources.gpak.
//
//   ui.swf     bitmap 1816  la maison sur la colline   1984 x 1134
//              bitmap 3805  le soleil rayonnant        2000 x 2000
//              bitmap 1     la feuille de papier        920 x  923
//   fonts.swf  Peralta                  les titres
//              TikaFont Adjusted        le texte
//              EdmundMcMillen (INTLv4)  les notes manuscrites
//
// Les identifiants et tailles sont ceux du build 1.1.21239. Si une mise a jour
// du jeu renumerote ui.swf, une image de la mauvaise taille est refusee plutot
// qu'affichee de travers.
//
// CE QUI MANQUE NE BLOQUE RIEN : archive absente ou illisible, SWF abime, image
// absente ou de mauvaise taille, police introuvable -- l'element reste vide
// (Image::empty(), Font::glyphs.empty()), une phrase en francais s'ajoute a
// `problems`, et le reste est charge quand meme. L'interface dessine alors son
// repli (aplat de couleur, police du systeme) et peut ecrire `problems` dans le
// journal. Un launcher qui ne s'ouvre pas parce que l'art manque ne pourrait
// meme plus dire au joueur que l'installation est cassee.
//
// Pas de Windows ici : teste sous Linux (tests/test_assets_live.cpp).
#pragma once

#include "ml_swf.h"

#include <filesystem>
#include <future>
#include <string>
#include <vector>

namespace ml {

struct Assets {
    Image house, sun, paper;
    Font title, text, hand;   // Peralta, TikaFont Adjusted, EdmundMcMillen
    std::vector<std::string> problems;   // vide si tout est la ; textes pour le joueur
    bool complete() const { return problems.empty(); }
};

// Chemin de l'installation Steam par defaut. Le launcher passe plutot le
// dossier du jeu qu'il connait (mgmp.json) : resources.gpak est a cote de
// Mewgenics.exe.
std::filesystem::path default_gpak_path();

// Charge tout, sur le fil appelant (0,15 s mesure sur un SSD, cache chaud : ui.swf
// est lu jusqu'a la derniere des trois images, fonts.swf en entier).
// Ne leve pas d'exception pour une donnee absente ou abimee (voir plus haut) ;
// seul un manque de memoire peut lever std::bad_alloc.
Assets load_assets(const std::filesystem::path& gpak);

// La meme chose sur un fil a part : l'interface s'ouvre tout de suite avec ses
// replis, attend future.wait_for(0) == ready a chaque image, puis prend get()
// (une seule fois) et cree ses bitmaps et geometries Direct2D sur son fil.
std::future<Assets> load_assets_async(std::filesystem::path gpak);

// Les polices du jeu pour l'alphabet d'une langue, dans swfs/international_fonts.swf
// (85 Mo : lu seulement quand la langue en a besoin). Elles y sont nommees par leur
// classe (SymbolClass), celle de data/font_remap.gon :
//   ru     TikaFontCyr  EdmundmCyr   latin et cyrillique, 336 et 327 glyphes
//   ko     KoreanAll    (aucune)     hangul, 11325 glyphes ; `hand` reste vide :
//                                    le jeu ecrit aussi a la main avec celle-ci
//   ja     TikaFontJP   EdmundmJP    kana et kanji, 3449 et 7485 glyphes
//   zh-cn  TikaFontCN   EdmundmCN    hanzi simplifies, 8959 et 6923 glyphes
// Les autres langues n'en ont pas besoin : intl_fonts_needed() est faux. Le texte
// sert aussi aux titres (le jeu remplace Peralta par TikaFont hors du latin).
// Ce qui manque ne bloque rien, comme load_assets : la police reste vide et
// `problems` le dit (pour le journal).
struct IntlFonts {
    std::string lang;   // le code demande ("ja")
    Font text, hand;
    std::vector<std::string> problems;
};
bool intl_fonts_needed(const std::string& lang);
IntlFonts load_intl_fonts(const std::filesystem::path& gpak, const std::string& lang);
std::future<IntlFonts> load_intl_fonts_async(std::filesystem::path gpak, std::string lang);

} // namespace ml
