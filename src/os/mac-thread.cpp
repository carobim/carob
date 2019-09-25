/********************************
** Tsunagari Tile Engine       **
** mac-thread.cpp              **
** Copyright 2019 Paul Merrill **
********************************/

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

#include "os/mac-thread.h"

#include "util/int.h"

// mach/thread_act.h
extern "C" {
int32_t
thread_policy_set(uint32_t, uint32_t, uint32_t*, uint32_t) noexcept;
}

// mach/mach_init.h
extern "C" {
uint32_t
mach_thread_self() noexcept;
}

// mach/thread_policy.h
#define THREAD_EXTENDED_POLICY 1
#define THREAD_EXTENDED_POLICY_COUNT 1
struct thread_extended_policy {
    int32_t timeshare;
};

void
Thread::disableTimerCoalescing() noexcept {
    thread_extended_policy policyInfo = {
        .timeshare = false,
    };

    thread_policy_set(mach_thread_self(),
                      THREAD_EXTENDED_POLICY,
                      (uint32_t*)&policyInfo,
                      THREAD_EXTENDED_POLICY_COUNT);
}
