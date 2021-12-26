#ifndef SRC_UTIL_ASSERT_H_
#define SRC_UTIL_ASSERT_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/likely.h"

#ifndef NDEBUG
#    if CLANG || GCC
#        define assert_(expr)       \
            (likely(expr) ? (void)0 \
                          : assert__(__func__, __FILE__, __LINE__, #expr))
#    else
#        define assert_(expr)       \
            (likely(expr) ? (void)0 \
                          : assert__(__FUNCTION__, __FILE__, __LINE__, #expr))
#    endif


void
assert__(const char* func,
         const char* file,
         I32 line,
         const char* expr) noexcept;

void
debugger() noexcept;

#else
#    define assert_(expr)
#    define debugger()
#endif

#endif  // SRC_UTIL_ASSERT_H_
