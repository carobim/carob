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
rvec2
operator+(rvec2 a, rvec2 b) noexcept {
    return {a.x + b.x, a.y + b.y};
}
rvec3
operator+(rvec3 a, rvec3 b) noexcept {
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
rvec2
operator-(rvec2 a, rvec2 b) noexcept {
    return {a.x - b.x, a.y - b.y};
}
rvec3
operator-(rvec3 a, rvec3 b) noexcept {
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
rvec2
operator*(rvec2 a, rvec2 b) noexcept {
    return {a.x * b.x, a.y * b.y};
}
rvec2
operator*(float co, rvec2 a) noexcept {
    return {a.x * co, a.y * co};
}
rvec3
operator*(float co, rvec3 a) noexcept {
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
rvec2
operator/(rvec2 a, float co) noexcept {
    return {a.x / co, a.y / co};
}
rvec3
operator/(rvec3 a, float co) noexcept {
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
operator==(rvec2 a, rvec2 b) noexcept {
    return a.x == b.x && a.y == b.y;
}
bool
operator==(rvec3 a, rvec3 b) noexcept {
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
operator!=(rvec2 a, rvec2 b) noexcept {
    return a.x != b.x || a.y != b.y;
}
bool
operator!=(rvec3 a, rvec3 b) noexcept {
    return a.x != b.x || a.y != b.y || a.z != b.z;
}

float
distanceTo(rvec2 a, rvec2 b) noexcept {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}
float
distanceTo(rvec3 a, rvec3 b) noexcept {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
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
hash_(rvec2 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
size_t
hash_(rvec3 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
