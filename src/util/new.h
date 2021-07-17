#ifndef SRC_UTIL_NEW_H_
#define SRC_UTIL_NEW_H_

#include "util/compiler.h"
#include "util/int.h"

extern "C" {
#if MSVC
__declspec(dllimport) void free(void*) noexcept;
__declspec(dllimport) __declspec(restrict) void* malloc(size_t) noexcept;
#else
void* malloc(size_t) noexcept;
void
free(void*) noexcept;
#endif
}

inline void*
operator new(size_t, void* p) noexcept {
    return p;
}

// The cast to uint32_t fixes a Walloc-size-larger-than on GCC 8.
#define xmalloc(T, count) \
    static_cast<T*>(malloc(static_cast<uint32_t>(sizeof(T) * (count))))

#endif  // SRC_UTIL_NEW_H_
