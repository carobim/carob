#ifndef SRC_CORE_LOG_H_
#define SRC_CORE_LOG_H_

#include "util/compiler.h"
#include "util/string-view.h"

enum LogVerbosity {
    QUIET = 1,  // Display fatals.
    NORMAL,     // Display fatals and errors.
    VERBOSE     // Display fatals, errors and info.
};

// Initialize the clock for log timestamps.
bool
logInit() noexcept;

// Set the logging verbosity. Some log messages may be suppressed depending
// on this setting.
void
logSetVerbosity(LogVerbosity mode) noexcept;

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

// Used by main() to report the verbosity setting on engine startup.
void
logReportVerbosityOnStartup() noexcept;

#endif  // SRC_CORE_LOG_H_
