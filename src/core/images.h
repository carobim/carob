/***************************************
** Tsunagari Tile Engine              **
** images.h                           **
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

#ifndef SRC_CORE_IMAGES_H_
#define SRC_CORE_IMAGES_H_

#include "util/int.h"
#include "util/string-view.h"

struct Image {
    void* texture;

    uint32_t x      : 16;
    uint32_t y      : 16;

    uint32_t width  : 16;
    uint32_t height : 16;
};

struct TiledImage {
    Image image;

    uint32_t tileWidth  : 10;  // 0-1023
    uint32_t tileHeight : 10;  // 0-1023

    uint32_t numTiles   : 12;  // 0-4095
};

void
imageInit() noexcept;

// Load an image from the file at the given path.
Image
imageLoad(StringView path) noexcept;

#define IMAGE_VALID(image) (image.texture != 0)

void
imageDraw(Image image, float x, float y, float z) noexcept;

void
imageRelease(Image image) noexcept;

// Load an image of tiles from the file at the given path. Each tile with width
// and heigh as specified. If the image does not have the expected number of
// tiles across and high, loading fails.
TiledImage
tilesLoad(StringView path,
          uint32_t tileWidth,
          uint32_t tileHeight,
          uint32_t numAcross,
          uint32_t numHigh) noexcept;

#define TILES_VALID(tiles) (tiles.image.texture != 0)

void
tilesRelease(TiledImage tiles) noexcept;

Image
tileAt(TiledImage tiles, uint32_t index) noexcept;

// Free images and tiled images not recently used.
void
imagesPrune(time_t latestPermissibleUse) noexcept;

/**
 * Draws a rectangle on the screen of the specified color. Coordinates
 * are in virtual pixels.
 */
void
imageDrawRect(float x1, float x2, float y1, float y2, uint32_t argb) noexcept;


#endif  // SRC_CORE_IMAGES_H_
