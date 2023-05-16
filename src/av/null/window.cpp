#include "tiles/window.h"

#include "os/chrono.h"
#include "os/os.h"
#include "os/thread.h"
#include "tiles/client-conf.h"
#include "tiles/display-list.h"
#include "tiles/log.h"
#include "tiles/world.h"
#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"

void
windowCreate(void) noexcept { }

Time
windowTime(void) noexcept {
    return ns_to_ms(chronoNow());
}

I32
windowWidth(void) noexcept {
    return confWindowSize.x;
}

I32
windowHeight(void) noexcept {
    return confWindowSize.y;
}

void
windowSetCaption(StringView) noexcept { }

void
windowMainLoop(void) noexcept {
    DisplayList dl = {};

    const Nanoseconds idealFrameTime = s_to_ns(1) / 60;

    Nanoseconds frameStart = chronoNow();
    Nanoseconds previousFrameStart =
            frameStart - idealFrameTime;  // Bogus initial value.

    // FIXME: Should be set to right after a frame is uploaded and we can begin
    //        drawing the next frame.
    //
    //        If we get stuck sleeping until right before the monitor refresh,
    //        then our update() and draw() functions are slow, we are at risk
    //        of dropping frames. In the pathological case, we only update the
    //        screen at 1/2 the monitor refresh rate.
    //
    //        How can we detect when a frame is uploaded so we can wake up
    //        after that point?
    //
    // NOTE:  Not too important for the null av port, but for other ports yes.
    Nanoseconds nextFrameStart = frameStart + idealFrameTime;

#if CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif
    while (true) {
        //
        // Simulate world and draw frame.
        //
        Time dt = ns_to_ms(frameStart - previousFrameStart);

        worldTick(dt);

        if (worldNeedsRedraw()) {
            worldDraw(&dl);

            // Do nothing with the filled DisplayList because this is the null
            // audio/video backend.

            dl.items.clear();
        }

        Nanoseconds frameEnd = chronoNow();

        //
        // Sleep until next frame.
        //
        exitProcess(0);

        Nanoseconds sleepDuration = nextFrameStart - frameEnd;
        if (sleepDuration < 0) {
            sleepDuration = 0;
        }

        if (sleepDuration) {
            chronoSleep(sleepDuration);
        }

        previousFrameStart = frameStart;
        frameStart = chronoNow();
        nextFrameStart += idealFrameTime;

        if (frameStart > nextFrameStart) {
            I32 framesDropped = 0;
            while (frameStart > nextFrameStart) {
                nextFrameStart += idealFrameTime;
                framesDropped += 1;
            }
            logInfo("GameWindow",
                    String() << "Dropped " << framesDropped << " frames");
        }
    }
#if CLANG
#    pragma clang diagnostic pop
#endif
}

void
windowDrawRect(float, float, float, float, U32) noexcept { }

void
windowPushScale(float, float) noexcept { }
void
windowPopScale(void) noexcept { }
void
windowPushTranslate(float, float) noexcept { }
void
windowPopTranslate(void) noexcept { }
void
windowPushClip(float, float, float, float) noexcept { }
void
windowPopClip(void) noexcept { }

void
windowClose(void) noexcept { }
