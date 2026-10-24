// ml_text.cpp -- voir ml_text.h.
#include "ml_text.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>

namespace ml {
namespace {

// Une espace ou l'on peut couper : pas devant la ponctuation haute ni le guillemet
// fermant, pas apres le guillemet ouvrant.
bool space_breaks(const std::u16string& s, size_t i) {
    char16_t next = i + 1 < s.size() ? s[i + 1] : 0;
    char16_t prev = i > 0 ? s[i - 1] : 0;
    return !(next == u':' || next == u';' || next == u'!' || next == u'?' || next == u'»' ||
             prev == u'«');
}

void wrap_paragraph(const std::u16string& s, float width,
                    const std::function<float(char16_t)>& advance, std::vector<std::u16string>& out) {
    size_t start = 0;
    if (s.empty()) out.emplace_back();
    while (start < s.size()) {
        float w = 0;
        size_t cut = std::u16string::npos, resume = 0, i = start;
        for (; i < s.size(); ++i) {
            char16_t c = s[i];
            if ((c == u' ' && space_breaks(s, i)) || c == u'\u200B') { cut = i; resume = i + 1; }
            w += advance(c);
            if (w > width && i > start && c != u' ') break;
            if (c == u'\\' || c == u'/') { cut = i + 1; resume = i + 1; }
        }
        if (i == s.size()) {
            out.push_back(s.substr(start));
            break;
        }
        if (cut == std::u16string::npos || cut <= start) { cut = i; resume = i; }   // mot trop long
        size_t end = cut;
        while (end > start && s[end - 1] == u' ') --end;
        out.push_back(s.substr(start, end - start));
        start = resume;
        while (start < s.size() && s[start] == u' ') ++start;
    }
}

} // namespace

std::u16string utf8_to_u16(const std::string& s) {
    std::u16string out;
    out.reserve(s.size());
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        int n = c < 0x80 ? 0 : (c >> 5) == 0x6 ? 1 : (c >> 4) == 0xE ? 2 : (c >> 3) == 0x1E ? 3 : -1;
        uint32_t cp = n == 0 ? c : n == 1 ? (c & 0x1F) : n == 2 ? (c & 0x0F) : (c & 0x07);
        bool ok = n >= 0;
        for (int k = 1; ok && k <= n; ++k) {
            if (i + k >= s.size() || ((unsigned char)s[i + k] >> 6) != 0x2) ok = false;
            else cp = cp << 6 | ((unsigned char)s[i + k] & 0x3F);
        }
        if (!ok) {
            out += u'�';
            ++i;
            continue;
        }
        // Formes trop longues et hors plan de base : pas pour nos polices.
        bool overlong = (n == 1 && cp < 0x80) || (n == 2 && cp < 0x800) || n == 3;
        out += overlong || (cp >= 0xD800 && cp < 0xE000) ? u'�' : (char16_t)cp;
        i += n + 1;
    }
    return out;
}

std::string u16_to_utf8(const std::u16string& s) {
    std::string out;
    out.reserve(s.size());
    for (char16_t c : s) {
        if (c < 0x80) {
            out += (char)c;
        } else if (c < 0x800) {
            out += (char)(0xC0 | (c >> 6));
            out += (char)(0x80 | (c & 0x3F));
        } else {
            out += (char)(0xE0 | (c >> 12));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
    }
    return out;
}

std::vector<std::u16string> wrap_lines(const std::u16string& text, float width,
                                       const std::function<float(char16_t)>& advance) {
    std::vector<std::u16string> out;
    size_t p = 0;
    for (;;) {
        size_t e = text.find(u'\n', p);
        wrap_paragraph(text.substr(p, e == std::u16string::npos ? std::u16string::npos : e - p), width,
                       advance, out);
        if (e == std::u16string::npos) break;
        p = e + 1;
    }
    return out;
}

std::string fmt(const char* f, ...) {
    va_list a, b;
    va_start(a, f);
    va_copy(b, a);
    const int n = vsnprintf(nullptr, 0, f, a);
    va_end(a);
    std::string out(n > 0 ? n : 0, '\0');
    if (n > 0) vsnprintf(&out[0], out.size() + 1, f, b);
    va_end(b);
    return out;
}

} // namespace ml
