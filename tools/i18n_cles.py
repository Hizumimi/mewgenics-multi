#!/usr/bin/env python3
# i18n_cles.py -- Mewgenics Multi : toutes les cles a traduire, pour les traducteurs.
#
#   python tools/i18n_cles.py [-o i18n_cles.csv] [--jeu "...\Mewgenics\resources.gpak"]
#
# Lit les sources (src/, launcher/) comme tests/test_i18n.cpp : chaque litteral passe
# a tr( ou trk( est une cle (le texte francais exact). Ecrit un CSV (UTF-8, ouvrable
# dans un tableur) : la cle (\n ecrits \n), ou elle est (fichier:ligne), son contexte
# (jeu ou lanceur ; fenetre du jeu, bouton ou texte ; la fonction), la largeur de ses
# lignes en colonnes et le maximum permis a la traduction (docs/TRADUCTION.md), puis
# la traduction deja faite dans chaque langue (src/i18n/lang_*.cpp), vide sinon.
#
# Avec le jeu installe (--jeu, ou l'installation Steam par defaut), une derniere
# colonne donne, pour chaque nom de bouton du jeu cite entre « » dans la cle, sa
# cle de data/text/combined.csv et son texte officiel dans chaque langue -- lu chez
# celui qui lance l'outil, jamais copie dans le depot.
import argparse
import csv
import io
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LANGS = ["en", "sp", "de", "it", "pt-br", "ru", "ko", "ja", "zh-cn"]
STEAM_GPAK = r"C:\Program Files (x86)\Steam\steamapps\common\Mewgenics\resources.gpak"


def literal(s, i):
    """Sur le guillemet ouvrant : (texte decode, index apres le guillemet fermant)."""
    out = bytearray()
    i += 1
    while i < len(s) and s[i] != '"':
        c = s[i]
        if c == "\n":
            break
        if c != "\\":
            out += c.encode("utf-8")
            i += 1
            continue
        e = s[i + 1]
        i += 2
        if e in "ntr":
            out += {"n": b"\n", "t": b"\t", "r": b"\r"}[e]
        elif e in "01234567":
            v = int(e)
            for _ in range(2):
                if i < len(s) and s[i] in "01234567":
                    v = v * 8 + int(s[i])
                    i += 1
            out.append(v & 0xFF)
        elif e == "x":
            m = re.match(r"[0-9a-fA-F]+", s[i:])
            out.append(int(m.group(0), 16) & 0xFF)
            i += len(m.group(0))
        elif e in "uU":
            n = 4 if e == "u" else 8
            out += chr(int(s[i:i + n], 16)).encode("utf-8")
            i += n
        else:
            out += e.encode("utf-8")
    return out.decode("utf-8", "replace"), i + 1


def literal_start(s, i):
    """Index du guillemet si un litteral (u8"..., "..., L"...) commence ici, sinon -1."""
    m = re.match(r'(u8|u|U|L)?"', s[i:i + 3])
    return i + m.end() - 1 if m else -1


def skip_space(s, i):
    while i < len(s):
        if s[i] in " \t\r\n":
            i += 1
        elif s.startswith("//", i):
            e = s.find("\n", i)
            i = len(s) if e < 0 else e
        elif s.startswith("/*", i):
            e = s.find("*/", i + 2)
            i = len(s) if e < 0 else e + 2
        else:
            break
    return i


