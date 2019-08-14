/***************************************
** Tsunagari Tile Engine              **
** images.h                           **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

#include "util/rc.h"
#include "util/string-view.h"

class Image {
 public:
    virtual ~Image() = default;

    virtual void draw(float dstX, float dstY, float z) noexcept = 0;
    virtual void drawSubrect(float dstX,
                             float dstY,
                             float z,
                             float srcX,
                             float srcY,
                             float srcW,
                             float srcH) noexcept = 0;

    unsigned width() const noexcept;
    unsigned height() const noexcept;

 protected:
    Image(unsigned width, unsigned height) noexcept;

    unsigned _width;
    unsigned _height;

 private:
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
};


class TiledImage {
 public:
    virtual ~TiledImage() = default;

    virtual size_t size() const noexcept = 0;

    virtual Rc<Image> operator[](size_t n) const noexcept = 0;

 protected:
    TiledImage() = default;

 private:
    TiledImage(const TiledImage&) = delete;
    TiledImage& operator=(const TiledImage&) = delete;
};


class Images {
 public:
    //! Load an image from the file at the given path.
    static Rc<Image> load(StringView path) noexcept;

    //! Load an image of tiles from the file at the given path. Each tile
    //! with have width and heigh as specified.
    static Rc<TiledImage> loadTiles(StringView path,
                                     unsigned tileW,
                                     unsigned tileH) noexcept;

    //! Free images not recently used.
    static void garbageCollect() noexcept;
};

#endif  // SRC_CORE_IMAGES_H_
