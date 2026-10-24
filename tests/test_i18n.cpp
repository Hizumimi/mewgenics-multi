// test_i18n.cpp -- Mewgenics Multi : la traduction (src/i18n/mm_lang.h) et les
// tables des traducteurs (src/i18n/lang_*.cpp), d'apres les regles de
// docs/TRADUCTION.md.
//
// Le test relit les sources du mod et du lanceur (MM_SRC_DIR : src/ et launcher/)
// et y trouve chaque cle : les litteraux passes a tr( ou trk( (u8"..." colles,
// sur plusieurs lignes, avec leurs \n \" \\ \t \uXXXX). Puis, pour chaque
// traduction de chaque table :
//   - erreur : sa cle n'existe plus dans le code (traduction orpheline), ou elle
//     est en double dans la table ;
//   - erreur : pas les memes marqueurs printf (%s, %d, %u, %lld...), dans le meme
//     ordre et le meme nombre ;
//   - erreur : pas le meme nombre de \n, ou une ligne trop longue (colonnes :
//     lettre = 1, ideogramme, kana, hangul, pleine chasse = 2 ; U+200B et les
//     balises [c:red] [/c] = 0) -- au plus max(36, 1,3 x la ligne francaise), ou
//     pour une ligne francaise courte (16 colonnes au plus : boutons, titres)
//     max(1,6 x, + 6) ;
//   - erreur, japonais et chinois : une ligne de plus de 36 colonnes sans U+200B
//     ni espace (le jeu la couperait n'importe ou) ;
//   - erreur : la cle cite entre « » un texte qui est lui-meme une cle traduite
//     (un bouton : « Pas encore ») et la traduction n'en contient pas la traduction ;
//   - avertissement : les cles encore sans traduction, par langue. Bloquant avec
//     MM_I18N_STRICT=1 (variable d'environnement) ou -DMM_I18N_STRICT.
// Et tr() elle-meme : repli sur le francais, langue fr, cle inconnue ; la lecture
// de settings.txt et la langue de Windows.
#include "mm_lang.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifndef MM_SRC_DIR
#error "MM_SRC_DIR : la racine du depot (tests/CMakeLists.txt, tools/linux/tests.sh)"
#endif

namespace fs = std::filesystem;
using namespace mm;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}
void fail(const std::string& what) {
    printf("  FAIL  %s\n", what.c_str());
    ++g_fail;
}

// --- les cles du code ---------------------------------------------------------

struct Key {
    std::string text;
    std::string where;   // fichier:ligne
};

void put_utf8(std::string& o, uint32_t cp) {
    if (cp < 0x80) o += (char)cp;
    else if (cp < 0x800) { o += (char)(0xC0 | cp >> 6); o += (char)(0x80 | (cp & 0x3F)); }
    else if (cp < 0x10000) {
        o += (char)(0xE0 | cp >> 12); o += (char)(0x80 | ((cp >> 6) & 0x3F)); o += (char)(0x80 | (cp & 0x3F));
    } else {
        o += (char)(0xF0 | cp >> 18); o += (char)(0x80 | ((cp >> 12) & 0x3F));
        o += (char)(0x80 | ((cp >> 6) & 0x3F)); o += (char)(0x80 | (cp & 0x3F));
    }
}

