#ifndef SRC_UTIL_INT_H_
#define SRC_UTIL_INT_H_

#include "util/compiler.h"

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#define INT32_MIN ((int32_t)-1)
#define INT32_MAX ((int32_t)0x7fffffff)
#define UINT32_MAX ((uint32_t)0xffffffff)
#define INT64_MAX ((int64_t)0x7fffffffffffffffLL)
#define INT64_MIN ((int64_t)-1)
#define UINT64_MAX ((uint64_t)0xffffffffffffffffULL)

#if SIZE == 64
#    if MSVC
#        if CXX == 0
typedef uint64_t size_t;
#        endif
#    else
typedef unsigned long size_t;
#    endif
#    define SIZE_MAX ((size_t)0xffffffffffffffff)
#else
#    if MSVC
#        if CXX == 0
typedef uint32_t size_t;
#        endif
#    else
typedef unsigned int size_t;
#    endif
#    define SIZE_MAX ((size_t)0xffffffff)
#endif

#if SIZE == 64
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif

#if defined(__APPLE__) || defined(__EMSCRIPTEN__)
typedef long time_t;
#else
typedef int64_t time_t;
#endif

/* Raspberry Pi OS - Buster */
#define __time_t_defined

#if defined(__EMSCRIPTEN__)
#    define __DEFINED_time_t
#endif

#define FLT_MIN ((float)1.17549435082228750796873653722224568e-38f)
#define FLT_MAX ((float)3.402823466e+38f)

#define M_PI ((float)3.14159265358979323846)

#endif
