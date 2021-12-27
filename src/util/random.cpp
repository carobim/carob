#include "util/random.h"

#include "os/chrono.h"
#include "util/compiler.h"
#include "util/int.h"

U32 state;

void
initRandom() noexcept {
    state = static_cast<U32>(chronoNow());
}

/* https://en.wikipedia.org/wiki/Lehmer_random_number_generator */
static U32
generate() noexcept {
    state = (48271 * state) % 2147483647;
    return state;
}

U32
randU32(U32 min, U32 max) noexcept {
    U32 range = max - min + 1;
    U32 scaling = UINT32_MAX / range;
    U32 over = range * scaling;

    U32 r;
    do {
        r = generate();
    } while (r >= over);
    return r / scaling + min;
}

float
randFloat(float min, float max) noexcept {
    U32 u = generate();
    float f = static_cast<float>(u) / 2147483648.0f - 1.0f;
    return f * (max - min) + min;
}
