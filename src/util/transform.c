/*************************************
** Tsunagari Tile Engine            **
** transform.c                      **
** Copyright 2017-2021 Paul Merrill **
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

#include "transform.h"

struct Transform
transformIdentity() {
    struct Transform result;

    result.m[0] = 1;
    result.m[1] = 0;
    result.m[2] = 0;
    result.m[3] = 0;
    result.m[4] = 0;
    result.m[5] = 1;
    result.m[6] = 0;
    result.m[7] = 0;
    result.m[8] = 0;
    result.m[9] = 0;
    result.m[10] = 1;
    result.m[11] = 0;
    result.m[12] = 0;
    result.m[13] = 0;
    result.m[14] = 0;
    result.m[15] = 1;

    return result;
}

struct Transform
transformScale(float x, float y) {
    struct Transform result;

    result.m[0] = x;
    result.m[1] = 0;
    result.m[2] = 0;
    result.m[3] = 0;
    result.m[4] = 0;
    result.m[5] = y;
    result.m[6] = 0;
    result.m[7] = 0;
    result.m[8] = 0;
    result.m[9] = 0;
    result.m[10] = 1;
    result.m[11] = 0;
    result.m[12] = 0;
    result.m[13] = 0;
    result.m[14] = 0;
    result.m[15] = 1;

    return result;
}

struct Transform
transformTranslate(float x, float y) {
    struct Transform result;

    result.m[0] = 1;
    result.m[1] = 0;
    result.m[2] = 0;
    result.m[3] = 0;
    result.m[4] = 0;
    result.m[5] = 1;
    result.m[6] = 0;
    result.m[7] = 0;
    result.m[8] = 0;
    result.m[9] = 0;
    result.m[10] = 1;
    result.m[11] = 0;
    result.m[12] = x;
    result.m[13] = y;
    result.m[14] = 0;
    result.m[15] = 1;

    return result;
}

struct Transform
transformMultiply(struct Transform a, struct Transform b) {
    struct Transform result;
    int i;

    for (i = 0; i < 16; i++) {
        result.m[i] = a.m[(i & ~3)    ] * b.m[(i & 3)     ] +
                      a.m[(i & ~3) | 1] * b.m[(i & 3) |  4] +
                      a.m[(i & ~3) | 2] * b.m[(i & 3) |  8] +
                      a.m[ i       | 3] * b.m[(i & 3) | 12];
    }

    return result;
}
