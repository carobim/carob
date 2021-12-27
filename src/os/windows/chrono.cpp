#include "os/c.h"
#include "os/chrono.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"

extern "C" {
WINBASEAPI
BOOL WINAPI
QueryPerformanceFrequency(LARGE_INTEGER*) noexcept;
WINBASEAPI
BOOL WINAPI
QueryPerformanceCounter(LARGE_INTEGER*) noexcept;
WINBASEAPI
VOID WINAPI Sleep(DWORD) noexcept;
}

static bool haveFreq = false;
static LARGE_INTEGER freq;

Nanoseconds
chronoNow() noexcept {
    if (!haveFreq) {
        haveFreq = true;

        BOOL ok = QueryPerformanceFrequency(&freq);
        assert_(ok);
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return static_cast<Nanoseconds>(s_to_ns(counter.QuadPart) / freq.QuadPart);
}

void
chronoSleep(Nanoseconds ns) noexcept {
    if (ns <= 0) {
        return;
    }

    DWORD ms = static_cast<DWORD>((ns + 999999) / 1000000);
    Sleep(ms);
}
