#ifndef SRC_CORE_COOLDOWN_H_
#define SRC_CORE_COOLDOWN_H_

#include "util/compiler.h"
#include "util/int.h"

/**
 * Cooldown is a timer that repeatedly expires after a specified number of
 * milliseconds.  A cooldown may hold multiple expirations, and will keep track
 * of all until told to wrap() each one.
 */
class Cooldown {
 public:
    Cooldown() noexcept;
    Cooldown(Time duration) noexcept;

    /**
     * Sets a new duration for the Cooldown and resets any current
     * expirations.
     */
    void
    setDuration(Time duration) noexcept;

    /**
     * Let the Cooldown know that the specified number of milliseconds have
     * passed.
     */
    void
    advance(Time dt) noexcept;

    /**
     * Whether enough time has passed that the Cooldown has expired at
     * least once over.
     */
    bool
    hasExpired() noexcept;

    /**
     * Begin the next session, rolling over any time passed since the
     * previous expiration.
     *
     * Advances only one expiration, even if enough time has passed that
     * the cooldown has expired multiple times.  In that case, hasExpired()
     * will still return true and you may wrap again.
     */
    void
    wrapOnce() noexcept;

    void
    wrapAll() noexcept;

 private:
    Time duration, passed;
};

#endif  // SRC_CORE_COOLDOWN_H_
