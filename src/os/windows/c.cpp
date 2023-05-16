#include "os/windows/c.h"

#include "util/compiler.h"


#if MSVC >= 2015

__pragma(pack(push, 8));
extern "C" {
typedef struct {
    struct __crt_locale_data* locinfo;
    struct __crt_multibyte_data* mbcinfo;
}* _locale_t;
typedef char* va_list;

CRTIMP int
__stdio_common_vfprintf(unsigned __int64, FILE*, const char*, _locale_t,
                        va_list) noexcept;
CRTIMP int
__stdio_common_vsprintf(unsigned __int64, char*, Size, const char*, _locale_t,
                        va_list) noexcept;

#    if SIZE == 64
#        define _VA_ALIGN       8
#        define _SLOTSIZEOF(t)  ((sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1))
#        define _APALIGN(t, ap) (((va_list)0 - (ap)) & (__alignof(t) - 1))

void
__va_start(va_list*, ...);
#        define __crt_va_start_a(ap, v) \
            ((void)(__va_start(&ap, &v, _SLOTSIZEOF(v), __alignof(v), &v)))
#        define __crt_va_arg(ap, t)                                   \
            ((sizeof(t) > (2 * sizeof(__int64)))                      \
                 ? **(t**)((ap += sizeof(__int64)) - sizeof(__int64)) \
                 : *(t*)((ap += _SLOTSIZEOF(t) + _APALIGN(t, ap)) -   \
                         _SLOTSIZEOF(t)))
#        define __crt_va_end(ap) ((void)(ap = (va_list)0))
#    else
#        define _INTSIZEOF(n) \
            ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#        define __crt_va_start_a(ap, v) \
            ((void)(ap = (va_list)&v + _INTSIZEOF(v)))
#        define __crt_va_arg(ap, t) \
            (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#        define __crt_va_end(ap) ((void)(ap = (va_list)0))
#    endif
}
__pragma(pack(pop));

#    define __crt_va_start(ap, x)                                 __crt_va_start_a(ap, x)
#    define _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION (1ULL << 0)

__declspec(noinline) __inline unsigned __int64* __local_stdio_printf_options(
    void) noexcept {
    static unsigned __int64 _OptionsStorage;
    return &_OptionsStorage;
}

inline int
_vfprintf_l(FILE* const _Stream, const char* const _Format,
            const _locale_t _Locale, va_list _ArgList) noexcept {
    return __stdio_common_vfprintf(*__local_stdio_printf_options(), _Stream,
                                   _Format, _Locale, _ArgList);
}

inline int
_vsnprintf_l(char* const _Buffer, const Size _BufferCount,
             const char* const _Format, const _locale_t _Locale,
             va_list _ArgList) noexcept {
    const int _Result = __stdio_common_vsprintf(
        *__local_stdio_printf_options() |
            _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
        _Buffer, _BufferCount, _Format, _Locale, _ArgList);

    return _Result < 0 ? -1 : _Result;
}

inline int
_vsprintf_l(char* const _Buffer, const char* const _Format,
            const _locale_t _Locale, va_list _ArgList) noexcept {
    return _vsnprintf_l(_Buffer, (Size)-1, _Format, _Locale, _ArgList);
}

int
fprintf(FILE* const _Stream, const char* const _Format, ...) noexcept {
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);
    _Result = _vfprintf_l(_Stream, _Format, 0, _ArgList);
    __crt_va_end(_ArgList);
    return _Result;
}

int
printf(const char* const _Format, ...) noexcept {
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);
    _Result = _vfprintf_l(stdout, _Format, 0, _ArgList);
    __crt_va_end(_ArgList);
    return _Result;
}

int
sprintf(char* const _Buffer, const char* const _Format, ...) noexcept {
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);
    _Result = _vsprintf_l(_Buffer, _Format, 0, _ArgList);
    __crt_va_end(_ArgList);
    return _Result;
}

#endif
