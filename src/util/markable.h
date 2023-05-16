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
    Markable(M) noexcept
            : x(MarkedValue) { }
    constexpr11
    Markable(Markable&& other) noexcept
            : x(static_cast<T&&>(other.x)) { }
    constexpr11
    Markable(const Markable& other) noexcept
            : x(other.x) { }

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
