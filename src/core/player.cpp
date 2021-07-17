#include "core/player.h"

#include "core/area.h"
#include "core/client-conf.h"
#include "core/entity.h"
#include "core/log.h"
#include "core/tile.h"
#include "core/window.h"
#include "core/world.h"
#include "util/compiler.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

static Player* globalPlayer = 0;

Player&
Player::instance() noexcept {
    return *globalPlayer;
}


Player::Player() noexcept : Character(), velocity({0, 0}), numMovements(0) {
    globalPlayer = this;
    nowalkFlags = TILE_NOWALK | TILE_NOWALK_PLAYER;
    nowalkExempt = TILE_NOWALK_EXIT;
}

void
Player::destroy() noexcept {
    logFatal("Player", "destroy(): Player should not be destroyed");
}

void
Player::startMovement(ivec2 delta) noexcept {
    switch (confMoveMode) {
    case MoveMode::TURN:
        moveByTile(delta);
        break;
    case MoveMode::TILE:
        movements[numMovements++] = delta;
        velocity = delta;
        moveByTile(velocity);
        break;
    case MoveMode::NOTILE:
        // TODO
        break;
    }
}

void
Player::stopMovement(ivec2 delta) noexcept {
    switch (confMoveMode) {
    case MoveMode::TURN:
        break;
    case MoveMode::TILE:
        for (size_t i = 0; i < numMovements; i++) {
            if (movements[i] == delta) {
                // Erase movement.
                for (size_t j = i; j < numMovements - 1; j++) {
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
            velocity = {0, 0};
        }
        break;
    case MoveMode::NOTILE:
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
    if (confMoveMode == MoveMode::TILE && (velocity.x || velocity.y)) {
        moveByTile(velocity);
    }
}

void
Player::takeExit(Exit& exit) noexcept {
    if (!worldFocusArea(exit.area, exit.coords)) {
        // Roll back movement if exit failed to open.
        setTileCoords(fromCoord);
        logErr("Exit", String() << exit.area << ": failed to load properly");
    }
}
