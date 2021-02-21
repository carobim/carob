/***************************************
** Tsunagari Tile Engine              **
** vec.cpp                            **
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

#include "core/vec.h"

#include "os/c.h"
#include "util/fnv.h"
#include "util/int.h"
#include "util/noexcept.h"

ivec2
operator+(ivec2 a, ivec2 b) noexcept {
    return {a.x + b.x, a.y + b.y};
}
ivec3
operator+(ivec3 a, ivec3 b) noexcept {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
fvec2
operator+(fvec2 a, fvec2 b) noexcept {
    return {a.x + b.x, a.y + b.y};
}
fvec3
operator+(fvec3 a, fvec3 b) noexcept {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

ivec2
operator-(ivec2 a, ivec2 b) noexcept {
    return {a.x - b.x, a.y - b.y};
}
ivec3
operator-(ivec3 a, ivec3 b) noexcept {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
fvec2
operator-(fvec2 a, fvec2 b) noexcept {
    return {a.x - b.x, a.y - b.y};
}
fvec3
operator-(fvec3 a, fvec3 b) noexcept {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

ivec2
operator*(ivec2 a, ivec2 b) noexcept {
    return {a.x * b.x, a.y * b.y};
}
ivec2
operator*(int co, ivec2 a) noexcept {
    return {a.x * co, a.y * co};
}
ivec3
operator*(int co, ivec3 a) noexcept {
    return {a.x * co, a.y * co, a.z * co};
}
fvec2
operator*(fvec2 a, fvec2 b) noexcept {
    return {a.x * b.x, a.y * b.y};
}
fvec2
operator*(float co, fvec2 a) noexcept {
    return {a.x * co, a.y * co};
}
fvec3
operator*(float co, fvec3 a) noexcept {
    return {a.x * co, a.y * co, a.z * co};
}

ivec2
operator/(ivec2 a, int co) noexcept {
    return {a.x / co, a.y / co};
}
ivec3
operator/(ivec3 a, int co) noexcept {
    return {a.x / co, a.y / co, a.z / co};
}
fvec2
operator/(fvec2 a, float co) noexcept {
    return {a.x / co, a.y / co};
}
fvec3
operator/(fvec3 a, float co) noexcept {
    return {a.x / co, a.y / co, a.z / co};
}

bool
operator==(ivec2 a, ivec2 b) noexcept {
    return a.x == b.x && a.y == b.y;
}
bool
operator==(ivec3 a, ivec3 b) noexcept {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}
bool
operator==(fvec2 a, fvec2 b) noexcept {
    return a.x == b.x && a.y == b.y;
}
bool
operator==(fvec3 a, fvec3 b) noexcept {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool
operator!=(ivec2 a, ivec2 b) noexcept {
    return a.x != b.x || a.y != b.y;
}
bool
operator!=(ivec3 a, ivec3 b) noexcept {
    return a.x != b.x || a.y != b.y || a.z != b.z;
}
bool
operator!=(fvec2 a, fvec2 b) noexcept {
    return a.x != b.x || a.y != b.y;
}
bool
operator!=(fvec3 a, fvec3 b) noexcept {
    return a.x != b.x || a.y != b.y || a.z != b.z;
}

float
distanceTo(fvec2 a, fvec2 b) noexcept {
    double dx = static_cast<double>(a.x) - static_cast<double>(b.x);
    double dy = static_cast<double>(a.y) - static_cast<double>(b.y);
    return static_cast<float>(sqrt(dx * dx + dy * dy));
}
float
distanceTo(fvec3 a, fvec3 b) noexcept {
    double dx = static_cast<double>(a.x) - static_cast<double>(b.x);
    double dy = static_cast<double>(a.y) - static_cast<double>(b.y);
    return static_cast<float>(sqrt(dx * dx + dy * dy));
}

size_t
hash_(ivec2 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
size_t
hash_(ivec3 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
size_t
hash_(fvec2 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
size_t
hash_(fvec3 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
