#ifndef SRC_UTIL_RANDOM_H_
#define SRC_UTIL_RANDOM_H_

#include "util/compiler.h"
#include "util/int.h"

void
initRandom() noexcept;

//! Produce a random integer.
/*!
    @param min Minimum value.
    @param max Maximum value.
    @return random integer between min and max.
*/
U32
randInt(U32 min, U32 max) noexcept;

//! Produce a random float floating point number.
/*!
    @param min Minimum value.
    @param max Maximum value.
    @return random floating point number between min and max.
*/
float
randFloat(float min, float max) noexcept;

#endif  // SRC_UTIL_RANDOM_H_
