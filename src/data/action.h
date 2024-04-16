#ifndef SRC_DATA_ACTION_H_
#define SRC_DATA_ACTION_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

/**
 * InProgress objects contain logic that is to be evaluated over time from
 * within a DataArea. They are run only when an Area is in focus.
 *
 * When a game script wants to run some logic during an event (e.g., during a
 * timer), or after an event happens (e.g., after a sound plays), an InProgress
 * object can handle the timing for it.
 *
 * InProgress objects are generally only indirectly used through DataArea's
 * public methods, such as:
 *   - playSoundAndThen(string sound, fn then);
 *   - timerProgressAndThen(I32 duration, fn progress, fn then);
 *
 * These functions are essentially wrappers around the constructors of
 * InProgress subclasses, but they tie the constructed InProgress to that
 * particular DataArea, meaning they will be run when that Area is in focus.
 *
 * InProgress objects are invoked right before a DataArea's onTick().
 */

class DataArea;

enum ActionStatus {
    AS_CONTINUE,
    AS_END
};

struct Action {
    enum ActionStatus (*tick)(DataArea*, void* data, Time dt) noexcept;
    void* data;
    void (*free)(void* data) noexcept;
    struct Action* next;
};

// An action that calls a function once.
struct Action
makeUnitAction(void (*fn)(DataArea*, void* data) noexcept, void* data,
               void (*free)(void* data) noexcept) noexcept;

// An action that waits a set amount of time.
struct Action
makeDelayAction(Time duration) noexcept;

// An action that plays a sound and waits for it to finish.
struct Action
makeSoundAction(StringView sound) noexcept;

// An action that calls a function each tick through a set duration.
struct Action
makeTimerAction(Time duration,
                void (*tick)(DataArea*, void* data, float progress) noexcept,
                void* data, void (*free)(void* data) noexcept) noexcept;

#endif  // SRC_DATA_ACTION_H_
