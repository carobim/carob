#ifndef SRC_OS_LINUX_THREAD_H_
#define SRC_OS_LINUX_THREAD_H_

#include "os/thread.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/function.h"
#include "util/int.h"
#include "util/new.h"

static void*
run(void* data) noexcept {
    Function fn = *static_cast<Function*>(data);
    free(data);
    fn.fn(fn.data);
    return 0;
}

class Thread {
 public:
    inline explicit Thread(Function fn) noexcept {
        Function* data = xmalloc(Function, 1);
        *data = fn;

        int err = pthread_create(&t, 0, run, static_cast<void*>(data));
        (void)err;
        assert_(err == 0);
    }

    Thread(Thread&& other) noexcept : t(other.t) { other.t = 0; }
    inline ~Thread() noexcept { assert_(t == 0); }

    inline void
    join() noexcept {
        assert_(t != 0);

        int err = pthread_join(t, 0);
        (void)err;
        assert_(err == 0);

        t = 0;
    }

 private:
    Thread(const Thread&);
    Thread&
    operator=(const Thread&);

 public:
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
