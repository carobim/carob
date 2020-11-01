/*************************************
** Tsunagari Tile Engine            **
** windows-chrono.cpp               **
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

#include "os/c.h"
#include "os/chrono.h"
#include "util/assert.h"
#include "util/int.h"

extern "C" {
WINBASEAPI
BOOL WINAPI
QueryPerformanceFrequency(LARGE_INTEGER*);
WINBASEAPI
BOOL WINAPI
QueryPerformanceCounter(LARGE_INTEGER*);
WINBASEAPI
VOID WINAPI Sleep(DWORD);
}

static bool haveFreq = false;
static LARGE_INTEGER freq;

Nanoseconds
chronoNow() noexcept {
    if (!haveFreq) {
        haveFreq = true;

        BOOL ok = QueryPerformanceFrequency(&freq);
        (void)ok;
        assert_(ok);
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return static_cast<Nanoseconds>(s_to_ns(counter.QuadPart) / freq.QuadPart);
}

void
chronoSleep(Nanoseconds ns) noexcept {
    if (ns <= 0) {
        return;
    }

    DWORD ms = static_cast<DWORD>((ns + 999999) / 1000000);
    Sleep(ms);
}
