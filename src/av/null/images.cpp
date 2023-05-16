#include "tiles/images.h"

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
tilesLoad(StringView path, U32 tileWidth, U32 tileHeight, U32 numAcross,
          U32 numWide) noexcept {
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
tileAt(TiledImage tiles, U32 index) noexcept {
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
imagesPrune(Time latestPermissibleUse) noexcept { }
