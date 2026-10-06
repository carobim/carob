#ifndef SRC_UTIL_COMPILER_H_
#define SRC_UTIL_COMPILER_H_

#ifdef NDEBUG
#    define DEBUG 0
#else
#    define DEBUG 1
#endif

#if defined(_MSC_VER)
// Set MSVC to the MSVC Build Tools version.
#    if _MSC_VER < 1900
#        define MSVC (_MSC_VER - 600)
#    else
#        define MSVC (_MSC_VER - 500)
#    endif
#    define CLANG 0
#    define GCC   0
#    ifdef _WIN64
#        define SIZE 64
#    else
#        define SIZE 32
#    endif
#elif defined(__clang__)
#    define MSVC  0
#    define CLANG (__clang_major__ * 10 + __clang_minor__)
#    define GCC   0
#    define SIZE  (__SIZEOF_SIZE_T__ * 8)
#elif defined(__GNUC__)
#    define MSVC  0
#    define CLANG 0
#    define GCC   (__GNUC__ * 10 + __GNUC_MINOR__)
#    define SIZE  (__SIZEOF_SIZE_T__ * 8)
#endif

// Always use range queries, never equality, with these.
#define VS2010 1000
#define VS2012 1100
#define VS2013 1200
#define VS2015 1400
#define VS2017 1410
#define VS2019 1420
#define VS2022 1430
#define VS2026 1450
// v1950 was released November 2025.
// A new version (1951, 1952, ...) is released every 6 months.
// https://learn.microsoft.com/en-us/cpp/overview/compiler-versions
#define MSVC_MODERN(y, m) (1950 + ((y) - 2025) * 2 + ((m) >= 11) - ((m) < 5) - 1)

/* https://clang.llvm.org/cxx_status.html */
/* https://gcc.gnu.org/projects/cxx-status.html */

#ifdef __cplusplus
#    define CXX 1
#else
#    define CXX 0
#endif

#if CXX
#    if MSVC >= VS2015 && MSVC < VS2017
/* 'noexcept' used but no exception handling is enabled. */
#        pragma warning(disable : 4577)
#    endif
#    if (MSVC >= VS2010 && MSVC < VS2015) || (0 < GCC && GCC < 46)
#        define noexcept throw()
#    endif
#else
#    define noexcept
#endif

#if CXX
#    if (MSVC >= VS2010 && MSVC < VS2015) || (0 < GCC && GCC < 46)
#        define constexpr
#    endif
#    if __cplusplus >= 201103L || MSVC >= VS2015
#        define constexpr11 constexpr
#    else
#        define constexpr11
#    endif
#    if __cplusplus >= 201402L || MSVC >= VS2017
#        define constexpr14 constexpr
#    else
#        define constexpr14
#    endif
#else
#    define constexpr
#    define constexpr11
#    define constexpr14
#endif

#if CLANG || GCC >= 45
#    define unreachable __builtin_unreachable()
#else
#    define unreachable __assume(0)
#endif

#endif
