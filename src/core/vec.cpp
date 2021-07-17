#include "core/vec.h"

#include "os/c.h"
#include "util/compiler.h"
#include "util/fnv.h"
#include "util/int.h"

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
