/***************************************
** Tsunagari Tile Engine              **
** player.cpp                         **
** Copyright 2011-2013 PariahSoft LLC **
** Copyright 2016 Paul Merrill        **
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

#include <algorithm>

#include "core/area.h"
#include "core/client-conf.h"
#include "core/entity.h"
#include "core/log.h"
#include "core/player.h"
#include "core/tile.h"
#include "core/world.h"
#include "core/window.h"

#define ASSERT(x)  if (!(x)) { return false; }

static Player* globalPlayer = nullptr;

Player& Player::instance()
{
    return *globalPlayer;
}


Player::Player()
    : Character(), velocity(0, 0)
{
    globalPlayer = this;
    nowalkFlags = TILE_NOWALK | TILE_NOWALK_PLAYER;
    nowalkExempt = TILE_NOWALK_EXIT;
}

void Player::destroy()
{
    Log::fatal("Player", "destroy(): Player should not be destroyed");
}

void Player::startMovement(ivec2 delta)
{
    switch (conf.moveMode) {
    case TURN:
        moveByTile(delta);
        break;
    case TILE:
        movements.push_back(delta);
        velocity = delta;
        moveByTile(velocity);
        break;
    case NOTILE:
        // TODO
        break;
    }
}

void Player::stopMovement(ivec2 delta)
{
    switch (conf.moveMode) {
    case TURN:
        break;
    case TILE:
        movements.erase(std::find(movements.begin(), movements.end(), delta));
        velocity = movements.size() ?
                   movements.back() :
               ivec2(0, 0);
        if (velocity) {
            moveByTile(velocity);
        }
        break;
    case NOTILE:
        // TODO
        break;
    }
}

void Player::moveByTile(ivec2 delta)
{
    if (frozen) {
        return;
    }
    if (moving) {
        return;
    }

    setFacing(delta);

    // Left SHIFT allows changing facing, but disallows movement.
    if (GameWindow::instance().isKeyDown(KBLeftShift) ||
        GameWindow::instance().isKeyDown(KBRightShift)) {
        setAnimationStanding();
        redraw = true;
        return;
    }

    Character::moveByTile(delta);

    World::instance().turn();
}

void Player::useTile()
{
    Tile* t = area->getTile(moveDest(facing));
    if (t) {
        t->runUseScript(this);
    }
}

void Player::setFrozen(bool b)
{
    World& world = World::instance();

    if (b) {
        world.storeKeys();
    }
    else {
        world.restoreKeys();
    }

    Entity::setFrozen(b);
    if (!frozen && velocity) {
        moveByTile(velocity);
    }
}

void Player::arrived()
{
    Entity::arrived();

    if (destExit) {
        takeExit(destExit);
    }

    // If we have a velocity, keep moving.
    if (conf.moveMode == TILE && velocity) {
        moveByTile(velocity);
    }
}

void Player::takeExit(Exit* exit)
{
    World& world = World::instance();
    Area* newArea = world.getArea(exit->area);
    if (newArea) {
        world.focusArea(newArea, exit->coords);
    }
    else {
        // Roll back movement if exit failed to open.
        setTileCoords(fromCoord);
        Log::err("Exit", exit->area + ": failed to load properly");
    }
}