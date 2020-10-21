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
#include "util/bitrecord.h"
#include "util/function.h"
#include "util/hashtable.h"
#include "util/unique.h"
#include "util/vector.h"

class Image;
class Player;

// ScriptRef keydownScript, keyupScript;

static Hashmap<String, Area*> areas;
static Area* worldArea = nullptr;
static Unique<Player> player = new Player;

/**
 * Last time engine state was updated. See World::update().
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

static Vector<BitRecord> keyStates;

bool
World::init() noexcept {
    alive = true;

    Conf::moveMode = DataWorld::moveMode;

    if (!player->init(DataWorld::playerFile, DataWorld::playerStartPhase)) {
        logFatal("World", "failed to load player");
        return false;
    }

    if (!focusArea(DataWorld::startArea, DataWorld::startCoords)) {
        logFatal("World", "failed to load initial Area");
        return false;
    }

    Viewport::setSize(DataWorld::viewportResolution);
    Viewport::trackEntity(player.get());

    return true;
}

time_t
World::time() noexcept {
    assert_(total >= 0);
    return total;
}

void
World::buttonDown(KeyboardKey key) noexcept {
    switch (key) {
    case KBEscape:
        setPaused(paused == 0);
        redraw = true;
        break;
    default:
        if (!paused && keyStates.empty()) {
            worldArea->buttonDown(key);
            // if (keydownScript)
            //     keydownScript->invoke();
        }
        break;
    }
}

void
World::buttonUp(KeyboardKey key) noexcept {
    switch (key) {
    case KBEscape:
        break;
    default:
        if (!paused && keyStates.empty()) {
            worldArea->buttonUp(key);
            // if (keyupScript)
            //     keyupScript->invoke();
        }
        break;
    }
}

void
World::draw(DisplayList* display) noexcept {
    // TimeMeasure m("Drew world");

    redraw = false;

    display->loopX = worldArea->grid.loopX;
    display->loopY = worldArea->grid.loopY;

    display->padding = Viewport::getLetterboxOffset();
    display->scale = Viewport::getScale();
    display->scroll = Viewport::getMapOffset();
    display->size = Viewport::getPhysRes();

    display->colorOverlayARGB = worldArea->getColorOverlay();
    display->paused = paused > 0;

    worldArea->draw(display);
}

bool
World::needsRedraw() noexcept {
    return redraw || (!paused && worldArea->needsRedraw());
}

void
World::tick(time_t dt) noexcept {
    if (paused) {
        return;
    }

    total += dt;

    worldArea->tick(dt);
}

void
World::turn() noexcept {
    if (Conf::moveMode == Conf::TURN) {
        worldArea->turn();
    }
}

bool
World::focusArea(StringView filename, vicoord playerPos) noexcept {
    Optional<Area**> cachedArea = areas.tryAt(filename);
    if (cachedArea) {
        Area* area = **cachedArea;
        focusArea(area, playerPos);
        return true;
    }

    Area* newArea = makeAreaFromJSON(player.get(), filename);
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

    focusArea(newArea, playerPos);

    return true;
}

void
World::focusArea(Area* area_, vicoord playerPos) noexcept {
    worldArea = area_;
    player->setArea(worldArea, playerPos);
    Viewport::setArea(worldArea);
    worldArea->focus();
}

void
World::setPaused(bool b) noexcept {
    if (!alive) {
        return;
    }

    if (!paused && !b) {
        logErr("World", "trying to unpause, but not paused");
        return;
    }

    // If just pausing.
    if (!paused) {
        storeKeys();
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
        restoreKeys();
    }
}

void
World::storeKeys() noexcept {
    keyStates.push_back(GameWindow::keysDown);
}

void
World::restoreKeys() noexcept {
    BitRecord& now = GameWindow::keysDown;
    BitRecord then = keyStates.back();
    Vector<size_t> diffs = now.diff(then);

    keyStates.pop_back();

    for (size_t key : diffs) {
        KeyboardKey key_ = static_cast<KeyboardKey>(key);
        if (now[key_]) {
            buttonDown(key_);
        }
        else {
            buttonUp(key_);
        }
    }
}

void
World::garbageCollect() noexcept {
    time_t latestPermissibleUse = total - Conf::cacheTTL * 1000;

    Images::prune(latestPermissibleUse);
    Music::garbageCollect();
    Sounds::prune(latestPermissibleUse);
}