int hexval(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

bool ident(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; }

// Un lecteur de C++ juste assez fin : commentaires, chaines, caracteres, et les
// appels tr( / trk( suivis de litteraux.
struct Scanner {
    const std::string& s;
    std::string file;
    size_t i = 0;
    int line = 1;
    std::vector<Key>* keys;
    std::vector<std::string>* notes;   // appels sans litteral, pour information

    char at(size_t k) const { return k < s.size() ? s[k] : '\0'; }
    void adv(size_t n = 1) {
        for (size_t k = 0; k < n && i < s.size(); ++k, ++i)
            if (s[i] == '\n') ++line;
    }
    // Passe blancs et commentaires.
    void skip_space() {
        for (;;) {
            if (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) adv();
            else if (at(i) == '/' && at(i + 1) == '/') { while (i < s.size() && s[i] != '\n') adv(); }
            else if (at(i) == '/' && at(i + 1) == '*') {
                adv(2);
                while (i < s.size() && !(s[i] == '*' && at(i + 1) == '/')) adv();
                adv(2);
            } else return;
        }
    }
    // Sur le guillemet ouvrant : lit la chaine (sequences d'echappement decodees).
    std::string literal() {
        std::string o;
        adv();   // "
        while (i < s.size() && s[i] != '"') {
            if (s[i] == '\n') return o;   // chaine non fermee : on s'arrete
            if (s[i] != '\\') { o += s[i]; adv(); continue; }
            const char e = at(i + 1);
            adv(2);
            switch (e) {
            case 'n': o += '\n'; break;
            case 't': o += '\t'; break;
            case 'r': o += '\r'; break;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
                int v = e - '0';
                for (int k = 0; k < 2 && at(i) >= '0' && at(i) <= '7'; ++k) { v = v * 8 + (at(i) - '0'); adv(); }
                o += (char)v;
                break;
            }
            case 'x': {
                int v = 0;
                while (hexval(at(i)) >= 0) { v = v * 16 + hexval(at(i)); adv(); }
                o += (char)v;
                break;
            }
            case 'u': case 'U': {
                uint32_t cp = 0;
                for (int k = 0; k < (e == 'u' ? 4 : 8) && hexval(at(i)) >= 0; ++k) { cp = cp * 16 + hexval(at(i)); adv(); }
                put_utf8(o, cp);
                break;
            }
            default: o += e; break;   // \" \' \\ \?
            }
        }
        adv();   // "
        return o;
    }
    // Un litteral commence ici (u8"..., "..., L"..., u"..., U"...) ? Avance jusqu'au guillemet.
    bool literal_start() {
        size_t k = i;
        if (at(k) == 'u' && at(k + 1) == '8') k += 2;
        else if (at(k) == 'u' || at(k) == 'U' || at(k) == 'L') k += 1;
        if (at(k) != '"') return false;
        adv(k - i);
        return true;
    }
    void call(const std::string& name, int at_line) {
        // Sur la parenthese ouvrante.
        adv();
        skip_space();
        std::string text;
        int n = 0;
        while (literal_start()) {
            text += literal();
            ++n;
            skip_space();
        }
        const std::string where = file + ":" + std::to_string(at_line);
        if (n == 0) {
            notes->push_back(where + " : " + name + "( sans litteral");
            return;
        }
        if (at(i) != ')') {
            notes->push_back(where + " : " + name + "( avec autre chose que des litteraux (ignore)");
            return;
        }
        keys->push_back({text, where});
    }
    void run() {
        while (i < s.size()) {
            const char c = s[i];
            if (c == '/' && (at(i + 1) == '/' || at(i + 1) == '*')) { skip_space(); continue; }
            if (c == '"') { literal(); continue; }
            if (c == '\'') {   // caractere ; 'a' '\'' '\\'
                adv();
                while (i < s.size() && s[i] != '\'' && s[i] != '\n') adv(s[i] == '\\' ? 2 : 1);
                adv();
                continue;
            }
            if (ident(c) && (i == 0 || !ident(s[i - 1]))) {
                size_t e = i;
                while (e < s.size() && ident(s[e])) ++e;
                const std::string word = s.substr(i, e - i);
                // R"(...)" : chaine brute, passee telle quelle.
                if ((word == "R" || word == "u8R" || word == "LR" || word == "uR" || word == "UR") && at(e) == '"') {
                    const size_t open = s.find('(', e);
                    const std::string delim = ")" + s.substr(e + 1, open - e - 1) + "\"";
                    const size_t close = s.find(delim, open);
                    adv((close == std::string::npos ? s.size() : close + delim.size()) - i);
                    continue;
                }
                const int at_line = line;
                const char before = i > 0 ? s[i - 1] : ' ';
                adv(e - i);
                if ((word == "tr" || word == "trk") && before != '.' && before != '>') {
                    const size_t save = i;
                    const int save_line = line;
                    skip_space();
                    if (at(i) == '(') { call(word, at_line); continue; }
                    i = save;
                    line = save_line;
                }
                continue;
            }
            adv();
        }
    }
};

