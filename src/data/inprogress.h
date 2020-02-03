/***************************************
** Tsunagari Tile Engine              **
** inprogress.h                       **
** Copyright 2014      Michael Reiley **
** Copyright 2014-2020 Paul Merrill   **
***************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifndef SRC_DATA_INPROGRESS_H_
#define SRC_DATA_INPROGRESS_H_

#include "util/int.h"
#include "util/noexcept.h"

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
    virtual ~InProgress() = default;

    virtual void
    tick(time_t dt) noexcept = 0;
    bool
    isOver() noexcept;

 protected:
    InProgress() noexcept = default;

    bool over = false;

 private:
    InProgress(const InProgress&) = delete;
    InProgress&
    operator=(const InProgress&) = delete;
};

#endif  // SRC_DATA_INPROGRESS_H_
