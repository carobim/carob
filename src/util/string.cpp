/*************************************
** Tsunagari Tile Engine            **
** string.cpp                       **
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

// For additional copyright information pertaining to this file, please refer
// to musl's COPYRIGHT file.

#include "util/string.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/fnv.h"
#include "util/new.h"
#include "util/noexcept.h"

#if __LDBL_MANT_DIG__ == 53 || __LDBL_MANT_DIG__ == 113
#error Not implemented
#endif

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

static inline unsigned
__FLOAT_BITS(float __f) noexcept
{
    union {float __f; unsigned __i;} __u;
    __u.__f = __f;
    return __u.__i;
}
static inline unsigned long long
__DOUBLE_BITS(double __f) noexcept
{
    union {double __f; unsigned long long __i;} __u;
    __u.__f = __f;
    return __u.__i;
}

union ldshape {
    long double f;
    struct {
        uint64_t m;
        uint16_t se;
    } i;
};

static int
__fpclassifyl(long double x) noexcept
{
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    int msb = u.i.m>>63;
    if (!e && !msb)
        return u.i.m ? FP_SUBNORMAL : FP_ZERO;
    if (e == 0x7fff) {
        /* The x86 variant of 80-bit extended precision only admits
         * one representation of each infinity, with the mantissa msb
         * necessarily set. The version with it clear is invalid/nan.
         * The m68k variant, however, allows either, and tooling uses
         * the version with it clear. */
        if (!msb)
            return FP_NAN;
        return u.i.m << 1 ? FP_NAN : FP_INFINITE;
    }
    if (!msb)
        return FP_NAN;
    return FP_NORMAL;
}

static int
__signbitl(long double x) noexcept
{
    union ldshape u = {x};
    return u.i.se >> 15;
}

static long double
frexpl(long double x, int *e) noexcept
{
    union ldshape u = {x};
    int ee = u.i.se & 0x7fff;

    if (!ee) {
        if (x) {
            x = frexpl(x*0x1p120, e);
            *e -= 120;
        } else *e = 0;
        return x;
    } else if (ee == 0x7fff) {
        return x;
    }

    *e = ee - 0x3ffe;
    u.i.se &= 0x8000;
    u.i.se |= 0x3ffe;
    return u.f;
}

#define isfinite(x) ( \
    sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) < 0x7f800000 : \
    sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL>>1) < 0x7ffULL<<52 : \
    __fpclassifyl(x) > FP_INFINITE)

#define signbit(x) ( \
    sizeof(x) == sizeof(float) ? (int)(__FLOAT_BITS(x)>>31) : \
    sizeof(x) == sizeof(double) ? (int)(__DOUBLE_BITS(x)>>63) : \
    __signbitl(x) )


#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

static char*
fmt_u(uint32_t x, char *s) noexcept
{
    for (; x; x/=10) {
        *--s = '0' + x%10;
    }
    return s;
}

struct Buf {
    char buf[64];
    char* p;  // Needs to be initialized to &buf[0].
};

static void
out(Buf *f, const char *s, size_t l) noexcept
{
    static_cast<char*>(memcpy(f->p, s, l));
    f->p += l;
}

static const char xdigits[17] = "0123456789ABCDEF";

