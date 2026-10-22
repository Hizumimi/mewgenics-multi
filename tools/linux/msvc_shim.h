#pragma once
// msvc_shim.h -- Mewgenics Multi : les deux fonctions « _s » de MSVC qu'utilisent
// quelques en-tetes testes (mgmp_proto.h...), pour compiler les tests avec g++
// sous Linux (tools/linux/tests.sh, par -include). Rien de ceci n'entre dans le mod.
#include <cstddef>
#include <cstdio>
#include <cstring>

template <size_t N>
inline int strcpy_s(char (&d)[N], const char* s) {
    strncpy(d, s, N - 1);
    d[N - 1] = 0;
    return 0;
}

template <size_t N, class... A>
inline int sprintf_s(char (&d)[N], const char* f, A... a) {
    return snprintf(d, N, f, a...);
}
