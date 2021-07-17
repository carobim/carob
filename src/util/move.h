#ifndef SRC_UTIL_MOVE_H_
#define SRC_UTIL_MOVE_H_

#include "util/compiler.h"

template<typename T>
struct Refless {
    typedef T value;
};
template<typename T>
struct Refless<T&> {
    typedef T value;
};
template<typename T>
struct Refless<T&&> {
    typedef T value;
};

//
// Forward
//   forward_()  same as std::forward
//

template<typename T>
inline constexpr11 T&&
forward_(typename Refless<T>::value& x) noexcept {
    return static_cast<T&&>(x);
}

template<typename T>
inline constexpr11 T&&
forward_(typename Refless<T>::value&& x) noexcept {
    return static_cast<T&&>(x);
}

//
// Swap
//   swap_()  same as std::swap
//

template<typename T>
inline void
swap_(T& a, T& b) noexcept {
    T temp(static_cast<T&&>(a));
    a = static_cast<T&&>(b);
    b = static_cast<T&&>(temp);
}

#endif  // SRC_UTIL_MOVE_H_
