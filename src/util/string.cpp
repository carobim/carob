/*************************************
** Tsunagari Tile Engine            **
** string.cpp                       **
** Copyright 2019-2020 Paul Merrill **
**************************************/

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

// Original source downloaded from:
// http://git.musl-libc.org/cgit/musl/plain/src/stdio/vfprintf.c

// **********
// Copyright 2005-2020 Rich Felker, et al.
//
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


#include "util/string.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/fnv.h"
#include "util/new.h"
#include "util/noexcept.h"

static size_t
grow1(size_t current) noexcept {
    return current == 0 ? 4 : current * 2;
}

static size_t
growN(size_t current, size_t addition) noexcept {
    size_t newSize = current == 0 ? 4 : current * 2;
    while (newSize < current + addition) {
        newSize *= 2;
    }
    return newSize;
}

static char*
fmtI(int64_t x, char* s) noexcept {
    s = fmtU(x < 0 ? -x : x, s);
    if (x < 0) {
        *--s = '-';
    }
    return s;
}

static char*
fmtU(uint64_t x, char* s) noexcept {
    if (sizeof(size_t) == 4) {
        for (; x > UINT32_MAX; x /= 10) {
            *--s = '0' + x % 10;
        }
        for (uint32_t y = x; y; y /= 10) {
            *--s = '0' + y % 10;
        }
    }
    else {
        for (; x; x /= 10) {
            *--s = '0' + x % 10;
        }
    }
    return s;
}

static void
pad(FILE* f, char c, int w, int l, int fl) noexcept {
    char pad[256];
    if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w) {
        return;
    }
    l = w - l;
    memset(pad, c, l > sizeof pad ? sizeof pad : l);
    for (; l >= sizeof pad; l -= sizeof pad) {
        write(f, pad, sizeof pad);
    }
    write(f, pad, l);
}

#ifdef _MSC_VER
// MSVC is unique in limiting long doubles to be the same as doubles.
#define LDBL_EPSILON double(2.22044604925031308084726333618164062e-16L)
#define LDBL_MANT_DIG 53
#define LDBL_MAX_EXP 1024
#else
#define LDBL_EPSILON 1.08420217248550443400745280086994171e-19L
#define LDBL_MANT_DIG 64
#define LDBL_MAX_EXP 16384
#endif

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
union ldshape {
    long double f;
    struct {
        uint64_t m;
        uint16_t se;
    } i;
};
static int
__signbitl(long double x) noexcept {
    union ldshape u = {x};
    return u.i.se >> 15;
}
#elif LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
static int
__signbitl(long double x) noexcept {
    return __signbit(x);
}
#endif

#define signbit(x) ( \
    sizeof(x) == sizeof(float) ? (int)(__FLOAT_BITS(x)>>31) : \
    sizeof(x) == sizeof(double) ? (int)(__DOUBLE_BITS(x)>>63) : \
    __signbitl(x) )

