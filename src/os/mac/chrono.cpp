#include "os/chrono.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"

// mach/arm/kern_return.h
// mach/i386/kern_return.h
typedef I32 kern_return_t;

// mach/mach_time.h
struct mach_timebase_info {
    U32 numer;
    U32 denom;
};
extern "C" {
U64
mach_absolute_time() noexcept;
kern_return_t
mach_timebase_info(mach_timebase_info*) noexcept;
kern_return_t
mach_wait_until(U64 deadline) noexcept;
}

#define KERN_SUCCESS 0

static struct mach_timebase_info timebase = {0, 0};

static Nanoseconds
fromAbsolute(U64 machTime) noexcept {
    U64 ns = machTime *
        static_cast<U64>(timebase.numer) /
        static_cast<U64>(timebase.denom);
    return static_cast<Nanoseconds>(ns);
}

static U64
toAbsolute(Nanoseconds ns) noexcept {
    return static_cast<U64>(ns) *
        static_cast<U64>(timebase.denom) /
        static_cast<U64>(timebase.numer);
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
    U64 deadline = mach_absolute_time() + toAbsolute(ns);

    kern_return_t err;
    do {
        err = mach_wait_until(deadline);
    } while (err != KERN_SUCCESS);
}
