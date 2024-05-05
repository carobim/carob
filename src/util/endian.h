#ifndef SRC_UTIL_ENDIAN_H_
#define SRC_UTIL_ENDIAN_H_

#include "util/compiler.h"
#include "util/cpu.h"
#include "util/int.h"

static inline U16
bswapu16(U16 x) noexcept {
    return ((x & 0xFF00u) >>  8u) |
           ((x & 0x00FFu) <<  8u);
}

static inline I16
bswapi16(I16 x) noexcept {
    return (I16)bswapu16((U16)x);
}

static inline U32
bswapu32(U32 x) noexcept {
#if GCC || CLANG
    return __builtin_bswap32(x);
#else
    return ((x & 0xFF000000u) >> 24u) |
           ((x & 0x00FF0000u) >>  8u) |
           ((x & 0x0000FF00u) <<  8u) |
           ((x & 0x000000FFu) << 24u);
#endif
}

static inline I32
bswapi32(I32 x) noexcept {
    return (I32)bswapu32((U32)x);
}

static inline U64
bswapu64(U64 x) noexcept {
#if GCC || CLANG
    return __builtin_bswap64(x);
#else
    return ((x & 0xFF00000000000000u) >> 56u) |
           ((x & 0x00FF000000000000u) >> 40u) |
           ((x & 0x0000FF0000000000u) >> 24u) |
           ((x & 0x000000FF00000000u) >>  8u) |
           ((x & 0x00000000FF000000u) <<  8u) |
           ((x & 0x0000000000FF0000u) << 24u) |
           ((x & 0x000000000000FF00u) << 40u) |
           ((x & 0x00000000000000FFu) << 56u);
#endif
}

static inline I64
bswapi64(I64 x) noexcept {
    return (I64)bswapu64((U64)x);
}

#if BE
#define be(x) (x)
#else
static inline U16 be(U16 x) { return bswapu16(x); }
static inline I16 be(I16 x) { return bswapi16(x); }
static inline U32 be(U32 x) { return bswapu32(x); }
static inline I32 be(I32 x) { return bswapi32(x); }
static inline U64 be(U64 x) { return bswapu64(x); }
static inline I64 be(I64 x) { return bswapi64(x); }
#endif

#endif  // SRC_UTIL_ENDIAN_H_
