#ifndef SRC_UTIL_MATH2_H_
#define SRC_UTIL_MATH2_H_

// pow2()
//
// Original source downloaded from:
//   https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

/*
 * Individually, the code snippets here are in the public domain (unless
 * otherwise noted)--feel free to use them however you please. The aggregate
 * collection and descriptions are (c) 1997-2005 Sean Eron Anderson. The code
 * and descriptions are distributed in the hope that they will be useful, but
 * WITHOUT ANY WARRANTY and without even the implied warranty of
 * merchantability or fitness for a particular purpose.
 */

#include "util/compiler.h"
#include "util/int.h"

template<typename T>
static T
max(T a, T b) noexcept {
    return a < b ? b : a;
}

template<typename T>
static T
min(T a, T b) noexcept {
    return a < b ? a : b;
}

template<typename T>
static T
bound(T value, T min, T max) noexcept {
    if (value < min) {
        return min;
    }
    else if (value > max) {
        return max;
    }
    else {
        return value;
    }
}

template<typename T>
static T
wrap(T min, T value, T max) noexcept {
    while (value < min) {
        value += max;
    }
    return value % max;
}

// Rounds a number up to the nearest power of 2, or to 0, whichever is lower.
static inline U32
pow2(U32 i) noexcept {
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i++;
    return i;
}
static inline U64
pow2(U64 i) noexcept {
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i |= i >> 32;
    i++;
    return i;
}

// FIXME: Why does Apple need this?
#if defined(__APPLE__)
static inline Size
pow2(Size i) noexcept {
    return pow2(static_cast<U64>(i));
}
#endif

template<typename T>
static T
align32(T x) {
    return (x + 31) & 0xffffffe0;
}

template<typename T>
static T
align64(T x) {
    return (x + 63) & 0xffffffc0;
}

#endif  // SRC_UTIL_MATH2_H_
