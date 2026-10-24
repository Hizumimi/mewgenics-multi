// test_launcher.cpp -- Mewgenics Multi : la logique du launcher (launcher/
// ml_config.h, ml_loader.h, ml_text.h). Les reglages dans mgmp.json (le reste du
// fichier garde, dans l'ordre ; un chemin avec des accents ; "steam" absent =
// vrai), ce que steam_appid.txt peut contenir, ce que disent les lignes du
// loader, et la coupe en lignes du texte du papier.
// Les lignes ci-dessous sont des copies de loader/mgmp_loader.cpp ; pour qu'un
// message change la-bas casse quand meme ce test, il lit aussi cette source
// (MM_LOADER_SRC) et y cherche chaque texte que ml_loader reconnait.
#include "ml_config.h"
#include "ml_loader.h"
#include "ml_text.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#ifndef MM_LOADER_SRC
#error "MM_LOADER_SRC : le chemin de loader/mgmp_loader.cpp (tests/CMakeLists.txt, tools/linux/tests.sh)"
#endif

using namespace ml;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}

// Le mgmp.json livre dans le zip de l'ami (paquet/mgmp.json), raccourci.
const char* kShipped =
    "{\n"
    "  \"game\": \"C:\\\\Program Files (x86)\\\\Steam\\\\steamapps\\\\common\\\\Mewgenics\\\\Mewgenics.exe\",\n"
    "  \"log\": \"mgmp_trace.log\",\n"
    "  \"net\": {\n"
    "    \"role\": \"off\",\n"
    "    \"addr\": \"127.0.0.1\",\n"
    "    \"port\": 27600,\n"
    "    \"control\": \"auto\"\n"
    "  },\n"
    "  \"ui\": {\n"
    "    \"enabled\": true,\n"
    "    \"visible\": false,\n"
    "    \"key\": \"F1\"\n"
    "  }\n"
    "}\n";

