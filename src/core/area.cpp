/***************************************
** Tsunagari Tile Engine              **
** area.cpp                           **
** Copyright 2011-2015 Michael Reiley **
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

#include "core/area.h"

#include "core/algorithm.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/entity.h"
#include "core/images.h"
#include "core/log.h"
#include "core/music.h"
#include "core/npc.h"
#include "core/overlay.h"
#include "core/player.h"
#include "core/tile.h"
#include "core/viewport.h"
#include "core/window.h"
#include "core/world.h"
#include "data/data-world.h"
#include "os/c.h"
#include "util/assert.h"
#include "util/hashtable.h"
#include "util/math2.h"

Area::Area() noexcept
    : ok(true),
      beenFocused(false),
      redraw(true),
      colorOverlayARGB(0) { }

void
Area::focus() noexcept {
    if (!beenFocused) {
        beenFocused = true;
        if (dataArea) {
            dataArea->onLoad();
        }
    }

    if (musicPath.size) {
        musicPlay(musicPath);
    }

    if (dataArea) {
        dataArea->onFocus();
    }
}

void
Area::buttonDown(Key key) noexcept {
    switch (key) {
    case KEY_LEFT_ARROW:
        player->startMovement({-1, 0});
        break;
    case KEY_RIGHT_ARROW:
        player->startMovement({1, 0});
        break;
    case KEY_UP_ARROW:
        player->startMovement({0, -1});
        break;
    case KEY_DOWN_ARROW:
        player->startMovement({0, 1});
        break;
    case KEY_SPACE:
        player->useTile();
        break;
    default:
        break;
    }
}

void
Area::buttonUp(Key key) noexcept {
    switch (key) {
    case KEY_LEFT_ARROW:
        player->stopMovement({-1, 0});
        break;
    case KEY_RIGHT_ARROW:
        player->stopMovement({1, 0});
        break;
    case KEY_UP_ARROW:
        player->stopMovement({0, -1});
        break;
    case KEY_DOWN_ARROW:
        player->stopMovement({0, 1});
        break;
    default:
        break;
    }
}

void
Area::draw(DisplayList* display) noexcept {
    icube tiles = visibleTiles();
    int maxZ = grid.dim.z;

    assert_(tiles.z1 == 0);
    assert_(tiles.z2 == maxZ);

    for (int z = 0; z < maxZ; z++) {
        switch (grid.layerTypes[z]) {
        case TileGrid::LayerType::TILE_LAYER:
            drawTiles(display, tiles, z);
            break;
        case TileGrid::LayerType::OBJECT_LAYER:
            drawEntities(display, tiles, z);
            break;
        }
    }

    redraw = false;
}

bool
Area::needsRedraw() noexcept {
    if (redraw) {
        return true;
    }

    icube tiles = visibleTiles();
    icube pixels = {
            tiles.x1 * grid.tileDim.x,
            tiles.y1 * grid.tileDim.y,
            tiles.z1,
            tiles.x2 * grid.tileDim.x,
            tiles.y2 * grid.tileDim.y,
            tiles.z2,
    };

    if (player->needsRedraw(pixels)) {
        return true;
    }
    for (Character** character = characters.begin(); character != characters.end(); character++) {
        if ((*character)->needsRedraw(pixels)) {
            return true;
        }
    }
    for (Overlay** overlay = overlays.begin(); overlay != overlays.end(); overlay++) {
        if ((*overlay)->needsRedraw(pixels)) {
            return true;
        }
    }

    // Do any on-screen tile types need to update their animations?
    if (tileGraphics.size > checkedForAnimation.size) {
        checkedForAnimation.resize(tileGraphics.size);
    }
    for (bool* checked = checkedForAnimation.begin(); checked != checkedForAnimation.end(); checked++) {
        *checked = false;
    }

    time_t now = worldTime();

    for (int z = tiles.z1; z < tiles.z2; z++) {
        if (grid.layerTypes[z] != TileGrid::LayerType::TILE_LAYER) {
            continue;
        }
        for (int y = tiles.y1; y < tiles.y2; y++) {
            for (int x = tiles.x1; x < tiles.x2; x++) {
                int type = grid.getTileType(ivec3{x, y, z});

                if (type == 0) {
                    continue;
                }

                if (checkedForAnimation[type]) {
                    continue;
                }
                checkedForAnimation[type] = true;

                if (tileGraphics[type].needsRedraw(now)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void
Area::requestRedraw() noexcept {
    redraw = true;
}

void
Area::tick(time_t dt) noexcept {
    if (dataArea) {
        dataArea->tick(dt);
    }

    for (Overlay** overlay = overlays.begin(); overlay != overlays.end(); overlay++) {
        (*overlay)->tick(dt);
    }
    erase_if(overlays, [](Overlay* o) { return o->isDead(); });

    if (confMoveMode != MoveMode::TURN) {
        player->tick(dt);

        for (Character** character = characters.begin(); character != characters.end(); character++) {
            (*character)->tick(dt);
        }
        erase_if(characters, [](Character* c) -> bool {
            bool dead = c->isDead();
            if (dead) {
                c->setArea(0, {0, 0, 0.0});
            }
            return dead;
        });
    }

    viewportTick(dt);
}

void
Area::turn() noexcept {
    if (dataArea) {
        dataArea->turn();
    }

    player->turn();

    for (Character** character = characters.begin(); character != characters.end(); character++) {
        (*character)->turn();
    }
    erase_if(characters, [](Character* c) -> bool {
        bool dead = c->isDead();
        if (dead) {
            c->setArea(0, {0, 0, 0.0});
        }
        return dead;
    });

    viewportTurn();
}


uint32_t
Area::getColorOverlay() noexcept {
    return colorOverlayARGB;
}

void
Area::setColorOverlay(uint8_t a, uint8_t r, uint8_t g, uint8_t b) noexcept {
    colorOverlayARGB = (uint32_t)(a << 24u) + (uint32_t)(r << 16u) +
                       (uint32_t)(g << 8u) + (uint32_t)b;
    redraw = true;
}


TileSet*
Area::getTileSet(StringView imagePath) noexcept {
    if (!tileSets.contains(imagePath)) {
        logErr("Area", String() << "tileset " << imagePath << " not found");
        return 0;
    }
    return &tileSets[imagePath];
}


icube
Area::visibleTiles() noexcept {
    fvec2 screen = viewportGetVirtRes();
    fvec2 off = viewportGetMapOffset();

    int x1 = static_cast<int>(floor(off.x / grid.tileDim.x));
    int y1 = static_cast<int>(floor(off.y / grid.tileDim.y));
    int x2 = static_cast<int>(ceil((screen.x + off.x) / grid.tileDim.x));
    int y2 = static_cast<int>(ceil((screen.y + off.y) / grid.tileDim.y));

    if (!grid.loopX) {
        x1 = bound(x1, 0, grid.dim.x);
        x2 = bound(x2, 0, grid.dim.x);
    }
    if (!grid.loopY) {
        y1 = bound(y1, 0, grid.dim.y);
        y2 = bound(y2, 0, grid.dim.y);
    }

    return icube{x1, y1, 0, x2, y2, grid.dim.z};
}

bool
    Area::inBounds(Entity* ent) {
        return grid.inBounds(ent->getPixelCoord());
}


Character*
Area::spawnNPC(StringView descriptor,
               vicoord coord,
               StringView phase) noexcept {
    Character* c = new Character;
    if (!c->init(descriptor, phase)) {
        logErr("Area", String() << "Failed to load entity " << descriptor);
        delete c;
        return 0;
    }
    c->setArea(this, coord);
    characters.push_back(c);
    return c;
}

Overlay*
Area::spawnOverlay(StringView descriptor,
                   vicoord coord,
                   StringView phase) noexcept {
    Overlay* o = new Overlay;
    if (!o->init(descriptor, phase)) {
        logErr("Area", String() << "Failed to load entity " << descriptor);
        delete o;
        return 0;
    }
    o->setArea(this);
    o->teleport(coord);
    overlays.push_back(o);
    return o;
}


DataArea*
Area::getDataArea() noexcept {
    return dataArea;
}

void
Area::runScript(TileGrid::ScriptType type,
                ivec3 tile,
                Entity* triggeredBy) noexcept {
    DataArea::TileScript* script = grid.scripts[type].tryAt(tile);
    if (script) {
        (dataArea->*(*script))(*triggeredBy, tile);
    }
}


void
Area::drawTiles(DisplayList* display, icube& tiles, int z) noexcept {
    Vector<DisplayItem>& items = display->items;

    time_t now = worldTime();

    // FIXME: Same.
    if (tileGraphics.size > tilesAnimated.size) {
        tilesAnimated.resize(tileGraphics.size);
    }

    // FIXME: Only do this once per draw() call. Don't do it per drawTiles.
    for (bool* animated = tilesAnimated.begin(); animated != tilesAnimated.end(); animated++) {
        *animated = false;
    }

    size_t maxTiles = (tiles.y2 - tiles.y1) * (tiles.x2 - tiles.x1);
    size_t itemCount = items.size;

    // FIXME: Resize once before any drawTiles calls.
    //        Do maxTiles * count(TILE_LAYERS).
    items.resize(maxTiles + itemCount);

    float depth = grid.idx2depth[(size_t)z];

    int width = 16;
    int height = 16;

    for (int y = tiles.y1; y < tiles.y2; y++) {
        for (int x = tiles.x1; x < tiles.x2; x++) {
            // We are certain the Tile exists.
            int type = grid.getTileType(ivec3{x, y, z});

            if (type == 0) {
                continue;
            }

            if (tileGraphics[type].id == NO_ANIMATION) {
                continue;
            }

            if (!tilesAnimated[type]) {
                tilesAnimated[type] = true;
                tileGraphics[type].setFrame(now);
            }

            // Image guaranteed to exist because Animation won't hold a null
            // ImageID.
            Image img = tileGraphics[type].getFrame();

            fvec3 drawPos{float(x * width), float(y * height), depth};
            // drawPos.z = depth + drawPos.y / tileDimY *
            // ISOMETRIC_ZOFF_PER_TILE;
            items[itemCount++] = DisplayItem{img, drawPos};
        }
    }

    items.size = itemCount;
}

void
Area::drawEntities(DisplayList* display, icube& tiles, int z) noexcept {
    float depth = grid.idx2depth[(size_t)z];

    for (Character** character = characters.begin(); character != characters.end(); character++) {
        if ((*character)->getTileCoords_i().z == z) {
            (*character)->draw(display);
        }
    }

    for (Overlay** overlay = overlays.begin(); overlay != overlays.end(); overlay++) {
        if ((*overlay)->getPixelCoord().z == depth) {
            (*overlay)->draw(display);
        }
    }

    if (player->getTileCoords_i().z == z) {
        player->draw(display);
    }
}
