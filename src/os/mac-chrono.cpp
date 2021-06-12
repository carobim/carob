/*************************************
** Tsunagari Tile Engine            **
** mac-chrono.cpp                   **
** Copyright 2019-2021 Paul Merrill **
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

#include "os/c.h"
#include "os/chrono.h"
#include "util/assert.h"
#include "util/int.h"

// mach/arm/kern_return.h
// mach/i386/kern_return.h
typedef int kern_return_t;

// mach/mach_time.h
struct mach_timebase_info {
    uint32_t numer;
    uint32_t denom;
};
extern "C" {
uint64_t
mach_absolute_time() noexcept;
kern_return_t
mach_timebase_info(mach_timebase_info*) noexcept;
kern_return_t
mach_wait_until(uint64_t deadline) noexcept;
}

#define KERN_SUCCESS 0

static struct mach_timebase_info timebase = {0, 0};

static Nanoseconds
fromAbsolute(uint64_t machTime) noexcept {
    uint64_t ns = machTime *
        static_cast<uint64_t>(timebase.numer) /
        static_cast<uint64_t>(timebase.denom);
    return static_cast<Nanoseconds>(ns);
}

static uint64_t
toAbsolute(Nanoseconds ns) noexcept {
    return static_cast<uint64_t>(ns) *
        static_cast<uint64_t>(timebase.denom) /
        static_cast<uint64_t>(timebase.numer);
}

static void
initTimebase(void) {
    if (timebase.numer == 0 && timebase.denom == 0) {
        kern_return_t err = mach_timebase_info(&timebase);
        (void)err;
        assert_(err == KERN_SUCCESS);
    }
}

Nanoseconds
chronoNow(void) noexcept {
    initTimebase();
    return fromAbsolute(mach_absolute_time());
}

void
chronoSleep(Nanoseconds ns) noexcept {
    if (ns <= 0) {
        return;
    }

    initTimebase();
    uint64_t deadline = mach_absolute_time() + toAbsolute(ns);

    kern_return_t err;
    do {
        err = mach_wait_until(deadline);
    } while (err != KERN_SUCCESS);
}
