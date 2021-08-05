#include "os/c.h"
#include "os/chrono.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"

// mach/arm/kern_return.h
// mach/i386/kern_return.h
typedef int kern_return_t;

// mach/mach_time.h
struct mach_timebase_info {
    uint32_t numer;
    uint32_t denom;
};
extern "C" {
uint64_t
mach_absolute_time() noexcept;
kern_return_t
mach_timebase_info(mach_timebase_info*) noexcept;
kern_return_t
mach_wait_until(uint64_t deadline) noexcept;
}

#define KERN_SUCCESS 0

static struct mach_timebase_info timebase = {0, 0};

static Nanoseconds
fromAbsolute(uint64_t machTime) noexcept {
    uint64_t ns = machTime *
        static_cast<uint64_t>(timebase.numer) /
        static_cast<uint64_t>(timebase.denom);
    return static_cast<Nanoseconds>(ns);
}

static uint64_t
toAbsolute(Nanoseconds ns) noexcept {
    return static_cast<uint64_t>(ns) *
        static_cast<uint64_t>(timebase.denom) /
        static_cast<uint64_t>(timebase.numer);
}

static void
initTimebase(void) {
    if (timebase.numer == 0 && timebase.denom == 0) {
        kern_return_t err = mach_timebase_info(&timebase);
        (void)err;
        assert_(err == KERN_SUCCESS);
    }
}

Nanoseconds
chronoNow(void) noexcept {
    initTimebase();
    return fromAbsolute(mach_absolute_time());
}

void
chronoSleep(Nanoseconds ns) noexcept {
    if (ns <= 0) {
        return;
    }

    initTimebase();
    uint64_t deadline = mach_absolute_time() + toAbsolute(ns);

    kern_return_t err;
    do {
        err = mach_wait_until(deadline);
    } while (err != KERN_SUCCESS);
}