void scan_text(const std::string& text, const std::string& file, std::vector<Key>& keys, std::vector<std::string>& notes) {
    Scanner sc{text, file};
    sc.keys = &keys;
    sc.notes = &notes;
    sc.run();
}

std::vector<Key> scan_sources(std::vector<std::string>& notes, int* files) {
    std::vector<Key> keys;
    *files = 0;
    const fs::path root = fs::u8path(MM_SRC_DIR);
    for (const char* dir : {"src", "launcher"}) {
        std::error_code ec;
        for (fs::recursive_directory_iterator it(root / dir, ec), end; !ec && it != end; it.increment(ec)) {
            const fs::path p = it->path();
            const std::string ext = p.extension().string();
            if (ext != ".cpp" && ext != ".h") continue;
            if (p.parent_path().filename() == "i18n") continue;   // tr() elle-meme et les tables
            std::ifstream in(p, std::ios::binary);
            std::ostringstream ss;
            ss << in.rdbuf();
            ++*files;
            scan_text(ss.str(), fs::relative(p, root).generic_string(), keys, notes);
        }
    }
    return keys;
}

// --- les regles de docs/TRADUCTION.md ---------------------------------------------

// Les marqueurs printf, dans l'ordre : "%s", "%02u", "%lld", "%%"...
std::vector<std::string> printf_specs(const std::string& s) {
    std::vector<std::string> out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] != '%') continue;
        size_t k = i + 1;
        if (k < s.size() && s[k] == '%') { out.push_back("%%"); i = k; continue; }
        while (k < s.size() && strchr("-+ #0'", s[k])) ++k;
        while (k < s.size() && (isdigit((unsigned char)s[k]) || s[k] == '*')) ++k;
        if (k < s.size() && s[k] == '.') { ++k; while (k < s.size() && (isdigit((unsigned char)s[k]) || s[k] == '*')) ++k; }
        while (k < s.size() && strchr("hlLzjtIq", s[k])) ++k;
        if (k < s.size() && s.compare(k - 1, 1, "I") == 0 && (s.compare(k, 2, "64") == 0 || s.compare(k, 2, "32") == 0)) k += 2;
        if (k < s.size() && strchr("diouxXeEfFgGaAcspnSC", s[k])) out.push_back(s.substr(i, k - i + 1));
        i = k;
    }
    return out;
}

std::vector<std::string> split_lines(const std::string& s) {
    std::vector<std::string> out(1);
    for (char c : s) {
        if (c == '\n') out.emplace_back();
        else out.back() += c;
    }
    return out;
}

bool wide(uint32_t cp) {
    return (cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2E80 && cp <= 0xA4CF) || (cp >= 0xAC00 && cp <= 0xD7A3) ||
           (cp >= 0xF900 && cp <= 0xFAFF) || (cp >= 0xFE30 && cp <= 0xFE4F) || (cp >= 0xFF00 && cp <= 0xFF60) ||
           (cp >= 0xFFE0 && cp <= 0xFFE6);
}

// Les colonnes d'une ligne (voir l'en-tete) ; `breaks` : contient U+200B ou une espace.
int cols(const std::string& s, bool* breaks = nullptr) {
    int n = 0;
    bool b = false;
    for (size_t i = 0; i < s.size();) {
        if (s[i] == '[' && (s.compare(i, 3, "[c:") == 0 || s.compare(i, 4, "[/c]") == 0)) {
            const size_t e = s.find(']', i);
            if (e != std::string::npos) { i = e + 1; continue; }
        }
        const unsigned char c = (unsigned char)s[i];
        const int len = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 1;
        uint32_t cp = len == 1 ? c : len == 2 ? (c & 0x1F) : len == 3 ? (c & 0x0F) : (c & 0x07);
        for (int k = 1; k < len && i + k < s.size(); ++k) cp = cp << 6 | ((unsigned char)s[i + k] & 0x3F);
        i += len;
        if (cp == 0x200B || cp == ' ' || cp == 0x3000) b = true;
        if (cp == 0x200B || (cp >= 0x300 && cp <= 0x36F)) continue;
        n += wide(cp) ? 2 : 1;
    }
    if (breaks) *breaks = b;
    return n;
}

