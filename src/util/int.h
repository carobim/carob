#ifndef SRC_UTIL_INT_H_
#define SRC_UTIL_INT_H_

#include "util/compiler.h"

typedef signed char I8;
typedef signed short I16;
typedef signed int I32;
typedef signed long long I64;
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long U64;
#define INT32_MIN  ((I32)-1)
#define INT32_MAX  ((I32)0x7fffffff)
#define UINT32_MAX ((U32)0xffffffff)
#define INT64_MAX  ((I64)0x7fffffffffffffffLL)
#define INT64_MIN  ((I64)-1)
#define UINT64_MAX ((U64)0xffffffffffffffffULL)

#if SIZE == 64
#    if MSVC
#        if CXX == 0
typedef U64 Size;
#        else
typedef size_t Size;
#        endif
#    else
typedef unsigned long Size;
#    endif
#    define SIZE_MAX ((Size)0xffffffffffffffff)
#else
#    if MSVC
#        if CXX == 0
typedef U32 Size;
#        else
typedef size_t Size;
#        endif
#    else
typedef unsigned int Size;
#    endif
#    define SIZE_MAX ((Size)0xffffffff)
#endif

#if SIZE == 64
typedef I64 SSize;
#else
typedef I32 SSize;
#endif

// TODO: Check the definition of time_t on Unix compilers:
//       - glibc
//         /usr/include/<triple>/bits/typesizes.h
//         arm-linux-gnueabihf is long
//         x86_64-linux-gnu is long
//       - Check Apple again
//       - Who has I64?
//       - Try other 32-bit platforms
#if defined(__APPLE__) || defined(__EMSCRIPTEN__) || defined(GCC) || \
        defined(CLANG)
typedef long Time;
#else
typedef I64 Time;
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
