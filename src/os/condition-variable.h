#ifndef SRC_OS_CONDITION_VARIBLE_H_
#define SRC_OS_CONDITION_VARIBLE_H_

#if MSVC
#    include "os/windows/condition-variable.h"
#else
#    include "os/unix/condition-variable.h"
#endif

#endif  // SRC_OS_CONDITION_VARIBLE_H_
