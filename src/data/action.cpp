#include "data/action.h"

#include "tiles/sounds.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"

//
// Unit action
//

struct UnitParams {
    void (*fn)(DataArea*, void* data) noexcept;
    void* data;
    void (*free)(void* data) noexcept;
};

static ActionStatus
callUnit(DataArea* area, void* data, Time) noexcept {
    fromCast(struct UnitParams, params, data);
    params->fn(area, params->data);
    return AS_END;
}

static void
freeUnit(void* data) noexcept {
    fromCast(struct UnitParams, params, data);
    params->free(params->data);
    free(params);
}

struct Action
makeUnitAction(void (*fn)(DataArea* area, void* data) noexcept, void* data,
               void (*free)(void* data) noexcept) noexcept {
    new3(struct UnitParams, params, fn, fn, data, data, free, free);

    struct Action action;
    action.tick = callUnit;
    action.data = params;
    action.free = freeUnit;
    action.next = 0;

    return action;
}

//
// Delay action
//

struct DelayParams {
    Time duration;
    Time passed;
};

static enum ActionStatus
delayTick(DataArea*, void* data, Time dt) noexcept {
    fromCast(struct DelayParams, params, data);

    params->passed += dt;

    return params->passed >= params->duration ? AS_END : AS_CONTINUE;
}

// An action that waits a set amount of time.
struct Action
makeDelayAction(Time duration) noexcept {
    new2(struct DelayParams, params, duration, duration, passed, 0);

    struct Action action;
    action.tick = delayTick;
    action.data = params;
    action.free = free;
    action.next = 0;

    return action;
}

//
// Sound action
//

static enum ActionStatus
soundTick(DataArea*, void* data, Time) noexcept {
    PlayingSoundID psid;
    psid = reinterpret_cast<Size>(data);

    if (!playingSoundIsPlaying(psid)) {
        playingSoundRelease(psid);
        return AS_END;
    }
    return AS_CONTINUE;
}

struct Action
makeSoundAction(StringView sound) noexcept {
    SoundID sid = soundLoad(sound);
    PlayingSoundID psid = soundPlay(sid);
    soundRelease(sid);

    struct Action action;
    action.tick = soundTick;
    action.data = reinterpret_cast<void*>(*psid);
    action.free = 0;
    action.next = 0;

    return action;
}

//
// Timer action
//

struct TimerData {
    Time duration;
    Time passed;
    void (*tick)(DataArea*, void* userData, float progress) noexcept;
    void* userData;
    void (*free)(void* userData) noexcept;
};

static enum ActionStatus
timerTick(DataArea* area, void* data_, Time dt) noexcept {
    fromCast(struct TimerData, data, data_);

    data->passed += dt;

    float progress = static_cast<float>(data->passed) /
                     static_cast<float>(data->duration);
    if (progress > 1.0f) {
        progress = 1.0f;
    }

    data->tick(area, data->userData, progress);

    return progress == 1.0f ? AS_END : AS_CONTINUE;
}

static void
timerFree(void* data_) noexcept {
    fromCast(struct TimerData, data, data_);
    if (data->free) {
        data->free(data->userData);
    }
    free(data);
}

struct Action
makeTimerAction(Time duration,
                void (*tick)(DataArea*, void* userData,
                             float progress) noexcept,
                void* userData,
                void (*free)(void* userData) noexcept) noexcept {
    new0(struct TimerData, data);
    data->duration = duration;
    data->passed = 0;
    data->tick = tick;
    data->userData = userData;
    data->free = free;

    struct Action action;
    action.tick = timerTick;
    action.data = data;
    action.free = timerFree;
    action.next = 0;

    return action;
}
