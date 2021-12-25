#include "data/inprogress.h"

#include "data/inprogress-sound.h"
#include "data/inprogress-timer.h"
#include "tiles/sounds.h"
#include "util/assert.h"
#include "util/compiler.h"

InProgress::InProgress() noexcept : over(false) { }
InProgress::~InProgress() noexcept { }

bool
InProgress::isOver() noexcept {
    return over;
}


InProgressSound::InProgressSound(StringView sound, ThenFn onThen) noexcept
        : onThen(onThen) {
    SoundID sid = soundLoad(sound);
    this->sound = soundPlay(sid);
    soundRelease(sid);

    assert_(this->onThen.fn);
}

void
InProgressSound::tick(Time) noexcept {
    if (over) {
        return;
    }

    if (!playingSoundIsPlaying(sound)) {
        playingSoundRelease(sound);
        over = true;
        onThen.fn(onThen.data);
    }
}

InProgressTimer::InProgressTimer(Time duration, ThenFn onThen) noexcept
        : duration(duration), passed(0), onThen(onThen) {
    assert_(this->onThen.fn);
}

InProgressTimer::InProgressTimer(Time duration,
                                 ProgressFn onProgress,
                                 ThenFn onThen) noexcept
        : duration(duration), passed(0), onProgress(onProgress), onThen(onThen) {
    assert_(this->onProgress.fn);
    // then can be empty
}

void
InProgressTimer::tick(Time dt) noexcept {
    if (over) {
        return;
    }

    passed += dt;

    if (passed < duration) {
        if (onProgress.fn) {
            // Range is [0.0, 1.0)
            onProgress.fn(onProgress.data, (float)passed / (float)duration);
        }
    }
    else {
        over = true;
        if (onThen.fn) {
            onThen.fn(onThen.data);
        }
    }
}
