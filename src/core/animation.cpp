/***************************************
** Tsunagari Tile Engine              **
** animation.cpp                      **
** Copyright 2011-2013 Michael Reiley **
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

#include "core/animation.h"

#include "util/assert.h"
#include "util/move.h"
#include "util/pool.h"

struct AnimationData {
    /** List of images in animation. */
    Vector<ImageID> frames;

    /** Length of each frame in animation in milliseconds. */
    time_t frameTime;

    /** Length of one complete cycle through animation in milliseconds. */
    time_t cycleTime;

    /** Time offset to find current animation frame. */
    time_t offset;

    /** Index of frame currently displaying on screen. */
    uint32_t currentIndex;

    ImageID currentImage;
};

static Pool<AnimationData> pool;

static AnimationID
makeSingleFrame(ImageID iid) noexcept {
    return -*iid - 1;
}

static bool
isSingleFrame(AnimationID aid) noexcept {
    return aid < 0;
}

static ImageID
getSingleFrame(AnimationID aid) noexcept {
    return ImageID(-aid - 1);
}

static void
copy(AnimationID& self, AnimationID other) noexcept {
    if (isSingleFrame(other)) {
        self = other;
    }
    else {
        self = pool.allocate();

        AnimationData& data = pool[self];
        AnimationData& otherData = pool[other];

        data = otherData;
    }
}

static void
move(AnimationID& self, AnimationID& other) noexcept {
    // Take over the AnimationID of the other.
    self = other;
    other = -1;
}

static void
destroy(AnimationID aid) noexcept {
    if (!isSingleFrame(aid)) {
        pool.release(aid);
    }
}

Animation::Animation() noexcept {
    aid = -1;
}

Animation::Animation(ImageID frame) noexcept {
    assert_(frame.exists());

    aid = makeSingleFrame(frame);
}

Animation::Animation(Vector<ImageID> frames, time_t frameTime) noexcept {
    assert_(frames.size() > 0);
    assert_(frameTime > 0);
    for (ImageID frame : frames) {
        assert_(frame.exists());
    }

    aid = pool.allocate();
    AnimationData& data = pool[aid];

    new (&data.frames) Vector<ImageID>();

    data.frames = move_(frames);
    data.frameTime = frameTime;
    data.cycleTime = frameTime * static_cast<time_t>(data.frames.size());
    data.offset = 0;
    data.currentIndex = 0;
    data.currentImage = data.frames[0];
}

Animation::Animation(const Animation& other) noexcept {
    copy(aid, other.aid);
}

Animation::Animation(Animation&& other) noexcept {
    move(aid, other.aid);
}

Animation::~Animation() noexcept {
    destroy(aid);
}

Animation&
Animation::operator=(const Animation& other) noexcept {
    destroy(aid);
    copy(aid, other.aid);
    return *this;
}

Animation&
Animation::operator=(Animation&& other) noexcept {
    destroy(aid);
    move(aid, other.aid);
    return *this;
}

void
Animation::restart(time_t now) noexcept {
    if (isSingleFrame(aid)) {
        return;
    }

    AnimationData& data = pool[aid];

    data.offset = now;
    data.currentIndex = 0;
    data.currentImage = data.frames[0];
}

bool
Animation::needsRedraw(time_t now) const noexcept {
    if (isSingleFrame(aid)) {
        return false;
    }

    AnimationData& data = pool[aid];

    time_t pos = now - data.offset;
    size_t index = static_cast<size_t>((pos % data.cycleTime) / data.frameTime);

    return index != data.currentIndex;
}

ImageID
Animation::setFrame(time_t now) noexcept {
    if (isSingleFrame(aid)) {
        return getSingleFrame(aid);
    }

    AnimationData& data = pool[aid];

    time_t pos = now - data.offset;
    uint32_t index =
            static_cast<uint32_t>((pos % data.cycleTime) / data.frameTime);
    ImageID image = data.frames[index];

    data.currentIndex = index;
    data.currentImage = image;

    return image;
}

ImageID
Animation::getFrame() const noexcept {
    if (isSingleFrame(aid)) {
        return getSingleFrame(aid);
    }

    AnimationData& data = pool[aid];

    return data.currentImage;
}
