/*************************************
** Tsunagari Tile Engine            **
** null-world.cpp                   **
** Copyright 2019-2020 Paul Merrill **
*************************************/

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

#include "data/data-area.h"
#include "data/data-world.h"

static DataArea nullArea;

DataArea*
dataWorldArea(StringView areaName) noexcept {
    return &nullArea;
}

StringView dataWorldName = "Null World";
StringView dataWorldAuthor = "Paul Merrill";
StringView dataWorldVersion = "1";

enum MoveMode dataWorldMoveMode = MoveMode::TILE;
rvec2 dataWorldViewportResolution = {240, 160};
int dataWorldInputPersistDelayInitial = 300;
int dataWorldInputPersistDelayConsecutive = 100;
StringView dataWorldStartArea = "null-area.json";
StringView dataWorldPlayerFile = "null-player.json";
StringView dataWorldPlayerStartPhase = "down";
vicoord dataWorldStartCoords = {0, 0, 0};

StringView dataWorldDatafile = "./null.world";

bool
dataWorldInit() noexcept {
    return true;
}
