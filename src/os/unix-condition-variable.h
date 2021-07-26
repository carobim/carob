#ifndef SRC_OS_UNIX_CONDITION_VARIBLE_H_
#define SRC_OS_UNIX_CONDITION_VARIBLE_H_

#include "os/c.h"
#include "os/mutex.h"
#include "util/assert.h"
#include "util/compiler.h"

class ConditionVariable {
 public:
    inline ConditionVariable() : cv PTHREAD_COND_INITIALIZER { }

    inline ~ConditionVariable() noexcept {
        int err = pthread_cond_destroy(&cv);
        (void)err;
        assert_(err == 0);
    }

    inline void
    notifyOne() noexcept {
        int err = pthread_cond_signal(&cv);
        (void)err;
        assert_(err == 0);
    }

    inline void
    notifyAll() noexcept {
        int err = pthread_cond_broadcast(&cv);
        (void)err;
        assert_(err == 0);
    }

    inline void
    wait(LockGuard& lock) noexcept {
        int err = pthread_cond_wait(&cv, &lock.m.m);
        (void)err;
        assert_(err == 0);
    }

 private:
    ConditionVariable(const ConditionVariable&);
    ConditionVariable&
    operator=(const ConditionVariable&);

 public:
    pthread_cond_t cv;
};

#endif  // SRC_OS_UNIX_CONDITION_VARIBLE_H_
