#include "core/cooldown.h"

#include "core/log.h"
#include "util/compiler.h"

Cooldown::Cooldown() noexcept : duration(0), passed(0) { }

Cooldown::Cooldown(time_t duration) noexcept : duration(duration), passed(0) { }

void
Cooldown::setDuration(time_t duration) noexcept {
    this->duration = duration;
    passed = 0;
}

void
Cooldown::advance(time_t dt) noexcept {
    passed += dt;
}

bool
Cooldown::hasExpired() noexcept {
    return passed > duration;
}

void
Cooldown::wrapOnce() noexcept {
    if (hasExpired()) {
        passed -= duration;
    }
    else {
        logErr("Cooldown", "wrapping when not expired");
    }
}

void
Cooldown::wrapAll() noexcept {
    if (hasExpired()) {
        passed %= duration;
    }
    else {
        logErr("Cooldown", "wrapping when not expired");
    }
}
