#ifndef SRC_TILES_IMAGES_H_
#define SRC_TILES_IMAGES_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

struct Image {
    void* texture;

    U32 x      : 16;
    U32 y      : 16;

    U32 width  : 16;
    U32 height : 16;
};

struct TiledImage {
    Image image;

    U32 tileWidth  : 10;  // 0-1023
    U32 tileHeight : 10;  // 0-1023

    U32 numTiles   : 12;  // 0-4095
};

void
imageInit() noexcept;

// Load an image from the file at the given path.
Image
imageLoad(StringView path) noexcept;

#define IMAGE_VALID(image) ((image).texture != 0)

void
imageDraw(Image image, float x, float y, float z) noexcept;

void
imageRelease(Image image) noexcept;

// Load an image of tiles from the file at the given path. Each tile with width
// and heigh as specified. If the image does not have the expected number of
// tiles across and high, loading fails.
TiledImage
tilesLoad(StringView path,
          U32 tileWidth,
          U32 tileHeight,
          U32 numAcross,
          U32 numHigh) noexcept;

#define TILES_VALID(tiles) (tiles.image.texture != 0)

void
tilesRelease(TiledImage tiles) noexcept;

Image
tileAt(TiledImage tiles, U32 index) noexcept;

// Free images and tiled images not recently used.
void
imagesPrune(Time latestPermissibleUse) noexcept;

/**
 * Draws a rectangle on the screen of the specified color. Coordinates
 * are in virtual pixels.
 */
void
imageDrawRect(float left, float right, float top, float bottom, float z,
              U32 argb) noexcept;

void
imageFlushImages() noexcept;

void
imageFlushRects() noexcept;


#endif  // SRC_TILES_IMAGES_H_
