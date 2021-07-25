#include "core/images.h"

#include "util/compiler.h"

#define NULL_TEXTURE reinterpret_cast<void*>(1)

void
imageInit() noexcept { }

Image
imageLoad(StringView path) noexcept {
    Image image = {NULL_TEXTURE, 0, 0, 1, 1};
    return image;
}

void
imageDraw(Image image, float x, float y, float z) noexcept { }

void
imageRelease(Image image) noexcept { }

TiledImage
tilesLoad(StringView path,
          uint32_t tileWidth,
          uint32_t tileHeight,
          uint32_t numAcross,
          uint32_t numWide) noexcept {
    Image image = {
            NULL_TEXTURE,
            0,
            0,
            tileWidth * numAcross,
            tileHeight * numWide,
    };
    TiledImage tiles = {image, tileWidth, tileHeight, numAcross * numWide};
    return tiles;
}

void
tilesRelease(TiledImage tiles) noexcept { }

Image
tileAt(TiledImage tiles, uint32_t index) noexcept {
    Image image = {
            NULL_TEXTURE,
            0,
            0,
            tiles.tileWidth,
            tiles.tileHeight,
    };
    return image;
}

void
imagesPrune(time_t latestPermissibleUse) noexcept { }
