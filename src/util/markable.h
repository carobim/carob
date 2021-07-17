/*************************************
** Tsunagari Tile Engine            **
** markable.h                       **
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

#ifndef SRC_UTIL_MARKABLE_H_
#define SRC_UTIL_MARKABLE_H_

#include "util/assert.h"
#include "util/compiler.h"

struct M { };

#define mark M()

template<typename T, T MarkedValue>
class Markable {
    typedef Markable<T, MarkedValue> This;

    T x;

 public:
    explicit constexpr11
    Markable() noexcept
            : x(MarkedValue) { }
    explicit constexpr11
    Markable(T&& x) noexcept
            : x(static_cast<T&&>(x)) { }
    explicit constexpr11
    Markable(const T& x) noexcept
            : x(x) { }
    constexpr11
    Markable(M) noexcept : x(MarkedValue) { }
    constexpr11
    Markable(Markable&& other) noexcept : x(static_cast<T&&>(other.x)) { }
    constexpr11
    Markable(const Markable& other) noexcept : x(other.x) { }

    inline void
    operator=(T&& x_) noexcept {
        x = static_cast<T&&>(x_);
    }
    inline void
    operator=(const T& x_) noexcept {
        x = x_;
    }
    inline void
    operator=(This&& other) noexcept {
        x = static_cast<T&&>(other.x);
    }
    inline void
    operator=(const This& other) noexcept {
        x = other.x;
    }
    inline void
    operator=(M) noexcept {
        x = MarkedValue;
    }

    inline constexpr11 bool
    exists() const noexcept {
        return x != MarkedValue;
    }
    inline constexpr11 operator bool() const noexcept { return exists(); }

    inline constexpr14 T*
    operator->() noexcept {
        assert_(exists());
        return &x;
    }

    inline constexpr14 T&
    operator*() noexcept {
        assert_(exists());
        return x;
    }
};

#endif  // SRC_UTIL_MARKABLE_H_
