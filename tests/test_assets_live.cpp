// test_assets_live.cpp -- Mewgenics Multi (launcher) : l'art du jeu installe
// (launcher/ml_assets.h), lu pour de vrai dans resources.gpak. Sans le jeu (CI,
// Linux), le test le dit et reussit : il ne prouve rien de plus que test_swf.
//
// Il imprime aussi ce dont l'interface a besoin de savoir : le temps de
// chargement, les metriques des polices et les caracteres francais absents
// d'une police (a remplacer par un repli au dessin).
#include "ml_assets.h"

#include <chrono>
#include <cstdio>
#include <filesystem>

using namespace ml;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}

void font(const char* label, const Font& f) {
    size_t empty = 0;
    for (const Glyph& g : f.glyphs) empty += g.path.empty();
    printf("  %s : \"%s\", %zu glyphes (%zu sans contour), ascent %.3f descent %.3f leading %.3f EM, %zu paires de crenage\n",
           label, f.name.c_str(), f.glyphs.size(), empty, f.ascent, f.descent, f.leading, f.kerning.size());
    const Glyph* a = f.find('A');
    check(f.glyphs.size() > 100, "plus de 100 glyphes");
    check(a && !a->path.empty() && a->advance > 0.2f && a->advance < 1.5f, "A dessine, avance plausible");

    const struct { char16_t c; const char* name; } want[] = {
        {'A', "A"}, {0x00E9, "e aigu"}, {0x00E8, "e grave"}, {0x00EA, "e circonflexe"}, {0x00E0, "a grave"},
        {0x00E7, "c cedille"}, {0x00F4, "o circonflexe"}, {0x00C9, "E aigu"}, {'\'', "apostrophe"},
        {0x2019, "apostrophe courbe"}, {0x00AB, "guillemet ouvrant"}, {0x00BB, "guillemet fermant"},
        {0x2014, "tiret cadratin"}, {0x2026, "points de suspension"}, {0x00A0, "espace insecable"},
    };
    printf("    absents :");
    int missing = 0;
    for (const auto& w : want)
        if (!f.find(w.c)) { printf(" %s (U+%04X)", w.name, (unsigned)w.c); ++missing; }
    printf("%s\n", missing ? "" : " aucun");
}

} // namespace

int main() {
    const std::filesystem::path gpak = default_gpak_path();
    std::error_code ec;
    if (!std::filesystem::exists(gpak, ec)) {
        printf("jeu absent (%s) : rien a verifier\ntest_assets_live : tout passe (sans le jeu)\n",
               gpak.u8string().c_str());
        return 0;
    }

    const auto t0 = std::chrono::steady_clock::now();
    Assets a = load_assets_async(gpak).get();
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    printf("charge en %.0f ms\n", ms);
    for (const std::string& p : a.problems) printf("  probleme : %s\n", p.c_str());
    check(a.complete(), "aucun probleme");

    printf("images\n");
    check(a.house.w == 1984 && a.house.h == 1134 && a.house.bgra.size() == 1984u * 1134 * 4, "maison 1984x1134");
    check(a.sun.w == 2000 && a.sun.h == 2000, "soleil 2000x2000");
    check(a.paper.w == 920 && a.paper.h == 923, "papier 920x923");

    printf("polices\n");
    font("titres", a.title);
    font("texte", a.text);
    font("notes", a.hand);

    // Les polices des autres alphabets (international_fonts.swf), langue par langue.
    printf("polices internationales\n");
    const struct { const char* lang; char16_t text, hand; } intl[] = {
        {"ru", 0x0416, 0x0416},      // Ж
        {"ko", 0xD55C, 0},           // 한 (pas de police a la main)
        {"ja", 0x3042, 0x3042},      // あ
        {"zh-cn", 0x6C49, 0x6C49},   // 汉
    };
    for (const auto& w : intl) {
        const auto t1 = std::chrono::steady_clock::now();
        IntlFonts f = load_intl_fonts_async(gpak, w.lang).get();
        const double ms1 = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count();
        printf("  %s : \"%s\" %zu glyphes, \"%s\" %zu glyphes, %.0f ms\n", w.lang, f.text.name.c_str(),
               f.text.glyphs.size(), f.hand.name.c_str(), f.hand.glyphs.size(), ms1);
        for (const std::string& p : f.problems) printf("    probleme : %s\n", p.c_str());
        const Glyph* g = f.text.find(w.text);
        check(f.problems.empty() && g && !g->path.empty() && g->advance > 0.2f, "texte : le caractere de la langue");
        check(w.hand ? (f.hand.find(w.hand) && !f.hand.find(w.hand)->path.empty()) : f.hand.glyphs.empty(),
              w.hand ? "a la main : le caractere de la langue" : "a la main : aucune (comme le jeu)");
    }
    check(!intl_fonts_needed("fr") && !intl_fonts_needed("pt-br") && intl_fonts_needed("ja"),
          "seuls ru, ko, ja, zh-cn en ont besoin");
    check(load_intl_fonts(gpak, "de").text.glyphs.empty(), "allemand : rien a lire");

    printf("archive absente\n");
    Assets none = load_assets(gpak.parent_path() / "absent.gpak");
    check(none.problems.size() == 1 && none.house.empty() && none.title.glyphs.empty(),
          "un seul probleme, tout vide, pas d'exception");

    if (g_fail) { printf("%d echec(s)\n", g_fail); return 1; }
    printf("test_assets_live : tout passe\n");
    return 0;
}
