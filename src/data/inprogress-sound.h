#ifndef SRC_DATA_INPROGRESS_SOUND_H_
#define SRC_DATA_INPROGRESS_SOUND_H_

#include "core/sounds.h"
#include "data/inprogress.h"
#include "util/compiler.h"
#include "util/function.h"
#include "util/int.h"
#include "util/string-view.h"

/**
 * InProgressSound can call a function after a sound finishes playing.
 *
 * The then function is called on the first tick where the sound is not
 * playing.
 */
class InProgressSound : public InProgress {
 public:
    typedef Function ThenFn;

    InProgressSound(StringView sound, ThenFn onThen) noexcept;

    void
    tick(Time dt) noexcept;

 private:
    PlayingSoundID sound;
    ThenFn onThen;
};

#endif  // SRC_DATA_INPROGRESS_SOUND_H_
