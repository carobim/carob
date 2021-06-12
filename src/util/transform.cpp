/*************************************
** Tsunagari Tile Engine            **
** transform.cpp                    **
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

#include "util/noexcept.h"

Transform
transformScale(float factor) noexcept {
    return {{factor, 0, 0, 0, 0, factor, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
}

Transform
transformScale(float x, float y) noexcept {
    return {{x, 0, 0, 0, 0, y, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
}

Transform
transformTranslate(float x, float y) noexcept {
    return {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, 0, 1}};
}

Transform
operator*(Transform a, Transform b) noexcept {
    Transform result;
    for (int i = 0; i < 16; i++) {
        result.m[i] = a.m[(i & ~3)    ] * b.m[(i & 3)     ] +
                      a.m[(i & ~3) | 1] * b.m[(i & 3) |  4] +
                      a.m[(i & ~3) | 2] * b.m[(i & 3) |  8] +
                      a.m[ i       | 3] * b.m[(i & 3) | 12];
    }
    return result;
}
