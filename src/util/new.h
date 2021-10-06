#ifndef SRC_UTIL_NEW_H_
#define SRC_UTIL_NEW_H_

#include "util/compiler.h"
#include "util/int.h"

extern "C" {
#if MSVC
__declspec(dllimport) void
free(void*) noexcept;
__declspec(dllimport) __declspec(restrict) void*
malloc(Size) noexcept;
#else
void*
malloc(Size) noexcept;
void
free(void*) noexcept;
#endif
}

inline void*
operator new(Size, void* p) noexcept {
    return p;
}

/* Staying with Size here ensures we do not receive a -Wstringop-overflow on GCC 10. */
#define xmalloc(T, count) \
    ((T*)malloc(sizeof(T) * (count)))
#define xrealloc(ptr, T, count) \
    ((T*)realloc(ptr, sizeof(T) * (count)))

#endif
