#ifndef SRC_UTIL_RANDOM_H_
#define SRC_UTIL_RANDOM_H_

#include "util/compiler.h"

//! Produce a random integer.
/*!
    @param min Minimum value.
    @param max Maximum value.
    @return random integer between min and max.
*/
int
randInt(int min, int max) noexcept;

//! Produce a random float floating point number.
/*!
    @param min Minimum value.
    @param max Maximum value.
    @return random floating point number between min and max.
*/
float
randFloat(float min, float max) noexcept;

#endif  // SRC_UTIL_RANDOM_H_
