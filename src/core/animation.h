#ifndef SRC_CORE_ANIMATION_H_
#define SRC_CORE_ANIMATION_H_

#include "core/images.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/vector.h"

typedef U32 AnimationID;

// Value of .id when default constructed. Do not use these objects.
#define NO_ANIMATION UINT32_MAX

/**
 * An Animation is a sequence of bitmap images (called frames) used to creates
 * the illusion of motion. Frames are cycled over with an even amount of time
 * given to each, and the whole animation starts over after the last frame is
 * displayed.
 *
 * Mechanically, it is a list of images and a period of time over which to
 * play.
 */
class Animation {
 public:
    /**
     * Constructs a null Animation. Not safe to use, but it allows Animation to
     * be put into containers.
     */
    Animation() noexcept;

    /**
     * Constructs a single-frame Animation. It will function like a static
     * image.
     *
     * @param frame static image
     */
    explicit Animation(Image frame) noexcept;

    /**
     * Constructs a Animation from a list of frames.
     *
     * If given more than one frame, frameTime must be a positive,
     * non-zero value.
     *
     * @param frames list of frames to cycle through
     * @param frameTime length of time in milliseconds that each frame
     *        will display for
     */
    Animation(Vector<Image> frames, Time frameTime) noexcept;

    Animation(Animation& other) noexcept;
    Animation(Animation&& other) noexcept;
    ~Animation() noexcept;

    void
    operator=(Animation& other) noexcept;
    void
    operator=(Animation&& other) noexcept;

    /**
     * Starts the animation over.
     *
     * @now current time in milliseconds
     */
    void
    restart(Time now) noexcept;

    /**
     * Has this Animation switched frames since frame() was last called?
     *
     * @now current time in milliseconds
     */
    bool
    needsRedraw(Time now) noexcept;

    /**
     * Returns the image that should be displayed at this time.
     *
     * @now current time in milliseconds
     */
    Image
    setFrame(Time now) noexcept;

    /**
     * Returns the last image that should have been displayed.
     */
    Image
    getFrame() noexcept;

 public:
    AnimationID id;
};

#endif  // SRC_CORE_ANIMATION_H_
