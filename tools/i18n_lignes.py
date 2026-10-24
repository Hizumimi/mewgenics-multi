#!/usr/bin/env python3
# i18n_lignes.py -- Mewgenics Multi : combien de lignes chaque texte du mod prend a l'ecran
# du jeu, dans chaque langue, compare au francais.
#
#   python tools/i18n_lignes.py <cles.csv de i18n_cles.py> [langue ...] [--jeu resources.gpak]
#
# Simule la mise en page du jeu (SwfSetText, 0x140990480 ; docs/TRADUCTION.md section 3) :
# la boite de confirmation (champ « prompt », 536 px, TikaFont 24 px) ou un bouton (223 px,
# 28 px), coupee a la derniere espace ou U+200B quand la ligne deborde, avec les avances des
# glyphes des polices du jeu (swfs/fonts.swf, swfs/international_fonts.swf, lues dans le
# resources.gpak du joueur) dans l'ordre de data/font_remap.gon. Liste les textes du jeu qui
# prennent plus de lignes que le francais (et les boutons de plus de 3 lignes). Les %s et %d
# ne sont pas remplaces : une estimation, pas une mesure en jeu.
import csv, os, re, struct, sys, zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gpak import GPak

BS = chr(92)
LANGS = ["en", "sp", "de", "it", "pt-br", "ru", "ko", "ja", "zh-cn"]
STEAM_GPAK = r"C:\Program Files (x86)\Steam\steamapps\common\Mewgenics\resources.gpak"
# data/font_remap.gon, glyph_lookup_order de TikaFont
DEFAULT = ["TikaFont", "TikaFontCyr", "TikaFontCN", "TikaFontJP", "KoreanAll"]
ORDER = {"ru": ["TikaFontCyr"] + DEFAULT, "ko": ["TikaFont", "KoreanAll"] + DEFAULT,
         "ja": ["TikaFont", "TikaFontJP"] + DEFAULT, "zh-cn": ["TikaFont", "TikaFontCN"] + DEFAULT}


def swf_fonts(raw, wanted):
    """{nom de police: {code: avance en em}} des DefineFont3 (75) nommees par SymbolClass (76)."""
    data = raw[:8] + zlib.decompress(raw[8:]) if raw[:3] == b"CWS" else raw
    p = 8 + (5 + 4 * (data[8] >> 3) + 7) // 8 + 4
    fonts, syms = {}, {}
    while p + 2 <= len(data):
        hdr = struct.unpack_from("<H", data, p)[0]
        p += 2
        code, ln = hdr >> 6, hdr & 0x3F
        if ln == 0x3F:
            ln = struct.unpack_from("<I", data, p)[0]
            p += 4
        if code == 0:
            break
        if code == 75:
            fonts[struct.unpack_from("<H", data, p)[0]] = p
        elif code == 76:
            q = p + 2
            for _ in range(struct.unpack_from("<H", data, p)[0]):
                cid = struct.unpack_from("<H", data, q)[0]
                e = data.index(b"\0", q + 2)
                syms[data[q + 2:e].decode()] = cid
                q = e + 1
        p += ln
    out = {}
    for name in wanted:
        q = fonts[syms[name]]
        flags = data[q + 2]
        q += 5 + data[q + 4]
        ng = struct.unpack_from("<H", data, q)[0]
        q += 2
        wide = 4 if flags & 0x08 else 2
        offs = [struct.unpack_from("<I" if wide == 4 else "<H", data, q + i * wide)[0] for i in range(ng + 1)]
        q += offs[ng]
        codes = []
        for _ in range(ng):
            if flags & 0x04:
                codes.append(struct.unpack_from("<H", data, q)[0])
                q += 2
            else:
                codes.append(data[q])
                q += 1
        adv = {}
        if flags & 0x80:   # FontFlagsHasLayout : ascent, descent, leading, puis les avances
            q += 6
            for c in codes:
                adv.setdefault(c, struct.unpack_from("<h", data, q)[0] / 20480.0)
                q += 2
        out[name] = adv
    return out


def lines(text, lang, adv_of, width, size):
    text = re.sub(r"\[/?[a-z]+(:[^\]]*)?\]", "", text)   # balises [c:red] : sans largeur
    n = 0
    for para in text.split("\n"):
        cur, w = [], 0.0
        n += 1
        for ch in para:
            a = adv_of(ch, lang) * size
            if w + a > width and cur:
                brk = max((i for i, c in enumerate(cur) if c in " \u200b"), default=-1)
                cur = cur[brk + 1:] if brk >= 0 else []
                n += 1
                w = sum(adv_of(c, lang) * size for c in cur)
            cur.append(ch)
            w += a
    return n


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    args = sys.argv[1:]
    gpak = STEAM_GPAK
    if "--jeu" in args:
        i = args.index("--jeu")
        gpak = args[i + 1]
        del args[i:i + 2]
    if not args:
        sys.exit("usage : i18n_lignes.py <cles.csv> [langue ...] [--jeu resources.gpak]")
    g = GPak(gpak)
    adv = swf_fonts(g.read("swfs/fonts.swf"), ["TikaFont"])
    adv.update(swf_fonts(g.read("swfs/international_fonts.swf"), ["TikaFontCyr", "TikaFontCN", "TikaFontJP", "KoreanAll"]))

    def adv_of(ch, lang):
        if ch == "\u200b":
            return 0.0
        for f in ORDER.get(lang, DEFAULT):
            if ord(ch) in adv[f]:
                return adv[f][ord(ch)]
        return 1.0 if ord(ch) >= 0x2E80 else 0.6   # NotoSans, la police de secours du jeu

    rows = list(csv.reader(open(args[0], encoding="utf-8-sig")))
    head = rows[0]
    for lang in args[1:] or LANGS:
        worse = []
        for r in rows[1:]:
            if not r[2].startswith("jeu") or not r[head.index(lang)]:
                continue
            button = ", bouton" in r[2]
            width, size = (219, 28) if button else (532, 24)   # moins les marges du champ (2 px)
            fr, tr = norm(r[0]), norm(r[head.index(lang)])
            a, b = lines(fr, "fr", adv_of, width, size), lines(tr, lang, adv_of, width, size)
            if b > a or (button and b > 3):
                worse.append((b - a, a, b, r[1].split(" ; ")[0], tr.replace("\n", BS + "n")))
        worse.sort(key=lambda x: (-x[0], -x[2]))
        print("== %s : %d texte(s) du jeu avec plus de lignes qu'en francais" % (lang, len(worse)))
        for dl, a, b, where, tr in worse:
            print("  +%d (%d -> %d lignes) %s : %s" % (dl, a, b, where, tr[:150]))


def norm(s):
    return s.replace(BS + "n", "\n")


if __name__ == "__main__":
    main()
