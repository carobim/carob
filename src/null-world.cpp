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
#include "util/unique.h"

static DataArea nullArea;

DataArea*
DataWorld::area(StringView areaName) noexcept {
    return &nullArea;
}

DataWorld::About DataWorld::about = {
    .name = "Null World",
    .author = "Paul Merrill",
    .version = "1",
};

DataWorld::Parameters DataWorld::parameters = {
    .moveMode = Conf::TILE,
    .viewportResolution = {240, 160},
    .input = {
        .persistDelay = {
            .initial = 300,
            .consecutive = 100,
        },
    },
    .gameStart = {
        .player = {
            .file = "null-player.json",
            .phase = "down",
        },
        .area = "null-area.json",
        .coords = {0, 0, 0},
    },
};

StringView DataWorld::datafile = "./null.world";

bool
DataWorld::init() noexcept {
    return true;
}
