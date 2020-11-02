/*************************************
** Tsunagari Tile Engine            **
** chrono.h                         **
** Copyright 2019-2020 Paul Merrill **
*************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifndef SRC_OS_CHRONO_H_
#define SRC_OS_CHRONO_H_

#include "util/int.h"
#include "util/noexcept.h"

typedef int64_t Seconds;
typedef int64_t Milliseconds;
typedef int64_t Nanoseconds;

inline Milliseconds
ns_to_ms(Nanoseconds d) noexcept {
    return d / 1000000;
}
inline Seconds
ns_to_s(Nanoseconds d) noexcept {
    return d / 1000000000;
}
inline Nanoseconds
s_to_ns(Seconds d) noexcept {
    return d * 1000000000;
}

inline float
ms_to_s_d(Milliseconds ms) noexcept {
    return ms / 1000.0f;
}
inline float
ns_to_s_d(Nanoseconds ns) noexcept {
    return ns / 1000000000.0f;
}

Nanoseconds
chronoNow() noexcept;

void
chronoSleep(Nanoseconds ns) noexcept;

#endif  // SRC_OS_CHRONO_H_
