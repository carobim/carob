#ifndef SRC_OS_UNIX_MUTEX_H_
#define SRC_OS_UNIX_MUTEX_H_

#include "os/c.h"
#include "util/assert.h"
#include "util/compiler.h"

class Mutex {
 public:
    inline Mutex() noexcept : m PTHREAD_MUTEX_INITIALIZER { }

    inline ~Mutex() noexcept {
        I32 err = pthread_mutex_destroy(&m);
        (void)err;
        assert_(err == 0);
    }

    inline void
    lock() noexcept {
        I32 err = pthread_mutex_lock(&m);
        (void)err;
        assert_(err == 0);
    };

    inline void
    unlock() noexcept {
        I32 err = pthread_mutex_unlock(&m);
        (void)err;
        assert_(err == 0);
    }

 private:
    Mutex(const Mutex&);
    Mutex&
    operator=(const Mutex&);

 public:
    pthread_mutex_t m;
};

#endif  // SRC_OS_UNIX_MUTEX_H_
