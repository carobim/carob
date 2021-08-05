#ifndef SRC_OS_MUTEX_H_
#define SRC_OS_MUTEX_H_

#if MSVC
#    include "os/windows/mutex.h"
#else
#    include "os/unix/mutex.h"
#endif

#include "util/compiler.h"

class LockGuard {
 public:
    inline explicit LockGuard(Mutex& m) noexcept : m(m) { m.lock(); }
    inline ~LockGuard() noexcept { m.unlock(); }

 private:
    LockGuard(const LockGuard&);
    LockGuard&
    operator=(const LockGuard&);

 public:
    Mutex& m;
};

#endif  // SRC_OS_MUTEX_H_
