/********************************
** Tsunagari Tile Engine       **
** io.h                        **
** Copyright 2020 Paul Merrill **
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

#ifndef SRC_UTIL_IO_H_
#define SRC_UTIL_IO_H_

#include "util/int.h"
#include "util/noexcept.h"
#include "util/string-view.h"

class Output {
 public:
    Output(void* data) noexcept;

    Output&
    operator<<(char x) noexcept;
    Output&
    operator<<(const char* x) noexcept;
    Output&
    operator<<(StringView x) noexcept;

    Output&
    operator<<(bool b) noexcept;
    Output&
    operator<<(int i) noexcept;
    Output&
    operator<<(unsigned int u) noexcept;
    Output&
    operator<<(long l) noexcept;
    Output&
    operator<<(unsigned long ul) noexcept;
    Output&
    operator<<(long long ll) noexcept;
    Output&
    operator<<(unsigned long long ull) noexcept;
    Output&
    operator<<(float f) noexcept;

 public:
    // Platform dependent.
    void* data;
};

extern Output sout;
extern Output serr;

#endif  // SRC_UTIL_STRING_H_
