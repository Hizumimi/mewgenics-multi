#pragma once
// ml_text -- Mewgenics Multi, le launcher : le texte avant le dessin. Les textes
// du launcher sont en UTF-8 ; les polices du jeu (ml_swf.h) sont indexees en
// UCS-2. Et la coupe des phrases en lignes, a la largeur du papier.
//
// La partie portable (sans Windows), verifiee par tests/test_launcher.cpp ; le
// dessin des glyphes est dans ml_main.cpp.

#include <functional>
#include <string>
#include <vector>

namespace ml {

// UTF-8 -> UCS-2. Un octet invalide ou un caractere hors du plan de base (les
// polices du jeu n'en ont pas) donne U+FFFD.
std::u16string utf8_to_u16(const std::string& s);
std::string    u16_to_utf8(const std::u16string& s);   // l'inverse (plan de base)

// Coupe `text` en lignes d'au plus `width` (unites de `advance`). On coupe a une
// espace ou a U+200B (qui disparaissent ; U+200B marque les coupures permises du
// japonais et du chinois, comme dans le jeu), ou apres \ et / (un chemin) ; jamais
// devant : ; ! ? ni le guillemet fermant, ni apres l'ouvrant, comme en typographie
// francaise. Un mot plus large que la ligne est coupe la ou il deborde. '\n' force
// une ligne ; une ligne vide reste.
std::vector<std::u16string> wrap_lines(const std::u16string& text, float width,
                                       const std::function<float(char16_t)>& advance);

// printf vers une std::string, sans limite de longueur : une phrase traduite
// (tr(), mm_lang.h) et ses valeurs, fmt(tr(u8"Jeu trouvé : %s."), dir.c_str()).
std::string fmt(const char* f, ...);

} // namespace ml
