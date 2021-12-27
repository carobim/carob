#ifndef SRC_OS_UNIX_CONDITION_VARIABLE_H_
#define SRC_OS_UNIX_CONDITION_VARIABLE_H_

#include "os/c.h"
#include "os/mutex.h"
#include "util/assert.h"
#include "util/compiler.h"

class ConditionVariable {
 public:
    inline ConditionVariable() noexcept : cv PTHREAD_COND_INITIALIZER { }

    inline ~ConditionVariable() noexcept {
        I32 err = pthread_cond_destroy(&cv);
        assert_(err == 0);
    }

    inline void
    notifyOne() noexcept {
        I32 err = pthread_cond_signal(&cv);
        assert_(err == 0);
    }

    inline void
    notifyAll() noexcept {
        I32 err = pthread_cond_broadcast(&cv);
        assert_(err == 0);
    }

    inline void
    wait(LockGuard& lock) noexcept {
        I32 err = pthread_cond_wait(&cv, &lock.m.m);
        assert_(err == 0);
    }

 private:
    ConditionVariable(const ConditionVariable&);
    ConditionVariable&
    operator=(const ConditionVariable&);

 public:
    pthread_cond_t cv;
};

#endif  // SRC_OS_UNIX_CONDITION_VARIABLE_H_
