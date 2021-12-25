#include "config.h"
#include "data/data-world.h"
#include "os/thread.h"
#include "tiles/client-conf.h"
#include "tiles/images.h"
#include "tiles/log.h"
#include "tiles/measure.h"
#include "tiles/resources.h"
#include "tiles/window.h"
#include "tiles/world.h"
#include "util/compiler.h"
#include "util/random.h"

#if MSVC
#    include "os/windows.h"
#endif

#ifdef __APPLE__
#    include "os/mac/gui.h"
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

    initRandom();

    if (!logInit()) {
        return 1;
    }

#if defined(__APPLE__) && (!defined(WINDOW_NULL) || !defined(AUDIO_NULL))
    macSetWorkingDirectory();
#endif

    // TODO: Use CVDisplayLink and remove this.
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
