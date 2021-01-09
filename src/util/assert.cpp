/*************************************
** Tsunagari Tile Engine            **
** assert.cpp                       **
** Copyright 2017-2021 Paul Merrill **
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

#ifndef NDEBUG

#include "util/assert.h"

#include "util/io.h"
#include "util/noexcept.h"

#ifdef _WIN32
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent() noexcept;
void __cdecl __debugbreak();  // Cannot be noexcept.
#endif

#ifdef __linux__
enum ptrace_request {
    PTRACE_TRACEME = 0,
    PTRACE_DETACH = 17,
};
extern "C" long ptrace(enum ptrace_request, ...) noexcept;
#endif

static bool
haveDebugger() {
    static int result = 2;

    if (result == 2) {
#if defined(_WIN32)
        result = IsDebuggerPresent();
#elif defined(__linux__)
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
            result = true;
        }
        else {
            ptrace(PTRACE_DETACH, 0, 0, 0);
            result = false;
        }
#endif
    }

    return static_cast<bool>(result);
}

static void
triggerDebugger() {
#if defined(_WIN32)
    __debugbreak();
#elif defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__("int3");
//#elif defined(__ARMCC_VERSION)
//    __breakpoint(42)
#elif defined(__thumb__)
    __asm__ __volatile__(".inst 0xde01");
#elif defined(__aarch64__)
    __asm__ __volatile__(".inst 0xd4200000");
#elif defined(__arm__)
    __asm__ __volatile__(".inst 0xe7f001f0");
#endif
}

void
assert__(const char* func,
         const char* file,
         int line,
         const char* expr) noexcept {
    sout << "Assertion failed: " << expr << ", function " << func << ", file "
         << file << ", line " << line << "\n";

    if (haveDebugger()) {
        triggerDebugger();
    }
}

#endif