def scan(s):
    """Les cles d'une source : [(texte, ligne, index)] ; et les appels sans litteral."""
    keys, notes, i, n = [], [], 0, len(s)
    while i < n:
        c = s[i]
        if s.startswith("//", i) or s.startswith("/*", i):
            i = skip_space(s, i)
        elif c == '"':
            i = literal(s, i)[1]
        elif c == "'":
            i += 1
            while i < n and s[i] not in "'\n":
                i += 2 if s[i] == "\\" else 1
            i += 1
        elif (c.isalnum() or c == "_") and (i == 0 or not (s[i - 1].isalnum() or s[i - 1] == "_")):
            m = re.match(r"\w+", s[i:])
            word, start = m.group(0), i
            i += len(word)
            if word in ("R", "u8R", "LR", "uR", "UR") and i < n and s[i] == '"':
                op = s.find("(", i)
                delim = ")" + s[i + 1:op] + '"'
                e = s.find(delim, op)
                i = n if e < 0 else e + len(delim)
                continue
            before = s[start - 1] if start else " "
            if word in ("tr", "trk") and before not in ".>":
                j = skip_space(s, i)
                if j < n and s[j] == "(":
                    j = skip_space(s, j + 1)
                    text, count = "", 0
                    while True:
                        q = literal_start(s, j)
                        if q < 0:
                            break
                        t, j = literal(s, q)
                        text += t
                        count += 1
                        j = skip_space(s, j)
                    line = s.count("\n", 0, start) + 1
                    if count and j < n and s[j] == ")":
                        keys.append((text, line, start))
                    else:
                        notes.append((line, word))
                    i = j
        else:
            i += 1
    return keys, notes


def strip_comments(s):
    out, i, n = [], 0, len(s)
    while i < n:
        if s.startswith("//", i) or s.startswith("/*", i):
            j = skip_space(s, i)
            out.append(" ")
            i = j
        elif s[i] == '"':
            j = literal(s, i)[1]
            out.append(s[i:j])
            i = j
        else:
            out.append(s[i])
            i += 1
    return "".join(out)


LIT = r'(?:u8)?"(?:[^"\\\n]|\\.)*"'
PAIR = re.compile(r"\{\s*((?:%s\s*)+),\s*((?:%s\s*)+)\}" % (LIT, LIT))


def join_literals(group):
    text, i = "", 0
    while True:
        q = literal_start(group, skip_space(group, i))
        if q < 0:
            return text
        t, i = literal(group, q)
        text += t


def table(lang):
    path = os.path.join(ROOT, "src", "i18n", "lang_%s.cpp" % lang.replace("-", "_"))
    with open(path, encoding="utf-8") as f:
        src = strip_comments(f.read())
    return {join_literals(m.group(1)): join_literals(m.group(2)) for m in PAIR.finditer(src)}


WIDE = [(0x1100, 0x115F), (0x2E80, 0xA4CF), (0xAC00, 0xD7A3), (0xF900, 0xFAFF), (0xFE30, 0xFE4F),
        (0xFF00, 0xFF60), (0xFFE0, 0xFFE6)]


def cols(line):
    line = re.sub(r"\[c:[^\]]*\]|\[/c\]", "", line)
    n = 0
    for ch in line:
        cp = ord(ch)
        if cp == 0x200B or 0x300 <= cp <= 0x36F:
            continue
        n += 2 if any(a <= cp <= b for a, b in WIDE) else 1
    return n


