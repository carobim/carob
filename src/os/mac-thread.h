#ifndef SRC_OS_MAC_THREAD_H_
#define SRC_OS_MAC_THREAD_H_

#include "os/thread.h"

#include "util/compiler.h"
#include "util/function.h"

class Thread {
 public:
    explicit Thread(Function fn) noexcept;
    Thread(Thread&& other) noexcept;
    ~Thread() noexcept;

    void
    join() noexcept;

 private:
    Thread(const Thread&);
    Thread&
    operator=(const Thread&);

 public:
    void* t;
};

void
threadDisableTimerCoalescing() noexcept;
unsigned
threadHardwareConcurrency() noexcept;

#endif  // SRC_OS_MAC_THREAD_H_
