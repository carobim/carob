#include "util/fnv.h"

#include "util/compiler.h"
#include "util/int.h"

#if SIZE == 64

Size
fnvHash(const char* data, Size size) noexcept {
    Size hash = 0xcbf29ce484222325;

    const U8* begin = (const U8*)data;
    const U8* end = begin + size;

    while (begin < end) {
        hash ^= (Size)*begin++;
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) +
                (hash << 8) + (hash << 40);
    }
    return hash;
}

#else

Size
fnvHash(const char* data, Size size) noexcept {
    Size hash = 0x811c9dc5;

    const U8* begin = (const U8*)data;
    const U8* end = begin + size;

    while (begin < end) {
        hash ^= (Size)*begin++;
        hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) +
                (hash << 24);
    }
    return hash;
}

#endif
