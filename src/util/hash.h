#ifndef SRC_UTIL_HASH_H_
#define SRC_UTIL_HASH_H_

#include "util/compiler.h"
#include "util/int.h"

template<typename T>
Size
hash_(const T&) noexcept;

Size
hash_(int d) noexcept;

Size
hash_(float d) noexcept;

#endif  // SRC_UTIL_HASH_H_
