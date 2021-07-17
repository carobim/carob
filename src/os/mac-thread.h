#ifndef SRC_OS_MAC_THREAD_H_
#define SRC_OS_MAC_THREAD_H_

#include "util/compiler.h"
#include "util/function.h"

class Thread {
 public:
    explicit Thread(Function<void() noexcept> f) noexcept;
    Thread(Thread&& other) noexcept;
    ~Thread() noexcept;

    Thread(const Thread&) = delete;
    Thread&
    operator=(const Thread&) = delete;

    void
    join() noexcept;

 public:
    void* t = 0;
};

void
threadDisableTimerCoalescing() noexcept;
unsigned
threadHardwareConcurrency() noexcept;

#endif  // SRC_OS_MAC_THREAD_H_
