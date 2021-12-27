#include "tiles/area.h"

#include "os/c.h"
#include "tiles/client-conf.h"
#include "tiles/display-list.h"
#include "tiles/entity.h"
#include "tiles/images.h"
#include "tiles/log.h"
#include "tiles/music.h"
#include "tiles/npc.h"
#include "tiles/overlay.h"
#include "tiles/player.h"
#include "tiles/tile.h"
#include "tiles/viewport.h"
#include "tiles/window.h"
#include "tiles/world.h"
#include "util/algorithm.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/math2.h"

Area::Area() noexcept
        : ok(true),
          beenFocused(false),
          redraw(true),
          colorOverlayARGB(0),
          dataArea(0),
          player(0) { }

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
    I32 maxZ = grid.dim.z;

    assert_(tiles.z1 == 0);
    assert_(tiles.z2 == maxZ);

    for (I32 z = 0; z < maxZ; z++) {
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
    for (Overlay** overlay = overlays.begin(); overlay != overlays.end();
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

    Time now = worldTime();

    for (I32 z = tiles.z1; z < tiles.z2; z++) {
        if (grid.layerTypes[z] != TileGrid::TILE_LAYER) {
            continue;
        }
        for (I32 y = tiles.y1; y < tiles.y2; y++) {
            for (I32 x = tiles.x1; x < tiles.x2; x++) {
                ivec3 coord = {x, y, z};
                I32 type = grid.getTileType(coord);

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
Area::tick(Time dt) noexcept {
    if (dataArea) {
        dataArea->tick(dt);
    }

    for (Overlay** overlay = overlays.begin(); overlay != overlays.end();
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


U32
Area::getColorOverlay() const noexcept {
    return colorOverlayARGB;
}

void
Area::setColorOverlay(U8 a, U8 r, U8 g, U8 b) noexcept {
    colorOverlayARGB =
            (U32)(a << 24u) + (U32)(r << 16u) + (U32)(g << 8u) + (U32)b;
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

    I32 x1 = static_cast<I32>(floorf(off.x / grid.tileDim.x));
    I32 y1 = static_cast<I32>(floorf(off.y / grid.tileDim.y));
    I32 x2 = static_cast<I32>(ceilf((screen.x + off.x) / grid.tileDim.x));
    I32 y2 = static_cast<I32>(ceilf((screen.y + off.y) / grid.tileDim.y));

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
Area::spawnNPC(StringView descriptor_,
               vicoord coord,
               StringView phase) noexcept {
    Character* c = new Character;
    if (!c->init(descriptor_, phase)) {
        logErr("Area", String() << "Failed to load entity " << descriptor_);
        delete c;
        return 0;
    }
    c->setArea(this, coord);
    characters.push(c);
    return c;
}

Overlay*
Area::spawnOverlay(StringView descriptor_,
                   vicoord coord,
                   StringView phase) noexcept {
    Overlay* o = new Overlay;
    if (!o->init(descriptor_, phase)) {
        logErr("Area", String() << "Failed to load entity " << descriptor_);
        delete o;
        return 0;
    }
    o->setArea(this);
    o->teleport(coord);
    overlays.push(o);
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
    void (**script)(DataArea*, Entity*, ivec3) = grid.scripts[type].tryAt(tile);
    if (script) {
        (*script)(dataArea, triggeredBy, tile);
    }
}


void
Area::drawTiles(DisplayList* display, icube& tiles, I32 z) noexcept {
    Vector<DisplayItem>& items = display->items;

    Time now = worldTime();

    // FIXME: Same.
    if (tileGraphics.size > tilesAnimated.size) {
        tilesAnimated.resize(tileGraphics.size);
    }

    // FIXME: Only do this once per draw() call. Don't do it per drawTiles.
    memset(tilesAnimated.data, 0, tilesAnimated.size);

    Size maxTiles = (tiles.y2 - tiles.y1) * (tiles.x2 - tiles.x1);
    Size itemCount = items.size;

    // FIXME: Resize once before any drawTiles calls.
    //        Do maxTiles * count(TILE_LAYERS).
    items.resize(maxTiles + itemCount);

    float depth = grid.idx2depth[(Size)z];

    I32 width = 16;
    I32 height = 16;

    for (I32 y = tiles.y1; y < tiles.y2; y++) {
        for (I32 x = tiles.x1; x < tiles.x2; x++) {
            // We are certain the Tile exists.
            ivec3 coord = {x, y, z};
            I32 type = grid.getTileType(coord);

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
Area::drawEntities(DisplayList* display, icube& tiles, I32 z) noexcept {
    float depth = grid.idx2depth[(Size)z];

    for (Character** character = characters.begin();
         character != characters.end();
         character++) {
        if ((*character)->getTileCoords_i().z == z) {
            (*character)->draw(display);
        }
    }

    for (Overlay** overlay = overlays.begin(); overlay != overlays.end();
         overlay++) {
        if ((*overlay)->getPixelCoord().z == depth) {
            (*overlay)->draw(display);
        }
    }

    if (player->getTileCoords_i().z == z) {
        player->draw(display);
    }
}