static int
fmtF(FILE* f,
     long double y,
     int w = 0,
     int p = -1,
     int fl = 0,
     int t = 'f') noexcept {
    // In "%3f", w = 3
    // In "%.3f", p = 3
    // In "%03f", fl & ZERO_PAD = ZERO_PAD
    uint32_t big[(LDBL_MANT_DIG + 28) / 29 + 1  // mantissa expansion
                 + (LDBL_MAX_EXP + LDBL_MANT_DIG + 28 + 8) /
                           9];  // exponent expansion
    uint32_t *a, *d, *r, *z;
    int e2 = 0, e, i, j, l;
    char buf[9 + LDBL_MANT_DIG / 4], *s;
    const char* prefix = "-0X+0X 0X-0x+0x 0x";
    int pl;
    char ebuf0[3 * sizeof(int)], *ebuf = &ebuf0[3 * sizeof(int)], *estr;

    pl = 1;
    if (signbit(y)) {
        y = -y;
    }
    else if (fl & MARK_POS) {
        prefix += 3;
    }
    else if (fl & PAD_POS) {
        prefix += 6;
    }
    else
        prefix++, pl = 0;

    if (!isfinite(y)) {
        char* s = (t & 32) ? "inf" : "INF";
        if (y != y)
            s = (t & 32) ? "nan" : "NAN";
        pad(f, ' ', w, 3 + pl, fl & ~ZERO_PAD);
        write(f, prefix, pl);
        write(f, s, 3);
        pad(f, ' ', w, 3 + pl, fl ^ LEFT_ADJ);
        return MAX(w, 3 + pl);
    }

    y = frexpl(y, &e2) * 2;
    if (y)
        e2--;

    if ((t | 32) == 'a') {
        long double round = 8.0;
        int re;

        if (t & 32)
            prefix += 9;
        pl += 2;

        if (p < 0 || p >= LDBL_MANT_DIG / 4 - 1)
            re = 0;
        else
            re = LDBL_MANT_DIG / 4 - 1 - p;

        if (re) {
            round *= 1 << (LDBL_MANT_DIG % 4);
            while (re--)
                round *= 16;
            if (*prefix == '-') {
                y = -y;
                y -= round;
                y += round;
                y = -y;
            }
            else {
                y += round;
                y -= round;
            }
        }

        estr = fmt_u(e2 < 0 ? -e2 : e2, ebuf);
        if (estr == ebuf)
            *--estr = '0';
        *--estr = (e2 < 0 ? '-' : '+');
        *--estr = t + ('p' - 'a');

        s = buf;
        do {
            int x = y;
            *s++ = xdigits[x] | (t & 32);
            y = 16 * (y - x);
            if (s - buf == 1 && (y || p > 0 || (fl & ALT_FORM)))
                *s++ = '.';
        } while (y);

        if (p > INT32_MAX - 2 - (ebuf - estr) - pl)
            return -1;
        if (p && s - buf - 2 < p)
            l = (p + 2) + (ebuf - estr);
        else
            l = (s - buf) + (ebuf - estr);

        pad(f, ' ', w, pl + l, fl);
        write(f, prefix, pl);
        pad(f, '0', w, pl + l, fl ^ ZERO_PAD);
        write(f, buf, s - buf);
        pad(f, '0', l - (ebuf - estr) - (s - buf), 0, 0);
        write(f, estr, ebuf - estr);
        pad(f, ' ', w, pl + l, fl ^ LEFT_ADJ);
        return MAX(w, pl + l);
    }
    if (p < 0)
        p = 6;

    if (y)
        y *= 0x1p28, e2 -= 28;

    if (e2 < 0)
        a = r = z = big;
    else
        a = r = z = big + sizeof(big) / sizeof(*big) - LDBL_MANT_DIG - 1;

    do {
        *z = y;
        y = 1000000000 * (y - *z++);
    } while (y);

    while (e2 > 0) {
        uint32_t carry = 0;
        int sh = MIN(29, e2);
        for (d = z - 1; d >= a; d--) {
            uint64_t x = ((uint64_t)*d << sh) + carry;
            *d = x % 1000000000;
            carry = x / 1000000000;
        }
        if (carry)
            *--a = carry;
        while (z > a && !z[-1])
            z--;
        e2 -= sh;
    }
    while (e2 < 0) {
        uint32_t carry = 0, *b;
        int sh = MIN(9, -e2), need = 1 + (p + LDBL_MANT_DIG / 3U + 8) / 9;
        for (d = a; d < z; d++) {
            uint32_t rm = *d & (1 << sh) - 1;
            *d = (*d >> sh) + carry;
            carry = (1000000000 >> sh) * rm;
        }
        if (!*a)
            a++;
        if (carry)
            *z++ = carry;
        /* Avoid (slow!) computation past requested precision */
        b = (t | 32) == 'f' ? r : a;
        if (z - b > need)
            z = b + need;
        e2 += sh;
    }

    if (a < z)
        for (i = 10, e = 9 * (r - a); *a >= i; i *= 10, e++)
            ;
    else
        e = 0;

    /* Perform rounding: j is precision after the radix (possibly neg) */
    j = p - ((t | 32) != 'f') * e - ((t | 32) == 'g' && p);
    if (j < 9 * (z - r - 1)) {
        uint32_t x;
        /* We avoid C's broken division of negative numbers */
        d = r + 1 + ((j + 9 * LDBL_MAX_EXP) / 9 - LDBL_MAX_EXP);
        j += 9 * LDBL_MAX_EXP;
        j %= 9;
        for (i = 10, j++; j < 9; i *= 10, j++)
            ;
        x = *d % i;
        /* Are there any significant digits past j? */
        if (x || d + 1 != z) {
            long double round = 2 / LDBL_EPSILON;
            long double small;
            if ((*d / i & 1) || (i == 1000000000 && d > a && (d[-1] & 1)))
                round += 2;
            if (x < i / 2)
                small = 0x0.8p0;
            else if (x == i / 2 && d + 1 == z)
                small = 0x1.0p0;
            else
                small = 0x1.8p0;
            if (pl && *prefix == '-')
                round *= -1, small *= -1;
            *d -= x;
            /* Decide whether to round by probing round+small */
            if (round + small != round) {
                *d = *d + i;
                while (*d > 999999999) {
                    *d-- = 0;
                    if (d < a)
                        *--a = 0;
                    (*d)++;
                }
                for (i = 10, e = 9 * (r - a); *a >= i; i *= 10, e++)
                    ;
            }
        }
        if (z > d + 1)
            z = d + 1;
    }
    for (; z > a && !z[-1]; z--)
        ;

    if ((t | 32) == 'g') {
        if (!p)
            p++;
        if (p > e && e >= -4) {
            t--;
            p -= e + 1;
        }
        else {
            t -= 2;
            p--;
        }
        if (!(fl & ALT_FORM)) {
            /* Count trailing zeros in last place */
            if (z > a && z[-1])
                for (i = 10, j = 0; z[-1] % i == 0; i *= 10, j++)
                    ;
            else
                j = 9;
            if ((t | 32) == 'f')
                p = MIN(p, MAX(0, 9 * (z - r - 1) - j));
            else
                p = MIN(p, MAX(0, 9 * (z - r - 1) + e - j));
        }
    }
    if (p > INT_MAX - 1 - (p || (fl & ALT_FORM)))
        return -1;
    l = 1 + p + !!p;
    if (t == 'f') {
        if (e > INT32_MAX - l) {
            return -1;
        }
        if (e > 0) {
            l += e;
        }
    }
    else {
        estr = fmt_u(e < 0 ? -e : e, ebuf);
        while (ebuf - estr < 2)
            *--estr = '0';
        *--estr = (e < 0 ? '-' : '+');
        *--estr = t;
        if (ebuf - estr > INT_MAX - l)
            return -1;
        l += ebuf - estr;
    }

    if (l > INT32_MAX - pl)
        return -1;
    pad(f, ' ', w, pl + l, fl);
    write(f, prefix, pl);
    pad(f, '0', w, pl + l, fl ^ ZERO_PAD);

    if ((t | 32) == 'f') {
        if (a > r)
            a = r;
        for (d = a; d <= r; d++) {
            char* s = fmtU(*d, buf + 9);
            if (d != a)
                while (s > buf)
                    *--s = '0';
            else if (s == buf + 9)
                *--s = '0';
            write(f, s, buf + 9 - s);
        }
        if (p)
            write(f, ".", 1);
        for (; d < z && p > 0; d++, p -= 9) {
            char* s = fmtU(*d, buf + 9);
            while (s > buf)
                *--s = '0';
            write(f, s, MIN(9, p));
        }
        pad(f, '0', p + 9, 9, 0);
    }
    else {
        if (z <= a)
            z = a + 1;
        for (d = a; d < z && p >= 0; d++) {
            char* s = fmtU(*d, buf + 9);
            if (s == buf + 9)
                *--s = '0';
            if (d != a)
                while (s > buf)
                    *--s = '0';
            else {
                write(f, s++, 1);
            }
            write(f, s, MIN(buf + 9 - s, p));
            p -= buf + 9 - s;
        }
        pad(f, '0', p + 18, 18, 0);
        write(f, estr, ebuf - estr);
    }

    return MAX(w, pl + l);
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
    free(data);
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

void
String::operator=(StringView s) noexcept {
    free(data);
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

void
String::operator=(const String& s) noexcept {
    free(data);
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

void
String::operator=(String&& s) noexcept {
    free(data);
    data = s.data;
    size = s.size;
    capacity = s.capacity;
    s.data = 0;
    s.size = s.capacity = 0;
}

char& String::operator[](size_t i) noexcept {
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
    if (s == 0) {
        return *this << "(null)";
    }

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
    return *this << (b ? "true" : "false");
}

String&
String::operator<<(int32_t i) noexcept {
    char buf[64];
    return *this << fmtI(i, buf + sizeof(buf));
}

String&
String::operator<<(uint32_t u) noexcept {
    char buf[64];
    return *this << fmtU(u, buf + sizeof(buf));
}

String&
String::operator<<(int64_t i) noexcept {
    char buf[64];
    return *this << fmtI(i, buf + sizeof(buf));
}

String&
String::operator<<(uint64_t u) noexcept {
    char buf[64];
    return *this << fmtU(u, buf + sizeof(buf));
}

String&
String::operator<<(float f) noexcept {
    char buf[64];
    sprintf(buf, "%f", f);
    return *this << buf;
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
