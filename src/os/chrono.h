#ifndef SRC_OS_CHRONO_H_
#define SRC_OS_CHRONO_H_

#include "util/compiler.h"
#include "util/int.h"

typedef int64_t Seconds;
typedef int64_t Milliseconds;
typedef int64_t Nanoseconds;

inline Milliseconds
ns_to_ms(Nanoseconds d) noexcept {
    return d / 1000000;
}
inline Seconds
ns_to_s(Nanoseconds d) noexcept {
    return d / 1000000000;
}
inline Nanoseconds
s_to_ns(Seconds d) noexcept {
    return d * 1000000000;
}

inline float
ms_to_s_d(Milliseconds ms) noexcept {
    return ms / 1000.0f;
}
inline float
ns_to_s_d(Nanoseconds ns) noexcept {
    return ns / 1000000000.0f;
}

Nanoseconds
chronoNow() noexcept;

void
chronoSleep(Nanoseconds ns) noexcept;

#endif  // SRC_OS_CHRONO_H_
