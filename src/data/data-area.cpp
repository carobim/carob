#include "data/data-area.h"

#include "core/algorithm.h"
#include "core/sounds.h"
#include "util/compiler.h"
#include "util/random.h"

static bool
isOver(InProgress* ip) noexcept {
    return ip->isOver();
}

void
DataArea::onLoad() noexcept { }

void
DataArea::onFocus() noexcept { }

void
DataArea::onTick(Time) noexcept { }

void
DataArea::onTurn() noexcept { }

void
DataArea::tick(Time dt) noexcept {
    // Only iterate over inProgresses that existed at the time of the
    // beginning of the loop.  Also, iterate by index instead of by
    // iterator because iterators are invalidated if the vector is
    // pushed_back.
    for (InProgress** inProgress = inProgresses.begin();
         inProgresses.end();
         inProgress++) {
        (*inProgress)->tick(dt);
    }
    erase_if(inProgresses, isOver);
    onTick(dt);
}

void
DataArea::turn() noexcept {
    onTurn();
}

void
DataArea::playSoundEffect(StringView sound) noexcept {
    SoundID sid = soundLoad(sound);
    PlayingSoundID psid = soundPlay(sid);
    playingSoundSpeed(psid, 1.0f + randFloat(-0.03f, 0.03f));
    playingSoundRelease(psid);
    soundRelease(sid);
}

void
DataArea::add(InProgress* inProgress) noexcept {
    inProgresses.push(inProgress);
}
