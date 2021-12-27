#include "data/data-area.h"

#include "data/action.h"
#include "tiles/sounds.h"
#include "util/algorithm.h"
#include "util/compiler.h"
#include "util/random.h"

void
playSoundEffect(StringView sound) noexcept {
    SoundID sid = soundLoad(sound);
    PlayingSoundID psid = soundPlay(sid);
    playingSoundSpeed(psid, 1.0f + randFloat(-0.03f, 0.03f));
    playingSoundRelease(psid);
    soundRelease(sid);
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
    onTick(dt);

    // Only iterate over Actions that existed at the time of the beginning of
    // the loop.
    Size size = actions.size;

    Size i = 0;
    while (i < size) {
        Action* action = actions.data + i;
        enum ActionStatus status = action->tick(this, action->data, dt);
        if (status == AS_CONTINUE) {
            i++;
        }
        else if (status == AS_END) {
            action->free(action->data);
            actions.eraseUnordered(i);
            size--;
        }
    }
}

void
DataArea::turn() noexcept {
    onTurn();
}

void
DataArea::add(struct Action action) noexcept {
    actions.push(action);
}
