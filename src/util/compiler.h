#ifndef SRC_UTIL_COMPILER_H_
#define SRC_UTIL_COMPILER_H_

#if defined(_MSC_VER)
#    if _MSC_VER == 1600
#        define MSVC 2010
#    elif _MSC_VER == 1700
#        define MSVC 2012
#    elif _MSC_VER == 1800
#        define MSVC 2013
#    elif _MSC_VER == 1900
#        define MSVC 2015
#    elif 1910 <= _MSC_VER && _MSC_VER < 1920
#        define MSVC 2017
#    elif 1920 <= _MSC_VER && _MSC_VER < 1930
#        define MSVC 2019
#    endif
#    define CLANG 0
#    define GCC 0
#    ifdef _WIN64
#        define SIZE 64
#    else
#        define SIZE 32
#    endif
#elif defined(__clang__)
#    define MSVC 0
#    define CLANG (__clang_major__ * 10 + __clang_minor__)
#    define GCC 0
#    define SIZE (__SIZEOF_SIZE_T__ * 8)
#elif defined(__GNUC__)
#    define MSVC 0
#    define CLANG 0
#    define GCC (__GNUC__ * 10 + __GNUC_MINOR__)
#    define SIZE (__SIZEOF_SIZE_T__ * 8)
#endif

/* https://clang.llvm.org/cxx_status.html */
/* https://gcc.gnu.org/projects/cxx-status.html */

#ifdef __cplusplus
#    define CXX 1
#else
#    define CXX 0
#endif

#if CXX
#    if MSVC == 2015
/* 'noexcept' used but no exception handling is enabled. */
#        pragma warning(disable : 4577)  
#    endif
#    if MSVC == 2010 || MSVC == 2012 || MSVC == 2013 || (0 < GCC && GCC < 46)
#        define noexcept throw()
#    endif
#else
#    define noexcept
#endif

#if CXX
#    if MSVC == 2010 || MSVC == 2012 || MSVC == 2013 || (0 < GCC && GCC < 46)
#        define constexpr
#    endif
#    if __cplusplus >= 201103L || MSVC >= 2015
#        define constexpr11 constexpr
#    else
#        define constexpr11
#    endif
#    if __cplusplus >= 201402L || MSVC >= 2017
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
#    define unreachable (void)0
#endif

#endif
