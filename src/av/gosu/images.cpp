/***************************************
** Tsunagari Tile Engine              **
** images.cpp                         **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2020 Paul Merrill   **
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

#include "core/images.h"

#include "av/gosu/cbuffer.h"
#include "av/gosu/gosu.h"
#include "av/gosu/window.h"
#include "cache/rc-cache-impl.h"
#include "cache/rc-reader-cache.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/window.h"
#include "util/assert.h"
#include "util/move.h"
#include "util/noexcept.h"
#include "util/pool.h"
#include "util/rc.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/unique.h"
#include "util/vector.h"

struct GosuImage {
    enum { STANDALONE, FROM_TILED_IMAGE } origin;

    int numUsers;
    time_t lastUse;

    Gosu::Image image;
};

struct GosuTiledImage {
    int numUsers;
    time_t lastUse;

    Vector<Gosu::Image> images;
};

static Hashmap<String, ImageID> imageIDs;
static Hashmap<String, TiledImageID> tiledImageIDs;
static Pool<GosuImage> imagePool;
static Pool<GosuTiledImage> tiledImagePool;

class GosuImages {
 public:
    Rc<Image>
    load(const std::string& path) noexcept;

    Rc<TiledImage>
    loadTiles(const std::string& path, unsigned tileW, unsigned tileH) noexcept;

    void
    garbageCollect() noexcept;
};

static Optional<GosuImage>
makeImage(StringView path) {
    Optional<StringView> r = Resources::load(path);
    if (!r) {
        // Error logged.
        return none;
    }

    GosuCBuffer buffer;
    buffer.data_ = r->data;
    buffer.size_ = r->size;

    Gosu::Bitmap bitmap;

    {
        TimeMeasure m(String() << "Bitmapped " << path);
        Gosu::load_image_file(bitmap, Gosu::Reader{&buffer, 0});
    }

    Log::info("Images",
              String() << "Bitmap " << path << " is " << bitmap.w << "×"
                       << bitmap.h);
    TimeMeasure m(String() << "Constructed " << path << " as image");
    return Optional<GosuImage>(
            GosuImage{GosuImage::STANDALONE,
                      0,
                      0,
                      Gosu::Image(bitmap, Gosu::IF_TILEABLE | Gosu::IF_RETRO)});
}

static Optional<GosuTiledImage>
makeTiledImage(StringView path, unsigned tileW, unsigned tileH) {
    Optional<StringView> r = Resources::load(path);
    if (!r) {
        // Error logged.
        return none;
    }

    GosuCBuffer buffer;
    buffer.data_ = r->data;
    buffer.size_ = r->size;

    Gosu::Bitmap bitmap;

    {
        TimeMeasure m(String() << "Bitmapped " << path);
        Gosu::load_image_file(bitmap, Gosu::Reader{&buffer, 0});
    }

    Log::info("Images",
              String() << "Bitmap " << path << " is " << bitmap.w << "×"
                       << bitmap.h);

    GosuTiledImage tiledImage;
    tiledImage.images.reserve(bitmap.w * bitmap.h / (tileW * tileH));

    {
        TimeMeasure m(String() << "Constructed " << path << " as tiles");

        for (unsigned y = 0; y < bitmap.h; y += tileH) {
            for (unsigned x = 0; x < bitmap.w; x += tileW) {
                tiledImage.images.push_back(
                        Gosu::Image(bitmap,
                                    x,
                                    y,
                                    tileW,
                                    tileH,
                                    Gosu::IF_TILEABLE | Gosu::IF_RETRO));
            }
        }
    }

    Log::info("Images",
              String() << "TiledImage " << path << " has "
                       << tiledImage.images.size() << " tiles");
    return Optional<GosuTiledImage>(move_(tiledImage));
}

ImageID
Images::load(StringView path) noexcept {
    Optional<ImageID*> cachedId = imageIDs.tryAt(path);
    if (cachedId) {
        int iid = ***cachedId;
        GosuImage& image = imagePool[iid];
        image.numUsers += 1;
        return ImageID(iid);
    }

    Optional<GosuImage> image = makeImage(path);
    if (!image) {
        imageIDs[path] = mark;
        return mark;
    }

    int iid = imagePool.allocate();
    imagePool[iid] = move_(*image);

    imageIDs[path] = iid;

    return ImageID(iid);
}

TiledImageID
Images::loadTiles(StringView path, int tileWidth, int tileHeight) noexcept {
    Optional<TiledImageID*> cachedId = tiledImageIDs.tryAt(path);
    if (cachedId) {
        int tiid = ***cachedId;
        GosuTiledImage& tiledImage = tiledImagePool[tiid];
        tiledImage.numUsers += 1;
        return TiledImageID(tiid);
    }

    Optional<GosuTiledImage> tiledImage =
            makeTiledImage(path, tileWidth, tileHeight);
    if (!tiledImage) {
        tiledImageIDs[path] = mark;
        return mark;
    }

    int tiid = tiledImagePool.allocate();
    tiledImagePool[tiid] = move_(*tiledImage);

    tiledImageIDs[path] = tiid;

    return TiledImageID(tiid);
}

void
Images::prune(time_t latestPermissibleUse) noexcept {
    // TODO
}

int
TiledImage::size(TiledImageID tiid) noexcept {
    assert_(tiid);

    GosuTiledImage& tiledImage = tiledImagePool[*tiid];
    return tiledImage.images.size();
}

ImageID
TiledImage::getTile(TiledImageID tiid, int i) noexcept {
    assert_(tiid);

    GosuTiledImage& ti = tiledImagePool[*tiid];

    int iid = imagePool.allocate();
    GosuImage& image = imagePool[iid];
    new (&image) GosuImage{GosuImage::FROM_TILED_IMAGE, 0, 0, ti.images[i]};

    return ImageID(iid);
}

void
TiledImage::release(TiledImageID tiid) noexcept {
    if (!tiid) {
        return;
    }

    GosuTiledImage& tiledImage = tiledImagePool[*tiid];

    tiledImage.numUsers -= 1;
    assert_(tiledImage.numUsers >= 0);

    if (tiledImage.numUsers == 0) {
        tiledImage.lastUse = World::time();
    }
}

void
Image::draw(ImageID iid, float x, float y, float z) noexcept {
    assert_(iid);

    GosuImage& image = imagePool[*iid];
    image.image.draw(
            x, y, z, 1.0, 1.0, Gosu::Color{0x00000000}, Gosu::AM_DEFAULT);
}

int
Image::width(ImageID iid) noexcept {
    assert_(iid);

    GosuImage& image = imagePool[*iid];
    return static_cast<int>(image.image.width());
}

int
Image::height(ImageID iid) noexcept {
    assert_(iid);

    GosuImage& image = imagePool[*iid];
    return static_cast<int>(image.image.height());
}

void
Image::release(ImageID iid) noexcept {
    if (!iid) {
        return;
    }

    GosuImage& image = imagePool[*iid];

    if (image.origin == GosuImage::FROM_TILED_IMAGE) {
        imagePool.release(iid);
    }
    else {
        image.numUsers -= 1;
        assert_(image.numUsers >= 0);

        if (image.numUsers == 0) {
            image.lastUse = World::time();
        }
    }
}
