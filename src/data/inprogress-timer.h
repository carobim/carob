#ifndef SRC_DATA_INPROGRESS_TIMER_H_
#define SRC_DATA_INPROGRESS_TIMER_H_

#include "data/inprogress.h"
#include "util/compiler.h"
#include "util/function.h"
#include "util/int.h"

/**
 * InProgressTimer can call a function every tick during its timer as well as
 * once its timer finishes.
 *
 * Its progress function gets called every tick with a number from 0.0 to 1.0
 * signifying the percentage of the duration that has passed.
 *
 * On the first tick where the timer has expired, progress is not called, but
 * the then function is.
 */
class InProgressTimer : public InProgress {
 public:
    typedef Function<void(float)> ProgressFn;
    typedef Function<void()> ThenFn;

    InProgressTimer(time_t duration, ThenFn then) noexcept;
    InProgressTimer(time_t duration, ProgressFn progress, ThenFn then) noexcept;

    void
    tick(time_t dt) noexcept;

 private:
    time_t duration, passed;
    ProgressFn progress;
    ThenFn then;
};

#endif  // SRC_DATA_INPROGRESS_TIMER_H_
