/********************************
** Tsunagari Tile Engine       **
** compiler.h                  **
** Copyright 2021 Paul Merrill **
********************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifndef SRC_UTIL_COMPILER_H_
#define SRC_UTIL_COMPILER_H_

#if defined(_MSC_VER)
#    if _MSC_VER == 1800
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
#if MSVC == 2013 || (0 < GCC && GCC < 46)
#    define noexcept throw()
#endif
#else
#    define noexcept
#endif

#if CXX
#    if MSVC == 2013
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

#if CLANG || GCC
#    define unreachable __builtin_unreachable()
#else
#    define unreachable (void)
#endif

#endif