void settings() {
    printf("reglages\n");
    Settings s;
    check(settings_read(kShipped, &s) && s.game == kDefaultGame && s.steam,
          "fichier livre : le jeu chez Steam, Steam vrai (absent)");

    std::string out;
    check(settings_write(kShipped, s, &out) && out == kShipped,
          "reecrit sans changement : le meme texte, octet pour octet");

    // Un chemin avec des accents, en UTF-8 dans le JSON.
    Settings t;
    t.game = u8"D:\\Jeux\\Été\\Mewgenics\\Mewgenics.exe";
    t.steam = false;
    check(settings_write(kShipped, t, &out), "chemin accentue et Steam faux : ecrit");
    check(out.find(u8"\"game\": \"D:\\\\Jeux\\\\Été\\\\Mewgenics\\\\Mewgenics.exe\"") != std::string::npos,
          "l'accent reste tel quel (pas de \\u00c9)");
    check(out.find("\"control\": \"auto\",\n    \"steam\": false\n  },\n  \"ui\"") != std::string::npos,
          "steam ajoute a la fin de net, ui garde a sa place");
    check(out.find("\"log\": \"mgmp_trace.log\"") < out.find("\"net\"") &&
          out.find("\"key\": \"F1\"") != std::string::npos, "les autres champs gardes, dans l'ordre");
    Settings back;
    check(settings_read(out, &back) && back.game == t.game && !back.steam, "relu tel qu'ecrit");

    Settings u = back;
    u.steam = true;
    check(settings_write(out, u, &out) && settings_read(out, &back) && back.steam &&
          out.find("\"steam\": true") != std::string::npos, "steam deja present : remis a vrai");

    check(settings_write("", s, &out) && settings_read(out, &back) && back.game == kDefaultGame &&
          out.find("\"steam\"") == std::string::npos, "pas de fichier : cree avec le jeu seulement");
    check(settings_read("", &back) && back.game == kDefaultGame && back.steam,
          "pas de fichier : les valeurs par defaut");

    check(!settings_write("{ \"game\": ", s, &out), "JSON casse : jamais reecrit");
    back.game = "x";
    check(!settings_read("{ \"game\": ", &back) && back.game == kDefaultGame,
          "JSON casse : refuse, valeurs par defaut");
    check(!settings_read("[1, 2]", &back), "un tableau n'est pas un mgmp.json");
    check(settings_read("{ \"game\": 3, \"net\": { \"steam\": \"non\" } }", &back) &&
          back.game == kDefaultGame && back.steam, "valeurs du mauvais type : ignorees");
    check(settings_read("// note\n{ \"game\": \"C:\\\\a.exe\" }", &back) && back.game == "C:\\a.exe",
          "commentaires toleres, comme le loader");
    check(settings_read("\xEF\xBB\xBF{ \"game\": \"C:\\\\b.exe\" }", &back) && back.game == "C:\\b.exe",
          "BOM du Bloc-notes tolere");
    check(settings_write("{ \"net\": 5 }", t, &out) && settings_read(out, &back) && !back.steam,
          "net qui n'est pas un objet : remplace pour y mettre steam");

    // Le vrai fichier livre est en CRLF (paquet/, .gitattributes).
    std::string crlf;
    for (const char* c = kShipped; *c; ++c) {
        if (*c == '\n') crlf += '\r';
        crlf += *c;
    }
    check(settings_write(crlf, s, &out) && out == crlf, "CRLF reecrit sans changement, octet pour octet");
    check(settings_write(crlf, t, &out) && out.find("\"steam\": false\r\n") != std::string::npos &&
          out.find('\n') == out.find("\r\n") + 1, "CRLF garde quand un reglage change");

    // La DLL et le loader tolerent les commentaires ; dump() les effacerait.
    const char* noted = "{\n  // le jeu chez moi\n  \"game\": \"C:\\\\a.exe\"\n}\n";
    check(!settings_write(noted, s, &out), "commentaire // : jamais reecrit");
    check(!settings_write("{ /* note */ \"game\": \"C:\\\\a.exe\" }", s, &out), "commentaire /* */ : jamais reecrit");
    check(settings_read(noted, &back) && back.game == "C:\\a.exe", "mais toujours lu");

    // La langue du lanceur : "ui"."lang", "auto" s'il manque.
    check(settings_read(kShipped, &back) && back.lang == "auto", "lang absent : auto");
    check(settings_write(kShipped, s, &out) && out.find("\"lang\"") == std::string::npos,
          "lang auto : rien d'ajoute");
    Settings j = s;
    j.lang = "ja";
    check(settings_write(kShipped, j, &out) &&
          out.find("\"key\": \"F1\",\n    \"lang\": \"ja\"\n  }") != std::string::npos,
          "lang ja : ajoute a la fin de ui, les autres champs gardes");
    check(settings_read(out, &back) && back.lang == "ja" && back.game == kDefaultGame, "lang relu tel qu'ecrit");
    j.lang = "auto";
    check(settings_write(out, j, &out) && settings_read(out, &back) && back.lang == "auto" &&
          out.find("\"lang\": \"auto\"") != std::string::npos, "lang deja present : remis a auto");
    check(settings_write("{ \"ui\": 3 }", j, &out) && out.find("\"ui\": 3") != std::string::npos,
          "ui qui n'est pas un objet et lang auto : laisse tel quel");
    j.lang = "ru";
    check(settings_write("{ \"ui\": 3 }", j, &out) && settings_read(out, &back) && back.lang == "ru",
          "ui qui n'est pas un objet : remplace pour y mettre lang");
    check(settings_read("{ \"ui\": { \"lang\": 5 } }", &back) && back.lang == "auto", "lang du mauvais type : auto");
}

void appid() {
    printf("steam_appid.txt\n");
    check(appid_ok("686060"), "686060");
    check(appid_ok("686060\r\n"), "avec fin de ligne Windows");
    check(appid_ok("  686060 \n\n"), "avec espaces");
    check(appid_ok("\t686060\t"), "avec tabulations");
    check(!appid_ok(""), "vide");
    check(!appid_ok(" \r\n"), "que des blancs");
    check(!appid_ok("68606"), "un chiffre de moins");
    check(!appid_ok("6860600"), "un chiffre de plus");
    check(!appid_ok("480"), "un autre jeu");
    check(!appid_ok("686 060"), "coupe en deux");
    check(!appid_ok("\xEF\xBB\xBF" "686060"), "avec BOM : Steam lirait mal");
}

LineKind kind(const char* line) { return classify_line(line).kind; }

