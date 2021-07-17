/***************************************
** Tsunagari Tile Engine              **
** main.cpp                           **
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

#include "config.h"
#include "core/client-conf.h"
#include "core/images.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/window.h"
#include "core/world.h"
#include "data/data-world.h"
#include "os/c.h"
#include "os/chrono.h"
#include "os/thread.h"
#include "util/compiler.h"
#include "util/int.h"

#if MSVC
#    include "os/windows.h"
#endif

#ifdef __APPLE__
#    include "os/mac-gui.h"
#endif

/**
 * Load client config and instantiate window.
 *
 * The client config tells us our window parameters along with which World
 * we're going to load. The GameWindow class then loads and plays the game.
 */
int
main() noexcept {
#if MSVC && !defined(NDEBUG)
    wFixConsole();
#endif

    srand(static_cast<unsigned>(chronoNow()));

    if (!logInit()) {
        return 1;
    }

#if defined(__APPLE__) && (!defined(WINDOW_NULL) || !defined(AUDIO_NULL))
    macSetWorkingDirectory();
#endif

    threadDisableTimerCoalescing();

    confParse(CLIENT_CONF_PATH);

    logSetVerbosity(confVerbosity);
    logInfo("Main", String() << "Starting " << TSUNAGARI_RELEASE_VERSION);
    logReportVerbosityOnStartup();

    windowCreate();
    imageInit();

    if (!dataWorldInit()) {
        logFatal("Main", "Data World initialization");
        return 1;
    }

    {
        TimeMeasure m("Constructed world");
        if (!worldInit()) {
            logFatal("Main", "World initialization");
            return 1;
        }
    }

    windowSetCaption(dataWorldName);

    windowMainLoop();

    return 0;
}

#if MSVC
int __stdcall WinMain(void*, void*, void*, int) {
    return main();
}
#endif
