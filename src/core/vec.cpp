#include "core/vec.h"

#include "os/c.h"
#include "util/compiler.h"
#include "util/fnv.h"
#include "util/int.h"

ivec2
operator+(ivec2 a, ivec2 b) noexcept {
    ivec2 result = {a.x + b.x, a.y + b.y};
    return result;
}
ivec3
operator+(ivec3 a, ivec3 b) noexcept {
    ivec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}
fvec2
operator+(fvec2 a, fvec2 b) noexcept {
    fvec2 result = {a.x + b.x, a.y + b.y};
    return result;
}
fvec3
operator+(fvec3 a, fvec3 b) noexcept {
    fvec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

ivec2
operator-(ivec2 a, ivec2 b) noexcept {
    ivec2 result = {a.x - b.x, a.y - b.y};
    return result;
}
ivec3
operator-(ivec3 a, ivec3 b) noexcept {
    ivec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}
fvec2
operator-(fvec2 a, fvec2 b) noexcept {
    fvec2 result = {a.x - b.x, a.y - b.y};
    return result;
}
fvec3
operator-(fvec3 a, fvec3 b) noexcept {
    fvec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

ivec2
operator*(ivec2 a, ivec2 b) noexcept {
    ivec2 result = {a.x * b.x, a.y * b.y};
    return result;
}
ivec2
operator*(int co, ivec2 a) noexcept {
    ivec2 result = {a.x * co, a.y * co};
    return result;
}
ivec3
operator*(int co, ivec3 a) noexcept {
    ivec3 result = {a.x * co, a.y * co, a.z * co};
    return result;
}
fvec2
operator*(fvec2 a, fvec2 b) noexcept {
    fvec2 result = {a.x * b.x, a.y * b.y};
    return result;
}
fvec2
operator*(float co, fvec2 a) noexcept {
    fvec2 result = {a.x * co, a.y * co};
    return result;
}
fvec3
operator*(float co, fvec3 a) noexcept {
    fvec3 result = {a.x * co, a.y * co, a.z * co};
    return result;
}

ivec2
operator/(ivec2 a, int co) noexcept {
    ivec2 result = {a.x / co, a.y / co};
    return result;
}
ivec3
operator/(ivec3 a, int co) noexcept {
    ivec3 result = {a.x / co, a.y / co, a.z / co};
    return result;
}
fvec2
operator/(fvec2 a, float co) noexcept {
    fvec2 result = {a.x / co, a.y / co};
    return result;
}
fvec3
operator/(fvec3 a, float co) noexcept {
    fvec3 result = {a.x / co, a.y / co, a.z / co};
    return result;
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
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}
float
distanceTo(fvec3 a, fvec3 b) noexcept {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

Size
hash_(ivec2 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
Size
hash_(ivec3 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
Size
hash_(fvec2 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
Size
hash_(fvec3 a) noexcept {
    return fnvHash(reinterpret_cast<char*>(&a), sizeof(a));
}
