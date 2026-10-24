// test_swf.cpp -- Mewgenics Multi (launcher) : la lecture des SWF du jeu
// (launcher/ml_swf.h) sur un petit SWF construit ici, sans le jeu : une image
// DefineBitsLossless2 2x2 et une police DefineFont3 a deux glyphes (un carre,
// et un triangle a cote courbe dessine en fillStyle0 et dans le desordre),
// avec metriques, avances et crenage. Le decodage doit etre exact, et un
// fichier tronque ou faux doit etre refuse sans lire hors limites.
#include "ml_swf.h"

#include <cmath>
#include <cstdio>
#include <cstring>

using namespace ml;

namespace {

int g_fail = 0;
void check(bool ok, const char* what) {
    printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++g_fail;
}

using Bytes = std::vector<uint8_t>;

void put16(Bytes& b, uint32_t v) { b.push_back((uint8_t)v); b.push_back((uint8_t)(v >> 8)); }
void put32(Bytes& b, uint32_t v) { put16(b, v & 0xffff); put16(b, v >> 16); }

// Ecriture de bits, poids fort d'abord.
struct BitW {
    Bytes out;
    int nbit = 0;
    void ub(uint32_t v, int n) {
        for (int i = n - 1; i >= 0; --i) {
            if (nbit == 0) out.push_back(0);
            if (v >> i & 1) out.back() |= (uint8_t)(0x80 >> nbit);
            nbit = (nbit + 1) & 7;
        }
    }
    void sb(int32_t v, int n) { ub((uint32_t)v & ((1u << n) - 1), n); }
    void align() { nbit = 0; }
};

// zlib sans compression (un bloc « stored ») : ce que les decodeurs doivent lire.
Bytes zstored(const Bytes& d) {
    Bytes z{0x78, 0x01, 0x01};
    put16(z, (uint32_t)d.size());
    put16(z, (uint32_t)(~d.size() & 0xffff));
    z.insert(z.end(), d.begin(), d.end());
    uint32_t a = 1, b = 0;
    for (uint8_t c : d) { a = (a + c) % 65521; b = (b + a) % 65521; }
    const uint32_t ad = b << 16 | a;
    for (int s = 24; s >= 0; s -= 8) z.push_back((uint8_t)(ad >> s));
    return z;
}

void tag(Bytes& swf, uint16_t code, const Bytes& body) {
    put16(swf, (uint32_t)code << 6 | 0x3f);   // toujours la forme longue
    put32(swf, (uint32_t)body.size());
    swf.insert(swf.end(), body.begin(), body.end());
}

// Formes : NumFillBits = 1, NumLineBits = 0, deltas sur 14 bits.
constexpr int kNb = 14;
void style(BitW& w, bool move, int32_t x, int32_t y, int f0, int f1) {
    w.ub(0, 1);
    w.ub((f1 >= 0 ? 4u : 0u) | (f0 >= 0 ? 2u : 0u) | (move ? 1u : 0u), 5);
    if (move) { w.ub(kNb, 5); w.sb(x, kNb); w.sb(y, kNb); }
    if (f0 >= 0) w.ub((uint32_t)f0, 1);
    if (f1 >= 0) w.ub((uint32_t)f1, 1);
}
void line(BitW& w, int32_t dx, int32_t dy) {
    w.ub(1, 1); w.ub(1, 1); w.ub(kNb - 2, 4);
    if (dx && dy) { w.ub(1, 1); w.sb(dx, kNb); w.sb(dy, kNb); }
    else if (dx) { w.ub(0, 1); w.ub(0, 1); w.sb(dx, kNb); }
    else { w.ub(0, 1); w.ub(1, 1); w.sb(dy, kNb); }
}
void curve(BitW& w, int32_t cx, int32_t cy, int32_t ax, int32_t ay) {
    w.ub(1, 1); w.ub(0, 1); w.ub(kNb - 2, 4);
    w.sb(cx, kNb); w.sb(cy, kNb); w.sb(ax, kNb); w.sb(ay, kNb);
}

Bytes glyph_square() {
    BitW w;
    w.ub(1, 4); w.ub(0, 4);
    style(w, true, 0, 0, -1, 1);
    line(w, 1024, 0);      // horizontale
    line(w, 0, -1024);     // verticale
    line(w, -1024, 0);
    line(w, 0, 1024);
    style(w, false, 0, 0, 1, -1);   // fill0 = fill1 = 1 : arete interieure, ignoree
    line(w, 512, -512);
    w.ub(0, 6);
    return w.out;
}

// Triangle P(0,0) Q(2048,0) R(0,-2048), Q->R courbe (controle 2048,-2048),
// en fillStyle0 et dans le desordre : Q->R, R->P, puis P->Q.
Bytes glyph_triangle() {
    BitW w;
    w.ub(1, 4); w.ub(0, 4);
    style(w, true, 2048, 0, 1, -1);
    curve(w, 0, -2048, -2048, 0);
    line(w, 0, 2048);
    style(w, true, 0, 0, -1, -1);
    line(w, 2048, 0);
    w.ub(0, 6);
    return w.out;
}

Bytes font_tag(bool wide_off) {
    Bytes b;
    put16(b, 1);
    b.push_back((uint8_t)(0x80 | 0x04 | (wide_off ? 0x08 : 0)));   // layout, wide codes
    b.push_back(1);
    const char name[] = "Peralta";
    b.push_back(sizeof(name));
    b.insert(b.end(), name, name + sizeof(name));   // avec le zero final, comme le jeu
    put16(b, 2);

    const Bytes g0 = glyph_square(), g1 = glyph_triangle();
    const uint32_t ow = wide_off ? 4 : 2, start = 3 * ow;
    for (uint32_t o : {start, start + (uint32_t)g0.size(), start + (uint32_t)(g0.size() + g1.size())})
        wide_off ? put32(b, o) : put16(b, o);
    b.insert(b.end(), g0.begin(), g0.end());
    b.insert(b.end(), g1.begin(), g1.end());
    put16(b, 'A');
    put16(b, 0x00E9);   // e accent aigu

    put16(b, 18000);           // ascent
    put16(b, 5000);            // descent
    put16(b, (uint16_t)-100);  // leading
    put16(b, 1100);
    put16(b, 2200);
    BitW r;                    // FontBoundsTable : un RECT de 12 bits, un vide
    r.ub(12, 5);
    for (int v : {0, 1024, -1024, 0}) r.sb(v, 12);
    r.align();
    r.ub(0, 5);
    b.insert(b.end(), r.out.begin(), r.out.end());
    put16(b, 1);               // crenage : A puis e -300
    put16(b, 'A');
    put16(b, 0x00E9);
    put16(b, (uint16_t)-300);
    return b;
}

Bytes bitmap_tag(uint16_t id, const Bytes& argb, uint16_t w, uint16_t h) {
    Bytes b;
    put16(b, id);
    b.push_back(5);
    put16(b, w);
    put16(b, h);
    const Bytes z = zstored(argb);
    b.insert(b.end(), z.begin(), z.end());
    return b;
}

const Bytes kArgb = {255, 255, 0, 0,  128, 64, 32, 16,  0, 0, 0, 0,  200, 250, 10, 20};

Bytes make_swf(bool wide_off) {
    Bytes body;
    body.push_back(0);   // RECT vide (Nbits = 0)
    put16(body, 30 << 8);
    put16(body, 1);
    tag(body, 36, bitmap_tag(7, kArgb, 2, 2));
    tag(body, 75, font_tag(wide_off));
    tag(body, 0, {});
    Bytes swf = {'F', 'W', 'S', 10};
    put32(swf, (uint32_t)(8 + body.size()));
    swf.insert(swf.end(), body.begin(), body.end());
    return swf;
}

bool near(float a, float b) { return std::fabs(a - b) < 1e-6f; }
bool cmd(const PathCmd& c, PathCmd::Op op, float x, float y, float cx = 0, float cy = 0) {
    return c.op == op && near(c.x, x / 20480.f) && near(c.y, y / 20480.f) &&
           (op != PathCmd::Quad || (near(c.cx, cx / 20480.f) && near(c.cy, cy / 20480.f)));
}

void decode(const Bytes& file, const char* label) {
    printf("%s\n", label);
    Swf swf;
    std::string err;
    check(swf_open(file, swf, err), "le SWF s'ouvre");

    Image img;
    Font font;
    int tags = 0;
    check(swf_each_tag(swf, [&](const SwfTag& t) {
        ++tags;
        uint16_t id = 0;
        if (t.code == 36) check(swf_char_id(t, id) && id == 7 && swf_decode_bitmap(t, img, err), "image decodee");
        if (t.code == 75) check(swf_decode_font(t, font, err), "police decodee");
        return true;
    }), "balises parcourues");
    check(tags == 2, "deux balises avant End");

    const Bytes want = {0, 0, 255, 255,  16, 32, 64, 128,  0, 0, 0, 0,  20, 10, 200, 200};
    check(img.w == 2 && img.h == 2 && img.bgra == want, "BGRA premultiplie exact (couleur > alpha ramenee)");

    check(font.name == "Peralta" && !font.bold && !font.italic, "nom sans le zero final");
    check(font.glyphs.size() == 2, "deux glyphes");
    check(near(font.ascent, 18000 / 20480.f) && near(font.descent, 5000 / 20480.f) &&
          near(font.leading, -100 / 20480.f), "ascent, descent, leading en EM");
    const Glyph* a = font.find('A');
    const Glyph* e = font.find(0x00E9);
    check(a == &font.glyphs[0] && e == &font.glyphs[1] && !font.find('B'), "table des codes");
    check(a && near(a->advance, 1100 / 20480.f) && e && near(e->advance, 2200 / 20480.f), "avances");
    check(near(font.kern('A', 0x00E9), -300 / 20480.f) && font.kern(0x00E9, 'A') == 0, "crenage");

    const auto& s = font.glyphs[0].path;
    check(s.size() == 5 && cmd(s[0], PathCmd::Move, 0, 0) && cmd(s[1], PathCmd::Line, 1024, 0) &&
          cmd(s[2], PathCmd::Line, 1024, -1024) && cmd(s[3], PathCmd::Line, 0, -1024) &&
          cmd(s[4], PathCmd::Line, 0, 0), "carre ferme, arete interieure ignoree");

    // fillStyle0 : chaque arete retournee, puis enchainees en un seul contour.
    const auto& t = font.glyphs[1].path;
    check(t.size() == 4 && cmd(t[0], PathCmd::Move, 0, -2048) &&
          cmd(t[1], PathCmd::Quad, 2048, 0, 2048, -2048) && cmd(t[2], PathCmd::Line, 0, 0) &&
          cmd(t[3], PathCmd::Line, 0, -2048), "triangle courbe : un contour ferme, retourne");
}

void rejects() {
    printf("refus\n");
    Swf swf;
    std::string err;
    Bytes z = make_swf(false);
    z[0] = 'Z';
    check(!swf_open(z, swf, err) && err.find("LZMA") != std::string::npos, "ZWS refuse avec un message");
    check(!swf_open(Bytes{'F', 'W', 'S'}, swf, err), "fichier trop court");

    // Une balise qui annonce plus que le fichier.
    Bytes cut = make_swf(false);
    cut.resize(cut.size() - 10);
    check(swf_open(cut, swf, err), "un SWF tronque s'ouvre encore");
    check(!swf_each_tag(swf, [](const SwfTag&) { return true; }), "mais sa balise qui deborde est refusee");

    // Image : donnees plus courtes ou plus longues que w*h*4.
    Image img;
    const Bytes small = bitmap_tag(7, Bytes(kArgb.begin(), kArgb.end() - 1), 2, 2);
    check(!swf_decode_bitmap({36, small.data(), (uint32_t)small.size()}, img, err) && img.empty(), "image trop courte");
    Bytes big = kArgb;
    big.push_back(1);
    const Bytes bigt = bitmap_tag(7, big, 2, 2);
    check(!swf_decode_bitmap({36, bigt.data(), (uint32_t)bigt.size()}, img, err) && img.empty(), "image trop longue");
    const Bytes huge = bitmap_tag(7, kArgb, 65535, 65535);
    check(!swf_decode_bitmap({36, huge.data(), (uint32_t)huge.size()}, img, err), "taille folle refusee");

    // Police coupee a chaque longueur : jamais de plantage, jamais de succes
    // avant la fin du crenage.
    const Bytes f = font_tag(true);
    int ok_before_end = 0;
    for (size_t n = 0; n < f.size(); ++n) {
        Bytes part(f.begin(), f.begin() + (long)n);
        Font font;
        if (swf_decode_font({75, part.data(), (uint32_t)n}, font, err)) ++ok_before_end;
    }
    check(ok_before_end == 0, "police tronquee a chaque octet : refusee");
}

// SymbolClass : les noms des polices de international_fonts.swf (« TikaFontJP »).
void symbols() {
    printf("SymbolClass\n");
    Bytes b;
    put16(b, 2);
    put16(b, 12);
    for (char c : std::string("TikaFontJP")) b.push_back((uint8_t)c);
    b.push_back(0);
    put16(b, 15);
    for (char c : std::string("KoreanAll")) b.push_back((uint8_t)c);
    b.push_back(0);
    std::vector<std::pair<uint16_t, std::string>> out;
    check(swf_symbol_classes({76, b.data(), (uint32_t)b.size()}, out) && out.size() == 2 && out[0].first == 12 &&
          out[0].second == "TikaFontJP" && out[1].first == 15 && out[1].second == "KoreanAll", "deux noms lus");
    check(!swf_symbol_classes({75, b.data(), (uint32_t)b.size()}, out), "une autre balise : refusee");
    int ok_before_end = 0;
    for (size_t n = 0; n + 1 < b.size(); ++n)
        if (swf_symbol_classes({76, b.data(), (uint32_t)n}, out)) ++ok_before_end;
    check(ok_before_end == 0, "tronquee a chaque octet : refusee");
}

} // namespace

int main() {
    symbols();
    decode(make_swf(false), "FWS, offsets sur 16 bits");
    decode(make_swf(true), "FWS, offsets sur 32 bits");

    Bytes fws = make_swf(true);
    Bytes cws = {'C', 'W', 'S', 10, fws[4], fws[5], fws[6], fws[7]};
    const Bytes z = zstored(Bytes(fws.begin() + 8, fws.end()));
    cws.insert(cws.end(), z.begin(), z.end());
    decode(cws, "CWS (zlib)");

    rejects();
    if (g_fail) { printf("%d echec(s)\n", g_fail); return 1; }
    printf("test_swf : tout passe\n");
    return 0;
}
