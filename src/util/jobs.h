#ifndef SRC_UTIL_JOBS_H_
#define SRC_UTIL_JOBS_H_

#include "util/compiler.h"
#include "util/function.h"

typedef Function<void() noexcept> Job;

void
JobsEnqueue(Job job) noexcept;
void
JobsFlush() noexcept;

#endif  // SRC_UTIL_JOBS_H_
