#ifndef SRC_UTIL_JOBS_H_
#define SRC_UTIL_JOBS_H_

#include "util/compiler.h"
#include "util/function.h"

void
JobsEnqueue(Function fn) noexcept;
void
JobsFlush() noexcept;

#endif  // SRC_UTIL_JOBS_H_
