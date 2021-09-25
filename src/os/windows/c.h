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
typedef int64_t LONGLONG;

typedef unsigned char BOOLEAN, BYTE;
typedef unsigned int UINT;
typedef unsigned long DWORD, *DWORD_PTR, ULONG, *LPDWORD;
typedef unsigned short WORD;
typedef uint64_t ULONGLONG;

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

#define CRTIMP       __declspec(dllimport)
#define WINAPI       __stdcall
#define WINBASEAPI   __declspec(dllimport)
#define WINUSERAPI   __declspec(dllimport)

// 2012 stdlib.h
#define errno (*_errno())

// 2012 stdio.h
typedef struct _iobuf {
    void* _Placeholder;
} FILE;
// 2012 time.h
typedef int64_t __time64_t;

__pragma(pack(push, 8));
extern "C" {
#if MSVC == 2012 || MSVC == 2013
// 2012 stdio.h
CRTIMP FILE*
__iob_func() noexcept;
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

// 2012 time.h
CRTIMP __time64_t
_time64(__time64_t*) noexcept;

// 2012 stdlib.h
//CRTIMP __declspec(noreturn) void exit(int) noexcept;
CRTIMP int
rand(void) noexcept;
CRTIMP void
srand(unsigned int) noexcept;

// 2012 string.h
void*
memchr(const void*, int, size_t) noexcept;
int
memcmp(void const*, void const*, size_t) noexcept;
void*
memcpy(void*, void const*, size_t) noexcept;
size_t
strlen(char const*) noexcept;

// 2012 math.h
int
abs(int) noexcept;
double
atan2(double, double) noexcept;
CRTIMP double
ceil(double) noexcept;
#if SIZE == 64
CRTIMP float
ceilf(float) noexcept;
#else
inline float
ceilf(float x) noexcept {
    return (float)ceil(x);
}
#endif
double
cos(double) noexcept;
CRTIMP double
floor(double) noexcept;
double
sin(double) noexcept;
double
sqrt(double) noexcept;

// 2012 stdlib.h
CRTIMP double
strtod(char const*, char**) noexcept;
CRTIMP long
strtol(char const*, char**, int) noexcept;
CRTIMP unsigned long
strtoul(char const*, char**, int) noexcept;

// 2012 stdlib.h
CRTIMP int*
_errno(void);
}  // extern "C"
__pragma(pack(pop));

extern "C" {
void*
memmem(const void*, size_t, const void*, size_t) noexcept;
}  // extern "C"

// 2012 string.h
extern "C" {
void*
memmove(void*, const void*, size_t) noexcept;
}  // extern "C"

// 2012 stdio.h
#if MSVC == 2012 || MSVC == 2013
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
printf(char const* const, ...) noexcept;
int
fprintf(FILE* const, char const* const, ...) noexcept;
int
sprintf(char* const, char const* const, ...) noexcept;
#endif

// 2012 string.h
// 2019 vcruntime_string.h
extern "C" {
void*
memset(void*, int, size_t);
}  // extern "C"

#endif  // SRC_OS_WINDOWS_C_H_