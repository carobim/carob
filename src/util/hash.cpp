#include "util/hash.h"

#include "util/compiler.h"
#include "util/int.h"

size_t
hash_(float d) noexcept {
    char* bits = reinterpret_cast<char*>(&d);
    return *reinterpret_cast<size_t*>(bits);
}
