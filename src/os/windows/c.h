#ifndef SRC_OS_WINDOWS_C_H_
#define SRC_OS_WINDOWS_C_H_

#include "util/compiler.h"
#include "util/int.h"

#define VOID void

// TCHAR type is 2 bytes in UNICODE, 1 byte otherwise
typedef char CHAR, *LPSTR, *LPTSTR, *NPSTR, *PSTR, TCHAR;
typedef int BOOL;
typedef long LONG, *PLONG;
typedef Size SIZE_T, *PSIZE_T;
typedef void *LPVOID, *HANDLE, *HWND, *PVOID;
typedef I64 LONGLONG;

typedef unsigned char BOOLEAN, BYTE;
typedef unsigned int UINT;
typedef unsigned long DWORD, *DWORD_PTR, ULONG, *LPDWORD;
typedef unsigned short WORD;
typedef U64 ULONGLONG;

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

// Defined as __declspec(dllimport) when building a DLL.
#if MSVC == 2010
// 2010 crtdefs.h
#    define CRTIMP
#else
// 2019 corecrt.h
#    define CRTIMP
#endif

#define WINAPI     __stdcall
#define WINBASEAPI __declspec(dllimport)
#define WINUSERAPI __declspec(dllimport)

// 2010 stdlib.h
// 2012 stdlib.h
#define errno (*_errno())

// 2010 stdio.h
// 2012 stdio.h
typedef struct _iobuf {
    void* reserved;
} FILE;
// 2010 time.h
// 2012 time.h
typedef I64 __time64_t;

__pragma(pack(push, 8));
extern "C" {
#if MSVC == 2010 || MSVC == 2012 || MSVC == 2013
// 2010 stdio.h
// 2012 stdio.h
CRTIMP FILE*
__iob_func(void) noexcept;
#    define stdin  (&__iob_func()[0])
#    define stdout (&__iob_func()[1])
#    define stderr (&__iob_func()[2])
#elif MSVC >= 2015
CRTIMP FILE*
__acrt_iob_func(unsigned) noexcept;
#    define stdin  (__acrt_iob_func(0))
#    define stdout (__acrt_iob_func(1))
#    define stderr (__acrt_iob_func(2))
#endif

// 2010 time.h
// 2012 time.h
CRTIMP __time64_t
_time64(__time64_t*) noexcept;

// 2010 stdlib.h
// 2012 stdlib.h
CRTIMP int
rand(void) noexcept;
CRTIMP void
srand(unsigned int) noexcept;

// 2010 string.h
// 2012 string.h
CRTIMP void*
memchr(const void*, int, Size) noexcept;
int
memcmp(const void*, const void*, Size) noexcept;
void*
memcpy(void*, const void*, Size) noexcept;
Size
strlen(const char*) noexcept;

// 2010 math.h
// 2012 math.h
// 2019 corecrt_math.h
#if SIZE == 64
CRTIMP double
atan2f(float, float) noexcept;
CRTIMP float
ceilf(float) noexcept;
CRTIMP float
cosf(float) noexcept;
CRTIMP double
floorf(double) noexcept;
CRTIMP float
sinf(float) noexcept;
CRTIMP float
sqrtf(float) noexcept;
#else
double
atan2(double, double) noexcept;
CRTIMP double
ceil(double) noexcept;
double
cos(double) noexcept;
CRTIMP double
floor(double) noexcept;
double
sin(double) noexcept;
double
sqrt(double) noexcept;
inline float
atan2f(float y, float x) noexcept {
    return (float)atan2(y, x);
}
inline float
ceilf(float x) noexcept {
    return (float)ceil(x);
}
inline float
cosf(float x) noexcept {
    return (float)cos(x);
}
inline float
floorf(float x) noexcept {
    return (float)floor(x);
}
inline float
sinf(float x) noexcept {
    return (float)sin(x);
}
inline float
sqrtf(float x) noexcept {
    return (float)sqrt(x);
}
#endif

// 2010 stdlib.h
// 2012 stdlib.h
CRTIMP double
strtod(const char*, char**) noexcept;
CRTIMP long
strtol(const char*, char**, int) noexcept;
CRTIMP unsigned long
strtoul(const char*, char**, int) noexcept;

// 2010 stdlib.h
// 2012 stdlib.h
CRTIMP int*
_errno(void) noexcept;
}  // extern "C"
__pragma(pack(pop));

// os/memmove.cpp
extern "C" {
void*
memmem(const void*, Size, const void*, Size) noexcept;
}  // extern "C"

// 2010 string.h
// 2012 string.h
extern "C" {
void*
memmove(void*, const void*, Size) noexcept;
}  // extern "C"

// 2010 stdio.h
// 2012 stdio.h
#if MSVC == 2010 || MSVC == 2012 || MSVC == 2013
extern "C" {
CRTIMP int
printf(const char*, ...) noexcept;
CRTIMP int
fprintf(FILE*, const char*, ...) noexcept;
CRTIMP int
sprintf(char*, const char*, ...) noexcept;
}  // extern "C"
#elif MSVC >= 2015
int
printf(const char* const, ...) noexcept;
int
fprintf(FILE* const, const char* const, ...) noexcept;
int
sprintf(char* const, const char* const, ...) noexcept;
#endif

// 2010 string.h
// 2012 string.h
// 2019 vcruntime_string.h
extern "C" {
void*
memset(void*, int, Size) noexcept;
}  // extern "C"

#endif  // SRC_OS_WINDOWS_C_H_