constexpr int kBoxCols = 36;    // une ligne de la boite du jeu (TikaFont 24 px, 536 px)
constexpr int kShortCols = 16;  // en dessous : bouton, titre

// En entiers : 40 x 1,3 = 52 tout rond (en double, 52,000...01).
int allowed_cols(int fr) {
    if (fr <= kShortCols) return std::max((fr * 16 + 9) / 10, fr + 6);
    return std::max(kBoxCols, (fr * 13 + 9) / 10);
}

// Les erreurs d'une traduction (vide : conforme).
std::vector<std::string> rule_errors(Lang l, const std::string& fr, const std::string& tx) {
    std::vector<std::string> e;
    if (printf_specs(fr) != printf_specs(tx)) e.push_back("marqueurs printf differents de la cle");
    const auto fl = split_lines(fr), tl = split_lines(tx);
    if (fl.size() != tl.size()) {
        e.push_back(std::to_string(tl.size() - 1) + " \\n au lieu de " + std::to_string(fl.size() - 1));
        return e;
    }
    for (size_t i = 0; i < fl.size(); ++i) {
        bool breaks = false;
        const int c = cols(tl[i], &breaks), max = allowed_cols(cols(fl[i]));
        if (c > max)
            e.push_back("ligne " + std::to_string(i + 1) + " : " + std::to_string(c) + " colonnes, " +
                        std::to_string(max) + " au plus");
        if ((l == Lang::ja || l == Lang::zh_cn) && c > kBoxCols && !breaks)
            e.push_back("ligne " + std::to_string(i + 1) + " : plus de 36 colonnes sans U+200B ni espace");
    }
    return e;
}

// Les textes cites entre « » (sans les espaces, insecables comprises, autour).
std::vector<std::string> quoted(const std::string& s) {
    static const std::string open = u8"«", close = u8"»", nbsp = u8"\u00A0", nnbsp = u8"\u202F";
    std::vector<std::string> out;
    for (size_t a = s.find(open); a != std::string::npos; a = s.find(open, a + 1)) {
        const size_t b = s.find(close, a);
        if (b == std::string::npos) break;
        std::string q = s.substr(a + open.size(), b - a - open.size());
        for (bool more = true; more;) {
            more = false;
            for (const std::string& sp : {std::string(" "), nbsp, nnbsp}) {
                if (q.compare(0, sp.size(), sp) == 0) { q.erase(0, sp.size()); more = true; }
                if (q.size() >= sp.size() && q.compare(q.size() - sp.size(), sp.size(), sp) == 0) {
                    q.erase(q.size() - sp.size());
                    more = true;
                }
            }
        }
        if (!q.empty()) out.push_back(q);
    }
    return out;
}

std::string no_zwsp(std::string s) {
    static const std::string z = u8"\u200B";
    for (size_t p; (p = s.find(z)) != std::string::npos;) s.erase(p, z.size());
    return s;
}

std::string shorten(const std::string& s) {
    std::string o;
    for (char c : s) o += c == '\n' ? std::string("\\n") : std::string(1, c);
    return o.size() > 70 ? o.substr(0, 67) + "..." : o;
}

// --- les parties du test ----------------------------------------------------------

void test_scanner() {
    printf("lecture des sources\n");
    const std::string src =
        "// tr(u8\"dans un commentaire\")\n"
        "/* tr(u8\"bloc\") */ const char* a = \"tr(u8\\\"chaine\\\")\";\n"
        "x = tr(u8\"Un\\n\"\n"
        "       u8\"deux \\\"trois\\\" \\\\ \\t\\u00A0quatre\") + ptr(u8\"non\") + str(\"non\");\n"
        "f(tr( /* c */ u8\"%s a rejoint\" ), trk(u8\"cl\xC3\xA9\"), tr(nom), obj.tr(u8\"non\"));\n"
        "char q = '\"'; tr(u8\"apres guillemet\");\n"
        "auto r = R\"(tr(u8\"brut\"))\"; tr(u8\"fin\" MACRO);\n";
    std::vector<Key> keys;
    std::vector<std::string> notes;
    scan_text(src, "t.cpp", keys, notes);
    check(keys.size() == 4, "quatre cles (commentaires, chaines, ptr(, str(, .tr( ignores)");
    check(keys.size() > 0 && keys[0].text == "Un\ndeux \"trois\" \\ \t\xC2\xA0quatre" && keys[0].where == "t.cpp:3",
          "litteraux colles sur deux lignes, echappements, ligne du tr(");
    check(keys.size() > 1 && keys[1].text == "%s a rejoint", "commentaire dans l'appel");
    check(keys.size() > 2 && keys[2].text == "cl\xC3\xA9", "trk( et UTF-8");
    check(keys.size() > 3 && keys[3].text == "apres guillemet" && keys[3].where == "t.cpp:6", "apres un caractere '\"'");
    check(notes.size() == 2, "tr(nom) et tr(u8\"...\" MACRO) notes a part");
}

