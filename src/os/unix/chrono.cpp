#include "os/chrono.h"
#include "os/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"

Nanoseconds
chronoNow() noexcept {
    struct timespec tp;

    I32 err = clock_gettime(CLOCK_MONOTONIC, &tp);
    assert_(err == 0);

    return static_cast<Nanoseconds>(
        s_to_ns(static_cast<Nanoseconds>(tp.tv_sec)) + tp.tv_nsec);
}

void
chronoSleep(Nanoseconds ns) noexcept {
    if (ns <= 0)
        return;
    Seconds s = ns_to_s(ns);
    timespec ts;
    ts.tv_sec = static_cast<Time>(s);
    ts.tv_nsec = static_cast<long>(ns - s_to_ns(s));
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
        ;
}