// w = 0 (no padding)
// p = -1 (auto precision)
// fl = 0 (misc. flags)
static int
fmt_fp(Buf *f, long double y, int w = 0, int p = -1, int fl = 0) noexcept
{
    uint32_t big[(__LDBL_MANT_DIG__+28)/29 + 1          // mantissa expansion
        + (__LDBL_MAX_EXP__+__LDBL_MANT_DIG__+28+8)/9]; // exponent expansion
    uint32_t *a, *d, *r, *z;
    int e2=0, e, i, j, l;
    char buf[9+__LDBL_MANT_DIG__/4], *s;
    const char *prefix="-0X+0X 0X-0x+0x 0x";
    int pl;
    char ebuf0[3*sizeof(int)], *ebuf=&ebuf0[3*sizeof(int)], *estr;

    pl=1;
    if (signbit(y)) {
        y=-y;
    } else prefix++, pl=0;

    if (!isfinite(y)) {
        const char *s = y == y ? "inf" : "nan";
        out(f, "-", pl);
        out(f, s, 3);
        return MAX(w, 3+pl);
    }

    y = frexpl(y, &e2) * 2;
    if (y) e2--;

    if (p<0) p=6;

    if (y) y *= 0x1p28, e2-=28;

    if (e2<0) a=r=z=big;
    else a=r=z=big+sizeof(big)/sizeof(*big) - __LDBL_MANT_DIG__ - 1;

    do {
        *z = y;
        y = 1000000000*(y-*z++);
    } while (y);

    while (e2>0) {
        uint32_t carry=0;
        int sh=MIN(29,e2);
        for (d=z-1; d>=a; d--) {
            uint64_t x = ((uint64_t)*d<<sh)+carry;
            *d = x % 1000000000;
            carry = x / 1000000000;
        }
        if (carry) *--a = carry;
        while (z>a && !z[-1]) z--;
        e2-=sh;
    }
    while (e2<0) {
        uint32_t carry=0, *b;
        int sh=MIN(9,-e2), need=1+(p+__LDBL_MANT_DIG__/3U+8)/9;
        for (d=a; d<z; d++) {
            uint32_t rm = *d & (1<<sh)-1;
            *d = (*d>>sh) + carry;
            carry = (1000000000>>sh) * rm;
        }
        if (!*a) a++;
        if (carry) *z++ = carry;
        /* Avoid (slow!) computation past requested precision */
        b = r;
        if (z-b > need) z = b+need;
        e2+=sh;
    }

    if (a<z) for (i=10, e=9*(r-a); *a>=i; i*=10, e++);
    else e=0;

    /* Perform rounding: j is precision after the radix (possibly neg) */
    j = p;
    if (j < 9*(z-r-1)) {
        uint32_t x;
        /* We avoid C's broken division of negative numbers */
        d = r + 1 + ((j+9*__LDBL_MAX_EXP__)/9 - __LDBL_MAX_EXP__);
        j += 9*__LDBL_MAX_EXP__;
        j %= 9;
        for (i=10, j++; j<9; i*=10, j++);
        x = *d % i;
        /* Are there any significant digits past j? */
        if (x || d+1!=z) {
            long double round = 2/__LDBL_EPSILON__;
            long double small;
            if ((*d/i & 1) || (i==1000000000 && d>a && (d[-1]&1)))
                round += 2;
            if (x<i/2) small=0x0.8p0;
            else if (x==i/2 && d+1==z) small=0x1.0p0;
            else small=0x1.8p0;
            if (pl && *prefix=='-') round*=-1, small*=-1;
            *d -= x;
            /* Decide whether to round by probing round+small */
            if (round+small != round) {
                *d = *d + i;
                while (*d > 999999999) {
                    *d--=0;
                    if (d<a) *--a=0;
                    (*d)++;
                }
                for (i=10, e=9*(r-a); *a>=i; i*=10, e++);
            }
        }
        if (z>d+1) z=d+1;
    }
    for (; z>a && !z[-1]; z--);

    if (p > INT32_MAX-1-(p != 0))
        return -1;
    l = 1 + p + (p != 0);
    if (e > INT32_MAX-l) return -1;
    if (e>0) l+=e;

    if (l > INT32_MAX-pl) return -1;
    out(f, prefix, pl);

    if (a>r) a=r;
    for (d=a; d<=r; d++) {
        char *s = fmt_u(*d, buf+9);
        if (d!=a) while (s>buf) *--s='0';
        else if (s==buf+9) *--s='0';
        out(f, s, buf+9-s);
    }
    if (d<z && p>0) {
        if (p) out(f, ".", 1);
    }
    else {
        l -= 1;
    }
    for (; d<z && p>0; d++, p-=9) {
        char *s = fmt_u(*d, buf+9);
        while (s>buf) *--s='0';
        out(f, s, MIN(9,p));
    }

    return MAX(w, pl+l);
}



static size_t
grow1(size_t current) noexcept {
    return current < 4 ? 4 : current * 2;
}

static size_t
growN(size_t current, size_t addition) noexcept {
    size_t newSize = current == 0 ? 4 : current * 2;
    while (newSize < current + addition) {
        newSize *= 2;
    }
    return newSize;
}

String::String() noexcept : data(0), size(0), capacity(0) {}

String::String(const char* s) noexcept {
    if (s == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        size_t len = strlen(s);
        // FIXME: Choose better size.
        data = static_cast<char*>(malloc(len));
        size = capacity = len;
        memcpy(data, s, len);
    }
}

String::String(StringView s) noexcept {
    if (s.size == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        // FIXME: Choose better size.
        data = static_cast<char*>(malloc(s.size));
        size = capacity = s.size;
        memcpy(data, s.data, s.size);
    }
}

String::String(const String& s) noexcept {
    if (s.capacity == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        data = static_cast<char*>(malloc(s.capacity));
        size = s.size;
        capacity = s.capacity;
        memcpy(data, s.data, size);
    }
}

