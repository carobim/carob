#ifndef SRC_UTIL_LIKELY_H_
#define SRC_UTIL_LIKELY_H_

#include "util/compiler.h"

#if CLANG || GCC
#    define likely(x)   __builtin_expect(!!(x), true)
#    define unlikely(x) __builtin_expect(!!(x), false)
#else
#    define likely(x)   (x)
#    define unlikely(x) (x)
#endif

#endif  // SRC_UTIL_LIKELY_H_
