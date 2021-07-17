#ifndef SRC_UTIL_HASH_H_
#define SRC_UTIL_HASH_H_

#include "util/compiler.h"
#include "util/int.h"

template<typename T>
size_t
hash_(const T&) noexcept;

size_t
hash_(float d) noexcept;

#endif  // SRC_UTIL_HASH_H_
