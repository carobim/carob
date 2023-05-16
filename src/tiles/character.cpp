#include "tiles/character.h"

#include "tiles/area.h"
#include "tiles/client-conf.h"
#include "tiles/sounds.h"
#include "tiles/tile.h"
#include "util/compiler.h"

Character::Character() noexcept
    : nowalkFlags(TILE_NOWALK | TILE_NOWALK_NPC),
      nowalkExempt(0),
      destExit(0) {
    fromCoord.x = 0.0;
    fromCoord.y = 0.0;
    fromCoord.z = 0.0;
    enterTile();
}

void
Character::tick(Time dt) noexcept {
    Entity::tick(dt);

    switch (confMoveMode) {
    case TURN:
        // Characters don't do anything on tick() for TURN mode.
        break;
    case TILE: moveTowardDestination(dt); break;
    case NOTILE: assert_(false && "not implemented"); break;
    }
}

void
Character::turn() noexcept { }

void
Character::destroy() noexcept {
    leaveTile();
    Entity::destroy();
}

ivec3
Character::getTileCoords_i() noexcept {
    return area->grid.virt2phys(r);
}

vicoord
Character::getTileCoords_vi() noexcept {
    return area->grid.virt2virt(r);
}

void
Character::setTileCoords(I32 x, I32 y) noexcept {
    leaveTile();
    redraw = true;
    vicoord virt = {x, y, r.z};
    r = area->grid.virt2virt(virt);
    enterTile();
}

void
Character::setTileCoords(ivec3 phys) noexcept {
    leaveTile();
    redraw = true;
    r = area->grid.phys2virt_r(phys);
    enterTile();
}

void
Character::setTileCoords(vicoord virt) noexcept {
    leaveTile();
    redraw = true;
    r = area->grid.virt2virt(virt);
    enterTile();
}

void
Character::setTileCoords(fvec3 virt) noexcept {
    leaveTile();
    redraw = true;
    r = virt;
    enterTile();
}

void
Character::setArea(Area* area, vicoord position) noexcept {
    leaveTile();
    Entity::setArea(area);
    r = area->grid.virt2virt(position);
    enterTile();
    redraw = true;
}

void
Character::moveByTile(ivec2 delta) noexcept {
    if (moving) {
        return;
    }

    setFacing(delta);

    ivec3 dest = moveDest(facing);

    ivec3 from = getTileCoords_i();
    setDestinationCoordinate(area->grid.phys2virt_r(dest));

    destExit = 0;
    if (area->grid.inBounds(from)) {
        destExit = area->grid.exitAt(from, delta);
    }
    if (!destExit && area->grid.inBounds(dest)) {
        destExit = area->grid.exits[EXIT_NORMAL].tryAt(dest);
    }

    if (!canMove(dest)) {
        setAnimationStanding();
        return;
    }

    setAnimationMoving();
    moving = true;

    // Process triggers.
    runTileExitScript();

    if (area->grid.inBounds(from)) {
        area->runScript(TileGrid::SCRIPT_TYPE_LEAVE, from, this);
    }

    // Modify tile's entity count.
    leaveTile(from);
    enterTile(dest);

    if (soundPathStep.size) {
        SoundID sid = soundLoad(soundPathStep);
        PlayingSoundID psid = soundPlay(sid);
        playingSoundRelease(psid);
        soundRelease(sid);
    }

    switch (confMoveMode) {
    case TURN:
        // Movement is instantaneous.
        redraw = true;
        r = destCoord;
        moving = false;
        setAnimationStanding();
        arrived();
        break;
    case TILE:
    case NOTILE:
        // Movement happens in Entity::moveTowardDestination() during
        // tick().
        break;
    }
}

ivec3
Character::moveDest(ivec2 facing) noexcept {
    ivec3 here = getTileCoords_i();

    if (area->grid.inBounds(here)) {
        // Handle layermod.
        return area->grid.moveDest(here, facing);
    }
    else {
        ivec3 facing3 = {facing.x, facing.y, 0};
        return here + facing3;
    }
}

bool
Character::canMove(ivec3 dest) noexcept {
    if (destExit) {
        // We can always take exits as long as we can take exits.
        // (Even if they would cause us to be out of bounds.)
        if (nowalkExempt & TILE_NOWALK_EXIT) {
            return true;
        }
    }

    bool inBounds = area->grid.inBounds(dest);
    if (inBounds) {
        // Tile is inside map. Can we move?
        if (nowalked(dest)) {
            return false;
        }
        if (area->grid.occupied.contains(dest)) {
            // Space is occupied by another Entity.
            return false;
        }

        return true;
    }

    // The tile is legitimately off the map.
    return (nowalkExempt & TILE_NOWALK_AREA_BOUND) != 0;
}

bool
Character::nowalked(ivec3 phys) noexcept {
    U32 flags = nowalkFlags & ~nowalkExempt;
    U32* tileFlags = area->grid.flags.tryAt(phys);
    return tileFlags && (*tileFlags & flags) != 0;
}

void
Character::arrived() noexcept {
    Entity::arrived();

    ivec3 dest = area->grid.virt2phys(destCoord);
    bool inBounds = area->grid.inBounds(dest);

    if (inBounds) {
        float* layermod = area->grid.layermods[EXIT_NORMAL].tryAt(dest);
        if (layermod) {
            r.z = *layermod;
        }

        // Process triggers.
        area->runScript(TileGrid::SCRIPT_TYPE_ENTER, dest, this);
    }

    runTileEntryScript();

    // TODO: move teleportation here
    /*
     * if (onExit()) {
     *      leaveTile();
     *      moveArea(getExit());
     *      Entity::arrived();
     *      enterTile();
     * }
     */
}

void
Character::leaveTile() noexcept {
    if (area) {
        leaveTile(getTileCoords_i());
    }
}

void
Character::leaveTile(ivec3 phys) noexcept {
    area->grid.occupied.erase(phys);
}

void
Character::enterTile() noexcept {
    if (area) {
        enterTile(getTileCoords_i());
    }
}

void
Character::enterTile(ivec3 phys) noexcept {
    area->grid.occupied[phys] = true;
}

void
Character::runTileExitScript() noexcept {
    // if (!tileExitScript) {
    //     return;
    // }
    // pythonSetGlobal("Area", area);
    // pythonSetGlobal("Character", this);
    // pythonSetGlobal("Tile", getTile());
    // tileExitScript->invoke();
}

void
Character::runTileEntryScript() noexcept {
    // if (!tileEntryScript) {
    //     return;
    // }
    // pythonSetGlobal("Area", area);
    // pythonSetGlobal("Character", this);
    // pythonSetGlobal("Tile", getTile());
    // tileEntryScript->invoke();
}
