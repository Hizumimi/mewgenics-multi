// mm_lang.cpp -- voir mm_lang.h.
#include "mm_lang.h"

#include <atomic>
#include <mutex>
#include <string_view>
#include <unordered_map>

namespace mm {

// Les tables, une par fichier (lang_en.cpp...).
extern const TrPair kTable_en[];
extern const TrPair kTable_sp[];
extern const TrPair kTable_de[];
extern const TrPair kTable_it[];
extern const TrPair kTable_pt_br[];
extern const TrPair kTable_ru[];
extern const TrPair kTable_ko[];
extern const TrPair kTable_ja[];
extern const TrPair kTable_zh_cn[];

namespace {

struct Info {
    const char*   code;
    const char*   name;    // CURRENT_LANGUAGE_NAME de combined.csv (sans son U+200B en chinois)
    const TrPair* table;
};
const Info kLangs[kLangCount] = {
    {"fr",    u8"Français",           nullptr},
    {"en",    u8"English",            kTable_en},
    {"sp",    u8"Español",            kTable_sp},
    {"de",    u8"Deutsch",            kTable_de},
    {"it",    u8"Italiano",           kTable_it},
    {"pt-br", u8"Português (Brasil)", kTable_pt_br},
    {"ru",    u8"Русский",            kTable_ru},
    {"ko",    u8"한국어",              kTable_ko},
    {"ja",    u8"日本語",              kTable_ja},
    {"zh-cn", u8"简体中文",            kTable_zh_cn},
};

std::atomic<uint8_t> g_lang{0};

// Les tables en dictionnaires, faits une fois (au premier tr(), de n'importe quel
// fil), puis seulement lus. Une cle en double garde sa premiere traduction
// (tests/test_i18n.cpp les signale).
std::once_flag g_once;
std::unordered_map<std::string_view, const char*> g_maps[kLangCount];

void build() {
    for (int l = 0; l < kLangCount; ++l)
        for (const TrPair* p = kLangs[l].table; p && p->fr; ++p)
            if (p->text && *p->text) g_maps[l].emplace(p->fr, p->text);
}

int idx(Lang l) { return (int)l < kLangCount ? (int)l : 0; }

char lower(char c) { return c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : c; }

} // namespace

const char* lang_code(Lang l) { return kLangs[idx(l)].code; }
const char* lang_name(Lang l) { return kLangs[idx(l)].name; }
const TrPair* lang_table(Lang l) { return kLangs[idx(l)].table; }

bool lang_from_code(const std::string& code, Lang* out) {
    size_t a = code.find_first_not_of(" \t\r\n");
    size_t b = code.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) return false;
    for (int l = 0; l < kLangCount; ++l) {
        const char* c = kLangs[l].code;
        size_t n = 0;
        while (c[n] && a + n <= b && lower(code[a + n]) == c[n]) ++n;
        if (!c[n] && a + n == b + 1) {
            *out = (Lang)l;
            return true;
        }
    }
    return false;
}

Lang lang_current() { return (Lang)g_lang.load(std::memory_order_relaxed); }
void lang_set(Lang l) { g_lang.store((uint8_t)idx(l), std::memory_order_relaxed); }

const char* tr_in(Lang l, const char* fr) {
    if (!fr) return "";
    if (l == Lang::fr) return fr;
    std::call_once(g_once, build);
    const auto& m = g_maps[idx(l)];
    auto it = m.find(fr);
    return it == m.end() ? fr : it->second;
}

const char* tr(const char* fr) { return tr_in(lang_current(), fr); }

bool lang_from_settings(const std::string& text, Lang* out) {
    static const char kKey[] = "current_language";
    const size_t klen = sizeof(kKey) - 1;
    size_t p = 0;
    while (p < text.size()) {
        size_t e = text.find('\n', p);
        if (e == std::string::npos) e = text.size();
        size_t a = text.find_first_not_of(" \t", p);
        // « current_language fr » : la cle, au moins un blanc, puis le code.
        if (a < e && text.compare(a, klen, kKey) == 0 && a + klen < e &&
            (text[a + klen] == ' ' || text[a + klen] == '\t'))
            return lang_from_code(text.substr(a + klen, e - a - klen), out);
        p = e + 1;
    }
    return false;
}

bool lang_from_windows(uint16_t langid, Lang* out) {
    // PRIMARYLANGID (winnt.h) : les dix bits du bas.
    switch (langid & 0x3ff) {
    case 0x0c: *out = Lang::fr; return true;      // LANG_FRENCH
    case 0x09: *out = Lang::en; return true;      // LANG_ENGLISH
    case 0x0a: *out = Lang::sp; return true;      // LANG_SPANISH
    case 0x07: *out = Lang::de; return true;      // LANG_GERMAN
    case 0x10: *out = Lang::it; return true;      // LANG_ITALIAN
    case 0x16: *out = Lang::pt_br; return true;   // LANG_PORTUGUESE (le Portugal aussi)
    case 0x19: *out = Lang::ru; return true;      // LANG_RUSSIAN
    case 0x12: *out = Lang::ko; return true;      // LANG_KOREAN
    case 0x11: *out = Lang::ja; return true;      // LANG_JAPANESE
    case 0x04: *out = Lang::zh_cn; return true;   // LANG_CHINESE (le traditionnel aussi)
    }
    return false;
}

} // namespace mm
