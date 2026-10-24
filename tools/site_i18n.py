# Écrit site/i18n/fr.js à partir des textes français de site/index.html, et vérifie les autres
# tables (mêmes clés, mêmes balises). python tools/site_i18n.py
import json, os, re, sys, glob
from html.parser import HTMLParser

RACINE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "site")
VIDES = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "source", "track", "wbr"}

class P(HTMLParser):
    def __init__(self, src):
        super().__init__(convert_charrefs=True)
        self.src, self.pile, self.cles = src, [], {}
        self.lignes = [0]
        for l in src.splitlines(keepends=True):
            self.lignes.append(self.lignes[-1] + len(l))

    def pos(self):
        l, c = self.getpos()
        return self.lignes[l - 1] + c

    def ajouter(self, cle, val):
        # pas \s : en Python il attrape aussi l'espace insécable (U+00A0) de la typographie française
        val = re.sub(r"[ \t\r\n]+", " ", val).strip(" \t\r\n")
        if cle in self.cles and self.cles[cle] != val:
            sys.exit(f"clé {cle} : deux textes différents\n  {self.cles[cle]}\n  {val}")
        self.cles[cle] = val

    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        debut = self.pos() + len(self.get_starttag_text())
        for paire in filter(None, (a.get("data-i18n-attr") or "").split(",")):
            nom, cle = (s.strip() for s in paire.split(":"))
            self.ajouter(cle, a[nom])
        if tag in VIDES:
            return
        self.pile.append((tag, debut, a.get("data-i18n"), a.get("data-i18n-html"), []))

    def handle_data(self, d):
        for e in self.pile:
            e[4].append(d)

    def handle_endtag(self, tag):
        while self.pile:
            t, debut, cle, cleh, textes = self.pile.pop()
            if cle:
                self.ajouter(cle, "".join(textes))
            if cleh:
                self.ajouter(cleh, self.src[debut:self.pos()])
            if t == tag:
                break

src = open(os.path.join(RACINE, "index.html"), encoding="utf-8").read()
p = P(src); p.feed(src)
cles = p.cles
corps = ",\n".join(f"  {json.dumps(k, ensure_ascii=False)}: {json.dumps(v, ensure_ascii=False)}" for k, v in cles.items())
open(os.path.join(RACINE, "i18n", "fr.js"), "w", encoding="utf-8").write(
    "// Textes français de la page (la source). Chaque langue a un fichier avec exactement les mêmes clés.\n"
    "// Les balises dans les textes (<strong>, <code>, <a>) se gardent telles quelles dans les traductions.\n"
    "// Fichier généré depuis index.html par tools/site_i18n.py, qui vérifie aussi les clés et balises des autres langues.\n"
    'window.MM_I18N = window.MM_I18N || {};\nMM_I18N["fr"] = {\n' + corps + "\n};\n")
print(len(cles), "clés écrites dans i18n/fr.js")

# la balise entière : un lien traduit garde aussi son adresse
balises = lambda s: sorted(re.findall(r"</?[a-z][^>]*>", s))
for f in sorted(glob.glob(os.path.join(RACINE, "i18n", "*.js"))):
    code = os.path.basename(f)[:-3]
    if code == "fr":
        continue
    texte = open(f, encoding="utf-8").read()
    m = re.search(r"MM_I18N\[\"([^\"]+)\"\]\s*=\s*(\{.*\})\s*;", texte, re.S)
    if not m or m.group(1) != code:
        sys.exit(f"{code}.js : en-tête attendu MM_I18N[\"{code}\"] = {{...}};")
    # tolère les lignes de commentaire // et une virgule finale, que les traducteurs ajoutent souvent
    corps_js = re.sub(r"^\s*//.*$", "", m.group(2), flags=re.M)
    t = json.loads(re.sub(r",(\s*)\}$", r"\1}", corps_js.strip()))
    inconnues = set(t) - set(cles)
    manquantes = set(cles) - set(t)
    if inconnues:
        sys.exit(f"{code}.js : clés inconnues {sorted(inconnues)}")
    for k, v in t.items():
        if balises(v) != balises(cles[k]):
            sys.exit(f"{code}.js : {k} n'a pas les mêmes balises que le français")
    print(f"{code}.js : {len(t)} clés, {len(manquantes)} encore en français")
