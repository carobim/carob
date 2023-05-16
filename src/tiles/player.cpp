#include "tiles/player.h"

#include "tiles/area.h"
#include "tiles/client-conf.h"
#include "tiles/entity.h"
#include "tiles/log.h"
#include "tiles/tile-grid.h"
#include "tiles/window.h"
#include "tiles/world.h"
#include "util/compiler.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

Player player;

Player::Player() noexcept : numMovements(0) {
    nowalkFlags = TILE_NOWALK | TILE_NOWALK_PLAYER;
    nowalkExempt = TILE_NOWALK_EXIT;
    velocity.x = 0;
    velocity.y = 0;
}

void
Player::destroy() noexcept {
    logFatal("Player", "destroy(): Player should not be destroyed");
}

void
Player::startMovement(ivec2 delta) noexcept {
    switch (confMoveMode) {
    case TURN: moveByTile(delta); break;
    case TILE:
        movements[numMovements++] = delta;
        velocity = delta;
        moveByTile(velocity);
        break;
    case NOTILE:
        // TODO
        break;
    }
}

void
Player::stopMovement(ivec2 delta) noexcept {
    switch (confMoveMode) {
    case TURN: break;
    case TILE:
        for (Size i = 0; i < numMovements; i++) {
            if (movements[i] == delta) {
                // Erase movement.
                for (Size j = i; j < numMovements - 1; j++) {
                    movements[j] = movements[j + 1];
                }
                numMovements--;
            }
        }
        if (numMovements) {
            velocity = movements[numMovements - 1];
            moveByTile(velocity);
        }
        else {
            velocity.x = 0;
            velocity.y = 0;
        }
        break;
    case NOTILE:
        // TODO
        break;
    }
}

void
Player::moveByTile(ivec2 delta) noexcept {
    if (frozen) {
        return;
    }
    if (moving) {
        return;
    }

    setFacing(delta);

    // Left SHIFT allows changing facing, but disallows movement.
    if (windowKeysDown & KEY_LEFT_SHIFT || windowKeysDown & KEY_RIGHT_SHIFT) {
        setAnimationStanding();
        redraw = true;
        return;
    }

    Character::moveByTile(delta);

    worldTurn();
}

void
Player::useTile() noexcept {
    ivec3 destCoord = moveDest(facing);
    bool inBounds = area->grid.inBounds(destCoord);
    if (inBounds) {
        area->runScript(TileGrid::SCRIPT_TYPE_USE, destCoord, this);
    }
}

void
Player::setFrozen(bool b) noexcept {
    Entity::setFrozen(b);

    if (b) {
        worldStoreKeys();
    }
    else {
        worldRestoreKeys();
    }
}

void
Player::arrived() noexcept {
    Character::arrived();

    if (destExit) {
        takeExit(*destExit);
    }

    // If we have a velocity, keep moving.
    if (confMoveMode == TILE && (velocity.x || velocity.y)) {
        moveByTile(velocity);
    }
}

void
Player::takeExit(Exit& exit) noexcept {
    worldFocusArea(exit.area, exit.coords);
}
