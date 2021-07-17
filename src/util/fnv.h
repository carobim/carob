#ifndef SRC_UTIL_FNV_H_
#define SRC_UTIL_FNV_H_

#include "util/compiler.h"
#include "util/int.h"

size_t
fnvHash(const char* data, size_t size) noexcept;

#endif  // SRC_UTIL_FNV_H_
