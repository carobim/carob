/***************************************
** Tsunagari Tile Engine              **
** vec.h                              **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2021 Paul Merrill   **
***************************************/

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

#ifndef SRC_CORE_VEC_H_
#define SRC_CORE_VEC_H_

#include "util/constexpr.h"
#include "util/int.h"
#include "util/noexcept.h"

// clang-format off

struct ivec2 { int x, y; };
struct ivec3 { int x, y, z; };
struct fvec2 { float x, y; };
struct fvec3 { float x, y, z; };

/**
 * Virtual integer coordinate.
 *
 * x and y are the same as a physical integer coordinate.
 * z is a virtual layer depth within an Area.
 */
struct vicoord {
    int x, y;
    float z;
};

struct icube {
    int x1, y1, z1;
    int x2, y2, z2;
};

ivec2 operator+(ivec2, ivec2) noexcept;
ivec3 operator+(ivec3, ivec3) noexcept;
fvec2 operator+(fvec2, fvec2) noexcept;
fvec3 operator+(fvec3, fvec3) noexcept;

ivec2 operator-(ivec2, ivec2) noexcept;
ivec3 operator-(ivec3, ivec3) noexcept;
fvec2 operator-(fvec2, fvec2) noexcept;
fvec3 operator-(fvec3, fvec3) noexcept;

ivec2 operator*(ivec2, ivec2) noexcept;
ivec2 operator*(int, ivec2) noexcept;
ivec3 operator*(int, ivec3) noexcept;
fvec2 operator*(fvec2, fvec2) noexcept;
fvec2 operator*(float, fvec2) noexcept;
fvec3 operator*(float, fvec3) noexcept;

ivec2 operator*(ivec2, int) noexcept;
ivec3 operator*(ivec3, int) noexcept;
fvec2 operator*(fvec2, float) noexcept;
fvec3 operator*(fvec3, float) noexcept;

ivec2 operator/(ivec2, int) noexcept;
ivec3 operator/(ivec3, int) noexcept;
fvec2 operator/(fvec2, float) noexcept;
fvec3 operator/(fvec3, float) noexcept;

bool operator==(ivec2, ivec2) noexcept;
bool operator==(ivec3, ivec3) noexcept;
bool operator==(fvec2, fvec2) noexcept;
bool operator==(fvec3, fvec3) noexcept;

bool operator!=(ivec2, ivec2) noexcept;
bool operator!=(ivec3, ivec3) noexcept;
bool operator!=(fvec2, fvec2) noexcept;
bool operator!=(fvec3, fvec3) noexcept;

float distanceTo(fvec2, fvec2) noexcept;
float distanceTo(fvec3, fvec3) noexcept;

size_t hash_(ivec2) noexcept;
size_t hash_(ivec3) noexcept;
size_t hash_(fvec2) noexcept;
size_t hash_(fvec3) noexcept;

static const CONSTEXPR11 ivec3 IVEC3_MIN = {
    INT32_MIN,
    INT32_MIN,
    INT32_MIN
};

// clang-format on

#endif  // SRC_CORE_VEC_H_
