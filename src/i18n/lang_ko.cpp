// lang_ko.cpp -- Mewgenics Multi : les textes du mod et du lanceur en coreen (ko).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : la forme polie simple des jeux video, comme combined.csv (colonne ko).
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_ko[];
const TrPair kTable_ko[] = {
    // {u8"Le texte francais exact", u8"sa traduction"},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
