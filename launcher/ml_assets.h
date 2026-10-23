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

} // namespace ml
