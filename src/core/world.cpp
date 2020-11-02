/***************************************
** Tsunagari Tile Engine              **
** world.cpp                          **
** Copyright 2011-2015 Michael Reiley **
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

#include "core/world.h"

#include "core/area-json.h"
#include "core/area.h"
#include "core/character.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/images.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/music.h"
#include "core/overlay.h"
#include "core/player.h"
#include "core/resources.h"
#include "core/sounds.h"
#include "core/viewport.h"
#include "core/window.h"
#include "data/data-world.h"
#include "util/hashtable.h"
#include "util/vector.h"

// ScriptRef keydownScript, keyupScript;

static Hashmap<String, Area*> areas;
static Area* worldArea = 0;
static Player player;

/**
 * Last time engine state was updated. See worldUpdate().
 */
static time_t lastTime = 0;

/**
 * Total unpaused game run time.
 */
static time_t total = 0;

static bool alive = false;
static bool redraw = false;
static bool userPaused = false;
static int paused = 0;

static Keys keyStates[10];
static size_t numKeyStates = 0;

bool
worldInit() noexcept {
    alive = true;

    Conf::moveMode = DataWorld::moveMode;

    if (!player.init(DataWorld::playerFile, DataWorld::playerStartPhase)) {
        logFatal("World", "failed to load player");
        return false;
    }

    if (!worldFocusArea(DataWorld::startArea, DataWorld::startCoords)) {
        logFatal("World", "failed to load initial Area");
        return false;
    }

    viewportSetSize(DataWorld::viewportResolution);
    viewportTrackEntity(&player);

    return true;
}

time_t
worldTime() noexcept {
    assert_(total >= 0);
    return total;
}

void
worldButtonDown(Key key) noexcept {
    switch (key) {
    case KEY_ESCAPE:
        worldSetPaused(!paused);
        redraw = true;
        break;
    default:
        if (!paused && numKeyStates == 0) {
            worldArea->buttonDown(key);
            // if (keydownScript)
            //     keydownScript->invoke();
        }
        break;
    }
}

void
worldButtonUp(Key key) noexcept {
    switch (key) {
    case KEY_ESCAPE:
        break;
    default:
        if (!paused && numKeyStates == 0) {
            worldArea->buttonUp(key);
            // if (keyupScript)
            //     keyupScript->invoke();
        }
        break;
    }
}

void
worldDraw(DisplayList* display) noexcept {
    // TimeMeasure m("Drew world");

    redraw = false;

    display->loopX = worldArea->grid.loopX;
    display->loopY = worldArea->grid.loopY;

    display->padding = viewportGetLetterboxOffset();
    display->scale = viewportGetScale();
    display->scroll = viewportGetMapOffset();
    display->size = viewportGetPhysRes();

    display->colorOverlayARGB = worldArea->getColorOverlay();
    display->paused = paused > 0;

    worldArea->draw(display);
}

bool
worldNeedsRedraw() noexcept {
    return redraw || (!paused && worldArea->needsRedraw());
}

void
worldTick(time_t dt) noexcept {
    if (paused) {
        return;
    }

    total += dt;

    worldArea->tick(dt);
}

void
worldTurn() noexcept {
    if (Conf::moveMode == Conf::TURN) {
        worldArea->turn();
    }
}

bool
worldFocusArea(StringView filename, vicoord playerPos) noexcept {
    Area** cachedArea = areas.tryAt(filename);
    if (cachedArea) {
        Area* area = *cachedArea;
        worldFocusArea(area, playerPos);
        return true;
    }

    Area* newArea = makeAreaFromJSON(&player, filename);
    if (!newArea) {
        return false;
    }

    if (!newArea->ok) {
        return false;
    }

    DataArea* dataArea = DataWorld::area(filename);
    if (!dataArea) {
        return false;
    }

    dataArea->area = newArea;  // FIXME: Pass Area by parameter, not
                               // member variable so we can avoid this
                               // pointer.
    areas[filename] = newArea;

    worldFocusArea(newArea, playerPos);

    return true;
}

void
worldFocusArea(Area* area_, vicoord playerPos) noexcept {
    worldArea = area_;
    player.setArea(worldArea, playerPos);
    viewportSetArea(worldArea);
    worldArea->focus();
}

void
worldSetPaused(bool b) noexcept {
    if (!alive) {
        return;
    }

    if (!paused && !b) {
        logErr("World", "trying to unpause, but not paused");
        return;
    }

    // If just pausing.
    if (!paused) {
        worldStoreKeys();
    }

    paused += b ? 1 : -1;

    if (paused) {
        Music::pause();
    }
    else {
        Music::resume();
    }

    // If finally unpausing.
    if (!paused) {
        worldRestoreKeys();
    }
}

void
worldStoreKeys() noexcept {
    keyStates[numKeyStates++] = windowKeysDown;
}

void
worldRestoreKeys() noexcept {
    Keys now = windowKeysDown;
    Keys then = keyStates[numKeyStates - 1];

    numKeyStates--;

    for (size_t i = 0; i < sizeof(Keys) * 8; i++) {
        Key key = (now ^ then) & (i << 1);
        if (key) {
            if (now & key) {
                worldButtonDown(key);
            }
            else {
                worldButtonUp(key);
            }
        }
    }
}

void
worldGarbageCollect() noexcept {
    time_t latestPermissibleUse = total - Conf::cacheTTL * 1000;

    imagesPrune(latestPermissibleUse);
    Music::garbageCollect();
    soundsPrune(latestPermissibleUse);
}
