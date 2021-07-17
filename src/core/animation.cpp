#include "core/animation.h"

#include "util/assert.h"
#include "util/compiler.h"
#include "util/pool.h"

struct AnimationData {
    /** List of images in animation. */
    Vector<Image> frames;

    /** Length of each frame in animation in milliseconds. */
    time_t frameTime;

    /** Length of one complete cycle through animation in milliseconds. */
    time_t cycleTime;

    /** Time offset to find current animation frame. */
    time_t offset;

    /** Index of frame currently displaying on screen. */
    uint32_t currentIndex;

    Image currentImage;

    uint32_t refCnt;
};

static Pool<AnimationData> pool;

static bool
isSingleFrame(AnimationID self) noexcept {
    assert_(self != NO_ANIMATION);

    return pool[self].frameTime == 0;
}

static void
destroy(AnimationID self) noexcept {
    if (self == NO_ANIMATION) {
        return;
    }

    AnimationData& data = pool[self];

    if (!isSingleFrame(self)) {
        data.frames.~Vector<Image>();
    }

    pool.release(self);
}

static void
incRef(AnimationID self) noexcept {
    if (self != NO_ANIMATION) {
        ++pool[self].refCnt;
    }
}

static void
decRef(AnimationID self) noexcept {
    if (self != NO_ANIMATION && --pool[self].refCnt == 0) {
        destroy(self);
    }
}

static void
assign(AnimationID& self, AnimationID& other) noexcept {
    decRef(self);
    self = other;
    incRef(other);
}

Animation::Animation() noexcept {
    id = NO_ANIMATION;
}

Animation::Animation(Image frame) noexcept {
    assert_(IMAGE_VALID(frame));

    id = pool.allocate();
    AnimationData& data = pool[id];

    data.frameTime = 0;
    data.currentImage = frame;
    data.refCnt = 1;
}

Animation::Animation(Vector<Image> frames, time_t frameTime) noexcept {
    assert_(frames.size > 0);
    assert_(frameTime > 0);
    for (Image* frame = frames.begin(); frame != frames.end(); frame++) {
        assert_(IMAGE_VALID(*frame));
        (void)frame;
    }

    id = pool.allocate();
    AnimationData& data = pool[id];

    new (&data.frames) Vector<Image>();
    data.frames = static_cast<Vector<Image>&&>(frames);
    data.frameTime = frameTime;
    data.cycleTime = frameTime * static_cast<time_t>(data.frames.size);
    data.offset = 0;
    data.currentIndex = 0;
    data.currentImage = data.frames[0];
    data.refCnt = 1;
}

Animation::Animation(Animation& other) noexcept {
    id = other.id;
    incRef(other.id);
}

Animation::Animation(Animation&& other) noexcept {
    id = other.id;
    incRef(other.id);
}

Animation::~Animation() noexcept {
    decRef(id);
}

void
Animation::operator=(Animation& other) noexcept {
    assign(id, other.id);
}

void
Animation::operator=(Animation&& other) noexcept {
    assign(id, other.id);
}

void
Animation::restart(time_t now) noexcept {
    assert_(id != NO_ANIMATION);

    if (isSingleFrame(id)) {
        return;
    }

    AnimationData& data = pool[id];

    data.offset = now;
    data.currentIndex = 0;
    data.currentImage = data.frames[0];
}

bool
Animation::needsRedraw(time_t now) noexcept {
    assert_(id != NO_ANIMATION);

    if (isSingleFrame(id)) {
        return false;
    }

    AnimationData& data = pool[id];

    time_t pos = now - data.offset;
    size_t index = static_cast<size_t>((pos % data.cycleTime) / data.frameTime);

    return index != data.currentIndex;
}

Image
Animation::setFrame(time_t now) noexcept {
    assert_(id != NO_ANIMATION);

    AnimationData& data = pool[id];

    if (!isSingleFrame(id)) {
        time_t pos = now - data.offset;
        uint32_t index =
                static_cast<uint32_t>((pos % data.cycleTime) / data.frameTime);
        Image image = data.frames[index];

        data.currentIndex = index;
        data.currentImage = image;
    }

    return data.currentImage;
}

Image
Animation::getFrame() noexcept {
    assert_(id != NO_ANIMATION);

    return pool[id].currentImage;
}
