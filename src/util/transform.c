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
