# Pure-logic unit tests -- no game, no minhook, no linking against mgmp.dll.
# Each covers a piece of the mod that can only otherwise be exercised in-game
# (a live desync, an AstroZombie fight), so the branch logic gets tested here
# and the in-game run is left to prove only the parts that need a game. See
# the header comment in each .cpp for which hook/wire-format it pins.

add_executable(test_proto test_proto.cpp)
target_include_directories(test_proto PRIVATE ${CMAKE_SOURCE_DIR}/src/net)
add_test(NAME test_proto COMMAND test_proto)

add_executable(test_hashring test_hashring.cpp)
target_include_directories(test_hashring PRIVATE ${CMAKE_SOURCE_DIR}/src/net)
add_test(NAME test_hashring COMMAND test_hashring)

add_executable(test_timedelay test_timedelay.cpp)
target_include_directories(test_timedelay PRIVATE ${CMAKE_SOURCE_DIR}/src/determinism)
add_test(NAME test_timedelay COMMAND test_timedelay)

add_executable(test_barrier test_barrier.cpp)
target_include_directories(test_barrier PRIVATE ${CMAKE_SOURCE_DIR}/src/session)
add_test(NAME test_barrier COMMAND test_barrier)

add_executable(test_split test_split.cpp)
target_include_directories(test_split PRIVATE ${CMAKE_SOURCE_DIR}/src/session)
add_test(NAME test_split COMMAND test_split)

# Mewgenics Multi : les membres d'un combat (protocole 48) -- le partage sur les
# membres de l'entree, la reprise des chats d'un parti par l'hote, l'adoption de
# ses membres par un invite, et la verification de chacun.
add_executable(test_members test_members.cpp)
target_include_directories(test_members PRIVATE ${CMAKE_SOURCE_DIR}/src/session)
add_test(NAME test_members COMMAND test_members)

add_executable(test_sigscan test_sigscan.cpp ${CMAKE_SOURCE_DIR}/src/core/mgmp_sigscan.cpp)
target_include_directories(test_sigscan PRIVATE ${CMAKE_SOURCE_DIR}/src/core)
add_test(NAME test_sigscan COMMAND test_sigscan)

add_executable(test_resolve_live test_resolve_live.cpp ${CMAKE_SOURCE_DIR}/src/core/mgmp_sigscan.cpp)
target_include_directories(test_resolve_live PRIVATE ${CMAKE_SOURCE_DIR}/src/core)
add_test(NAME test_resolve_live COMMAND test_resolve_live)

add_executable(test_battleid test_battleid.cpp)
target_include_directories(test_battleid PRIVATE ${CMAKE_SOURCE_DIR}/src/session)
add_test(NAME test_battleid COMMAND test_battleid)

# The one test that compiles a piece of the mod rather than a header: the config
# reader is a .cpp because it owns the Config singleton, and it is worth covering
# because it is the only parser in the project whose input a human types.
add_executable(test_config test_config.cpp ${CMAKE_SOURCE_DIR}/src/core/mgmp_config.cpp)
target_include_directories(test_config PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/third_party)
target_compile_definitions(test_config PRIVATE
    WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS NOMINMAX)
add_test(NAME test_config COMMAND test_config)

# Mewgenics Multi : le monde de l'hote recopie dans la save d'un invite, sur deux
# vraies bases sqlite construites par le test (mm_world_merge.cpp + mm_sqlite).
add_executable(test_world_merge test_world_merge.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_world_merge.cpp
               ${CMAKE_SOURCE_DIR}/src/session/mm_npc_state.cpp)
target_include_directories(test_world_merge PRIVATE
    ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/third_party/sqlite)
target_link_libraries(test_world_merge PRIVATE mm_sqlite)
target_compile_options(test_world_merge PRIVATE /utf-8)
add_test(NAME test_world_merge COMMAND test_world_merge)

# Mewgenics Multi : l'equipe mixte -- le message, le format des chats (LZ4 compris),
# house_state, les chats d'un invite chez l'hote et leur retour chez l'invite.
add_executable(test_team_merge test_team_merge.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_team_merge.cpp)
target_include_directories(test_team_merge PRIVATE
    ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/third_party/sqlite)
target_link_libraries(test_team_merge PRIVATE mm_sqlite)
target_compile_options(test_team_merge PRIVATE /utf-8)
add_test(NAME test_team_merge COMMAND test_team_merge)

