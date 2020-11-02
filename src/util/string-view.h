/*************************************
** Tsunagari Tile Engine            **
** string-view.h                    **
** Copyright 2019-2020 Paul Merrill **
**************************************/

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

#ifndef SRC_UTIL_STRING_VIEW_H_
#define SRC_UTIL_STRING_VIEW_H_

#include "util/int.h"
#include "util/noexcept.h"

typedef size_t StringPosition;

#define SV_NOT_FOUND SIZE_MAX

class StringView {
 public:
    const char* data;
    size_t size;

 public:
    StringView() noexcept;
    StringView(const char* data) noexcept;
    template<size_t N>
    StringView(const char (&data)[N]) noexcept
            : data(data), size(N){};
    StringView(const char* data, size_t size) noexcept;
    StringView(const StringView& s) noexcept;

    void
    operator=(const StringView& s) noexcept {
        data = s.data;
        size = s.size;
    }

    const char*
    begin() const noexcept;
    const char*
    end() const noexcept;

    StringPosition
    find(char needle) const noexcept;
    StringPosition
    find(StringView needle) const noexcept;
    StringPosition
    find(StringView needle, size_t start) const noexcept;
    StringPosition
    rfind(char needle) const noexcept;

    StringView
    substr(const size_t from) const noexcept;
    StringView
    substr(const size_t from, const size_t span) const noexcept;
};

bool
operator==(const StringView& a, const StringView& b) noexcept;
bool
operator!=(const StringView& a, const StringView& b) noexcept;
bool
operator>(const StringView& a, const StringView& b) noexcept;
bool
operator<(const StringView& a, const StringView& b) noexcept;

size_t
hash_(StringView s) noexcept;
size_t
hash_(const char* s) noexcept;

#endif  // SRC_UTIL_STRING_VIEW_H_
