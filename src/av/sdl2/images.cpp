/*************************************
** Tsunagari Tile Engine            **
** images.cpp                       **
** Copyright 2016-2020 Paul Merrill **
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

#include "av/sdl2/sdl2.h"
#include "av/sdl2/window.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "util/hash.h"
#include "util/hashvector.h"
#include "util/int.h"
#include "util/noexcept.h"
#include "util/optional.h"
#include "util/string-view.h"
#include "util/string.h"

#define ATLAS_WIDTH 2048
#define ATLAS_HEIGHT 512

static SDL_Texture* atlas = 0;
static uint32_t atlasUsed = 0;

static HashVector<TiledImage> images;

static void
initAtlas() noexcept {
    if (atlas == 0) {
        SDL_Renderer* renderer = SDL2GameWindow::renderer;
        atlas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_TARGET, ATLAS_WIDTH,
                                  ATLAS_HEIGHT);
        if (atlas == 0) {
            logFatal("SDL2", "Failed to create texture");
        }

        SDL_SetTextureBlendMode(atlas, SDL_BLENDMODE_BLEND);

        SDL_SetRenderTarget(renderer, atlas);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, 0);
    }
}

static TiledImage*
load(StringView path) noexcept {
    TiledImage& tiles = images.allocate(hash_(path));
    tiles = {};

    Optional<StringView> r = Resources::load(path);
    if (!r) {
        // Error logged.
        return 0;
    }

    // FIXME: Do this at the resource level.
    //assert_(r->size < INT32_MAX);

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r->data)),
                          static_cast<int>(r->size));

    int x = atlasUsed;
    int y = 0;
    int width;
    int height;

    {
        TimeMeasure m(String() << "Constructed " << path << " as image");

        SDL_Surface* surface = IMG_Load_RW(ops, 1);
        //SDL_Surface* surface = SDL_LoadBMP_RW(ops, 1);
        if (!surface) {
            logFatal("SDL2", String() << "Invalid image: " << path);
            return 0;
        }

        width = surface->w;
        height = surface->h;

        initAtlas();

        // Rectangle packing algorithm:
        //
        // Copy surface into the atlas to the right of the previous image,
        // or at the left edge, if there was no previous image.
        SDL_Renderer* renderer = SDL2GameWindow::renderer;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            logFatal("SDL2",
                     String() << "Failed to create texture: " << path);
            return 0;
        }

        // Copy this texture's data into the atlas texture.
        SDL_Rect src = { 0, 0, width, height  };
        SDL_Rect dst = { x, y, width, height  };

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

        SDL_SetRenderTarget(renderer, atlas);
        SDL_RenderCopy(renderer, texture, &src, &dst);
        SDL_SetRenderTarget(renderer, 0);

        // Done with this texture.
        SDL_DestroyTexture(texture);
    }

    tiles.image = {
        atlas,
        static_cast<uint32_t>(x),
        static_cast<uint32_t>(y),
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };

    atlasUsed += static_cast<uint32_t>(width);

    return &tiles;
}

Image
imageLoad(StringView path) noexcept {
    TiledImage* tiles = images.find(hash_(path));

    if (!tiles) {
        tiles = load(path);
    }

    return tiles->image;
}

void
imageRelease(Image image) noexcept {}

void
imageDraw(Image image, float x, float y, float z) noexcept {
    assert_(IMAGE_VALID(image));

    SDL_Renderer* renderer = SDL2GameWindow::renderer;
    rvec2 translation = SDL2GameWindow::translation;
    rvec2 scaling = SDL2GameWindow::scaling;

    SDL_Texture* texture = static_cast<SDL_Texture*>(image.texture);
    SDL_Rect src{image.x, image.y, image.width, image.height};
    SDL_Rect dst{static_cast<int>((x + translation.x) * scaling.x),
                 static_cast<int>((y + translation.y) * scaling.y),
                 static_cast<int>(image.width * scaling.x),
                 static_cast<int>(image.height * scaling.y)};
    SDL_SetRenderTarget(renderer, 0);
    SDL_RenderCopy(renderer, texture, &src, &dst);
}

TiledImage
tilesLoad(StringView path, uint32_t tileWidth, uint32_t tileHeight) noexcept {
    TiledImage* tiles = images.find(hash_(path));

    if (!tiles) {
        tiles = load(path);
        tiles->tileWidth = tileWidth;
        tiles->tileHeight = tileHeight;
        tiles->numTiles = (tiles->image.width / tileWidth) *
                          (tiles->image.height / tileHeight);
    }

    return *tiles;
}

void
tilesRelease(TiledImage tiles) noexcept {}

Image
tileAt(TiledImage tiles, uint32_t index) noexcept {
    assert_(TILES_VALID(tiles));

    Image image = tiles.image;

    return {
        tiles.image.texture,
        tiles.image.x + tiles.tileWidth * index % tiles.image.width,
        tiles.image.y + tiles.tileWidth * index / tiles.image.width *
                        tiles.tileHeight,
        tiles.tileWidth,
        tiles.tileHeight,
    };
}

void
imagesPrune(time_t latestPermissibleUse) noexcept {}
