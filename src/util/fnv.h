#ifndef SRC_UTIL_FNV_H_
#define SRC_UTIL_FNV_H_

#include "util/compiler.h"
#include "util/int.h"

Size
fnvHash(const char* data, Size size) noexcept;

#endif  // SRC_UTIL_FNV_H_
