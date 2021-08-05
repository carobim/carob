#ifndef SRC_OS_MAPPED_FILE_H_
#define SRC_OS_MAPPED_FILE_H_

#if MSVC
#    include "os/windows/mapped-file.h"
#else
#    include "os/unix/mapped-file.h"
#endif

#endif  // SRC_OS_MAPPED_FILE_H_