String::String(String&& s) noexcept {
    data = s.data;
    size = s.size;
    capacity = s.capacity;
    s.data = 0;
    s.size = s.capacity = 0;
}

String::~String() noexcept {
    free(data);
}

void
String::operator=(const char* s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(StringView s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(const String& s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(String&& s) noexcept {
    free(data);
    data = s.data;
    size = s.size;
    capacity = s.capacity;
    s.data = 0;
    s.size = s.capacity = 0;
}

char&
String::operator[](size_t i) noexcept {
    assert_(i < size);
    return data[i];
}

String&
String::operator<<(char c) noexcept {
    if (size == capacity) {
        reserve(grow1(size));
    }
    data[size++] = c;
    return *this;
}

String&
String::operator<<(const char* s) noexcept {
    // TODO: Use strncpy without knowing the size. If the string has more
    //       characters, then do a strlen & growN & memcpy like below.
    size_t len = strlen(s);
    if (capacity < size + len) {
        reserve(growN(size, len));
    }
    memcpy(data + size, s, len);
    size += len;
    return *this;
}

String&
String::operator<<(StringView s) noexcept {
    if (capacity < size + s.size) {
        reserve(growN(size, s.size));
    }
    memcpy(data + size, s.data, s.size);
    size += s.size;
    return *this;
}

String&
String::operator<<(bool b) noexcept {
    if (capacity < size + 5) {
        reserve(growN(size, 5));
    }
    if (b) {
        memcpy(data + size, "true", 4);
        size += 4;
    }
    else {
        memcpy(data + size, "false", 5);
        size += 5;
    }
    return *this;
}

String&
String::operator<<(int i) noexcept {
    // Minus sign & 10 digits.
    if (capacity < size + 11) {
        reserve(growN(size, 11));
    }

    if (i < 0) {
        if (i == INT32_MIN) {
            memcpy(data + size, "-2147483648", 11);
            size += 11;
            return *this;
        }

        data[size++] = '-';
        i = -i;
    }

    char buf[10];
    char* p = buf;

    do {
        *p++ = static_cast<char>(i % 10) + '0';
        i /= 10;
    } while (i > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(unsigned int u) noexcept {
    if (capacity < size + 10) {
        reserve(growN(size, 10));
    }

    char buf[10];
    char* p = buf;

    do {
        *p++ = static_cast<char>(u % 10) + '0';
        u /= 10;
    } while (u > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(long l) noexcept {
#ifdef _WIN32
    return *this << static_cast<int>(l);
#else
    return *this << static_cast<long long>(l);
#endif
}

String&
String::operator<<(unsigned long ul) noexcept {
#ifdef _WIN32
    return *this << static_cast<unsigned int>(ul);
#else
    return *this << static_cast<unsigned long long>(ul);
#endif
}

String&
String::operator<<(long long ll) noexcept {
    if (capacity < size + 20) {
        reserve(growN(size, 20));
    }

    if (ll < 0) {
        if (ll == INT64_MIN) {
            memcpy(data + size, "-9223372036854775808", 20);
            size += 20;
            return *this;
        }

        data[size++] = '-';
        ll = -ll;
    }

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(ll % 10) + '0';
        ll /= 10;
    } while (ll > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(unsigned long long ull) noexcept {
    if (capacity < size + 20) {
        reserve(growN(size, 20));
    }

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(ull % 10) + '0';
        ull /= 10;
    } while (ull > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(float f) noexcept {
    Buf b;
    b.p = b.buf;

    fmt_fp(&b, f);

    *b.p = 0;

    return *this << b.buf;
}

void
String::reserve(size_t n) noexcept {
    assert_(capacity < n);
    char* newData = static_cast<char*>(malloc(n));
    memmove(newData, data, size);
    data = newData;
    capacity = n;
}

void
String::resize(size_t n) noexcept {
    if (capacity < n) {
        reserve(n);
    }
    size = n;
}

void
String::clear() noexcept {
    size = 0;
}

void
String::reset() noexcept {
    data = 0;
    size = capacity = 0;
}

String::operator StringView() const noexcept {
    return StringView(data, size);
}

StringView
String::view() const noexcept {
    return StringView(data, size);
}

const char*
String::null() noexcept {
    if (size == capacity) {
        reserve(grow1(size));
    }
    data[size] = 0;
    return data;
}

bool
operator<(const String& a, const String& b) noexcept {
    return a.view() < b.view();
}

bool
operator>(const String& a, const String& b) noexcept {
    return a.view() > b.view();
}

size_t
hash_(const String& s) noexcept {
    return fnvHash(s.data, s.size);
}