# Mewgenics Multi : le vote du chemin -- le tour (30 s, absents, egalite), le
# message MM_VOTE, la fenetre de chaque joueur et son texte.
add_executable(test_vote test_vote.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_vote_core.cpp)
target_include_directories(test_vote PRIVATE ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/net)
target_compile_options(test_vote PRIVATE /utf-8)
add_test(NAME test_vote COMMAND test_vote)

# Mewgenics Multi : les bourses de l'aventure -- le partage, la part de chacun a la
# fin, le message MM_PURSE et les textes.
add_executable(test_purse test_purse.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_purse_core.cpp)
target_include_directories(test_purse PRIVATE ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/net)
target_compile_options(test_purse PRIVATE /utf-8)
add_test(NAME test_purse COMMAND test_purse)

# Mewgenics Multi : le defile du butin -- le message kTeamLoot, le gagnant de
# chaque objet (Besoin, Cupidite, egalites) et le deroulement chez l'hote.
add_executable(test_loot test_loot.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_loot_core.cpp
               ${CMAKE_SOURCE_DIR}/src/session/mm_team_merge.cpp)
target_include_directories(test_loot PRIVATE
    ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/third_party/sqlite)
target_link_libraries(test_loot PRIVATE mm_sqlite)
target_compile_options(test_loot PRIVATE /utf-8)
add_test(NAME test_loot COMMAND test_loot)

# Mewgenics Multi : les recrues -- a qui va un chat qui rejoint l'equipe en route
# (spectateur sans chat depuis le plus longtemps, sinon le moins de chats ; au sort).
add_executable(test_recruit test_recruit.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_recruit_core.cpp)
target_include_directories(test_recruit PRIVATE ${CMAKE_SOURCE_DIR}/src/session)
target_compile_options(test_recruit PRIVATE /utf-8)
add_test(NAME test_recruit COMMAND test_recruit)

# Mewgenics Multi : les dons de l'invite aux PNJ -- le message kTeamNpc, les
# capacites du chat, les objets nouveaux du rangement, a qui va la recompense.
add_executable(test_npcgift test_npcgift.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_npcgift_core.cpp
               ${CMAKE_SOURCE_DIR}/src/session/mm_team_merge.cpp)
target_include_directories(test_npcgift PRIVATE
    ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/third_party/sqlite)
target_link_libraries(test_npcgift PRIVATE mm_sqlite)
target_compile_options(test_npcgift PRIVATE /utf-8)
add_test(NAME test_npcgift COMMAND test_npcgift)

# Mewgenics Multi : les decisions de combat deja vues (relai de l'hote, reprise).
add_executable(test_seen test_seen.cpp)
target_include_directories(test_seen PRIVATE ${CMAKE_SOURCE_DIR}/src/session)
target_compile_options(test_seen PRIVATE /utf-8)
add_test(NAME test_seen COMMAND test_seen)

# Mewgenics Multi : les achats en boutique -- le message kTeamShop, qui peut
# acheter quoi, les refus, a qui est un objet achete.
add_executable(test_shop test_shop.cpp ${CMAKE_SOURCE_DIR}/src/session/mm_shop_core.cpp
               ${CMAKE_SOURCE_DIR}/src/session/mm_team_merge.cpp)
target_include_directories(test_shop PRIVATE
    ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/third_party/sqlite)
target_link_libraries(test_shop PRIVATE mm_sqlite)
target_compile_options(test_shop PRIVATE /utf-8)
add_test(NAME test_shop COMMAND test_shop)

# Mewgenics Multi : l'IA qui joue les chats d'un absent -- son rythme, le plafond
# de decisions par tour, ce qui part (protocole 52).
add_executable(test_ai test_ai.cpp)
target_include_directories(test_ai PRIVATE ${CMAKE_SOURCE_DIR}/src/session ${CMAKE_SOURCE_DIR}/src/determinism)
target_compile_options(test_ai PRIVATE /utf-8)
add_test(NAME test_ai COMMAND test_ai)

foreach(t test_proto test_hashring test_timedelay test_barrier test_split test_members test_battleid test_config test_world_merge test_team_merge test_vote test_purse test_loot test_recruit test_npcgift test_seen test_shop test_ai)
    target_compile_options(${t} PRIVATE /W3)
endforeach()
