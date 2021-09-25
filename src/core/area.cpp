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
#include "util/compiler.h"
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
    ivec2 delta;
    switch (key) {
    case KEY_LEFT_ARROW:
        delta.x = -1;
        delta.y = 0;
        player->startMovement(delta);
        break;
    case KEY_RIGHT_ARROW:
        delta.x = 1;
        delta.y = 0;
        player->startMovement(delta);
        break;
    case KEY_UP_ARROW:
        delta.x = 0;
        delta.y = -1;
        player->startMovement(delta);
        break;
    case KEY_DOWN_ARROW:
        delta.x = 0;
        delta.y = 1;
        player->startMovement(delta);
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
    ivec2 delta;
    switch (key) {
    case KEY_LEFT_ARROW:
        delta.x = -1;
        delta.y = 0;
        player->stopMovement(delta);
        break;
    case KEY_RIGHT_ARROW:
        delta.x = 1;
        delta.y = 0;
        player->stopMovement(delta);
        break;
    case KEY_UP_ARROW:
        delta.x = 0;
        delta.y = -1;
        player->stopMovement(delta);
        break;
    case KEY_DOWN_ARROW:
        delta.x = 0;
        delta.y = 1;
        player->stopMovement(delta);
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
        case TileGrid::TILE_LAYER:
            drawTiles(display, tiles, z);
            break;
        case TileGrid::OBJECT_LAYER:
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
    for (Character** character = characters.begin();
         character != characters.end();
         character++) {
        if ((*character)->needsRedraw(pixels)) {
            return true;
        }
    }
    for (Overlay** overlay = overlays.begin();
         overlay != overlays.end();
         overlay++) {
        if ((*overlay)->needsRedraw(pixels)) {
            return true;
        }
    }

    // Do any on-screen tile types need to update their animations?
    if (tileGraphics.size > checkedForAnimation.size) {
        checkedForAnimation.resize(tileGraphics.size);
    }
    memset(checkedForAnimation.data, 0, checkedForAnimation.size);

    time_t now = worldTime();

    for (int z = tiles.z1; z < tiles.z2; z++) {
        if (grid.layerTypes[z] != TileGrid::TILE_LAYER) {
            continue;
        }
        for (int y = tiles.y1; y < tiles.y2; y++) {
            for (int x = tiles.x1; x < tiles.x2; x++) {
                ivec3 coord = {x, y, z};
                int type = grid.getTileType(coord);

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

static bool
isCharacterDead(Character* c) noexcept {
    bool dead = c->isDead();
    if (dead) {
        vicoord coord = {0, 0, 0.0};
        c->setArea(0, coord);
    }
    return dead;
}

static bool
isOverlayDead(Overlay* o) noexcept {
    return o->isDead();
}

void
Area::tick(time_t dt) noexcept {
    if (dataArea) {
        dataArea->tick(dt);
    }

    for (Overlay** overlay = overlays.begin();
         overlay != overlays.end();
         overlay++) {
        (*overlay)->tick(dt);
    }
    erase_if(overlays, isOverlayDead);

    if (confMoveMode != TURN) {
        player->tick(dt);

        for (Character** character = characters.begin();
             character != characters.end();
             character++) {
            (*character)->tick(dt);
        }
        erase_if(characters, isCharacterDead);
    }

    viewportTick(dt);
}

void
Area::turn() noexcept {
    if (dataArea) {
        dataArea->turn();
    }

    player->turn();

    for (Character** character = characters.begin();
         character != characters.end();
         character++) {
        (*character)->turn();
    }
    erase_if(characters, isCharacterDead);

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

    int x1 = static_cast<int>(floorf(off.x / grid.tileDim.x));
    int y1 = static_cast<int>(floorf(off.y / grid.tileDim.y));
    int x2 = static_cast<int>(ceilf((screen.x + off.x) / grid.tileDim.x));
    int y2 = static_cast<int>(ceilf((screen.y + off.y) / grid.tileDim.y));

    if (!grid.loopX) {
        x1 = bound(x1, 0, grid.dim.x);
        x2 = bound(x2, 0, grid.dim.x);
    }
    if (!grid.loopY) {
        y1 = bound(y1, 0, grid.dim.y);
        y2 = bound(y2, 0, grid.dim.y);
    }

    icube cube = {x1, y1, 0, x2, y2, grid.dim.z};
    return cube;
}

bool
Area::inBounds(Entity* ent) noexcept {
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
    memset(tilesAnimated.data, 0, tilesAnimated.size);

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
            ivec3 coord = {x, y, z};
            int type = grid.getTileType(coord);

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

            fvec3 drawPos = {float(x * width), float(y * height), depth};
            // drawPos.z = depth + drawPos.y / tileDimY *
            // ISOMETRIC_ZOFF_PER_TILE;
            DisplayItem item = {img, drawPos};
            items[itemCount++] = item;
        }
    }

    items.size = itemCount;
}

void
Area::drawEntities(DisplayList* display, icube& tiles, int z) noexcept {
    float depth = grid.idx2depth[(size_t)z];

    for (Character** character = characters.begin();
         character != characters.end();
         character++) {
        if ((*character)->getTileCoords_i().z == z) {
            (*character)->draw(display);
        }
    }

    for (Overlay** overlay = overlays.begin();
         overlay != overlays.end();
         overlay++) {
        if ((*overlay)->getPixelCoord().z == depth) {
            (*overlay)->draw(display);
        }
    }

    if (player->getTileCoords_i().z == z) {
        player->draw(display);
    }
}
