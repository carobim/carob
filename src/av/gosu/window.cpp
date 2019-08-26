/***************************************
** Tsunagari Tile Engine              **
** window.cpp                         **
** Copyright 2011-2014 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

#include "av/gosu/window.h"

#include "av/gosu/gosu.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/keyboard.h"
#include "core/window.h"
#include "core/world.h"
#include "os/chrono.h"
#include "util/constexpr.h"
#include "util/function.h"
#include "util/int.h"
#include "util/move.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

// Garbage collection called every X milliseconds
static const TimePoint GC_CALL_PERIOD = 10 * 1000;

struct Keystate {
    TimePoint since = 0;
    bool consecutive = false;
    bool initiallyResolved = false;
};

static bool
operator==(const Keystate& a, const Keystate& b) noexcept {
    return a.since == b.since &&
           a.consecutive == b.consecutive &&
           a.initiallyResolved == b.initiallyResolved;
}

Gosu::Graphics* graphics = nullptr;

static TimePoint start = SteadyClock::nowMS();
static TimePoint lastGCtime = 0;

static Keystate keystates[Gosu::KB_HIGHEST + 1] = {};
static KeyboardKey gosuToTsunagariKey[Gosu::KB_HIGHEST + 1] = {};

static DisplayList display;

static void
handleKeyboardInput(time_t now) {
    // Persistent input handling code
    for (int i = 0; i < Gosu::KB_HIGHEST + 1; i++) {
        if (keystates[i] == Keystate()) {
            continue;
        }

        Keystate& state = keystates[i];
        KeyboardKey mapped = gosuToTsunagariKey[i];

        // If there is persistCons milliseconds of latency
        // between when a button is depressed and when we first look at
        // it here, we'll incorrectly try to fire off a second round of
        // input.
        // This can happen if an intermediary function blocks the thread
        // for a while.
        if (!state.initiallyResolved) {
            state.initiallyResolved = true;
            continue;
        }

        time_t delay =
                state.consecutive ? Conf::persistCons : Conf::persistInit;
        if (now >= state.since + delay) {
            state.since += delay;
            World::buttonDown(mapped);
            state.consecutive = true;
        }
    }
}

class GosuWindow : public Gosu::Window {
 public:
    GosuWindow() noexcept;

    void button_down(const Gosu::Button btn) noexcept final;
    void button_up(const Gosu::Button btn) noexcept final;
    void draw() noexcept final;
    bool needs_redraw() const noexcept final;
    void update() noexcept final;
};

static GosuWindow* window = nullptr;

GosuWindow::GosuWindow() noexcept
        // Gosu emulates the requested screen resolution on fullscreen,
        // but this breaks our aspect ratio-correcting letterbox.
        // Ergo we just make a window the size of the screen.
        : Gosu::Window(
                  Conf::fullscreen ? Gosu::screen_width(nullptr)
                                   : static_cast<unsigned>(Conf::windowSize.x),
                  Conf::fullscreen ? Gosu::screen_height(nullptr)
                                   : static_cast<unsigned>(Conf::windowSize.y),
                  Conf::fullscreen,
                  1000.0f / 60.0f) {
    ::graphics = &this->graphics();

    auto& keys = gosuToTsunagariKey;
    keys[Gosu::KB_ESCAPE] = KBEscape;
    keys[Gosu::KB_LEFT_SHIFT] = KBLeftShift;
    keys[Gosu::KB_RIGHT_SHIFT] = KBRightShift;
    keys[Gosu::KB_LEFT_CONTROL] = KBLeftControl;
    keys[Gosu::KB_RIGHT_CONTROL] = KBRightControl;
    keys[Gosu::KB_SPACE] = KBSpace;
    keys[Gosu::KB_LEFT] = KBLeftArrow;
    keys[Gosu::KB_RIGHT] = KBRightArrow;
    keys[Gosu::KB_UP] = KBUpArrow;
    keys[Gosu::KB_DOWN] = KBDownArrow;
}

void
GosuWindow::button_down(Gosu::Button btn) noexcept {
    if (btn > Gosu::KB_HIGHEST) {
        return;
    }

    if (keystates[btn] == Keystate()) {
        Keystate& state = keystates[btn];
        state.since = GameWindow::time();
        state.initiallyResolved = false;
        state.consecutive = false;
    }

    // We process the initial buttonDown here so that it
    // gets handled even if we receive a buttonUp before an
    // update.
    KeyboardKey mapped = gosuToTsunagariKey[btn];
    if (mapped) {
        GameWindow::emitKeyDown(mapped);
    }
}

void
GosuWindow::button_up(Gosu::Button btn) noexcept {
    if (btn > Gosu::KB_HIGHEST) {
        return;
    }

    keystates[btn] = Keystate();

    KeyboardKey mapped = gosuToTsunagariKey[btn];
    if (mapped) {
        GameWindow::emitKeyUp(mapped);
    }
}

void
GosuWindow::draw() noexcept {
    display.items.clear();
    World::draw(&display);
    displayListPresent(&display);
}

bool
GosuWindow::needs_redraw() const noexcept {
    return World::needsRedraw();
}

void
GosuWindow::update() noexcept {
    time_t now = GameWindow::time();

    if (Conf::moveMode == Conf::TURN) {
        handleKeyboardInput(now);
    }
    World::tick(now);

    if (now > lastGCtime + GC_CALL_PERIOD) {
        lastGCtime = now;
        World::garbageCollect();
    }
}

void
GameWindow::create() noexcept {
    window = new GosuWindow();
}

time_t
GameWindow::time() noexcept {
    return SteadyClock::nowMS() - start;
}

int
GameWindow::width() noexcept {
    return static_cast<int>(window->width());
}

int
GameWindow::height() noexcept {
    return static_cast<int>(window->height());
}

void
GameWindow::setCaption(StringView caption) noexcept {
    window->set_caption(std::string(caption.data, caption.size));
}

void
GameWindow::mainLoop() noexcept {
    window->show();
}

void
GameWindow::drawRect(float x1,
                     float x2,
                     float y1,
                     float y2,
                     uint32_t argb) noexcept {
    Gosu::Color c{argb};
    CONSTEXPR11 double top = std::numeric_limits<double>::max();
    graphics->draw_quad(
            x1, y1, c, x2, y1, c, x2, y2, c, x1, y2, c, top, Gosu::AM_DEFAULT);
}

void
GameWindow::scale(float x, float y, Function<void()> op) noexcept {
    graphics->transform(Gosu::scale(x, y, 0, 0), move_(op));
}

void
GameWindow::translate(float x, float y, Function<void()> op) noexcept {
    graphics->transform(Gosu::translate(x, y), move_(op));
}

void
GameWindow::clip(float x,
                 float y,
                 float width,
                 float height,
                 Function<void()> op) noexcept {
    graphics->clip_to(x, y, width, height, move_(op));
}

void
GameWindow::close() noexcept {
    window->close();
}
