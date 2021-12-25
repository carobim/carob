#ifndef SRC_UTIL_MEASURE_H_
#define SRC_UTIL_MEASURE_H_

#include "os/chrono.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string.h"

//
// Enable to profile with Xcode Instruments.
//
//#define MAKE_MACOS_SIGNPOSTS

class TimeMeasure {
 public:
    TimeMeasure(String description) noexcept;
    ~TimeMeasure() noexcept;

 private:
    String description;
    Nanoseconds start;
#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
    U32 signpost;
#endif
};

#endif  // SRC_UTIL_MEASURE_H_
