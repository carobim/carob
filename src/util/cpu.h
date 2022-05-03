#ifndef SRC_UTIL_CPU_H_
#define SRC_UTIL_CPU_H_

#include "util/compiler.h"

// Original source downloaded from: https://github.com/google/cpu_features
// Apache License 2.0

#if defined(__SSE4_1__)
#define CPU_FEATURES_COMPILED_X86_SSE4_1 1
#else
#define CPU_FEATURES_COMPILED_X86_SSE4_1 0
#endif  //  defined(__SSE4_1__)

#if defined(__SSE4_2__)
#define CPU_FEATURES_COMPILED_X86_SSE4_2 1
#else
#define CPU_FEATURES_COMPILED_X86_SSE4_2 0
#endif  //  defined(__SSE4_2__)

#if defined(__AVX__)
#define CPU_FEATURES_COMPILED_X86_AVX 1
#else
#define CPU_FEATURES_COMPILED_X86_AVX 0
#endif  //  defined(__AVX__)

#if defined(__AVX2__)
#define CPU_FEATURES_COMPILED_X86_AVX2 1
#else
#define CPU_FEATURES_COMPILED_X86_AVX2 0
#endif  //  defined(__AVX2__)

#if defined(CPU_FEATURES_ARCH_ANY_ARM)
#if defined(__ARM_NEON__)
#define CPU_FEATURES_COMPILED_ANY_ARM_NEON 1
#else
#define CPU_FEATURES_COMPILED_ANY_ARM_NEON 0
#endif  //  defined(__ARM_NEON__)
#endif  //  defined(CPU_FEATURES_ARCH_ANY_ARM)

#if defined(__i386__) || defined(__x86_64__)
typedef struct {
    int popcnt : 1;
    int rdrnd : 1;

    int sse4_1 : 1;
    int sse4_2 : 1;

    int avx : 1;
    int avx2 : 1;
} X86Features;

X86Features
getCpu() noexcept;
#elif defined(__aarch64__)
// Nothing.
#else
// Nothing. What is this?
#endif

#endif  // SRC_UTIL_FNV_H_
