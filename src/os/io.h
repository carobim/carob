#ifndef SRC_OS_IO_H_
#define SRC_OS_IO_H_

#if defined(_WIN32)
#    include "os/windows/io.h"
#else
#    include "os/unix/io.h"
#endif

#endif  // SRC_OS_IO_H_