void lines() {
    printf("lignes du loader\n");
    check(kind("[*] jeu lance en pause : pid 35200") == LineKind::Progress, "jeu lance en pause");
    check(kind("[*] demarrage du jeu, attente de la premiere image...") == LineKind::Progress,
          "attente de la premiere image");
    check(kind("[*] jeu en pause avant la premiere image ; injection de C:\\mgmp\\mgmp.dll") ==
          LineKind::Progress, "injection");
    check(kind("[*] crochets prets") == LineKind::Progress, "crochets prets");
    check(kind("[*] base de l'image 00007FF7711B0000, point d'entree 00007FF771EEE8AC") == LineKind::Info,
          "adresses : rien a dire");
    check(kind("    loader : 48 FF 25 39 BD 04 00 CC") == LineKind::Info, "ligne en retrait : rien a dire");
    check(kind("[+] jeu repris avec le mod. trace -> C:\\mgmp\\mgmp_trace.log") == LineKind::Success,
          "jeu repris avec le mod : reussite");
    check(kind("[+] mod injecte dans le jeu relance (injection tardive). trace -> C:\\m") ==
          LineKind::Success, "jeu relance injecte : reussite");
    check(kind("[!] delai depasse en attendant les crochets ; reprise quand meme") == LineKind::Warning,
          "crochets en retard : le loader continue");
    check(kind("[!] kernel32 n'est jamais apparu dans les modules du pid 4 ; injection quand meme") ==
          LineKind::Warning, "kernel32 en retard : le loader continue");

    LineInfo steam = classify_line(
        "[!] le jeu s'est ferme pendant son demarrage (code 3221226505, 0xC0000409), avant le "
        "chargement du mod : il n'est PAS injecte.");
    check(steam.kind == LineKind::Failure && steam.phrase.find("Steam") != std::string::npos,
          "jeu ferme au demarrage : echec, Steam");
    LineInfo relaunch = classify_line("[!] aucun jeu relance n'est apparu : le jeu n'est PAS injecte.");
    check(relaunch.kind == LineKind::Failure && relaunch.phrase.find("steam_appid.txt") != std::string::npos,
          "pas de relance : echec, steam_appid.txt");
    check(classify_line("[!] exe du jeu introuvable : C:\\x.exe").phrase.find(u8"Réglages") !=
          std::string::npos, "jeu introuvable : les reglages");
    check(classify_line("[!] mgmp.dll introuvable a cote du loader : C:\\m\\mgmp.dll").kind ==
          LineKind::Failure, "dll introuvable");
    check(classify_line("[!] le debut de FrameBegin ne correspond pas a la version du jeu visee").phrase
          .find("1.1.21239") != std::string::npos, "autre version du jeu");
    check(kind("usage : mgmp_loader.exe <chemin\\vers\\Mewgenics.exe> [args...]") == LineKind::Failure,
          "usage : pas de jeu dans mgmp.json");
    LineInfo other = classify_line("[!] CreateProcessW a echoue (5) : Acces refuse.");
    check(other.kind == LineKind::Failure && other.phrase.find("CreateProcessW a echoue (5)") !=
          std::string::npos, "echec inconnu : la raison du loader");

    printf("deroulement\n");
    LoaderReport ok;
    ok.feed("[*] jeu lance en pause : pid 35200");
    check(ok.state == LoaderReport::Running && !ok.phrase.empty(), "en cours, avec une phrase");
    ok.feed("[*] code de sortie du fil distant LoadLibraryW 0x82310000");
    ok.feed("[!] delai depasse en attendant les crochets ; reprise quand meme");
    check(ok.state == LoaderReport::Running, "un avertissement ne fait pas echouer");
    ok.feed("[+] jeu repris avec le mod. trace -> C:\\mgmp\\mgmp_trace.log");
    ok.finish(0);
    check(ok.state == LoaderReport::Loaded && ok.phrase == u8"Le jeu est lancé avec le mod.",
          "reussite, puis sortie 0");

    LoaderReport inj;
    inj.feed("[!] LoadLibraryW n'a pas charge la DLL dans le jeu (sortie du fil 0x00000000)");
    std::string first = inj.phrase;
    inj.feed("[!] l'injection a echoue : le jeu est ferme");
    inj.finish(1);
    check(inj.state == LoaderReport::Failed && inj.phrase == first, "le premier echec reste la phrase");

    LoaderReport silent;
    silent.feed("[*] jeu lance en pause : pid 1");
    silent.finish(-1073740791);
    check(silent.state == LoaderReport::Failed && silent.phrase.find("Steam") != std::string::npos &&
          silent.phrase.find("0xC0000409") != std::string::npos,
          "code non nul sans [!] : Steam ferme ou deconnecte, le code en hexadecimal");
    LoaderReport zero;
    zero.finish(0);
    check(zero.state == LoaderReport::Failed, "sortie 0 sans reussite : echec");

    printf("octets du loader\n");
    check(loader_bytes_to_utf8("[*] ok") == "[*] ok", "ASCII garde");
    check(loader_bytes_to_utf8("C:\\\xC9t\xE9") == u8"C:\\Été", "latin-1 (locale \"C\") en UTF-8");
    check(loader_bytes_to_utf8(u8"C:\\Été") == u8"C:\\Été", "UTF-8 valide garde");
    check(loader_bytes_to_utf8("a\xC3") == u8"aÃ", "UTF-8 coupe : lu en latin-1");

    printf("source du loader (%s)\n", MM_LOADER_SRC);
    std::ifstream in(MM_LOADER_SRC, std::ios::binary);
    std::ostringstream src;
    src << in.rdbuf();
    check(in.good() && src.str().size() > 1000, "lue");
    for (const std::string& t : loader_texts()) {
        std::string what = "\"" + t + "\" y est";
        check(src.str().find(t) != std::string::npos, what.c_str());
    }
}

