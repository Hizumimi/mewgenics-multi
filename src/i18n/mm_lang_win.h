#pragma once
// mm_lang_win -- Mewgenics Multi : la langue du jeu sur cette machine (Windows),
// pour la DLL et le lanceur.
//
// Le jeu garde ses reglages dans %APPDATA%\Glaiel Games\Mewgenics\<id Steam>\
// settings.txt (un dossier par compte Steam qui a joue, a cote de son dossier
// saves\) ; la langue y est la ligne « current_language fr » (mm_lang.h).

#include "mm_lang.h"

#include <cstdint>
#include <string>

namespace mm {

// Le settings.txt le plus recemment ecrit sous %APPDATA%\Glaiel Games\Mewgenics\*\ ;
// vide s'il n'y en a aucun.
std::wstring game_settings_newest();

// La date d'ecriture d'un fichier (FILETIME) ; faux s'il n'existe pas.
bool file_stamp(const std::wstring& path, uint64_t* out);

// La langue que dit un settings.txt ; faux s'il ne se lit pas ou n'en dit rien.
bool game_settings_lang(const std::wstring& path, Lang* out);

} // namespace mm
