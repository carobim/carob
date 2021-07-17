#include "util/fnv.h"

#include "util/compiler.h"
#include "util/int.h"

#if SIZE == 64

size_t
fnvHash(const char* data, size_t size) noexcept {
    size_t hash = 0xcbf29ce484222325;

    const uint8_t* begin = (const uint8_t*)data;
    const uint8_t* end = begin + size;

    while (begin < end) {
        hash ^= (size_t)*begin++;
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) +
                (hash << 8) + (hash << 40);
    }
    return hash;
}

#else

size_t
fnvHash(const char* data, size_t size) noexcept {
    size_t hash = 0x811c9dc5;

    const uint8_t* begin = (const uint8_t*)data;
    const uint8_t* end = begin + size;

    while (begin < end) {
        hash ^= (size_t)*begin++;
        hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) +
                (hash << 24);
    }
    return hash;
}

#endif
