#include "tiles/images.h"

#include "av/sdl2/error.h"
#include "av/sdl2/sdl2.h"
#include "av/sdl2/window.h"
#include "tiles/log.h"
#include "tiles/resources.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/hashvector.h"
#include "util/int.h"
#include "util/measure.h"
#include "util/string-view.h"
#include "util/string.h"

#define ATLAS_WIDTH  2048
#define ATLAS_HEIGHT 512

static SDL_Renderer* renderer = 0;
static SDL_Texture* atlas = 0;
static U32 atlasUsed = 0;

static HashVector<TiledImage> images;

void
imageInit() noexcept {
    TimeMeasure m("Created SDL2 renderer");

    renderer = SDL_CreateRenderer(sdl2Window,
                                  -1,
                                  SDL_RENDERER_ACCELERATED |
                                          SDL_RENDERER_PRESENTVSYNC |
                                          SDL_RENDERER_TARGETTEXTURE);

    if (renderer == 0) {
        sdlDie("SDL2", "SDL_CreateRenderer");
    }

    SDL_RendererInfo info;

    if (SDL_GetRendererInfo(renderer, &info) < 0) {
        sdlDie("SDL2", "SDL_GetRendererInfo");
    }

    StringView name = info.name;
    bool vsync = (info.flags & SDL_RENDERER_PRESENTVSYNC) != 0;

    logInfo("SDL2",
            String("Rendering will be done with ")
                    << name << (vsync ? " with vsync" : " without vsync"));

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

    // Blank until the start of a frame.
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
    //SDL_RenderClear(renderer);
    //SDL_RenderPresent(renderer);
}

void
imageStartFrame() noexcept {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void
imageEndFrame() noexcept {
    SDL_RenderPresent(renderer);
}

void
imageDrawRect(float left,
              float right,
              float top,
              float bottom,
              float z,
              U32 argb) noexcept {
    U8 a = static_cast<U8>((argb >> 24) & 0xFF);
    U8 r = static_cast<U8>((argb >> 16) & 0xFF);
    U8 g = static_cast<U8>((argb >> 8) & 0xFF);
    U8 b = static_cast<U8>((argb >> 0) & 0xFF);

    SDL_Rect rect = {static_cast<int>(left),
                     static_cast<int>(top),
                     static_cast<int>(right - left),
                     static_cast<int>(bottom - top)};

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &rect);
}

static void
initAtlas() noexcept {
    if (atlas == 0) {
        atlas = SDL_CreateTexture(renderer,
                                  SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_TARGET,
                                  ATLAS_WIDTH,
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

    String r;
    if (!resourceLoad(path, r)) {
        // Error logged.
        return 0;
    }

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r.data)),
                          static_cast<int>(r.size));

    int x = atlasUsed;
    int y = 0;
    int width;
    int height;

    {
        TimeMeasure m(String() << "Constructed " << path << " as image");

        //SDL_Surface* surface = IMG_Load_RW(ops, 1);
        SDL_Surface* surface = SDL_LoadBMP_RW(ops, 1);
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
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            logFatal("SDL2", String() << "Failed to create texture: " << path);
            return 0;
        }

        // Copy this texture's data into the atlas texture.
        SDL_Rect src = {0, 0, width, height};
        SDL_Rect dst = {x, y, width, height};

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

        SDL_SetRenderTarget(renderer, atlas);
        SDL_RenderCopy(renderer, texture, &src, &dst);
        SDL_SetRenderTarget(renderer, 0);

        // Done with this texture.
        SDL_DestroyTexture(texture);
    }

    tiles.image = {
            atlas,
            static_cast<U32>(x),
            static_cast<U32>(y),
            static_cast<U32>(width),
            static_cast<U32>(height),
    };

    atlasUsed += static_cast<U32>(width);

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
imageRelease(Image image) noexcept { }

void
imageDraw(Image image, float x, float y, float z) noexcept {
    assert_(IMAGE_VALID(image));

    fvec2 translation = sdl2Translation;
    fvec2 scaling = sdl2Scaling;

    SDL_Texture* texture = static_cast<SDL_Texture*>(image.texture);
    SDL_Rect src{static_cast<int>(image.x),
                 static_cast<int>(image.y),
                 static_cast<int>(image.width),
                 static_cast<int>(image.height)};
    SDL_Rect dst{static_cast<int>((x + translation.x) * scaling.x),
                 static_cast<int>((y + translation.y) * scaling.y),
                 static_cast<int>(image.width * scaling.x),
                 static_cast<int>(image.height * scaling.y)};
    SDL_SetRenderTarget(renderer, 0);
    SDL_RenderCopy(renderer, texture, &src, &dst);
}

TiledImage
tilesLoad(StringView path,
          U32 tileWidth,
          U32 tileHeight,
          U32 numAcross,
          U32 numHigh) noexcept {
    TiledImage* tiles = images.find(hash_(path));

    if (!tiles) {
        tiles = load(path);

        assert_(tiles->image.width == tileWidth * numAcross);
        assert_(tiles->image.height == tileHeight * numHigh);

        tiles->tileWidth = tileWidth;
        tiles->tileHeight = tileHeight;
        tiles->numTiles = numAcross * numHigh;
    }

    return *tiles;
}

void
tilesRelease(TiledImage tiles) noexcept { }

Image
tileAt(TiledImage tiles, U32 index) noexcept {
    assert_(TILES_VALID(tiles));

    Image image = tiles.image;

    return {
            image.texture,
            image.x + tiles.tileWidth * index % image.width,
            image.y + tiles.tileWidth * index / image.width * tiles.tileHeight,
            tiles.tileWidth,
            tiles.tileHeight,
    };
}

void
imagesPrune(Time latestPermissibleUse) noexcept { }

void
imageFlushImages() noexcept { }

void
imageFlushRects() noexcept { }
