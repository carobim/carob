/*************************************
** Tsunagari Tile Engine            **
** string-view.cpp                  **
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

#include "util/string-view.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/fnv.h"
#include "util/noexcept.h"

StringView::StringView() noexcept
            : data(0), size(0){}
StringView::StringView(const char* data) noexcept
        : data(data), size(strlen(data)) {}
StringView::StringView(const char* data, size_t size) noexcept
        : data(data), size(size){}
StringView::StringView(const StringView& s) noexcept
        : data(s.data), size(s.size){}

void
StringView::operator=(const StringView& s) noexcept {
    data = s.data;
    size = s.size;
}

char
StringView::operator[](size_t i) noexcept {
    return data[i];
}

const char*
StringView::begin() const noexcept {
    return data;
}
const char*
StringView::end() const noexcept {
    return data + size;
}

StringPosition
StringView::find(char needle) const noexcept {
    // Necessary check because GCC 8.3.0 assumes that the first parameter to
    // memchr cannot be 0, which can propagate a `data != 0` constraint up to
    // callers, which is a constraint that is not always true. E.g., it would
    // be nice to call this function with .data = 0 and .size = 0 and get a
    // SV_NOT_FOUND as a result and then later do a check against `.data == 0`,
    // but with GCC's assumption, that check will be optimized out.
    if (data == 0) {
        return SV_NOT_FOUND;
    }

    char* result = static_cast<char*>(memchr(data, needle, size));
    if (result == 0) {
        return SV_NOT_FOUND;
    }
    return StringPosition(result - data);
}

StringPosition
StringView::find(char needle, size_t start) const noexcept {
    if (data == 0) {
        return SV_NOT_FOUND;
    }

    char* result = static_cast<char*>(memchr(
        data + start,
        needle, size - start
    ));
    if (result == 0) {
        return SV_NOT_FOUND;
    }
    return StringPosition(result - data);
}

StringPosition
StringView::find(StringView needle) const noexcept {
    if (data == 0) {
        return SV_NOT_FOUND;
    }

    char* result =
            static_cast<char*>(memmem(data, size, needle.data, needle.size));
    if (result == 0) {
        return SV_NOT_FOUND;
    }
    return StringPosition(result - data);
}

StringPosition
StringView::find(StringView needle, size_t start) const noexcept {
    assert_(size >= start);

    if (data == 0) {
        return SV_NOT_FOUND;
    }

    char* result = static_cast<char*>(
            memmem(data + start, size - start, needle.data, needle.size));
    if (result == 0) {
        return SV_NOT_FOUND;
    }
    return StringPosition(result - data);
}

StringPosition
StringView::rfind(char needle) const noexcept {
    if (size == 0) {
        return SV_NOT_FOUND;
    }

    size_t i = size;
    do {
        i--;
        if (data[i] == needle) {
            return StringPosition(i);
        }
    } while (i > 0);

    return SV_NOT_FOUND;
}

StringView
StringView::substr(const size_t from) const noexcept {
    assert_(from <= this->size);
    return StringView(data + from, size - from);
}
StringView
StringView::substr(const size_t from, const size_t span) const noexcept {
    assert_(from <= size);
    assert_(from + span <= size);
    return StringView(data + from, span);
}

bool
operator==(const StringView& a, const StringView& b) noexcept {
    return (a.size == b.size) && memcmp(a.data, b.data, a.size) == 0;
}

bool
operator!=(const StringView& a, const StringView& b) noexcept {
    return !(a == b);
}

bool
operator>(const StringView& a, const StringView& b) noexcept {
    size_t s = a.size < b.size ? a.size : b.size;
    const char* ad = a.data;
    const char* bd = b.data;

    while (s--) {
        if (*ad != *bd) {
            return *ad > *bd;
        }
        ad++;
        bd++;
    }
    if (a.size != b.size) {
        return a.size > b.size;
    }
    return false;
}

bool
operator<(const StringView& a, const StringView& b) noexcept {
    size_t s = a.size < b.size ? a.size : b.size;
    const char* ad = a.data;
    const char* bd = b.data;

    while (s--) {
        if (*ad != *bd) {
            return *ad < *bd;
        }
        ad++;
        bd++;
    }
    if (a.size != b.size) {
        return a.size < b.size;
    }
    return false;
}

size_t
hash_(StringView s) noexcept {
    return fnvHash(s.data, s.size);
}

size_t
hash_(const char* s) noexcept {
    return hash_(StringView(s));
}
