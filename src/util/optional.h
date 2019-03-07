/*************************************
** Tsunagari Tile Engine            **
** optional.h                       **
** Copyright 2016-2019 Paul Merrill **
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

#ifndef SRC_UTIL_OPTIONAL_H_
#define SRC_UTIL_OPTIONAL_H_

#include "util/assert.h"
#include "util/move.h"
#include "util/new.h"

template<typename T>
class Optional {
    union {
        T x;
        char null;
    };
    bool exists;

 public:
    explicit constexpr Optional() noexcept : null(), exists(false) {}
    explicit constexpr Optional(T&& x) noexcept : x(move_(x)), exists(true) {}
    explicit constexpr Optional(const T& x) noexcept : x(x), exists(true) {}

    constexpr Optional(Optional<T>&& other) noexcept
            : null(), exists(other.exists) {
        if (other.exists) {
            x = move_(other.x);
            other.x.~T();
            other.exists = false;
        }
    }
    constexpr Optional(const Optional<T>& other) noexcept
            : exists(other.exists) {
        if (other.exists) {
            x = other.x;
        }
    }

    ~Optional() noexcept {
        if (exists) {
            x.~T();
        }
    }

    inline Optional& operator=(T&& x) {
        if (exists) {
            this->x = move_(x);
        }
        else {
            new (&this->x) T(move_(x));
            exists = true;
        }
        return *this;
    }
    inline Optional& operator=(const T& x) {
        if (exists) {
            this->x = x;
        }
        else {
            new (&this->x) T(x);
            exists = true;
        }
        return *this;
    }

    inline Optional& operator=(Optional<T>&& other) {
        if (exists) {
            if (other.exists) {
                x = move_(other.x);
            }
            else {
                x.~T();
            }
        }
        else {
            if (other.exists) {
                new (&x) T(move_(other.x));
            }
        }
        exists = other.exists;
        other.exists = false;
        return *this;
    }
    inline Optional& operator=(const Optional<T>& other) {
        if (exists) {
            if (other.exists) {
                x = other.x;
            }
            else {
                x.~T();
            }
        }
        else {
            if (other.exists) {
                new (&x) T(other.x);
            }
        }
        exists = other.exists;
        return *this;
    }

    inline constexpr operator bool() const { return exists; }

    inline constexpr const T* operator->() const {
        static_assert(exists, "");
        assert_(exists);
        return &x;
    }
    inline constexpr const T& operator*() const {
        static_assert(exists, "");
        assert_(exists);
        return x;
    }

    inline constexpr T* operator->() {
        assert_(exists);
        return &x;
    }
    inline constexpr T& operator*() {
        assert_(exists);
        return x;
    }
    
    friend constexpr bool operator==(const Optional<T>& a, const Optional<T>& b);
};

template<typename T>
inline constexpr bool operator==(const Optional<T>& a, const Optional<T>& b) {
    if (a.exists != b.exists) {
        return false;
    }
    if (!a.exists) {
        return true;
    }
    return a.x == b.x;
}

#endif  // SRC_UTIL_OPTIONAL_H_
