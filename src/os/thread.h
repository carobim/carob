#ifndef SRC_OS_THREAD_H_
#define SRC_OS_THREAD_H_

#include "util/function.h"

#if defined(_WIN32)
#    include "os/windows/thread.h"
#elif defined(__APPLE__)
#    include "os/mac/thread.h"
#elif defined(__linux__) || defined(__EMSCRIPTEN__)
#    include "os/unix/thread.h"
#elif defined(__FreeBSD__)
#    include "os/unix/thread.h"
#elif defined(__NetBSD__)
#    include "os/unix/thread.h"
#endif

#endif  // SRC_OS_THREAD_H_