void test_rules() {
    printf("regles\n");
    check(printf_specs("%s a %d ans, %02u%% et %lld %ls") ==
              std::vector<std::string>({"%s", "%d", "%02u", "%%", "%lld", "%ls"}), "marqueurs printf");
    check(cols(u8"abc") == 3 && cols(u8"日本語") == 6 && cols(u8"한국") == 4 && cols(u8"日\u200B本") == 4 &&
          cols(u8"[c:red]rouge[/c]") == 5 && cols(u8"été") == 3, "colonnes");
    check(rule_errors(Lang::en, "Jouer", "Play").empty(), "conforme");
    check(rule_errors(Lang::en, "Bonjour %s", "Hello").size() == 1, "marqueur oublie");
    check(rule_errors(Lang::en, "%s a %d", "%d has %s").size() == 1, "marqueurs dans un autre ordre");
    check(rule_errors(Lang::en, "a\nb", "a b").size() == 1, "\\n oublie");
    check(rule_errors(Lang::en, "Jouer", "Play the game right now").size() == 1, "bouton trop long");
    check(rule_errors(Lang::de, std::string(40, 'a'), std::string(52, 'b')).empty() &&
              rule_errors(Lang::de, std::string(40, 'a'), std::string(53, 'b')).size() == 1,
          "ligne longue : 1,3 fois la francaise");
    check(rule_errors(Lang::ja, std::string(40, 'a'), u8"冒険をやめますか冒険をやめますか冒険を").size() == 1 &&
              rule_errors(Lang::ja, std::string(40, 'a'), u8"冒険を\u200Bやめますか冒険を\u200Bやめますか冒険を").empty(),
          "japonais long : U+200B obligatoire");
    check(quoted(u8"réponds « Pas encore », puis «\u00A0Jouer\u00A0» ou «»") ==
              std::vector<std::string>({u8"Pas encore", u8"Jouer"}), "textes cites entre « »");
}

void test_tr() {
    printf("tr()\n");
    const char* k = u8"Une cle qui n'existe dans aucune table";
    check(lang_current() == Lang::fr, "francais au depart");
    check(tr(k) == k && tr_in(Lang::ja, k) == k, "cle inconnue : le texte francais lui-meme");
    check(tr(nullptr) != nullptr && *tr(nullptr) == 0, "nullptr : \"\"");
    lang_set(Lang::ru);
    check(lang_current() == Lang::ru && tr(k) == k, "autre langue, cle inconnue : le francais");
    lang_set(Lang::fr);

    std::set<std::string> codes;
    bool names = true;
    for (int l = 0; l < kLangCount; ++l) {
        codes.insert(lang_code((Lang)l));
        names = names && lang_name((Lang)l)[0];
        Lang back;
        check(lang_from_code(lang_code((Lang)l), &back) && back == (Lang)l, lang_code((Lang)l));
    }
    check(codes.size() == kLangCount && names && !lang_table(Lang::fr), "codes uniques, noms, pas de table fr");
    Lang l;
    check(lang_from_code(" PT-BR\r", &l) && l == Lang::pt_br && !lang_from_code("pt", &l) && !lang_from_code("", &l),
          "code : casse et blancs, pas de prefixe");

    check(lang_from_settings("MasterVolume 80\r\ncurrent_language ja\r\nprev_steam_language fr\r\n", &l) &&
              l == Lang::ja, "settings.txt : current_language (CRLF)");
    check(lang_from_settings("current_language_x en\n  current_language\tzh-cn\n", &l) && l == Lang::zh_cn,
          "settings.txt : la bonne cle, tabulation");
    check(!lang_from_settings("prev_steam_language fr\n", &l) && !lang_from_settings("current_language xx\n", &l) &&
              !lang_from_settings("", &l), "settings.txt : sans ligne ou code inconnu");

    check(lang_from_windows(0x040C, &l) && l == Lang::fr && lang_from_windows(0x0416, &l) && l == Lang::pt_br &&
              lang_from_windows(0x0816, &l) && l == Lang::pt_br && lang_from_windows(0x0804, &l) && l == Lang::zh_cn &&
              lang_from_windows(0x0411, &l) && l == Lang::ja && lang_from_windows(0x0C0A, &l) && l == Lang::sp &&
              !lang_from_windows(0x0415, &l), "langue de Windows (le polonais : aucune)");
}

