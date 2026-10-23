#!/bin/sh
# paquet.sh -- Mewgenics Multi : le zip a donner a l'ami pour une build publiee.
#
#   tools/linux/paquet.sh <N>            (N = le numero de la release build-N)
#
# Telecharge mgmp.dll, mgmp_loader.exe, Mewgenics-Multi.exe et SHA256SUMS.txt de la release build-N
# (gh, connecte avec un acces au depot), verifie les empreintes, puis met dans
# build_linux/paquet-N/ :
#   - mgmp.dll, mgmp_loader.exe, « Mewgenics Multi.exe » (a copier dans le dossier
#     mgmp de Hizu, jeu ferme) ;
#   - mgmp-pour-ton-ami.zip : un dossier mgmp/ avec les fichiers de paquet/
#     (LISEZ-MOI.txt, mgmp.json, steam_appid.txt), les deux binaires et le
#     launcher sous son nom « Mewgenics Multi.exe » (sans espace dans la release :
#     GitHub en ferait un point).
# Sous Windows, docs/PASSATION.md donne les memes etapes en PowerShell.
set -eu
N="${1:-}"
[ -n "$N" ] || { echo "usage : tools/linux/paquet.sh <N>"; exit 2; }
cd "$(dirname "$0")/../.."
R=Hizumimi/mewgenics-multi
O="build_linux/paquet-$N"
mkdir -p "$O/dl"

gh api "repos/$R/releases/tags/build-$N" --jq '.assets[] | "\(.id) \(.name)"' > "$O/dl/assets.txt"
while read -r id name; do
    gh api -H "Accept: application/octet-stream" "repos/$R/releases/assets/$id" > "$O/dl/$name"
done < "$O/dl/assets.txt"
(cd "$O/dl" && sha256sum -c SHA256SUMS.txt)

# Un dossier neuf a chaque fois (build_linux/ est ignore par git).
T=$(mktemp -d "$O/zip.XXXXXX")
mkdir "$T/mgmp"
cp paquet/* "$T/mgmp/"
cp "$O/dl/mgmp.dll" "$O/dl/mgmp_loader.exe" "$T/mgmp/"
cp "$O/dl/Mewgenics-Multi.exe" "$T/mgmp/Mewgenics Multi.exe"
(cd "$T" && zip -q -r -X -D paquet.zip mgmp)
mv -f "$T/paquet.zip" "$O/mgmp-pour-ton-ami.zip"
cp "$O/dl/mgmp.dll" "$O/dl/mgmp_loader.exe" "$O/"
cp "$O/dl/Mewgenics-Multi.exe" "$O/Mewgenics Multi.exe"
ls -la "$O"
unzip -l "$O/mgmp-pour-ton-ami.zip"
