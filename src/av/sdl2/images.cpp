/*************************************
** Tsunagari Tile Engine            **
** images.cpp                       **
** Copyright 2016-2018 Paul Merrill **
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

#include "av/sdl2/images.h"

#include <SDL2/SDL_image.h>

#include <limits.h>

#include "core/measure.h"
#include "core/resources.h"

#include "av/sdl2/window.h"


Images& Images::instance() {
    static auto globalImages = new SDL2Images;
    return *globalImages;
}


Rc<Image> genImage(const std::string& path) {
    Unique<Resource> r = Resources::instance().load(path);
    if (!r) {
        // Error logged.
        return Rc<Image>();
    }

    assert_(r->size() < INT_MAX);

    SDL_RWops* ops = SDL_RWFromMem(const_cast<void*>(r->data()),
                                   static_cast<int>(r->size()));

    TimeMeasure m("Constructed " + path + " as image");
    SDL_Surface* surface = IMG_Load_RW(ops, 1);
    SDL_Renderer* renderer = SDL2GameWindow::instance().renderer;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int width;
    int height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    assert_(width <= 4096);
    assert_(height <= 4096);

    return Rc<Image>(new SDL2Image(texture, width, height));
}

static Rc<TiledImage> genTiledImage(const std::string& path,
                                    unsigned tileW, unsigned tileH) {
    assert_(tileW <= 4096);
    assert_(tileH <= 4096);

    Unique<Resource> r = Resources::instance().load(path);
    if (!r) {
        // Error logged.
        return Rc<TiledImage>();
    }

    assert_(r->size() < INT_MAX);

    SDL_RWops* ops = SDL_RWFromMem(const_cast<void*>(r->data()),
                                   static_cast<int>(r->size()));

    TimeMeasure m("Constructed " + path + " as image");
    SDL_Surface* surface = IMG_Load_RW(ops, 1);
    SDL_Renderer* renderer = SDL2GameWindow::instance().renderer;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int width;
    int height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    assert_(width <= 4096);
    assert_(height <= 4096);

    return Rc<TiledImage>(new SDL2TiledImage(texture,
                                             width,
                                             height,
                                             static_cast<int>(tileW),
                                             static_cast<int>(tileH)));
}


SDL2Texture::SDL2Texture(SDL_Texture* texture) : texture(texture) {}

SDL2Texture::~SDL2Texture() {
    SDL_DestroyTexture(texture);
}


SDL2Image::SDL2Image(SDL_Texture* texture, int width, int height)
        : Image(static_cast<unsigned int>(width),
                static_cast<unsigned int>(height)),
          texture(texture) {}

void SDL2Image::draw(double dstX, double dstY, double /*z*/) {
    SDL_Renderer* renderer = SDL2GameWindow::instance().renderer;
    rvec2 translation = SDL2GameWindow::instance().translation;
    rvec2 scaling = SDL2GameWindow::instance().scaling;

    SDL_Rect src{0,
                 0,
                 static_cast<int>(_width),
                 static_cast<int>(_height)};
    SDL_Rect dst{static_cast<int>((dstX + translation.x) * scaling.x),
                 static_cast<int>((dstY + translation.y) * scaling.y),
                 static_cast<int>(_width * scaling.x),
                 static_cast<int>(_height * scaling.y)};
    SDL_RenderCopy(renderer, texture.texture, &src, &dst);
}

void SDL2Image::drawSubrect(double /*dstX*/, double /*dstY*/, double /*z*/,
                            double /*srcX*/, double /*srcY*/,
                            double /*srcW*/, double /*srcH*/) {
    assert_(false);
}


SDL2TiledSubImage::SDL2TiledSubImage(Rc<SDL2Texture> texture,
                                     int width, int height,
                                     int xOff, int yOff)
        : Image(static_cast<unsigned int>(width),
                static_cast<unsigned int>(height)),
          xOff(xOff), yOff(yOff),
          texture(move_(texture)) {}

void SDL2TiledSubImage::draw(double dstX, double dstY, double /*z*/) {
    SDL_Renderer* renderer = SDL2GameWindow::instance().renderer;
    rvec2 translation = SDL2GameWindow::instance().translation;
    rvec2 scaling = SDL2GameWindow::instance().scaling;

    SDL_Rect src{xOff,
                 yOff,
                 static_cast<int>(_width),
                 static_cast<int>(_height)};
    SDL_Rect dst{static_cast<int>((dstX + translation.x) * scaling.x),
                 static_cast<int>((dstY + translation.y) * scaling.y),
                 static_cast<int>(_width * scaling.x),
                 static_cast<int>(_height * scaling.y)};
    SDL_RenderCopy(renderer, texture->texture, &src, &dst);
}

void SDL2TiledSubImage::drawSubrect(double /*dstX*/, double /*dstY*/,
                                    double /*z*/,
                                    double /*srcX*/, double /*srcY*/,
                                    double /*srcW*/, double /*srcH*/) {
    assert_(false);
}


SDL2TiledImage::SDL2TiledImage(SDL_Texture* texture,
                               int width,
                               int height,
                               int tileW,
                               int tileH)
        : width(width),
          /*height(height),*/
          tileW(tileW),
          tileH(tileH),
          numTiles((width / tileW) * (height / tileH)),
          texture(new SDL2Texture(texture)) {}

size_t SDL2TiledImage::size() const {
    return static_cast<size_t>(numTiles);
}

Rc<Image> SDL2TiledImage::operator[](size_t n) const {
    int xOff = tileW * static_cast<int>(n) % width;
    int yOff = tileW * static_cast<int>(n) / width * tileH;

    Rc<Image>* image = new Rc<Image>(new SDL2TiledSubImage(texture,
                                                           tileW, tileH,
                                                           xOff, yOff));
    return *image;
}


Rc<Image> SDL2Images::load(const std::string& path) {
    return images.lifetimeRequest(path);
}

Rc<TiledImage> SDL2Images::loadTiles(const std::string& path,
                                     unsigned tileW, unsigned tileH) {
    auto tiledImage = tiledImages.lifetimeRequest(path);
    if (!tiledImage) {
        tiledImage = genTiledImage(path, tileW, tileH);
        tiledImages.lifetimePut(path, tiledImage);
    }
    return tiledImage;
}

void SDL2Images::garbageCollect() {
    images.garbageCollect();
    tiledImages.garbageCollect();
}
