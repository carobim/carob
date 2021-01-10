/*************************************
** Tsunagari Tile Engine            **
** string.cpp                       **
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

#include "util/string.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/fnv.h"
#include "util/new.h"
#include "util/noexcept.h"

static size_t
grow1(size_t current) noexcept {
    return current < 4 ? 4 : current * 2;
}

static size_t
growN(size_t current, size_t addition) noexcept {
    size_t newSize = current == 0 ? 4 : current * 2;
    while (newSize < current + addition) {
        newSize *= 2;
    }
    return newSize;
}

String::String() noexcept : data(0), size(0), capacity(0) {}

String::String(const char* s) noexcept {
    if (s == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        size_t len = strlen(s);
        // FIXME: Choose better size.
        data = static_cast<char*>(malloc(len));
        size = capacity = len;
        memcpy(data, s, len);
    }
}

String::String(StringView s) noexcept {
    if (s.size == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        // FIXME: Choose better size.
        data = static_cast<char*>(malloc(s.size));
        size = capacity = s.size;
        memcpy(data, s.data, s.size);
    }
}

String::String(const String& s) noexcept {
    if (s.capacity == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        data = static_cast<char*>(malloc(s.capacity));
        size = s.size;
        capacity = s.capacity;
        memcpy(data, s.data, size);
    }
}

String::String(String&& s) noexcept {
    data = s.data;
    size = s.size;
    capacity = s.capacity;
    s.data = 0;
    s.size = s.capacity = 0;
}

String::~String() noexcept {
    free(data);
}

void
String::operator=(const char* s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(StringView s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(const String& s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(String&& s) noexcept {
    free(data);
    data = s.data;
    size = s.size;
    capacity = s.capacity;
    s.data = 0;
    s.size = s.capacity = 0;
}

char&
String::operator[](size_t i) noexcept {
    assert_(i < size);
    return data[i];
}

String&
String::operator<<(char c) noexcept {
    if (size == capacity) {
        reserve(grow1(size));
    }
    data[size++] = c;
    return *this;
}

String&
String::operator<<(const char* s) noexcept {
    // TODO: Use strncpy without knowing the size. If the string has more
    //       characters, then do a strlen & growN & memcpy like below.
    size_t len = strlen(s);
    if (capacity < size + len) {
        reserve(growN(size, len));
    }
    memcpy(data + size, s, len);
    size += len;
    return *this;
}

String&
String::operator<<(StringView s) noexcept {
    if (capacity < size + s.size) {
        reserve(growN(size, s.size));
    }
    memcpy(data + size, s.data, s.size);
    size += s.size;
    return *this;
}

String&
String::operator<<(bool b) noexcept {
    return *this << (b ? "true" : "false");
}

String&
String::operator<<(int i) noexcept {
    // Minus sign & 10 digits.
    if (capacity < size + 11) {
        reserve(growN(size, 11));
    }

    if (i < 0) {
        if (i == INT32_MIN) {
            memcpy(data + size, "-2147483648", 11);
            size += 11;
            return *this;
        }

        data[size++] = '-';
        i = -i;
    }

    char buf[10];
    char* p = buf;

    do {
        *p++ = static_cast<char>(i % 10) + '0';
        i /= 10;
    } while (i > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(unsigned int u) noexcept {
    if (capacity < size + 10) {
        reserve(growN(size, 10));
    }

    char buf[10];
    char* p = buf;

    do {
        *p++ = static_cast<char>(u % 10) + '0';
        u /= 10;
    } while (u > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(long l) noexcept {
    if (capacity < size + 20) {
        reserve(growN(size, 20));
    }

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(l % 10) + '0';
        l /= 10;
    } while (l > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(unsigned long ul) noexcept {
    if (capacity < size + 20) {
        reserve(growN(size, 20));
    }

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(ul % 10) + '0';
        ul /= 10;
    } while (ul > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(long long ll) noexcept {
    if (capacity < size + 20) {
        reserve(growN(size, 20));
    }

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(ll % 10) + '0';
        ll /= 10;
    } while (ll > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(unsigned long long ull) noexcept {
    if (capacity < size + 20) {
        reserve(growN(size, 20));
    }

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(ull % 10) + '0';
        ull /= 10;
    } while (ull > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(float f) noexcept {
    char buf[64];
    sprintf(buf, "%f", f);
    return *this << buf;
}

void
String::reserve(size_t n) noexcept {
    assert_(capacity < n);
    char* newData = static_cast<char*>(malloc(n));
    memmove(newData, data, size);
    data = newData;
    capacity = n;
}

void
String::resize(size_t n) noexcept {
    if (capacity < n) {
        reserve(n);
    }
    size = n;
}

void
String::clear() noexcept {
    size = 0;
}

void
String::reset() noexcept {
    data = 0;
    size = capacity = 0;
}

String::operator StringView() const noexcept {
    return StringView(data, size);
}

StringView
String::view() const noexcept {
    return StringView(data, size);
}

const char*
String::null() noexcept {
    if (size == capacity) {
        reserve(grow1(size));
    }
    data[size] = 0;
    return data;
}

bool
operator<(const String& a, const String& b) noexcept {
    return a.view() < b.view();
}

bool
operator>(const String& a, const String& b) noexcept {
    return a.view() > b.view();
}

size_t
hash_(const String& s) noexcept {
    return fnvHash(s.data, s.size);
}
