#ifndef SRC_UTIL_TRANSFORM_H_
#define SRC_UTIL_TRANSFORM_H_

#include "util/compiler.h"
#include "util/int.h"

#if CXX
extern "C" {
#endif

/* An affine transformation. */
struct Transform {
    float m[16];
};

struct Transform
transformIdentity(void) noexcept;

struct Transform
transformScale(float x, float y) noexcept;

struct Transform
transformTranslate(float x, float y) noexcept;

struct Transform
transformMultiply(struct Transform a, struct Transform b) noexcept;

#if CXX
}  /* extern "C" */
#endif

#endif  /* SRC_UTIL_TRANSFORM_H_ */