void test_tables(const std::vector<Key>& keys) {
    std::map<std::string, std::string> where;   // cle -> premier endroit
    for (const Key& k : keys) where.emplace(k.text, k.where);
    printf("tables (%zu cles distinctes dans le code)\n", where.size());

    bool strict = getenv("MM_I18N_STRICT") && strcmp(getenv("MM_I18N_STRICT"), "1") == 0;
#ifdef MM_I18N_STRICT
    strict = true;
#endif
    for (int li = 1; li < kLangCount; ++li) {
        const Lang l = (Lang)li;
        std::set<std::string> done, seen;
        int errors = 0;
        for (const TrPair* p = lang_table(l); p && p->fr; ++p) {
            const std::string fr = p->fr, tx = p->text ? p->text : "";
            const std::string id = std::string(lang_code(l)) + " « " + shorten(fr) + " »";
            if (!seen.insert(fr).second) { fail(id + " : en double dans la table"); ++errors; continue; }
            if (!where.count(fr)) { fail(id + " : orpheline (cette cle n'est plus dans le code)"); ++errors; continue; }
            if (tx.empty()) continue;
            for (const std::string& e : rule_errors(l, fr, tx)) { fail(id + " : " + e); ++errors; }
            // Un bouton du mod cite entre « » (« Pas encore ») : sa traduction, telle quelle.
            for (const std::string& q : quoted(fr)) {
                const char* bt = tr_in(l, q.c_str());
                if (where.count(q) && strcmp(bt, q.c_str()) != 0 && no_zwsp(tx).find(no_zwsp(bt)) == std::string::npos) {
                    fail(id + " : cite « " + q + " », dont la traduction « " + bt + " » n'y est pas");
                    ++errors;
                }
            }
            if (strcmp(tr_in(l, p->fr), p->text) != 0) { fail(id + " : tr_in ne la rend pas"); ++errors; }
            done.insert(fr);
        }
        std::vector<std::string> missing;
        for (const auto& w : where)
            if (!done.count(w.first)) missing.push_back(w.first);
        printf("  %-5s : %zu / %zu traduites, %d erreur(s)", lang_code(l), done.size(), where.size(), errors);
        if (missing.empty()) printf("\n");
        else {
            printf(", %zu manquante(s)%s, dont « %s » (%s)\n", missing.size(), strict ? " -- BLOQUANT" : " (avertissement)",
                   shorten(missing[0]).c_str(), where[missing[0]].c_str());
            if (strict) ++g_fail;
        }
    }
}

} // namespace

int main() {
    test_scanner();
    test_rules();
    test_tr();

    std::vector<std::string> notes;
    int files = 0;
    const std::vector<Key> keys = scan_sources(notes, &files);
    printf("sources : %d fichiers lus, %zu appels tr( / trk( avec litteral\n", files, keys.size());
    check(files > 50, "les sources du mod et du lanceur sont trouvees (MM_SRC_DIR)");
    for (const std::string& n : notes) printf("  note  %s\n", n.c_str());
    for (const Key& k : keys)
        if (k.text.empty()) fail(k.where + " : cle vide");
    test_tables(keys);

    if (g_fail) printf("%d echec(s)\n", g_fail);
    else printf("test_i18n : tout passe\n");
    return g_fail ? 1 : 0;
}
