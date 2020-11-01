/***************************************
** Tsunagari Tile Engine              **
** viewport.h                         **
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

#ifndef SRC_CORE_VIEWPORT_H_
#define SRC_CORE_VIEWPORT_H_

#include "core/vec.h"
#include "util/int.h"

class Area;
class Entity;

//! General control over where and how the map is rendered.

void
viewportSetSize(rvec2 virtRes) noexcept;

void
viewportTick(time_t dt) noexcept;
void
viewportTurn() noexcept;

//! How far the map is scrolled in pixels, counting from the upper-left.
rvec2
viewportGetMapOffset() noexcept;

//! Size of the letterbox matte bars in pixels.
rvec2
viewportGetLetterboxOffset() noexcept;

//! Multiplier in X and Y dimensions to get from virtRes to physRes.
rvec2
viewportGetScale() noexcept;

//! The resolution our game is actually being drawn at.
rvec2
viewportGetPhysRes() noexcept;

//! The resolution our game thinks it is being drawn at. Chosen by a
//! world's creator. This allows graphics to look the same on any
//! setups of any resolution.
rvec2
viewportGetVirtRes() noexcept;

// Immediatly center render offset. Stop any tracking.
void
viewportJumpToPt(rvec2 pt) noexcept;
void
viewportJumpToEntity(Entity* e) noexcept;

// Continuously follow.
void
viewportTrackEntity(Entity* e) noexcept;

void
viewportSetArea(Area* a) noexcept;

#endif  // SRC_CORE_VIEWPORT_H_
