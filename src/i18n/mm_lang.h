#pragma once
// mm_lang -- Mewgenics Multi : la traduction des textes du mod (mgmp.dll) et du
// lanceur (« Mewgenics Multi.exe »), dans les dix langues de Mewgenics.
//
// Le francais est la langue SOURCE : chaque texte reste ecrit en francais dans le
// code, passe par tr(), et c'est ce texte exact qui sert de cle dans les tables
// (src/i18n/lang_<code>.cpp, une par langue : neuf traducteurs a la fois, sans
// conflit). Une traduction absente laisse le texte en francais.
//
//     open_popup(..., tr(u8"Multijoueur\n\nHéberge une partie..."), tr(u8"Héberger"), ...);
//     fmt(tr(u8"Connexion à %s..."), target.c_str());
//
// Les regles pour les traducteurs (retours a la ligne, U+200B, marqueurs %s,
// ton, glossaire) : docs/TRADUCTION.md. tests/test_i18n.cpp les verifie ;
// tools/i18n_cles.py liste les cles.
//
// La langue : la DLL suit celle du jeu (settings.txt, mm_lang_win.h) ; le
// lanceur, son reglage "ui"."lang" de mgmp.json, sinon le jeu, sinon Windows.
//
// Pas de Windows ici : teste sous Linux aussi.

#include <cstdint>
#include <string>

namespace mm {

// Les langues du jeu ; les codes sont ceux de settings.txt et de combined.csv.
enum class Lang : uint8_t { fr, en, sp, de, it, pt_br, ru, ko, ja, zh_cn };
constexpr int kLangCount = 10;

const char* lang_code(Lang l);   // "fr", "pt-br", "zh-cn"...
const char* lang_name(Lang l);   // le nom dans sa langue : "Français", "日本語"... (jamais traduit)
// Un code du jeu ("pt-br", casse ignoree, espaces autour toleres) ; faux s'il est inconnu.
bool lang_from_code(const std::string& code, Lang* out);

// La langue courante, lue par tous les fils (atomique). fr au depart.
Lang lang_current();
void lang_set(Lang l);

// Le texte `fr` (francais, exactement celui du code) dans la langue courante ; `fr`
// lui-meme si la langue est fr ou si la traduction manque. Jamais nullptr (nullptr
// donne "") ; le pointeur rendu vit tout le programme.
const char* tr(const char* fr);
// La meme chose dans une langue donnee.
const char* tr_in(Lang l, const char* fr);

// Marque une cle sans la traduire : pour un texte range dans une table et passe a
// tr() plus tard (tr(row.label)). tests/test_i18n.cpp et tools/i18n_cles.py
// trouvent les cles dans les appels tr(u8"...") et trk(u8"...") du code.
constexpr const char* trk(const char* fr) { return fr; }

// Une paire d'une table ; chaque table finit par {nullptr, nullptr}.
struct TrPair {
    const char* fr;     // la cle : le texte francais exact
    const char* text;   // la traduction ; "" = pas encore traduit
};
// La table d'une langue (nullptr pour fr) : pour les tests et les outils.
const TrPair* lang_table(Lang l);

// La langue du jeu d'apres le texte de son settings.txt (ligne « current_language xx »).
// Faux si la ligne manque ou si le code est inconnu.
bool lang_from_settings(const std::string& settings_text, Lang* out);

// La langue du jeu la plus proche d'une langue de Windows (LANGID, par exemple
// GetUserDefaultUILanguage()) ; faux si aucune ne l'est.
bool lang_from_windows(uint16_t langid, Lang* out);

} // namespace mm

// tr() et trk() sans prefixe, dans la DLL (namespace mgmp) comme dans le lanceur (ml).
using mm::tr;
using mm::trk;
