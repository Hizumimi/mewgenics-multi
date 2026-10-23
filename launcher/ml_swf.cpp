// ml_swf.cpp -- voir ml_swf.h. Les references de format sont la specification
// SWF 19 (Adobe) : chapitres « Bitmaps », « Shapes » et « Fonts and Text ».
#include "ml_swf.h"

#include <climits>
#include <cstring>
#include <unordered_map>

// Le seul fichier du launcher qui compile stb_image, et seulement son zlib.
// STB_IMAGE_STATIC : les symboles restent dans ce .cpp, donc un autre fichier
// du launcher peut compiler sa propre copie (PNG...) sans conflit au lien.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_ZLIB
#define STBI_SUPPORT_ZLIB   // sans lui, ONLY_ZLIB retire aussi le zlib (pas de PNG)
#define STBI_NO_STDIO
#include "stb_image.h"

namespace ml {
namespace {

constexpr uint32_t kMaxSwf    = 512u * 1024u * 1024u;   // ui.swf decompresse : 45 Mo
constexpr uint64_t kMaxPixels = 64u * 1024u * 1024u;    // 2000x2000 = 4 M
constexpr float    kEm        = 1.f / 20480.f;          // DefineFont3 : EM de 1024 x 20

// Lecture d'octets bornee : au-dela de la fin, rend 0 et leve `bad`.
struct Rd {
    const uint8_t* p;
    size_t n, i = 0;
    bool bad = false;
    bool need(size_t k) {
        if (bad || k > n - i) { bad = true; return false; }
        return true;
    }
    uint8_t u8() { return need(1) ? p[i++] : 0; }
    uint16_t u16() {
        if (!need(2)) return 0;
        uint16_t v = (uint16_t)(p[i] | p[i + 1] << 8);
        i += 2;
        return v;
    }
    uint32_t u32() {
        if (!need(4)) return 0;
        uint32_t v = (uint32_t)p[i] | (uint32_t)p[i + 1] << 8 | (uint32_t)p[i + 2] << 16 | (uint32_t)p[i + 3] << 24;
        i += 4;
        return v;
    }
};

// Lecture de bits bornee, poids fort d'abord (UB/SB de la specification).
struct Bits {
    const uint8_t* p;
    size_t n, bit = 0;
    bool bad = false;
    uint32_t ub(int k) {
        if (bad || k < 0 || k > 32 || bit + (size_t)k > n * 8) { bad = true; return 0; }
        uint32_t v = 0;
        for (int j = 0; j < k; ++j, ++bit) v = v << 1 | ((p[bit >> 3] >> (7 - (bit & 7))) & 1);
        return v;
    }
    int32_t sb(int k) {
        uint32_t v = ub(k);
        if (k > 0 && k < 32 && (v >> (k - 1) & 1)) v |= ~0u << k;
        return (int32_t)v;
    }
    void align() { bit = (bit + 7) & ~(size_t)7; }
};

// Une arete d'un glyphe, en unites du SWF.
struct Edge {
    int32_t x0, y0, cx, cy, x1, y1;
    bool quad;
};

uint64_t key(int32_t x, int32_t y) { return (uint64_t)(uint32_t)x << 32 | (uint32_t)y; }

// Les formes SWF portent le remplissage sur chaque arete (fillStyle0 a gauche,
// fillStyle1 a droite) et ne les rangent pas en contours : un meme contour peut
// etre coupe en morceaux dans n'importe quel ordre. On garde chaque arete
// d'un style dans le sens fillStyle1 (retournee si c'est fillStyle0), puis on
// les enchaine par extremites communes : des contours fermes, orientes pareil.
void chain(const std::vector<Edge>& edges, std::vector<PathCmd>& out) {
    std::unordered_multimap<uint64_t, size_t> from;
    for (size_t i = 0; i < edges.size(); ++i) from.emplace(key(edges[i].x0, edges[i].y0), i);
    std::vector<bool> used(edges.size(), false);

    auto pt = [](int32_t v) { return (float)v * kEm; };
    for (size_t first = 0; first < edges.size(); ++first) {
        if (used[first]) continue;
        const int32_t sx = edges[first].x0, sy = edges[first].y0;
        out.push_back({PathCmd::Move, 0, 0, pt(sx), pt(sy)});
        size_t cur = first;
        for (;;) {
            used[cur] = true;
            const Edge& e = edges[cur];
            out.push_back({e.quad ? PathCmd::Quad : PathCmd::Line, pt(e.cx), pt(e.cy), pt(e.x1), pt(e.y1)});
            if (e.x1 == sx && e.y1 == sy) break;
            size_t next = SIZE_MAX;
            auto range = from.equal_range(key(e.x1, e.y1));
            for (auto it = range.first; it != range.second; ++it)
                if (!used[it->second]) { next = it->second; break; }
            if (next == SIZE_MAX) {   // contour ouvert (donnee abimee) : on le ferme
                out.push_back({PathCmd::Line, 0, 0, pt(sx), pt(sy)});
                break;
            }
            cur = next;
        }
    }
}

// SHAPE d'un glyphe : NumFillBits, NumLineBits, puis les enregistrements.
bool decode_shape(const uint8_t* p, size_t n, std::vector<PathCmd>& out) {
    Bits b{p, n};
    const int fill_bits = (int)b.ub(4), line_bits = (int)b.ub(4);
    int32_t x = 0, y = 0;
    uint32_t f0 = 0, f1 = 0;
    std::map<uint32_t, std::vector<Edge>> by_style;

    for (;;) {
        if (b.bad) return false;
        if (b.ub(1) == 0) {   // StyleChangeRecord, ou EndShapeRecord si tout est nul
            const uint32_t flags = b.ub(5);
            if (flags == 0) break;
            if (flags & 0x10) return false;   // StateNewStyles : pas dans un glyphe
            if (flags & 0x01) {
                const int mb = (int)b.ub(5);
                x = b.sb(mb);
                y = b.sb(mb);
            }
            if (flags & 0x02) f0 = b.ub(fill_bits);
            if (flags & 0x04) f1 = b.ub(fill_bits);
            if (flags & 0x08) b.ub(line_bits);
            continue;
        }
        Edge e{x, y, 0, 0, 0, 0, false};
        const bool straight = b.ub(1) != 0;
        const int nb = (int)b.ub(4) + 2;
        if (straight) {
            int32_t dx = 0, dy = 0;
            if (b.ub(1)) { dx = b.sb(nb); dy = b.sb(nb); }
            else if (b.ub(1)) dy = b.sb(nb);
            else dx = b.sb(nb);
            e.x1 = x + dx;
            e.y1 = y + dy;
        } else {
            e.cx = x + b.sb(nb);
            e.cy = y + b.sb(nb);
            e.x1 = e.cx + b.sb(nb);
            e.y1 = e.cy + b.sb(nb);
            e.quad = true;
        }
        if (b.bad) return false;
        x = e.x1;
        y = e.y1;
        if (f0 == f1) continue;   // arete interieure (ou sans remplissage)
        if (f1) by_style[f1].push_back(e);
        if (f0) by_style[f0].push_back({e.x1, e.y1, e.cx, e.cy, e.x0, e.y0, e.quad});
    }
    for (auto& s : by_style) chain(s.second, out);
    return true;
}

} // namespace

bool swf_open(std::vector<uint8_t> file, Swf& out, std::string& err) {
    out = Swf{};
    if (file.size() < 8 || file.size() > kMaxSwf) { err = u8"SWF de taille invraisemblable"; return false; }
    const uint32_t full = (uint32_t)file[4] | (uint32_t)file[5] << 8 | (uint32_t)file[6] << 16 | (uint32_t)file[7] << 24;

    if (!memcmp(file.data(), "FWS", 3)) {
        out.data = std::move(file);
    } else if (!memcmp(file.data(), "CWS", 3)) {
        if (full < 8 || full > kMaxSwf) { err = u8"SWF compressé de taille invraisemblable"; return false; }
        out.data.resize(full);
        memcpy(out.data.data(), file.data(), 8);
        // Tampon de taille fixe : stbi echoue plutot que de grandir au-dela.
        const int got = stbi_zlib_decode_buffer((char*)out.data.data() + 8, (int)(full - 8),
                                                (const char*)file.data() + 8, (int)(file.size() - 8));
        if (got < 0) { err = u8"SWF compressé illisible (zlib)"; out.data.clear(); return false; }
        out.data.resize(8 + (size_t)got);
    } else if (!memcmp(file.data(), "ZWS", 3)) {
        err = u8"SWF compressé en LZMA (ZWS) : non géré";
        return false;
    } else {
        err = u8"ce n'est pas un SWF";
        return false;
    }

    // En-tete apres les 8 octets : RECT (taille variable), cadence u16, images u16.
    if (out.data.size() < 9) { err = u8"SWF tronqué"; out.data.clear(); return false; }
    const size_t nbits = out.data[8] >> 3;
    out.first_tag = 8 + (5 + 4 * nbits + 7) / 8 + 4;
    if (out.first_tag > out.data.size()) { err = u8"SWF tronqué"; out.data.clear(); return false; }
    return true;
}

bool swf_each_tag(const Swf& swf, const std::function<bool(const SwfTag&)>& fn) {
    const uint8_t* d = swf.data.data();
    const size_t n = swf.data.size();
    size_t p = swf.first_tag;
    while (p + 2 <= n) {
        const uint16_t hdr = (uint16_t)(d[p] | d[p + 1] << 8);
        p += 2;
        uint32_t len = hdr & 0x3f;
        if (len == 0x3f) {
            if (p + 4 > n) return false;
            len = (uint32_t)d[p] | (uint32_t)d[p + 1] << 8 | (uint32_t)d[p + 2] << 16 | (uint32_t)d[p + 3] << 24;
            p += 4;
        }
        if (len > n - p) return false;
        const SwfTag tag{(uint16_t)(hdr >> 6), d + p, len};
        if (tag.code == 0 || !fn(tag)) return true;
        p += len;
    }
    return true;
}

bool swf_char_id(const SwfTag& tag, uint16_t& id) {
    if (tag.len < 2) return false;
    id = (uint16_t)(tag.p[0] | tag.p[1] << 8);
    return true;
}

bool swf_decode_bitmap(const SwfTag& tag, Image& out, std::string& err) {
    out = Image{};
    Rd r{tag.p, tag.len};
    r.u16();
    const uint8_t format = r.u8();
    const uint32_t w = r.u16(), h = r.u16();
    if (r.bad || tag.code != 36) { err = u8"balise d'image invalide"; return false; }
    if (format != 5) { err = u8"format d'image non géré (" + std::to_string(format) + ")"; return false; }
    if (!w || !h || (uint64_t)w * h > kMaxPixels) { err = u8"image de taille invraisemblable"; return false; }

    const size_t bytes = (size_t)w * h * 4;
    out.bgra.resize(bytes);
    const int got = stbi_zlib_decode_buffer((char*)out.bgra.data(), (int)bytes, (const char*)tag.p + r.i,
                                            (int)(tag.len - r.i));
    if (got != (int)bytes) { err = u8"image illisible (zlib)"; out.bgra.clear(); return false; }

    // ARGB -> BGRA, les deux premultiplies. Une couleur plus forte que son alpha
    // n'est pas du premultiplie valide pour Direct2D : on la ramene a l'alpha.
    uint8_t* px = out.bgra.data();
    for (size_t i = 0; i < bytes; i += 4) {
        const uint8_t a = px[i], rr = px[i + 1], g = px[i + 2], bb = px[i + 3];
        px[i]     = bb < a ? bb : a;
        px[i + 1] = g < a ? g : a;
        px[i + 2] = rr < a ? rr : a;
        px[i + 3] = a;
    }
    out.w = w;
    out.h = h;
    return true;
}

bool swf_font_name(const SwfTag& tag, std::string& name) {
    if (tag.code != 75) return false;
    Rd r{tag.p, tag.len};
    r.u16();
    r.u8();
    r.u8();
    const uint8_t nl = r.u8();
    if (!r.need(nl)) return false;
    name.assign((const char*)tag.p + r.i, nl);
    while (!name.empty() && name.back() == 0) name.pop_back();
    return true;
}

bool swf_decode_font(const SwfTag& tag, Font& out, std::string& err) {
    out = Font{};
    err = u8"police tronquée ou mal formée";
    if (!swf_font_name(tag, out.name)) return false;

    Rd r{tag.p, tag.len};
    r.u16();
    const uint8_t flags = r.u8();
    r.u8();
    const uint8_t nl = r.u8();   // nom, deja lu
    if (!r.need(nl)) return false;
    r.i += nl;
    const uint16_t ng = r.u16();
    if (r.bad) return false;
    const bool layout = flags & 0x80, wide_off = flags & 0x08, wide_codes = flags & 0x04;
    out.italic = flags & 0x02;
    out.bold = flags & 0x01;
    if (ng == 0) return true;

    // OffsetTable puis CodeTableOffset, relatifs au debut de l'OffsetTable.
    const size_t table = r.i;
    std::vector<uint32_t> off(ng + 1u);
    for (auto& o : off) o = wide_off ? r.u32() : r.u16();
    if (r.bad) return false;
    for (uint32_t i = 0; i < ng; ++i)
        if (off[i] > off[i + 1]) return false;
    if (off[ng] > tag.len - table) return false;

    out.glyphs.resize(ng);
    for (uint32_t i = 0; i < ng; ++i)
        if (!decode_shape(tag.p + table + off[i], off[i + 1] - off[i], out.glyphs[i].path)) {
            err = u8"glyphe " + std::to_string(i) + u8" mal formé";
            return false;
        }

    r.i = table + off[ng];
    for (uint16_t i = 0; i < ng; ++i) {
        const char16_t c = (char16_t)(wide_codes ? r.u16() : r.u8());
        out.glyph_of.emplace(c, i);   // un code en double garde son premier glyphe
    }
    if (r.bad) return false;
    if (!layout) { err.clear(); return true; }

    // Metriques et avances : memes unites que les contours (EM de 20480).
    out.ascent = r.u16() * kEm;
    out.descent = r.u16() * kEm;
    out.leading = (int16_t)r.u16() * kEm;
    for (auto& g : out.glyphs) g.advance = (int16_t)r.u16() * kEm;
    if (r.bad) return false;

    // FontBoundsTable : un RECT aligne par glyphe. Flash ne s'en sert pas, nous
    // non plus ; il faut seulement le traverser pour atteindre le crenage.
    Bits b{tag.p + r.i, r.n - r.i};
    for (uint32_t i = 0; i < ng; ++i) {
        const int nb = (int)b.ub(5);
        for (int k = 0; k < 4; ++k) b.sb(nb);
        b.align();
    }
    if (b.bad) return false;
    r.i += b.bit / 8;

    const uint16_t nk = r.u16();
    if (r.bad) return false;
    for (uint32_t i = 0; i < nk; ++i) {
        const uint32_t a = wide_codes ? r.u16() : r.u8();
        const uint32_t c = wide_codes ? r.u16() : r.u8();
        const float adj = (int16_t)r.u16() * kEm;
        if (r.bad) return false;
        out.kerning[a << 16 | c] = adj;
    }
    err.clear();
    return true;
}

} // namespace ml
