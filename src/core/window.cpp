/***************************************
** Tsunagari Tile Engine              **
** window.cpp                         **
** Copyright 2011-2014 Michael Reiley **
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

#include "core/window.h"

#include "core/world.h"
#include "os/os.h"

Keys windowKeysDown = 0;

void
windowEmitKeyDown(Key key) noexcept {
    bool wasDown = !!(windowKeysDown & key);

    windowKeysDown |= key;

    if (windowKeysDown & KEY_ESCAPE &&
            (windowKeysDown & KEY_LEFT_SHIFT ||
             windowKeysDown & KEY_RIGHT_SHIFT)) {
        windowClose();
        exitProcess(0);
    }

    if (!wasDown) {
        World::buttonDown(key);
    }
}

void
windowEmitKeyUp(Key key) noexcept {
    bool wasDown = !!(windowKeysDown & key);

    windowKeysDown &= ~key;

    if (wasDown) {
        World::buttonUp(key);
    }
}
