/***************************************
** Tsunagari Tile Engine              **
** log.cpp                            **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2021 Paul Merrill   **
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
#include "os/mutex.h"
#include "os/os.h"
#include "util/assert.h"
#include "util/io.h"
#include "util/math2.h"

#ifdef _WIN32
#    include "os/windows.h"
#endif

#if defined(__APPLE__) && (!defined(WINDOW_NULL) || !defined(AUDIO_NULL))
#    include "os/mac-gui.h"
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
        v = v.substr(0, min(v.size, dot + 4));
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

        //setTermColor(TC_GREEN, Stdout);
        sout << makeTimestamp() << ' ';

        //setTermColor(TC_YELLOW, Stdout);
        sout << "Info [" << domain << ']';

        //setTermColor(TC_RESET, Stdout);
        sout << " - " << chomp(msg) << '\n';
    }
}

void
logErr(StringView domain, StringView msg) noexcept {
    if (verb > LogVerbosity::QUIET) {
        {
            LockGuard lock(stdoutMutex);

            //setTermColor(TC_GREEN, Stderr);
            serr << makeTimestamp() << ' ';

            //setTermColor(TC_RED, Stderr);
            String s = String() << "Error [" << domain << "]";
            serr << "Error [" << domain << ']';

            //setTermColor(TC_RESET, Stderr);
            serr << " - " << chomp(msg) << '\n';
        }

        String s = String() << "Error [" << domain << "] - " << chomp(msg);

#ifdef _WIN32
        wMessageBox("Tsunagari - Error", s);
#endif
#if defined(__APPLE__) && (!defined(WINDOW_NULL) || !defined(AUDIO_NULL))
        macMessageBox(StringView("Tsunagari - Error"), s);
#endif

        debugger();
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

        //setTermColor(TC_GREEN, Stderr);
        serr << makeTimestamp() << ' ';

        //setTermColor(TC_RED, Stderr);
        serr << "Fatal [" << domain << ']';

        //setTermColor(TC_RESET, Stderr);
        serr << " - " << chomp(msg) << '\n';
    }

    String s = String() << "Fatal [" << domain << "] - " << chomp(msg);

#ifdef _WIN32
    wMessageBox("Tsunagari - Fatal", s);
#endif
#if defined(__APPLE__) && (!defined(WINDOW_NULL) || !defined(AUDIO_NULL))
    macMessageBox(StringView("Tsunagari - Fatal"), s);
#endif

    debugger();

    exitProcess(1);
}

void
logReportVerbosityOnStartup() noexcept {
    LockGuard lock(stdoutMutex);

    StringView verbString;
    switch (confVerbosity) {
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

    //setTermColor(TC_GREEN, Stdout);
    sout << makeTimestamp() << ' ';

    //setTermColor(TC_RESET, Stdout);
    sout << "Reporting engine messages in " << verbString << " mode.\n";
}