def allowed(fr_cols):
    if fr_cols <= 16:
        return max((fr_cols * 16 + 9) // 10, fr_cols + 6)
    return max(36, (fr_cols * 13 + 9) // 10)


def function_at(src, index):
    """La fonction qui contient `index` : la derniere definition commencant en colonne 0."""
    head = src[:index]
    for m in reversed(list(re.finditer(r"^[A-Za-z_][\w:<>,\*& ]*?\b(\w+)\s*\([^;{]*\)\s*(?:const\s*)?\{", head, re.M))):
        return m.group(1)
    return ""


def context(path, src, index, text):
    zone = "lanceur" if path.startswith("launcher/") else "jeu"
    line = src[src.rfind("\n", 0, index) + 1:src.find("\n", index)]
    fn = function_at(src, index)
    short = "\n" not in text and cols(text) <= 24
    if zone == "lanceur":
        kind = "bouton" if re.search(r"\b(button|big_button|link)\(", line) else "texte"
    elif short and re.search(r"open_popup\(|box_open\(|set_button_label\(|label|yes|no\b", line):
        kind = "bouton"
    elif "\n" in text or fn.endswith("_text") or "open_popup(" in line or "set_text(" in line:
        kind = "fenetre du jeu"
    else:
        kind = "texte"
    return "%s, %s%s" % (zone, kind, ", " + fn + "()" if fn else "")


def game_terms(gpak_path):
    """{texte francais normalise: (cle, {langue: texte})} depuis combined.csv du jeu."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from gpak import GPak
    g = GPak(gpak_path)
    data = g.read("data/text/combined.csv").decode("utf-8-sig")
    rows = list(csv.reader(io.StringIO(data, newline="")))
    head = rows[0]
    out = {}
    for r in rows[1:]:
        if not r or not r[0] or r[0].startswith("//") or len(r) < len(head):
            continue
        fr = norm(r[head.index("fr")])
        if fr and fr not in out:
            out[fr] = (r[0], {l: r[head.index(l)].replace("\u200b", "") for l in LANGS})
    return out


def norm(s):
    return re.sub(r"\s+", " ", s.replace("&nbsp;", " ").replace("\u00a0", " ").replace("\u202f", " ")).strip().lower()


def main():
    ap = argparse.ArgumentParser(description="Les cles a traduire de Mewgenics Multi (CSV).")
    ap.add_argument("-o", "--sortie", default="i18n_cles.csv")
    ap.add_argument("--jeu", default=STEAM_GPAK, help="resources.gpak, pour les noms officiels des boutons du jeu")
    a = ap.parse_args()

    keys = {}   # texte -> [(chemin:ligne, contexte)]
    notes = []
    for top in ("src", "launcher"):
        for d, _, files in os.walk(os.path.join(ROOT, top)):
            if os.path.basename(d) == "i18n":
                continue
            for f in sorted(files):
                if not f.endswith((".cpp", ".h")):
                    continue
                path = os.path.relpath(os.path.join(d, f), ROOT).replace("\\", "/")
                with open(os.path.join(d, f), encoding="utf-8", errors="replace") as fh:
                    src = fh.read()
                found, missing = scan(src)
                for text, line, index in found:
                    keys.setdefault(text, []).append(("%s:%d" % (path, line), context(path, src, index, text)))
                notes += ["%s:%d : %s( sans litteral" % (path, l, w) for l, w in missing]

    tables = {l: table(l) for l in LANGS}
    terms = None
    if a.jeu and os.path.exists(a.jeu):
        terms = game_terms(a.jeu)

    with open(a.sortie, "w", encoding="utf-8-sig", newline="") as f:
        w = csv.writer(f)
        w.writerow(["cle", "ou", "contexte", "colonnes (max permis)"] + LANGS + (["termes du jeu"] if terms is not None else []))
        for text in sorted(keys, key=lambda t: keys[t][0][0]):
            where = keys[text]
            widths = " / ".join("%d (%d)" % (cols(l), allowed(cols(l))) for l in text.split("\n"))
            row = [text.replace("\n", "\\n"), " ; ".join(p for p, _ in where), where[0][1], widths]
            row += [tables[l].get(text, "").replace("\n", "\\n") for l in LANGS]
            if terms is not None:
                found = []
                for q in re.findall(r"«\s*([^»]+?)\s*»", text):
                    hit = terms.get(norm(q))
                    if hit:
                        found.append("« %s » = %s : %s" % (q, hit[0], " | ".join("%s %s" % (l, hit[1][l]) for l in LANGS)))
                row.append("\n".join(found))
            w.writerow(row)

    done = {l: sum(1 for t in keys if tables[l].get(t)) for l in LANGS}
    print("%d cles -> %s" % (len(keys), a.sortie))
    print("traduites : " + ", ".join("%s %d" % (l, done[l]) for l in LANGS))
    orphans = {l: [t for t in tables[l] if t not in keys] for l in LANGS}
    for l in LANGS:
        for t in orphans[l]:
            print("orpheline (%s) : %s" % (l, t.replace("\n", "\\n")[:70]))
    for n in notes:
        print("note : " + n)
    if terms is None:
        print("(jeu introuvable : pas de colonne « termes du jeu » ; --jeu chemin\\resources.gpak)")


if __name__ == "__main__":
    main()
