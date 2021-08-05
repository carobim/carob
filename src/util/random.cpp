#include "util/random.h"

#include "os/chrono.h"
#include "util/compiler.h"
#include "util/int.h"

uint32_t state;

void
initRandom() noexcept {
    state = chronoNow();
}

/* https://en.wikipedia.org/wiki/Lehmer_random_number_generator */
static uint32_t
generate() noexcept {
    state = (48271 * state) % 2147483647;
    return state;
}

uint32_t
randInt(uint32_t min, uint32_t max) noexcept {
    uint32_t range = max - min + 1;
    uint32_t scaling = UINT32_MAX / range;
    uint32_t over = range * scaling;

    uint32_t r;
    do {
        r = generate();
    } while (r >= over);
    return r / scaling + min;
}

float
randFloat(float min, float max) noexcept {
    uint32_t u = generate();
    float f = static_cast<float>(u) / 2147483648.0f - 1.0f;
    return f * (max - min) + min;
}
