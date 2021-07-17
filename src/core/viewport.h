#ifndef SRC_CORE_VIEWPORT_H_
#define SRC_CORE_VIEWPORT_H_

#include "core/vec.h"
#include "util/compiler.h"
#include "util/int.h"

class Area;
class Entity;

//! General control over where and how the map is rendered.

void
viewportSetSize(fvec2 virtRes) noexcept;

void
viewportTick(time_t dt) noexcept;
void
viewportTurn() noexcept;

//! How far the map is scrolled in pixels, counting from the upper-left.
fvec2
viewportGetMapOffset() noexcept;

//! Size of the letterbox matte bars in pixels.
fvec2
viewportGetLetterboxOffset() noexcept;

//! Multiplier in X and Y dimensions to get from virtRes to physRes.
fvec2
viewportGetScale() noexcept;

//! The resolution our game is actually being drawn at.
fvec2
viewportGetPhysRes() noexcept;

//! The resolution our game thinks it is being drawn at. Chosen by a
//! world's creator. This allows graphics to look the same on any
//! setups of any resolution.
fvec2
viewportGetVirtRes() noexcept;

// Immediatly center render offset. Stop any tracking.
void
viewportJumpToPt(fvec2 pt) noexcept;
void
viewportJumpToEntity(Entity* e) noexcept;

// Continuously follow.
void
viewportTrackEntity(Entity* e) noexcept;

void
viewportSetArea(Area* a) noexcept;

#endif  // SRC_CORE_VIEWPORT_H_
