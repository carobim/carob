#ifndef SRC_UTIL_FUNCTION_H_
#define SRC_UTIL_FUNCTION_H_

#include "util/compiler.h"

struct Function {
    void (*fn)(void*) noexcept;
    void* data;
};

#endif  // SRC_UTIL_FUNCTION_H_
