/***************************************
** Tsunagari Tile Engine              **
** player.cpp                         **
** Copyright 2011-2013 Michael Reiley **
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

#include "core/player.h"

#include "core/area.h"
#include "core/client-conf.h"
#include "core/entity.h"
#include "core/log.h"
#include "core/tile.h"
#include "core/window.h"
#include "core/world.h"

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
    icoord destCoord = moveDest(facing);
    bool inBounds = area->grid.inBounds(destCoord);
    if (inBounds) {
        area->runScript(TileGrid::SCRIPT_TYPE_USE, destCoord, this);
    }
}

void
Player::setFrozen(bool b) noexcept {
    if (b) {
        worldStoreKeys();
    }
    else {
        worldRestoreKeys();
    }

    Entity::setFrozen(b);
    if (!frozen && velocity) {
        moveByTile(velocity);
    }
}

void
Player::arrived() noexcept {
    Character::arrived();

    if (destExit) {
        takeExit(*destExit);
    }

    // If we have a velocity, keep moving.
    if (confMoveMode == MoveMode::TILE && velocity) {
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
