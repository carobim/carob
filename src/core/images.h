#ifndef SRC_CORE_IMAGES_H_
#define SRC_CORE_IMAGES_H_

#include "util/compiler.h"
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
imageDrawRect(float left, float right, float top, float bottom, float z,
              uint32_t argb) noexcept;

void
imageFlushImages() noexcept;

void
imageFlushRects() noexcept;


#endif  // SRC_CORE_IMAGES_H_
