#!/bin/sh
# tests.sh -- Mewgenics Multi : les tests de logique pure (tests/), compiles avec
# g++ et lances sous Linux. GitHub Actions lance les memes avec MSVC (ctest), plus
# trois qui demandent Windows : test_config, test_sigscan, test_resolve_live.
#
#   tools/linux/tests.sh [dossier de sortie]
#
# Le dossier de sortie (par defaut build_linux/tests, ignore par git) est relatif
# a la racine du depot. Une ligne par test : rc=0 et la derniere ligne du test ;
# le detail est dans <sortie>/<test>.out, une erreur de compilation dans
# <sortie>/<test>.err. Code de sortie : le nombre de tests en echec.
#
# Un test ajoute a tests/CMakeLists.txt s'ajoute aussi ici (meme liste de sources).
set -u
cd "$(dirname "$0")/../.." || exit 1
O="${1:-build_linux/tests}"
mkdir -p "$O" || exit 1

SHIM="-include tools/linux/msvc_shim.h"
CXX="g++ -std=c++17 -O1 -w"

# sqlite (third_party), compile une fois, avec les memes options que CMakeLists.txt.
SQ="$O/sqlite3.o"
if [ ! -f "$SQ" ]; then
    echo "sqlite3.o (une fois, une minute environ)..."
    gcc -O1 -w -c third_party/sqlite/sqlite3.c -o "$SQ" \
        -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_DEPRECATED -DSQLITE_DEFAULT_MEMSTATUS=0 \
        -DSQLITE_DQS=0 -DSQLITE_THREADSAFE=1 || exit 1
fi
SQL="$SQ -lpthread -ldl -lm"

fail=0
t() {
    name=$1
    shift
    # shellcheck disable=SC2086
    if ! $CXX "$@" -o "$O/$name" > "$O/$name.err" 2>&1; then
        echo "$name : NE COMPILE PAS"
        head -5 "$O/$name.err"
        fail=$((fail + 1))
        return
    fi
    "$O/$name" > "$O/$name.out" 2>&1
    rc=$?
    [ $rc -eq 0 ] || fail=$((fail + 1))
    echo "$name rc=$rc  $(tail -1 "$O/$name.out")"
}

S=src/session
t test_proto       $SHIM -Isrc/net -Isrc/determinism -I$S tests/test_proto.cpp
t test_barrier     $SHIM -Isrc/net -Isrc/determinism -I$S tests/test_barrier.cpp
t test_split       $SHIM -Isrc/net -Isrc/determinism -I$S tests/test_split.cpp
t test_battleid    $SHIM -Isrc/net -Isrc/determinism -I$S tests/test_battleid.cpp
t test_orphan      $SHIM -Isrc/net -I$S tests/test_orphan.cpp
t test_members     -I$S tests/test_members.cpp
t test_slot        -I$S tests/test_slot.cpp
t test_seen        -I$S tests/test_seen.cpp
t test_recruit     -I$S tests/test_recruit.cpp $S/mm_recruit_core.cpp
t test_ai          -Wno-comment -I$S -Isrc/determinism tests/test_ai.cpp
t test_hashring    -Isrc/net tests/test_hashring.cpp
t test_peerbook    -Isrc/net tests/test_peerbook.cpp
t test_recvq       -Isrc/net tests/test_recvq.cpp
t test_timedelay   -Isrc/determinism tests/test_timedelay.cpp
t test_vote        -I$S -Isrc/core -Isrc/net tests/test_vote.cpp $S/mm_vote_core.cpp
t test_purse       -I$S -Isrc/core -Isrc/net tests/test_purse.cpp $S/mm_purse_core.cpp
t test_team_merge  -I$S -Isrc/core -Ithird_party/sqlite tests/test_team_merge.cpp $S/mm_team_merge.cpp $SQL
t test_world_merge -I$S -Isrc/core -Ithird_party/sqlite tests/test_world_merge.cpp $S/mm_world_merge.cpp \
                   $S/mm_npc_state.cpp $SQL
t test_loot        -I$S -Isrc/core -Ithird_party/sqlite tests/test_loot.cpp $S/mm_loot_core.cpp \
                   $S/mm_team_merge.cpp $SQL
t test_npcgift     -I$S -Isrc/core -Ithird_party/sqlite tests/test_npcgift.cpp $S/mm_npcgift_core.cpp \
                   $S/mm_team_merge.cpp $SQL
t test_shop        -I$S -Isrc/core -Ithird_party/sqlite tests/test_shop.cpp $S/mm_shop_core.cpp \
                   $S/mm_team_merge.cpp $SQL
t test_gift        -I$S -Isrc/core -Isrc/net -Ithird_party/sqlite tests/test_gift.cpp $S/mm_gift_core.cpp \
                   $S/mm_purse_core.cpp $S/mm_team_merge.cpp $SQL

if [ $fail -eq 0 ]; then echo "tous les tests passent"; else echo "$fail test(s) en echec"; fi
exit $fail