// Le texte du papier : UTF-8 -> UCS-2 pour les polices du jeu, et la coupe en
// lignes (ici une unite par caractere).
void text() {
    printf("texte\n");
    check(utf8_to_u16(u8"Réparer « ok » …") == u"Réparer « ok » …", "accents, guillemets, points de suspension");
    check(utf8_to_u16("a\xC3") == u"a\uFFFD", "UTF-8 coupe : U+FFFD");
    check(utf8_to_u16("\xC0\xAF") == u"\uFFFD", "forme trop longue refusee (pas de '/')");
    check(utf8_to_u16(u8"chat \U0001F431") == u"chat \uFFFD", "hors plan de base : un seul U+FFFD");
    check(u16_to_utf8(u"C:\\Jeux\\\u00C9t\u00E9\u2026") == u8"C:\\Jeux\\\u00C9t\u00E9\u2026", "UCS-2 -> UTF-8 (1, 2 et 3 octets)");

    auto one = [](char16_t) { return 1.f; };
    auto lines = wrap_lines(u"un deux trois", 7, one);
    check(lines.size() == 2 && lines[0] == u"un deux" && lines[1] == u"trois", "coupe a l'espace, qui disparait");
    lines = wrap_lines(u"abcdefghij", 4, one);
    check(lines.size() == 3 && lines[0] == u"abcd" && lines[2] == u"ij", "mot trop long coupe ou il deborde");
    lines = wrap_lines(u"le jeu : trouve", 7, one);
    check(lines.size() == 3 && lines[0] == u"le" && lines[1] == u"jeu :" && lines[2] == u"trouve",
          "pas de coupe devant les deux-points");
    lines = wrap_lines(u"C:\\Jeux\\Mewgenics", 10, one);
    check(lines.size() == 2 && lines[0] == u"C:\\Jeux\\" && lines[1] == u"Mewgenics", "chemin coupe apres \\");
    lines = wrap_lines(u"a\n\nb", 10, one);
    check(lines.size() == 3 && lines[1].empty() && lines[2] == u"b", "\\n force une ligne, la ligne vide reste");
    lines = wrap_lines(u"", 10, one);
    check(lines.size() == 1 && lines[0].empty(), "texte vide : une ligne vide");
    lines = wrap_lines(u"tout tient", 100, one);
    check(lines.size() == 1 && lines[0] == u"tout tient", "assez de place : une ligne");

    // Japonais et chinois : U+200B marque les coupures permises (comme le jeu) ; il
    // ne prend pas de place et disparait a la coupe.
    auto zw = [](char16_t c) { return c == u'\u200B' ? 0.f : 1.f; };
    lines = wrap_lines(u"冒険を\u200Bやめますか？", 6, zw);
    check(lines.size() == 2 && lines[0] == u"冒険を" && lines[1] == u"やめますか？", "coupe a U+200B, qui disparait");
    lines = wrap_lines(u"冒険を\u200Bやめ", 6, zw);
    check(lines.size() == 1 && lines[0] == u"冒険を\u200Bやめ", "U+200B sans coupe : garde (sans largeur)");
    lines = wrap_lines(u"冒険をやめますか", 3, zw);
    check(lines.size() == 3 && lines[0] == u"冒険を" && lines[2] == u"すか", "sans U+200B : coupe ou ca deborde");
}

} // namespace

int main() {
    settings();
    appid();
    lines();
    text();
    if (g_fail) printf("%d echec(s)\n", g_fail);
    else printf("tous les tests passent\n");
    return g_fail ? 1 : 0;
}
