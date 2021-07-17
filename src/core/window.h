#ifndef SRC_CORE_WINDOW_H_
#define SRC_CORE_WINDOW_H_

#include "util/compiler.h"
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
