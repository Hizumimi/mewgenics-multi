#!/bin/sh
# compiler.sh -- Mewgenics Multi : verifier sous Linux que des fichiers du mod
# compilent pour Windows (zig c++, c'est-a-dire clang, cible x86_64-windows-gnu,
# -Wall), sans lier. Ce n'est pas MSVC : la compilation de GitHub Actions (/W3)
# reste la reference, et la seule qui produit mgmp.dll ; mais une erreur ou un
# avertissement d'ici en annonce presque toujours un la-bas, avant de pousser.
#
#   pip install ziglang                        (une fois ; zig 0.17 a servi)
#   tools/linux/compiler.sh [fichier.cpp ...]
#
# Les chemins sont relatifs a la racine du depot. Sans fichier : tous les .cpp de
# src/ que liste CMakeLists.txt (une dizaine de minutes la premiere fois). Une ligne
# par fichier : rc=0 s'il compile, diag = avertissements + erreurs ; le detail est
# dans build_linux/zig/<nom>.log. Code de sortie : le nombre de fichiers qui ne
# compilent pas.
#
# Deux fichiers a part :
#   - mm_menu.cpp verifie que std::function fait 64 octets, la taille MSVC que le
#     jeu attend ; la libc++ de zig a une autre taille : une copie sans cette ligne
#     est compilee a sa place (ses #include se trouvent par les -I) ;
#   - mgmp_sigscan.cpp utilise AVX2 sans attribut de cible (MSVC le permet, clang
#     non) : il est compile avec -mavx2 -mxsave.
# Avertissements connus, sans consequence (clang seulement, pas MSVC /W3) : une
# fonction cfg() inutilisee dans mgmp_cursor.cpp et mgmp_overlay.cpp (stb_image.h
# en ajoute deux), et __cpuidex declare implicitement dans mgmp_sigscan.cpp.
set -u
cd "$(dirname "$0")/../.." || exit 1
O=build_linux/zig
mkdir -p "$O/copies" || exit 1
# Un cache zig neuf a chaque lancement : un fichier deja compile a l'identique est
# repris du cache SANS ses avertissements (diag=0 trompeur). Une seconde par fichier.
ZIG_LOCAL_CACHE_DIR=$(mktemp -d "${TMPDIR:-/tmp}/mm-zig.XXXXXX") || exit 1
ZIG_GLOBAL_CACHE_DIR=$ZIG_LOCAL_CACHE_DIR
export ZIG_LOCAL_CACHE_DIR ZIG_GLOBAL_CACHE_DIR

if [ $# -eq 0 ]; then
    set -- $(grep -o 'src/[A-Za-z0-9_/]*\.cpp' CMakeLists.txt | sort -u)
fi

bad=0
for f in "$@"; do
    b=$(basename "$f" .cpp)
    src=$f
    extra=
    if [ "$b" = mm_menu ]; then
        src="$O/copies/mm_menu.cpp"
        grep -v 'static_assert(sizeof(std::function' "$f" > "$src"
    fi
    [ "$b" = mgmp_sigscan ] && extra="-mavx2 -mxsave"
    # shellcheck disable=SC2086
    timeout 900 python3 -m ziglang c++ -target x86_64-windows-gnu -std=c++17 $extra -c "$src" -o "$O/$b.o" \
        -Isrc/core -Isrc/determinism -Isrc/net -Isrc/session -Isrc/ui -Isrc/i18n \
        -Ithird_party -Ithird_party/imgui -Ithird_party/sqlite -Ithird_party/minhook/include \
        -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DMM_BUILD_ID=\"linux\" \
        -fms-extensions -Wall -Wno-unknown-pragmas -Wno-missing-braces > "$O/$b.log" 2>&1
    rc=$?
    [ $rc -eq 0 ] || bad=$((bad + 1))
    echo "$b rc=$rc diag=$(grep -c 'warning:\|error:' "$O/$b.log")"
done
exit $bad
