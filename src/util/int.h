/*************************************
** Tsunagari Tile Engine            **
** int.h                            **
** Copyright 2019-2021 Paul Merrill **
*************************************/

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

#ifndef SRC_UTIL_INT_H_
#define SRC_UTIL_INT_H_

#include "util/constexpr.h"

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#if defined(_MSC_VER)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#    ifdef _WIN64
typedef signed __int64 ssize_t;
#    else
typedef signed int ssize_t;
#    endif
#    if !defined(__cplusplus) && defined(_WIN64)
typedef uint64_t size_t;
#    elif !defined(__cplusplus)
typedef uint32_t size_t;
#    else
// size_t already defined.
#    endif
#elif defined(__clang__) || defined(__GNUC__)
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef __SIZE_TYPE__ size_t;
typedef __INTPTR_TYPE__ ssize_t;
#else
#    error Not implemented yet
#endif

#if defined(__APPLE__) || defined(__EMSCRIPTEN__)
typedef long time_t;
#else
typedef int64_t time_t;
#endif

#define INT32_MIN ((int32_t)0x80000000)
#define INT32_MAX ((int32_t)0x7fffffff)
#define INT64_MAX ((int64_t)0x7fffffffffffffff)
#define INT64_MIN ((int64_t)0x8000000000000000)
#define UINT32_MAX ((uint32_t)0xffffffff)
#define UINT64_MAX ((uint64_t)0xffffffffffffffff)

#if defined(_MSC_VER) && defined(_M_X64)
#define SIZE_MAX ((size_t)0xffffffffffffffff)
#elif defined(_MSC_VER)
#define SIZE_MAX ((size_t)0xffffffff)
#else
#define SIZE_MAX __SIZE_MAX__
#endif

#define FLT_MIN ((float)1.17549435082228750796873653722224568e-38f)
#define FLT_MAX ((float)3.402823466e+38f)

#define M_PI ((float)3.14159265358979323846)

#if defined(__EMSCRIPTEN__)
#    define __DEFINED_time_t
#endif

// Raspberry Pi OS - Buster
#define __time_t_defined

#define sizeof32(x) static_cast<uint32_t>(sizeof(x))

#endif  // SRC_UTIL_INT_H_
