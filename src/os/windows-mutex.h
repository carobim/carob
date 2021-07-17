#ifndef SRC_OS_WINDOWS_MUTEX_H_
#define SRC_OS_WINDOWS_MUTEX_H_

#include "os/c.h"
#include "util/compiler.h"

extern "C" {
typedef struct {
    PVOID Ptr;
} SRWLOCK, *PSRWLOCK;

#define SRWLOCK_INIT \
    { 0 }

WINBASEAPI VOID WINAPI
AcquireSRWLockExclusive(PSRWLOCK SRWLock) noexcept;
WINBASEAPI BOOLEAN WINAPI
TryAcquireSRWLockExclusive(PSRWLOCK SRWLock) noexcept;
WINBASEAPI VOID WINAPI
ReleaseSRWLockExclusive(PSRWLOCK SRWLock) noexcept;
}

class Mutex {
 public:
    constexpr Mutex() noexcept : m(SRWLOCK_INIT) { }

    inline void
    lock() noexcept {
        AcquireSRWLockExclusive(&m);
    }
    inline bool
    tryLock() noexcept {
        return TryAcquireSRWLockExclusive(&m) != 0;
    }
    inline void
    unlock() noexcept {
        ReleaseSRWLockExclusive(&m);
    }

    SRWLOCK m;

 private:
    Mutex(const Mutex&) = delete;
    Mutex&
    operator=(const Mutex&) = delete;
};

#endif  // SRC_OS_WINDOWS_MUTEX_H_
