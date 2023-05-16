#ifndef SRC_OS_WINDOWS_MUTEX_H_
#define SRC_OS_WINDOWS_MUTEX_H_

#include "os/c.h"
#include "util/compiler.h"

extern "C" {
typedef struct {
    PVOID Ptr;
} SRWLOCK, *PSRWLOCK;

WINBASEAPI VOID WINAPI
AcquireSRWLockExclusive(PSRWLOCK SRWLock) noexcept;
WINBASEAPI BOOLEAN WINAPI
TryAcquireSRWLockExclusive(PSRWLOCK SRWLock) noexcept;
WINBASEAPI VOID WINAPI
ReleaseSRWLockExclusive(PSRWLOCK SRWLock) noexcept;
}

class Mutex {
 public:
    inline Mutex() noexcept { m.Ptr = 0; }

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
    Mutex(const Mutex&);
    Mutex&
    operator=(const Mutex&);
};

#endif  // SRC_OS_WINDOWS_MUTEX_H_
