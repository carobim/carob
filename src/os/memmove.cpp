// Original source downloaded from:
// http://git.musl-libc.org/cgit/musl/plain/src/string/memmem.c

// **********
// Copyright 2005-2014 Rich Felker, et al.
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

#include "os/c.h"
#include "util/compiler.h"
#include "util/int.h"

extern "C" {

static char*
twoByteMemmem(const U8* h, Size k, const U8* n) noexcept {
    U16 nw = n[0] << 8 | n[1], hw = h[0] << 8 | h[1];
    for (h += 2, k -= 2; k; k--, hw = hw << 8 | *h++)
        if (hw == nw)
            return (char*)h - 2;
    return hw == nw ? (char*)h - 2 : 0;
}

static char*
threeByteMemmem(const U8* h, Size k, const U8* n) noexcept {
    U32 nw = n[0] << 24 | n[1] << 16 | n[2] << 8;
    U32 hw = h[0] << 24 | h[1] << 16 | h[2] << 8;
    for (h += 3, k -= 3; k; k--, hw = (hw | *h++) << 8)
        if (hw == nw)
            return (char*)h - 3;
    return hw == nw ? (char*)h - 3 : 0;
}

static char*
fourByteMemmem(const U8* h, Size k, const U8* n) noexcept {
    U32 nw = n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    U32 hw = h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
    for (h += 4, k -= 4; k; k--, hw = hw << 8 | *h++)
        if (hw == nw)
            return (char*)h - 4;
    return hw == nw ? (char*)h - 4 : 0;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BIT_OP(a, b, op)                           \
    ((a)[(Size)(b) / (8 * sizeof *(a))] op(Size) 1 \
     << ((Size)(b) % (8 * sizeof *(a))))

static char*
twoWayMemmem(const U8* h, const U8* z, const U8* n, Size l) noexcept {
    Size i, ip, jp, k, p, ms, p0, mem, mem0;
    Size byteSet[32 / sizeof(Size)] = {0};
    Size shift[256];

    /* Computing length of needle and fill shift table */
    for (i = 0; i < l; i++)
        BIT_OP(byteSet, n[i], |=), shift[n[i]] = i + 1;

    /* Compute maximal suffix */
    ip = -1;
    jp = 0;
    k = p = 1;
    while (jp + k < l) {
        if (n[ip + k] == n[jp + k]) {
            if (k == p) {
                jp += p;
                k = 1;
            }
            else
                k++;
        }
        else if (n[ip + k] > n[jp + k]) {
            jp += k;
            k = 1;
            p = jp - ip;
        }
        else {
            ip = jp++;
            k = p = 1;
        }
    }
    ms = ip;
    p0 = p;

    /* And with the opposite comparison */
    ip = -1;
    jp = 0;
    k = p = 1;
    while (jp + k < l) {
        if (n[ip + k] == n[jp + k]) {
            if (k == p) {
                jp += p;
                k = 1;
            }
            else
                k++;
        }
        else if (n[ip + k] < n[jp + k]) {
            jp += k;
            k = 1;
            p = jp - ip;
        }
        else {
            ip = jp++;
            k = p = 1;
        }
    }
    if (ip + 1 > ms + 1)
        ms = ip;
    else
        p = p0;

    /* Periodic needle? */
    if (memcmp(n, n + p, ms + 1)) {
        mem0 = 0;
        p = MAX(ms, l - ms - 1) + 1;
    }
    else
        mem0 = l - p;
    mem = 0;

    /* Search loop */
    for (;;) {
        /* If remainder of haystack is shorter than needle, done */
        if (static_cast<Size>(z - h) < l)
            return 0;

        /* Check last byte first; advance by shift on mismatch */
        if (BIT_OP(byteSet, h[l - 1], &)) {
            k = l - shift[h[l - 1]];
            if (k) {
                if (k < mem)
                    k = mem;
                h += k;
                mem = 0;
                continue;
            }
        }
        else {
            h += l;
            mem = 0;
            continue;
        }

        /* Compare right half */
        for (k = MAX(ms + 1, mem); k < l && n[k] == h[k]; k++)
            ;
        if (k < l) {
            h += k - ms;
            mem = 0;
            continue;
        }
        /* Compare left half */
        for (k = ms + 1; k > mem && n[k - 1] == h[k - 1]; k--)
            ;
        if (k <= mem)
            return (char*)h;
        h += p;
        mem = mem0;
    }
}

void*
memmem(const void* h0, Size k, const void* n0, Size l) noexcept {
    const U8* h = reinterpret_cast<const U8*>(h0);
    const U8* n = reinterpret_cast<const U8*>(n0);

    /* Return immediately on empty needle */
    if (!l) {
        return (void*)h;
    }

    /* Return immediately when needle is longer than haystack */
    if (k < l) {
        return 0;
    }

    /* Use faster algorithms for short needles */
    h = reinterpret_cast<const U8*>(memchr(h0, *n, k));
    if (!h || l == 1)
        return (void*)h;
    k -= h - (const U8*)h0;
    if (k < l)
        return 0;
    if (l == 2)
        return twoByteMemmem(h, k, n);
    if (l == 3)
        return threeByteMemmem(h, k, n);
    if (l == 4)
        return fourByteMemmem(h, k, n);

    return twoWayMemmem(h, h + k, n, l);
}
}
