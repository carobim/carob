#include "os/mac/thread.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"

extern "C" {
// mach/thread_act.h
I32
thread_policy_set(U32, U32, U32*, U32) noexcept;

// mach/mach_init.h
U32
mach_thread_self() noexcept;
}

// mach/thread_policy.h
#define THREAD_EXTENDED_POLICY       1
#define THREAD_EXTENDED_POLICY_COUNT 1
struct thread_extended_policy {
    I32 timeshare;
};

static void*
run(void* data) noexcept {
    Function fn = *static_cast<Function*>(data);
    free(data);
    fn.fn(fn.data);
    return 0;
}

Thread::Thread(Function fn) noexcept {
    new0(Function, data);
    *data = fn;

    I32 err = pthread_create(reinterpret_cast<pthread_t*>(&t), 0, run, data);
    assert_(err == 0);
}

Thread::Thread(Thread&& other) noexcept : t(other.t) {
    other.t = 0;
}

Thread::~Thread() noexcept {
    assert_(t == 0);
}

void
Thread::join() noexcept {
    assert_(t != 0);

    I32 err = pthread_join(static_cast<pthread_t>(t), 0);
    assert_(err == 0);

    t = 0;
}

void
threadDisableTimerCoalescing() noexcept {
    thread_extended_policy policyInfo = {};
    policyInfo.timeshare = false;

    thread_policy_set(mach_thread_self(),
                      THREAD_EXTENDED_POLICY,
                      reinterpret_cast<U32*>(&policyInfo),
                      THREAD_EXTENDED_POLICY_COUNT);
}

U32
threadHardwareConcurrency() noexcept {
    unsigned n;
    I32 mib[2] = {CTL_HW, HW_NCPU};
    Size s = sizeof(n);
    sysctl(mib, 2, &n, &s, 0, 0);
    return n;
}
