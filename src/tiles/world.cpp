#include "tiles/world.h"

#include "data/data-world.h"
#include "tiles/area-json.h"
#include "tiles/area.h"
#include "tiles/client-conf.h"
#include "tiles/display-list.h"
#include "tiles/log.h"
#include "tiles/music.h"
#include "tiles/overlay.h"
#include "tiles/player.h"
#include "tiles/resources.h"
#include "tiles/viewport.h"
#include "tiles/window.h"
#include "util/compiler.h"
#include "util/hashtable.h"
//#include "util/measure.h"
#include "util/vector.h"

// ScriptRef keydownScript, keyupScript;

static Hashmap<String, Area*> areas;
static Area* worldArea = 0;
static Player player;

/**
 * Total unpaused game run time.
 */
static Time total = 0;

static bool alive = false;
static bool redraw = false;
static I32 paused = 0;

static Keys keyStates[10];
static Size numKeyStates = 0;

void
worldInit() noexcept {
    alive = true;

    confMoveMode = dataWorldMoveMode;

    if (!player.init(dataWorldPlayerFile, dataWorldPlayerStartPhase)) {
        logFatal("World", "failed to load player");
        return;
    }

    worldFocusArea(dataWorldStartArea, dataWorldStartCoords);

    viewportSetSize(dataWorldViewportResolution);
    viewportTrackEntity(&player);
}

Time
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
worldTick(Time dt) noexcept {
    if (paused) {
        return;
    }

    total += dt;

    worldArea->tick(dt);
}

void
worldTurn() noexcept {
    if (confMoveMode == TURN) {
        worldArea->turn();
    }
}

void
worldFocusArea(StringView filename, vicoord playerPos) noexcept {
    Area** cachedArea = areas.tryAt(filename);
    if (cachedArea) {
        Area* area = *cachedArea;
        worldFocusArea(area, playerPos);
        return;
    }

    Area* newArea = makeAreaFromJSON(&player, filename);
    assert_(newArea);

    assert_(newArea->ok);

    DataArea* dataArea = dataWorldArea(filename);
    assert_(dataArea);

    dataArea->area = newArea;  // FIXME: Pass Area by parameter, not
                               // member variable so we can avoid this
                               // pointer.
    areas[filename] = newArea;

    worldFocusArea(newArea, playerPos);
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
        musicPause();
    }
    else {
        musicResume();
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
    Keys then = keyStates[--numKeyStates];

    for (Size i = 0; i < sizeof(Keys) * 8; i++) {
        Key key = (now ^ then) & (1 << i);
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
