#ifndef SRC_UTIL_NEW_H_
#define SRC_UTIL_NEW_H_

#include "util/compiler.h"
#include "util/int.h"

extern "C" {
#if MSVC
__declspec(dllimport) void free(void*) noexcept;
__declspec(dllimport) __declspec(restrict) void* malloc(Size) noexcept;
#else
void* malloc(Size) noexcept;
void
free(void*) noexcept;
#endif
}

inline void*
operator new(Size, void* p) noexcept {
    return p;
}

/* Staying with Size here ensures we do not receive a -Wstringop-overflow on GCC 10. */
#define xmalloc(T, count)       ((T*)malloc(sizeof(T) * (count)))
#define xrealloc(ptr, T, count) ((T*)realloc(ptr, sizeof(T) * (count)))

#define make2(Type, name, f1, v1, f2, v2) \
    Type name;                            \
    name.f1 = v1;                         \
    name.f2 = v2

#define make3(Type, name, f1, v1, f2, v2, f3, v3) \
    Type name;                                    \
    name.f1 = v1;                                 \
    name.f2 = v2;                                 \
    name.f3 = v3

#define new0(Type, name) Type* name = (Type*)malloc(sizeof(Type))

#define new1(Type, name, f1, v1)              \
    Type* name = (Type*)malloc(sizeof(Type)); \
    name->f1 = v1

#define new2(Type, name, f1, v1, f2, v2)      \
    Type* name = (Type*)malloc(sizeof(Type)); \
    name->f1 = v1;                            \
    name->f2 = v2

#define new3(Type, name, f1, v1, f2, v2, f3, v3) \
    Type* name = (Type*)malloc(sizeof(Type));    \
    name->f1 = v1;                               \
    name->f2 = v2;                               \
    name->f3 = v3

#define fromCast(Type, name, from) Type* name = reinterpret_cast<Type*>(from)

#endif
