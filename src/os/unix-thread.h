#ifndef SRC_OS_LINUX_THREAD_H_
#define SRC_OS_LINUX_THREAD_H_

#include "os/c.h"
#include "os/thread.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/function.h"
#include "util/int.h"

static void*
run(void* f) noexcept {
    Function<void()>* fun = reinterpret_cast<Function<void()>*>(f);
    (*fun)();
    return 0;
}

class Thread {
 public:
    inline explicit Thread(Function<void()> f) noexcept {
        Function<void()>* fun =
                new Function<void()>(static_cast<Function<void()>&&>(f));

        int err = pthread_create(&t, 0, run, static_cast<void*>(fun));
        (void)err;
        assert_(err == 0);
    }

    Thread(Thread&& other) noexcept : t(other.t) { other.t = 0; }
    inline ~Thread() noexcept { assert_(t == 0); }

    Thread(const Thread&) = delete;
    Thread&
    operator=(const Thread&) = delete;

    inline void
    join() noexcept {
        assert_(t != 0);

        int err = pthread_join(t, 0);
        (void)err;
        assert_(err == 0);

        t = 0;
    }

    pthread_t t;
};

static inline unsigned
threadHardwareConcurrency() noexcept {
    long result = sysconf(_SC_NPROCESSORS_ONLN);
    if (result < 0) {
        return 0;
    }
    return static_cast<unsigned>(result);
}

static inline void
threadDisableTimerCoalescing() noexcept { }

#endif  // SRC_OS_LINUX_THREAD_H_
