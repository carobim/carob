#include "tiles/cooldown.h"

#include "tiles/log.h"
#include "util/compiler.h"

Cooldown::Cooldown() noexcept : duration(0), passed(0) { }

Cooldown::Cooldown(Time duration) noexcept : duration(duration), passed(0) { }

void
Cooldown::setDuration(Time duration) noexcept {
    this->duration = duration;
    passed = 0;
}

void
Cooldown::advance(Time dt) noexcept {
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
