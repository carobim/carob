/*************************************
** Tsunagari Tile Engine            **
** window.cpp                       **
** Copyright 2016-2020 Paul Merrill **
*************************************/

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

#include "av/sdl2/window.h"

#include "av/sdl2/error.h"
#include "av/sdl2/sdl2.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/window.h"
#include "core/world.h"
#include "os/chrono.h"
#include "os/os.h"
#include "util/noexcept.h"
#include "util/transform.h"

SDL_Window* sdl2Window = 0;
rvec2 sdl2Translation = {0.0, 0.0};
rvec2 sdl2Scaling = {0.0, 0.0};

static Nanoseconds start = 0;

static Transform transformStack[10] = { Transform::identity() };
static size_t transformCount = 1;

static int
getRefreshRate(SDL_Window* window) noexcept {
    // SDL_GetWindowDisplayIndex computes which display the window is on each
    // time.
    int display = SDL_GetWindowDisplayIndex(window);
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(display, &mode);
    return mode.refresh_rate;
}

static void
handleEvent(const SDL_Event& event) noexcept {
    Key key;

    switch (event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            key = KEY_ESCAPE;
            break;
        case SDLK_LCTRL:
            key = KEY_LEFT_CONTROL;
            break;
        case SDLK_RCTRL:
            key = KEY_RIGHT_CONTROL;
            break;
        case SDLK_LSHIFT:
            key = KEY_LEFT_SHIFT;
            break;
        case SDLK_RSHIFT:
            key = KEY_RIGHT_SHIFT;
            break;
        case SDLK_SPACE:
            key = KEY_SPACE;
            break;
        case SDLK_LEFT:
            key = KEY_LEFT_ARROW;
            break;
        case SDLK_RIGHT:
            key = KEY_RIGHT_ARROW;
            break;
        case SDLK_UP:
            key = KEY_UP_ARROW;
            break;
        case SDLK_DOWN:
            key = KEY_DOWN_ARROW;
            break;
        default:
            return;
        }
        if (event.type == SDL_KEYUP) {
            windowEmitKeyUp(key);
        }
        else if (event.type == SDL_KEYDOWN) {
            windowEmitKeyDown(key);
        }
        break;

    case SDL_QUIT:
        SDL_HideWindow(sdl2Window);
        exitProcess(0);
        return;

    default:
        return;
    }
}

static void
handleEvents() noexcept {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }
}

static void
updateTransform() noexcept {
    int w, h;

    SDL_GetWindowSize(sdl2Window, &w, &h);

    Transform transform = transformStack[transformCount - 1];

    float xScale = transform[0];
    float yScale = transform[5];
    float x = transform[12];
    float y = transform[13];

    sdl2Translation = {x / xScale, y / yScale};
    sdl2Scaling = {xScale, yScale};
}

time_t
windowTime() noexcept {
    if (start == 0) {
        start = chronoNow();
    }
    return ns_to_ms(chronoNow() - start);
}

void
windowCreate() noexcept {
    {
        TimeMeasure m("Initialized SDL2 video subsystem");
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            sdlDie("SDL2", "SDL_Init(SDL_INIT_VIDEO)");
        }
    }

    {
        TimeMeasure m("Created SDL2 window");

        int width = Conf::windowSize.x;
        int height = Conf::windowSize.y;

        uint32_t flags = 0;
        if (Conf::fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        sdl2Window = SDL_CreateWindow(
                "Tsunagari", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                width, height, flags);

        if (sdl2Window == 0) {
            sdlDie("SDL2", "SDL_CreateWindow");
        }
    }
}

int
windowWidth() noexcept {
    int w, h;
    SDL_GetWindowSize(sdl2Window, &w, &h);
    return w;
}

int
windowHeight() noexcept {
    int w, h;
    SDL_GetWindowSize(sdl2Window, &w, &h);
    return h;
}

void
windowSetCaption(StringView caption) noexcept {
    SDL_SetWindowTitle(sdl2Window, String(caption).null());
}

void
windowMainLoop() noexcept {
    SDL_ShowWindow(sdl2Window);

    DisplayList display = {};

    int refreshRate = getRefreshRate(sdl2Window);
    const Nanoseconds idealFrameTime = s_to_ns(1) / refreshRate;

    Nanoseconds frameStart = chronoNow();
    Nanoseconds previousFrameStart =
        frameStart - idealFrameTime;  // Bogus initial value.

    Nanoseconds nextFrameStart = frameStart + idealFrameTime;

    while (sdl2Window != 0) {
        handleEvents();

        //
        // Simulate world and draw frame.
        //
        time_t dt = ns_to_ms(frameStart - previousFrameStart);

        assert_(dt >= 0);

        if (dt > 0) {
            worldTick(dt);
        }
        else {
            // FIXME: Why does this happen and what should be done when it
            //        occurrs?
            // logInfo("SDL2", "dt == 0");
        }

        bool drew = false;
        if (worldNeedsRedraw()) {
            drew = true;

            worldDraw(&display);

            imageStartFrame();
            displayListPresent(&display);
            imageEndFrame();

            display.items.clear();
        }

        Nanoseconds frameEnd = chronoNow();
        Nanoseconds timeTaken = frameEnd - frameStart;

        //
        // Sleep until next frame.
        //
        Nanoseconds sleepDuration = nextFrameStart - frameEnd;
        if (sleepDuration < 0) {
            sleepDuration = 0;
        }

        /*
        logInfo(
            "SDL2",
            String() << "dt " << ns_to_s_d(frameStart - previousFrameStart)
                     << " frameStart " << ns_to_s_d(frameStart)
                     << " drew " << drew
                     << " timeTaken " << ns_to_s_d(timeTaken)
                     << " nextFrameStart " << ns_to_s_d(nextFrameStart)
                     << " sleepDuration " << ns_to_s_d(sleepDuration)
        );
        */

        // Must sleep even if we drew a frame to handle the case where we don't
        // have vsync, but we are trying to limit frame rate.
        // if (!drew && sleepDuration) {
        if (sleepDuration) {
            chronoSleep(sleepDuration);
        }

        previousFrameStart = frameStart;
        frameStart = chronoNow();
        nextFrameStart += idealFrameTime;

        if (frameStart > nextFrameStart) {
            int framesDropped = 0;
            while (frameStart > nextFrameStart) {
                nextFrameStart += idealFrameTime;
                framesDropped += 1;
            }
            logInfo("SDL2",
                    String() << "Dropped " << framesDropped << " frames");
        }
    }
}

void
windowPushScale(float x, float y) noexcept {
    assert_(x == y);

    float factor = static_cast<float>(x);
    Transform transform = transformStack[transformCount - 1];

    transformStack[transformCount++] = Transform::scale(factor) * transform;
    updateTransform();
}

void
windowPopScale() noexcept {
    transformCount--;
    updateTransform();
}

void
windowPushTranslate(float x, float y) noexcept {
    Transform transform = transformStack[transformCount - 1];
    transformStack[transformCount++] =
            Transform::translate(static_cast<float>(x), static_cast<float>(y)) *
            transform;
    updateTransform();
}

void
windowPopTranslate() noexcept {
    transformCount--;
    updateTransform();
}

void
windowPushClip(float x, float y, float width, float height) noexcept {}

void
windowPopClip() noexcept {}

void
windowClose() noexcept {
    SDL_HideWindow(sdl2Window);
    sdl2Window = 0;
}
