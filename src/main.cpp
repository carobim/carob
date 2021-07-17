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
