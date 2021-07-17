#ifndef SRC_DATA_INPROGRESS_H_
#define SRC_DATA_INPROGRESS_H_

#include "util/compiler.h"
#include "util/int.h"

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
 *   - timerProgressAndThen(int duration, fn progress, fn then);
 *
 * These functions are essentially wrappers around the constructors of
 * InProgress subclasses, but they tie the constructed InProgress to that
 * particular DataArea, meaning they will be run when that Area is in focus.
 *
 * InProgress objects are invoked right before a DataArea's onTick().
 */
class InProgress {
 public:
    virtual ~InProgress() noexcept;

    virtual void
    tick(time_t dt) noexcept = 0;
    bool
    isOver() noexcept;

 protected:
    InProgress() noexcept;

    bool over;

 private:
    InProgress(const InProgress&) = delete;
    InProgress&
    operator=(const InProgress&) = delete;
};

#endif  // SRC_DATA_INPROGRESS_H_
