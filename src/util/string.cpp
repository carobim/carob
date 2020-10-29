/**************************************
 ** Tsunagari Tile Engine            **
 ** string.cpp                       **
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

#include "util/string.h"

#include "os/c.h"
#include "util/fnv.h"
#include "util/noexcept.h"

String::String(const char* value) noexcept {
    *this << value;
}

String::String(StringView value) noexcept {
    *this << value;
}

String::String(String&& other) noexcept {
    data = other.data;
    size = other.size;
    capacity = other.capacity;
    other.data = 0;
    other.size = other.capacity = 0;
}

String&
String::operator=(const char* other) noexcept {
    clear();
    return *this << other;
}

String&
String::operator=(StringView other) noexcept {
    clear();
    return *this << other;
}

String&
String::operator=(const String& other) noexcept {
    clear();
    return *this << StringView(other);
}

String&
String::operator=(String&& other) noexcept {
    free(data);
    data = other.data;
    size = other.size;
    capacity = other.capacity;
    other.data = 0;
    other.size = other.capacity = 0;
    return *this;
}

bool
String::operator<(const String& other) const noexcept {
    return view() < other.view();
}

bool
String::operator>(const String& other) const noexcept {
    return view() > other.view();
}

String&
String::operator<<(char value) noexcept {
    push_back(value);
    return *this;
}

String&
String::operator<<(const char* value) noexcept {
    append(strlen(value), value);
    return *this;
}

String&
String::operator<<(StringView value) noexcept {
    append(value.size, value.data);
    return *this;
}

String&
String::operator<<(bool value) noexcept {
    return *this << (value ? "true" : "false");
}

String&
String::operator<<(int value) noexcept {
    char buf[64];
    sprintf(buf, "%d", value);
    return *this << buf;
}

String&
String::operator<<(unsigned int value) noexcept {
    char buf[64];
    sprintf(buf, "%u", value);
    return *this << buf;
}

String&
String::operator<<(long value) noexcept {
    char buf[64];
    sprintf(buf, "%ld", value);
    return *this << buf;
}

String&
String::operator<<(unsigned long value) noexcept {
    char buf[64];
    sprintf(buf, "%lu", value);
    return *this << buf;
}

String&
String::operator<<(long long value) noexcept {
    char buf[64];
    sprintf(buf, "%lld", value);
    return *this << buf;
}

String&
String::operator<<(unsigned long long value) noexcept {
    char buf[64];
    sprintf(buf, "%llu", value);
    return *this << buf;
}

String&
String::operator<<(float value) noexcept {
    char buf[64];
    sprintf(buf, "%f", value);
    return *this << buf;
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
        reserve(size + 1);
    }
    data[size] = 0;
    return data;
}

size_t
hash_(const String& s) noexcept {
    return fnvHash(s.data, s.size);
}
