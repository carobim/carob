#include "util/random.h"

#include "os/c.h"
#include "util/compiler.h"
#include "util/int.h"

int
randInt(int min, int max) noexcept {
    return rand() % ((max + 1) - min) + min;
}

float
randFloat(float min, float max) noexcept {
    int i = rand();
    float d = static_cast<float>(i) / static_cast<float>(INT32_MAX);
    return d * (max - min) + min;
}
