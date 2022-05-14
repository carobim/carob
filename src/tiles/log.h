#ifndef SRC_TILES_LOG_H_
#define SRC_TILES_LOG_H_

#include "util/compiler.h"
#include "util/string-view.h"

// Initialize the clock for log timestamps.
void
logInit() noexcept;

// Log an info message to the console if verbosity is V_VERBOSE.
void
logInfo(StringView domain, StringView msg) noexcept;

// Log an error message to the console if verbosity is V_VERBOSE or
// V_NORMAL.
void
logErr(StringView domain, StringView msg) noexcept;

// Log a fatal error message to the console.
void
logFatal(StringView domain, StringView msg) noexcept;

#endif  // SRC_TILES_LOG_H_
