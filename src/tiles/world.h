#ifndef SRC_TILES_WORLD_H_
#define SRC_TILES_WORLD_H_

#include "tiles/vec.h"
#include "tiles/window.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

class Area;
struct DisplayList;

/**
 * Top class holding all data necessary to create a game. Such a collection of
 * data is called a "world". Materially, a world is just a set of graphics,
 * sound effects, music, and scripts. From the perspective from a player, each
 * world should be a separate game. Carob is an engine that powers worlds.
 */

/**
 * Initialize the world for use.
 */
bool
worldInit() noexcept;

/**
 * Syncronized time value used throughout the engine.
 */
Time
worldTime() noexcept;

/**
 * Process key presses.
 */
void
worldButtonDown(Key key) noexcept;
void
worldButtonUp(Key key) noexcept;

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
worldTick(Time dt) noexcept;

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

#endif  // SRC_TILES_WORLD_H_
