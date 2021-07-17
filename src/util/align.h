#ifndef SRC_UTIL_ALIGN_H_
#define SRC_UTIL_ALIGN_H_

#include "util/compiler.h"

template<typename T>
struct Align {
#if MSVC >= 2015 || CLANG || GCC >= 49
    alignas(alignof(T)) char storage[sizeof(T)];
#elif MSVC && SIZE == 64
    __declspec(align(8)) char storage[sizeof(T)];
#elif MSVC
    __declspec(align(4)) char storage[sizeof(T)];
#elif SIZE == 64
    __attribute__((aligned(8))) char storage[sizeof(T)];
#else
    __attribute__((aligned(4))) char storage[sizeof(T)];
#endif
};

#endif  // SRC_UTIL_ALIGN_H_
