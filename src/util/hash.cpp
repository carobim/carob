#include "util/hash.h"

#include "util/compiler.h"
#include "util/fnv.h"
#include "util/int.h"

Size
hash_(int i) noexcept {
    return fnvHash(reinterpret_cast<char*>(&i), sizeof(int));
}

Size
hash_(float d) noexcept {
    char* bits = reinterpret_cast<char*>(&d);
    return *reinterpret_cast<Size*>(bits);
}
