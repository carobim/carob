/***************************************
** Tsunagari Tile Engine              **
** window.h                           **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2021 Paul Merrill   **
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

#ifndef SRC_CORE_WINDOW_H_
#define SRC_CORE_WINDOW_H_

#include "util/string-view.h"

typedef uint32_t Key;
typedef uint32_t Keys;

#define KEY_ESCAPE        0x1
#define KEY_LEFT_CONTROL  0x2
#define KEY_RIGHT_CONTROL 0x4
#define KEY_LEFT_SHIFT    0x8
#define KEY_RIGHT_SHIFT   0x10
#define KEY_SPACE         0x20
#define KEY_LEFT_ARROW    0x40
#define KEY_RIGHT_ARROW   0x80
#define KEY_UP_ARROW      0x100
#define KEY_DOWN_ARROW    0x200

// This module is handles input and drawing.

void
windowCreate() noexcept;

//! Time since epoch.
time_t
windowTime() noexcept;

//! Width of the window in pixels.
int
windowWidth() noexcept;

//! Height of the window in pixels.
int
windowHeight() noexcept;

//! Set window manager caption.
void
windowSetCaption(StringView caption) noexcept;

//! Show the window and start the main loop.
void
windowMainLoop() noexcept;

void
windowPushScale(float x, float y) noexcept;
void
windowPopScale() noexcept;
void
windowPushTranslate(float x, float y) noexcept;
void
windowPopTranslate() noexcept;
void
windowPushClip(float x, float y, float width, float height) noexcept;
void
windowPopClip() noexcept;

void
windowEmitKeyDown(Key key) noexcept;
void
windowEmitKeyUp(Key key) noexcept;
void
windowClose() noexcept;

extern Keys windowKeysDown;

#endif  // SRC_CORE_WINDOW_H_
