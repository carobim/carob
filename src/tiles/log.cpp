#include "tiles/log.h"

#include "os/mutex.h"
#include "os/os.h"
#include "tiles/client-conf.h"
#include "tiles/window.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/io.h"
#include "util/math2.h"

#if MSVC
#    include "os/windows.h"
#endif

#if defined(__APPLE__) && (!defined(WINDOW_NULL) || !defined(AUDIO_NULL))
#    include "os/mac/gui.h"
#endif

static LogVerbosity verb = NORMAL;

static Time startTime;

static Mutex stdoutMutex;

static StringView
chomp(StringView str) noexcept {
    Size size = str.size;
    while (size > 0 &&
           (str.data[size - 1] == ' ' || str.data[size - 1] == '\t' ||
            str.data[size - 1] == '\n' || str.data[size - 1] == '\r')) {
        size -= 1;
    }
    return str.substr(0, size);
}

static String
makeTimestamp() noexcept {
    Time now = windowTime();

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
    if (verb > NORMAL) {
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
    if (verb > QUIET) {
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

#if MSVC
        wMessageBox("Tsunagari - Error", s);
#endif
#if defined(__APPLE__) && (!defined(WINDOW_NULL) || !defined(AUDIO_NULL))
        macMessageBox(StringView("Tsunagari - Error"), s);
#endif

        debugger();
    }
}

#if MSVC
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

#if MSVC
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
    case QUIET:
        verbString = "QUIET";
        break;
    case NORMAL:
        verbString = "NORMAL";
        break;
    case VERBOSE:
        verbString = "VERBOSE";
        break;
    }

    //setTermColor(TC_GREEN, Stdout);
    sout << makeTimestamp() << ' ';

    //setTermColor(TC_RESET, Stdout);
    sout << "Reporting engine messages in " << verbString << " mode.\n";
}
