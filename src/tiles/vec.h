#ifndef SRC_TILES_VEC_H_
#define SRC_TILES_VEC_H_

#include "util/compiler.h"
#include "util/int.h"

// clang-format off

struct ivec2 { I32 x, y; };
struct ivec3 { I32 x, y, z; };
struct fvec2 { float x, y; };
struct fvec3 { float x, y, z; };

/**
 * Virtual integer coordinate.
 *
 * x and y are the same as a physical integer coordinate.
 * z is a virtual layer depth within an Area.
 */
struct vicoord {
    I32 x, y;
    float z;
};

struct icube {
    I32 x1, y1, z1;
    I32 x2, y2, z2;
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
ivec2 operator*(I32, ivec2) noexcept;
ivec3 operator*(I32, ivec3) noexcept;
fvec2 operator*(fvec2, fvec2) noexcept;
fvec2 operator*(float, fvec2) noexcept;
fvec3 operator*(float, fvec3) noexcept;

ivec2 operator*(ivec2, I32) noexcept;
ivec3 operator*(ivec3, I32) noexcept;
fvec2 operator*(fvec2, float) noexcept;
fvec3 operator*(fvec3, float) noexcept;

ivec2 operator/(ivec2, I32) noexcept;
ivec3 operator/(ivec3, I32) noexcept;
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

Size hash_(ivec2) noexcept;
Size hash_(ivec3) noexcept;
Size hash_(fvec2) noexcept;
Size hash_(fvec3) noexcept;

static const constexpr11 ivec3 IVEC3_MIN = {
    INT32_MIN,
    INT32_MIN,
    INT32_MIN
};

// clang-format on

#endif  // SRC_TILES_VEC_H_
