/**************************************
 ** Tsunagari Tile Engine            **
 ** string.h                         **
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

#ifndef SRC_UTIL_STRING_H_
#define SRC_UTIL_STRING_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

class String {
 public:
    char* data;
    size_t size;
    size_t capacity;

 public:
    String() noexcept;
    String(const char* s) noexcept;
    String(StringView s) noexcept;
    String(const String& s) noexcept;
    String(String&& s) noexcept;
    ~String() noexcept;

    void
    operator=(const char* s) noexcept;
    void
    operator=(StringView s) noexcept;

    void
    operator=(const String& s) noexcept;
    void
    operator=(String&& s) noexcept;

    char&
    operator[](size_t i) noexcept;

    String&
    operator<<(char c) noexcept;
    String&
    operator<<(const char* s) noexcept;
    String&
    operator<<(StringView s) noexcept;

    String&
    operator<<(bool b) noexcept;
    String&
    operator<<(int i) noexcept;
    String&
    operator<<(unsigned int u) noexcept;
    String&
    operator<<(long l) noexcept;
    String&
    operator<<(unsigned long ul) noexcept;
    String&
    operator<<(long long ll) noexcept;
    String&
    operator<<(unsigned long long ull) noexcept;
    String&
    operator<<(float f) noexcept;

    void
    resize(size_t n) noexcept;
    void
    reserve(size_t n) noexcept;
    void
    clear() noexcept;
    void
    reset() noexcept;

    operator StringView() const noexcept;
    StringView
    view() const noexcept;

    const char*
    null() noexcept;
};

bool
operator<(const String& a, const String& b) noexcept;
bool
operator>(const String& a, const String& b) noexcept;

size_t
hash_(const String& s) noexcept;

#endif  // SRC_UTIL_STRING_H_
