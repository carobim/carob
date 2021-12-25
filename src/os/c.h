#ifndef SRC_OS_C_H_
#define SRC_OS_C_H_

#if defined(_WIN32)
#    include "os/windows/c.h"
#elif defined(__APPLE__)
#    include "os/mac/c.h"
#elif defined(__linux__) || defined(__EMSCRIPTEN__)
#    include "os/linux/c.h"
#elif defined(__FreeBSD__)
#    include "os/freebsd/c.h"
#elif defined(__NetBSD__)
#    include "os/netbsd/c.h"
#else
#    error Not implemented yet
#endif

#endif  // SRC_OS_C_H_
