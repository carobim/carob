#include "data/inprogress.h"

#include "core/log.h"
#include "core/sounds.h"
#include "data/inprogress-sound.h"
#include "data/inprogress-timer.h"
#include "util/compiler.h"

InProgress::InProgress() noexcept : over(false) { }
InProgress::~InProgress() noexcept { }

bool
InProgress::isOver() noexcept {
    return over;
}


InProgressSound::InProgressSound(StringView sound, ThenFn then) noexcept
        : then(static_cast<ThenFn&&>(then)) {
    SoundID sid = soundLoad(sound);
    this->sound = soundPlay(sid);
    soundRelease(sid);

    if (!this->then) {
        logErr("InProgressSound", "invalid 'then'");
    }
}

void
InProgressSound::tick(time_t) noexcept {
    if (over) {
        return;
    }

    if (!playingSoundIsPlaying(sound)) {
        playingSoundRelease(sound);
        over = true;
        then();
    }
}

InProgressTimer::InProgressTimer(time_t duration, ThenFn then) noexcept
        : duration(duration),
          passed(0),
          then(static_cast<ThenFn&&>(then)) {
    if (!this->then) {
        logErr("InProgressTimer", "invalid 'then'");
    }
}

InProgressTimer::InProgressTimer(time_t duration,
                                 ProgressFn progress,
                                 ThenFn then) noexcept
        : duration(duration),
          passed(0),
          progress(static_cast<ProgressFn&&>(progress)),
          then(static_cast<ThenFn&&>(then)) {
    if (!this->progress) {
        logErr("InProgressTimer", "invalid 'progress'");
    }
    // then can be empty
}

void
InProgressTimer::tick(time_t dt) noexcept {
    if (over) {
        return;
    }

    passed += dt;

    if (passed < duration) {
        if (progress) {
            // Range is [0.0, 1.0)
            progress((float)passed / (float)duration);
        }
    }
    else {
        over = true;
        if (then) {
            then();
        }
    }
}
