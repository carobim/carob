/***************************************
** Tsunagari Tile Engine              **
** log.cpp                            **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2020 Paul Merrill   **
***************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#include "core/log.h"

#include "core/client-conf.h"
#include "core/window.h"
#include "os/c.h"
#include "os/mutex.h"
#include "os/os.h"
#include "util/algorithm.h"

#ifdef _WIN32
#include "os/windows.h"
#endif

#ifdef __APPLE__
#include "os/mac-gui.h"
#endif

static LogVerbosity verb = LogVerbosity::NORMAL;

static time_t startTime;

static Mutex stdoutMutex;

static StringView
chomp(StringView str) noexcept {
    size_t size = str.size;
    while (size > 0 &&
           (str.data[size - 1] == ' ' || str.data[size - 1] == '\t' ||
            str.data[size - 1] == '\n' || str.data[size - 1] == '\r')) {
        size -= 1;
    }
    return str.substr(0, size);
}

static String
makeTimestamp() noexcept {
    time_t now = windowTime();

    float secs = (now - startTime) / (float)1000.0;

    String s;
    s << secs;

    StringView v = s.view();

    StringPosition dot = v.find('.');
    if (dot != SV_NOT_FOUND) {
        v = v.substr(0, min_(v.size, dot + 4));
    }

    String s2;
    s2 << "[" << v << "]";
    return s2;
}

bool
logInit() noexcept {
    startTime = windowTime();
    return true;
}

void
logSetVerbosity(LogVerbosity v) noexcept {
    verb = v;
}

void
logInfo(StringView domain, StringView msg) noexcept {
    if (verb > LogVerbosity::NORMAL) {
        LockGuard lock(stdoutMutex);

        setTermColor(TC_GREEN, Stdout);
        printf("%s ", makeTimestamp().null());

        setTermColor(TC_YELLOW, Stdout);
        String s;
        s << "Info [" << domain << "]";
        printf("%s", s.null());

        setTermColor(TC_RESET, Stdout);
        s.clear();
        s << " - " << chomp(msg) << "\n";
        printf("%s", s.null());
    }
}

void
logErr(StringView domain, StringView msg) noexcept {
    if (verb > LogVerbosity::QUIET) {
        {
            LockGuard lock(stdoutMutex);

            setTermColor(TC_GREEN, Stderr);
            fprintf(stderr, "%s ", makeTimestamp().null());

            setTermColor(TC_RED, Stderr);
            String s;
            s << "Error [" << domain << "]";
            fprintf(stderr, "%s", s.null());

            setTermColor(TC_RESET, Stderr);
            s.clear();
            s << " - " << chomp(msg) << "\n";
            fprintf(stderr, "%s", s.null());
        }

        String s;
        s << "Error [" << domain << "] - " << chomp(msg);

#ifdef _WIN32
        wMessageBox("Tsunagari - Error", s);
#endif
#ifdef __APPLE__
        macMessageBox(StringView("Tsunagari - Error"), s);
#endif
    }
}

#ifdef _WIN32
extern "C" {
__declspec(dllimport) int __stdcall IsDebuggerPresent();
}
void __cdecl __debugbreak();
#endif

void
logFatal(StringView domain, StringView msg) noexcept {
    {
        LockGuard lock(stdoutMutex);

        setTermColor(TC_GREEN, Stderr);
        fprintf(stderr, "%s ", makeTimestamp().null());

        setTermColor(TC_RED, Stderr);
        String s;
        s << "Fatal [" << domain << "]";
        fprintf(stderr, "%s", s.null());

        setTermColor(TC_RESET, Stderr);
        s.clear();
        s << " - " << chomp(msg) << "\n";
        fprintf(stderr, "%s", s.null());
    }

    String s;
    s << "Fatal [" << domain << "] - " << chomp(msg);

#ifdef _WIN32
    wMessageBox("Tsunagari - Fatal", s);

    if (IsDebuggerPresent()) {
        __debugbreak();
    }
#endif
#ifdef __APPLE__
    macMessageBox(StringView("Tsunagari - Fatal"), s);
#endif

    exitProcess(1);
}

void
logReportVerbosityOnStartup() noexcept {
    LockGuard lock(stdoutMutex);

    StringView verbString;
    switch (Conf::verbosity) {
    case LogVerbosity::QUIET:
        verbString = "QUIET";
        break;
    case LogVerbosity::NORMAL:
        verbString = "NORMAL";
        break;
    case LogVerbosity::VERBOSE:
        verbString = "VERBOSE";
        break;
    }

    setTermColor(TC_GREEN, Stdout);
    printf("%s ", makeTimestamp().null());

    setTermColor(TC_RESET, Stdout);
    String s;
    s << "Reporting engine messages in " << verbString << " mode.\n";
    printf("%s", s.null());
}
