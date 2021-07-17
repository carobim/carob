/*************************************
** Tsunagari Tile Engine            **
** windows-c.h                      **
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

#ifndef SRC_OS_WINDOWS_C_H_
#define SRC_OS_WINDOWS_C_H_

#include "util/compiler.h"
#include "util/int.h"

#define VOID void

// TCHAR type is 2 bytes in UNICODE, 1 byte otherwise
typedef char CHAR, *LPSTR, *LPTSTR, *NPSTR, *PSTR, TCHAR;
typedef int BOOL;
typedef long LONG, *PLONG;
typedef size_t SIZE_T, *PSIZE_T;
typedef void *LPVOID, *HANDLE, *HWND, *PVOID;
typedef __int64 LONGLONG;

typedef unsigned char BOOLEAN, BYTE;
typedef unsigned int UINT;
typedef unsigned long DWORD, *DWORD_PTR, ULONG, *LPDWORD;
typedef unsigned short WORD;
typedef unsigned __int64 ULONGLONG;

typedef const char *LPCSTR, *LPCSTR;
typedef const void* LPCVOID;

typedef struct {
    long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#if SIZE == 64
typedef long long INT_PTR, *PINT_PTR;
typedef unsigned long long UINT_PTR, *PUINT_PTR;
typedef long long LONG_PTR, *PLONG_PTR;
typedef unsigned long long ULONG_PTR, *PULONG_PTR;
#else
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;
typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
#endif

#define __CRTDECL    __cdecl
#define _ACRTIMP     __declspec(dllimport)
#define _ACRTIMP_ALT __declspec(dllimport)
#define _CRTIMP      __declspec(dllimport)
#define _CRTRESTRICT __declspec(restrict)
#define WINAPI       __stdcall
#define WINBASEAPI   __declspec(dllimport)
#define WINUSERAPI   __declspec(dllimport)

#define errno (*_errno())

typedef struct _iobuf {
    void* _Placeholder;
} FILE;
typedef __int64 __time64_t;

__pragma(pack(push, 8));
extern "C" {
#if MSVC == 2013
_CRTIMP FILE*
__iob_func() noexcept;
#    define stdin  (&__iob_func()[0])
#    define stdout (&__iob_func()[1])
#    define stderr (&__iob_func()[2])
#elif MSVC >= 2015
_ACRTIMP_ALT FILE*
__acrt_iob_func(unsigned) noexcept;
#    define stdin  (__acrt_iob_func(0))
#    define stdout (__acrt_iob_func(1))
#    define stderr (__acrt_iob_func(2))
#endif

_ACRTIMP __time64_t
_time64(__time64_t*) noexcept;
//_ACRTIMP __declspec(noreturn) void exit(int) noexcept;
_ACRTIMP int
rand(void) noexcept;
_ACRTIMP void
srand(unsigned int) noexcept;

void*
memchr(const void*, int, size_t) noexcept;
int
memcmp(void const*, void const*, size_t) noexcept;
void*
memcpy(void*, void const*, size_t) noexcept;
size_t
strlen(char const*) noexcept;

int
abs(int) noexcept;
double
atan2(double, double) noexcept;
_ACRTIMP double
ceil(double) noexcept;
#if SIZE == 64
_ACRTIMP float
ceilf(float) noexcept;
#else
inline float
ceilf(float x) noexcept {
    return (float)ceil(x);
}
#endif
double
cos(double) noexcept;
_ACRTIMP double
floor(double) noexcept;
double
sin(double) noexcept;
double
sqrt(double) noexcept;

_ACRTIMP double
strtod(char const*, char**) noexcept;
_ACRTIMP long
strtol(char const*, char**, int) noexcept;
_ACRTIMP unsigned long
strtoul(char const*, char**, int) noexcept;
_ACRTIMP int
atoi(char const*) noexcept;

_ACRTIMP int*
_errno(void);
}  // extern "C"
__pragma(pack(pop));

extern "C" {
void*
memmem(const void*, size_t, const void*, size_t) noexcept;
void*
memmove(void*, const void*, size_t) noexcept;
}  // extern "C"

#if MSVC == 2013
extern "C" {
_ACRTIMP int
printf(const char*, ...) noexcept;
_ACRTIMP int
fprintf(FILE*, const char*, ...) noexcept;
_ACRTIMP int
sprintf(char*, const char*, ...) noexcept;
}  // extern "C"
#elif MSVC >= 2015
int
printf(char const* const, ...) noexcept;
int
fprintf(FILE* const, char const* const, ...) noexcept;
int
sprintf(char* const, char const* const, ...) noexcept;
#endif

// vcruntime_string.h
extern "C" {
void*
memset(void*, int, size_t);
}  // extern "C"

#endif  // SRC_OS_WINDOWS_C_H_
