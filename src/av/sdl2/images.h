/*************************************
** Tsunagari Tile Engine            **
** images.h                         **
** Copyright 2016-2019 Paul Merrill **
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

#ifndef SRC_AV_SDL2_IMAGE_H_
#define SRC_AV_SDL2_IMAGE_H_

#include "av/sdl2/sdl2.h"
#include "cache/cache-template.h"
#include "cache/readercache.h"
#include "core/images.h"

struct SDL2Texture {
    explicit SDL2Texture(SDL_Texture* texture) noexcept;
    SDL2Texture(SDL2Texture&&) = delete;
    SDL2Texture(const SDL2Texture&) = delete;
    ~SDL2Texture() noexcept;

    void operator=(SDL2Texture&&) = delete;
    void operator=(const SDL2Texture&) = delete;

    SDL_Texture* texture;
};

class SDL2Image : public Image {
 public:
    SDL2Image(SDL_Texture* texture, int width, int height) noexcept;

    void draw(double dstX, double dstY, double z) noexcept final;
    void drawSubrect(double dstX,
                     double dstY,
                     double z,
                     double srcX,
                     double srcY,
                     double srcW,
                     double srcH) noexcept final;

    SDL2Texture texture;
};

class SDL2TiledSubImage : public Image {
 public:
    SDL2TiledSubImage(Rc<SDL2Texture> texture,
                      int xOff,
                      int yOff,
                      int width,
                      int height) noexcept;

    void draw(double dstX, double dstY, double z) noexcept final;
    void drawSubrect(double dstX,
                     double dstY,
                     double z,
                     double srcX,
                     double srcY,
                     double srcW,
                     double srcH) noexcept final;

    int xOff, yOff;
    Rc<SDL2Texture> texture;
};

class SDL2TiledImage : public TiledImage {
 public:
    SDL2TiledImage(SDL_Texture* texture,
                   int width,
                   int height,
                   int tileW,
                   int tileH) noexcept;

    size_t size() const noexcept final;

    Rc<Image> operator[](size_t n) const noexcept final;

 private:
    int width;
    /*int height;*/
    int tileW;
    int tileH;
    int numTiles;
    Rc<SDL2Texture> texture;
};


Rc<Image> genImage(StringView path) noexcept;

class SDL2Images : public Images {
 public:
    Rc<Image> load(StringView path) noexcept final;

    Rc<TiledImage> loadTiles(StringView path,
                             unsigned tileW,
                             unsigned tileH) noexcept final;

    void garbageCollect() noexcept final;

 private:
    ReaderCache<Rc<Image>, genImage> images;
    // We can't use a ReaderCache here because TiledImages are constructed
    // with three arguments, but a ReaderCache only supports the use of
    // one.
    Cache<Rc<TiledImage>> tiledImages;
};

#endif  // SRC_AV_SDL2_IMAGE_H_
