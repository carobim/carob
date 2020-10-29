/***************************************
** Tsunagari Tile Engine              **
** world.h                            **
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

#ifndef SRC_CORE_WORLD_H_
#define SRC_CORE_WORLD_H_

#include "core/vec.h"
#include "core/window.h"
#include "util/int.h"
#include "util/string-view.h"

class Area;
struct DisplayList;

/**
 * Top class holding all data necessary to create a game. Such a collection of
 * data is called a "world". Materially, a world is just a set of graphics,
 * sound effects, music, and scripts. From the perspective from a player, each
 * world should be a separate game. Tsunagari is an engine that powers worlds.
 */

/**
 * Initialize the world for use.
 */
bool
worldInit() noexcept;

/**
 * Syncronized time value used throughout the engine.
 */
time_t
worldTime() noexcept;

/**
 * Process key presses.
 */
void
worldButtonDown(Key btn) noexcept;
void
worldButtonUp(Key btn) noexcept;

/**
 * Draw game state to the screen.
 */
void
worldDraw(DisplayList* display) noexcept;

/**
 * Do we need to redraw the screen?
 */
bool
worldNeedsRedraw() noexcept;

/**
 * Updates the game state within this World as if dt milliseconds had
 * passed since the last call.
 *
 *                       MOVE MODE
 *                 TURN     TILE     NOTILE
 * Area            yes      yes      yes
 * Character       no       yes      yes
 * Overlay         yes      yes      yes
 */
void
worldTick(time_t dt) noexcept;

/**
 * Update the game world when the turn is over (Player moves).
 *
 *                       MOVE MODE
 *                 TURN     TILE     NOTILE
 * Area            yes      no       no
 * Character       yes      no       no
 * Overlay         yes      no       no
 */
void
worldTurn() noexcept;

/**
 * Switch the game to a new Area, moving the player to the specified
 * position in the Area.
 */
bool
worldFocusArea(StringView filename, vicoord playerPos) noexcept;
void
worldFocusArea(Area* area, vicoord playerPos) noexcept;

void
worldSetPaused(bool b) noexcept;

void
worldStoreKeys() noexcept;
void
worldRestoreKeys() noexcept;

void
worldRunAreaLoadScript(Area* area) noexcept;

//! Expunge old resources cached in memory. Decisions on which are
//! removed and which are kept are based on the global Conf struct.
void
worldGarbageCollect() noexcept;

#endif  // SRC_CORE_WORLD_H_
