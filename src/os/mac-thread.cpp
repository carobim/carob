#include "os/mac-thread.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"

extern "C" {
// mach/thread_act.h
int32_t
thread_policy_set(uint32_t, uint32_t, uint32_t*, uint32_t) noexcept;

// mach/mach_init.h
uint32_t
mach_thread_self() noexcept;
}

// mach/thread_policy.h
#define THREAD_EXTENDED_POLICY       1
#define THREAD_EXTENDED_POLICY_COUNT 1
struct thread_extended_policy {
    int32_t timeshare;
};

static void*
run(void* data) noexcept {
    Function fn = *static_cast<Function*>(data);
    free(data);
    fn->fn(fn->data);
    return 0;
}

Thread::Thread(Function fn) noexcept {
    Function* data = xmalloc(Function, 1);
    *data = fn;

    int err = pthread_create(static_cast<pthread_t*>(&t), 0, run, data);
    (void)err;
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

    int err = pthread_join(static_cast<pthread_t>(t), 0);
    (void)err;
    assert_(err == 0);

    t = 0;
}

void
threadDisableTimerCoalescing() noexcept {
    thread_extended_policy policyInfo = {};
    policyInfo.timeshare = false;

    thread_policy_set(mach_thread_self(),
                      THREAD_EXTENDED_POLICY,
                      reinterpret_cast<uint32_t*>(&policyInfo),
                      THREAD_EXTENDED_POLICY_COUNT);
}

unsigned
threadHardwareConcurrency() noexcept {
    unsigned n;
    int mib[2] = {CTL_HW, HW_NCPU};
    size_t s = sizeof(n);
    sysctl(mib, 2, &n, &s, 0, 0);
    return n;
}
