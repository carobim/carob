/*************************************
** Tsunagari Tile Engine            **
** images.cpp                       **
** Copyright 2016-2021 Paul Merrill **
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

#include "core/images.h"

#define NULL_TEXTURE reinterpret_cast<void*>(1)

void
imageInit() noexcept {}

Image
imageLoad(StringView path) noexcept {
    return { NULL_TEXTURE, 0, 0, 1, 1 };
}

void
imageDraw(Image image, float x, float y, float z) noexcept {}

void
imageRelease(Image image) noexcept {}

TiledImage
tilesLoad(StringView path, uint32_t tileWidth, uint32_t tileHeight,
          uint32_t numAcross, uint32_t numWide) noexcept {
    Image image = {
        NULL_TEXTURE,
        0,
        0,
        tileWidth * numAcross,
        tileHeight * numWide,
    };
    return { image, tileWidth, tileHeight, numAcross * numWide };
}

void
tilesRelease(TiledImage tiles) noexcept {}

Image
tileAt(TiledImage tiles, uint32_t index) noexcept {
    return {
        NULL_TEXTURE,
        0,
        0,
        tiles.tileWidth,
        tiles.tileHeight,
    };
}

void
imagesPrune(time_t latestPermissibleUse) noexcept {}
