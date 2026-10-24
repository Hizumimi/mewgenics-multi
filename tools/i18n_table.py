#!/usr/bin/env python3
# i18n_table.py -- Mewgenics Multi : ecrit une table de traduction (src/i18n/lang_<code>.cpp)
# depuis des fichiers JSON, ou l'inverse.
#
#   python tools/i18n_table.py <langue> <dossier> [--extraire]
#
# Le dossier contient un ou plusieurs *.json, lus par ordre de nom (un plus tardif l'emporte) :
# chacun un objet { "cle francaise exacte": "traduction" }. Un retour a la ligne s'y ecrit
# "\\n" (les deux caracteres, comme dans le CSV de i18n_cles.py) ou "\n" ; U+200B et U+00A0
# s'ecrivent "​" et " ". Les cles qui ne sont plus dans le code sont refusees ; la
# table est reecrite dans l'ordre du code, les caracteres invisibles en \uXXXX (l'outil Edit
# et bien des editeurs les perdent : ne pas retoucher la table a la main pour eux).
# --extraire : la table actuelle -> <dossier>/00_table.json (pour la retoucher, puis la
# reecrire ; les traductions dont la cle a disparu ne sont pas extraites).
import csv, glob, json, os, re, subprocess, sys, unicodedata

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BS = chr(92)


def norm(s):
    return s.replace(BS + "n", "\n")


def keys_in_code(lang, folder):
    """(cles dans l'ordre du code, {cle: traduction actuelle}) par i18n_cles.py."""
    out = os.path.join(folder, "_cles.csv")
    subprocess.run([sys.executable, "-I", os.path.join(ROOT, "tools", "i18n_cles.py"), "-o", out],
                   check=True, capture_output=True)
    rows = list(csv.reader(open(out, encoding="utf-8-sig")))
    col = rows[0].index(lang)
    return [norm(r[0]) for r in rows[1:]], {norm(r[0]): norm(r[col]) for r in rows[1:]}


def esc(s):
    o = []
    for c in s:
        if c == BS:
            o.append(BS + BS)
        elif c == '"':
            o.append(BS + '"')
        elif c == "\n":
            o.append(BS + "n")
        elif c == "\t":
            o.append(BS + "t")
        elif c != " " and ord(c) < 0x10000 and unicodedata.category(c) in ("Cf", "Cc", "Zs", "Zl", "Zp", "Mn"):
            o.append(BS + "u%04X" % ord(c))
        else:
            o.append(c)
    return "".join(o)


def lit(s, indent):
    """Un litteral u8"..." par ligne, comme les textes longs du code."""
    parts = re.findall(r"[^\n]*\n|[^\n]+", s) or [""]
    return ("\n" + indent).join('u8"%s"' % esc(p) for p in parts)


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    if len(sys.argv) < 3:
        sys.exit("usage : i18n_table.py <langue> <dossier> [--extraire]")
    lang, folder = sys.argv[1], sys.argv[2]
    os.makedirs(folder, exist_ok=True)
    fname = os.path.join(ROOT, "src", "i18n", "lang_%s.cpp" % lang.replace("-", "_"))
    sym = "kTable_" + lang.replace("-", "_")
    order, current = keys_in_code(lang, folder)

    if sys.argv[3:] == ["--extraire"]:
        tab = {k: v for k, v in current.items() if v}
        with open(os.path.join(folder, "00_table.json"), "w", encoding="utf-8") as fh:
            json.dump(tab, fh, ensure_ascii=False, indent=1)
        print("%d traductions extraites" % len(tab))
        return 0

    tr = {}
    for f in sorted(glob.glob(os.path.join(folder, "*.json"))):
        with open(f, encoding="utf-8") as fh:
            for k, v in json.load(fh).items():
                tr[norm(k)] = norm(v)
    known = set(order)
    bad = [k for k in tr if k not in known]
    for k in bad:
        print("CLE INCONNUE : %r" % k[:100])
    missing = [k for k in order if not tr.get(k)]
    print("%d/%d traduites, %d manquantes, %d inconnues" % (len(order) - len(missing), len(order), len(missing), len(bad)))
    for k in missing[:20]:
        print("  manque : %r" % k[:90])

    with open(fname, encoding="utf-8") as fh:
        src = fh.read()
    start = src.index("const TrPair %s[] = {" % sym)
    end = src.index("{nullptr, nullptr}", start)
    body = []
    for k in order:
        v = tr.get(k)
        if not v:
            continue
        if "\n" in k or "\n" in v or len(k) + len(v) > 90:
            body.append("    {%s,\n     %s},\n" % (lit(k, "     "), lit(v, "     ")))
        else:
            body.append("    {%s, %s},\n" % (lit(k, ""), lit(v, "")))
    with open(fname, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(src[:start] + "const TrPair %s[] = {\n" % sym + "".join(body) + "    " + src[end:])
    print("ecrit :", os.path.relpath(fname, ROOT))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
