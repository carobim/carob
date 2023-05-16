#include "util/compiler.h"
#include "util/int.h"

// Original source downloaded from:
//   https://github.com/nadavrot/memset_benchmark
// See also:
//   https://msrc-blog.microsoft.com/2021/01/11/building-faster-amd64-memset-routines/

#if defined(CLANG)
// https://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors
typedef char char8 __attribute__((ext_vector_type(8), aligned(1)));
typedef char char16 __attribute__((ext_vector_type(16), aligned(1)));
typedef char char32 __attribute__((ext_vector_type(32), aligned(1)));
typedef char char32a __attribute__((ext_vector_type(32), aligned(32)));
#elif defined(GCC)
// https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html
typedef char char8 __attribute__((vector_size(8), aligned(1)));
typedef char char16 __attribute__((vector_size(16), aligned(1)));
typedef char char32 __attribute__((vector_size(32), aligned(1)));
typedef char char32a __attribute__((vector_size(32), aligned(32)));
#endif

typedef U32 __attribute__((aligned(1))) u32;
typedef U64 __attribute__((aligned(1))) u64;

// Handle memsets of sizes 0..32
static inline char*
smallMemset(char* s, int c, Size n) noexcept {
    if (n < 5) {
        if (n == 0)
            return s;
        char* p = s;
        p[0] = c;
        p[n - 1] = c;
        if (n <= 2)
            return s;
        p[1] = c;
        p[2] = c;
        return s;
    }

    if (n <= 16) {
        U64 val8 = ((U64)0x0101010101010101L * ((U8)c));
        if (n >= 8) {
            char* first = s;
            char* last = s + n - 8;
            *((u64*)first) = val8;
            *((u64*)last) = val8;
            return s;
        }

        U32 val4 = val8;
        char* first = s;
        char* last = s + n - 4;
        *((u32*)first) = val4;
        *((u32*)last) = val4;
        return s;
    }

    char X = c;
    char* p = s;
    char16 val16 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};
    char* last = s + n - 16;
    *((char16*)last) = val16;
    *((char16*)p) = val16;
    return s;
}

static inline char*
hugeMemset(char* s, int c, Size n) noexcept {
    char* p = s;
    char X = c;
    char32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                    X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

    // Stamp the first 32byte store.
    *((char32*)p) = val32;

    char* first_aligned = p + 32 - ((U64)p % 32);
    char* buffer_end = p + n;
    char* last_word = buffer_end - 32;

    // Align the next stores.
    p = first_aligned;

    // Unroll the body of the loop to increase parallelism.
    while (p + (32 * 5) < buffer_end) {
        *((char32a*)p) = val32;
        p += 32;
        *((char32a*)p) = val32;
        p += 32;
        *((char32a*)p) = val32;
        p += 32;
        *((char32a*)p) = val32;
        p += 32;
        *((char32a*)p) = val32;
        p += 32;
    }

// Complete the last few iterations:
#define TRY_STAMP_32_BYTES      \
    if (p < last_word) {        \
        *((char32a*)p) = val32; \
        p += 32;                \
    }

    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES

    // Stamp the last unaligned word.
    *((char32*)last_word) = val32;
    return s;
}

char*
memset(char* s, int c, Size n) noexcept {
    char* p = s;
    char X = c;

    if (n < 32)
        return smallMemset(s, c, n);

    if (n > 160)
        return hugeMemset(s, c, n);

    char32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                    X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

    char* last_word = s + n - 32;

    // Stamp the 32-byte chunks.
    do {
        *((char32*)p) = val32;
        p += 32;
    } while (p < last_word);

    // Stamp the last unaligned 32 bytes of the buffer.
    *((char32*)last_word) = val32;
    return s;
}
