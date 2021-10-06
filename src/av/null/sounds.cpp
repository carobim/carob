#include "core/sounds.h"

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

SoundID
soundLoad(StringView path) noexcept {
    return mark;
}
void
soundsPrune(Time latestPermissibleUse) noexcept { }

PlayingSoundID
soundPlay(SoundID id) noexcept {
    return mark;
}
void
soundRelease(SoundID id) noexcept { }

bool
playingSoundIsPlaying(PlayingSoundID id) noexcept {
    return false;
}
void
playingSoundStop(PlayingSoundID id) noexcept { }
void
playingSoundVolume(PlayingSoundID id, float volume) noexcept { }
void
playingSoundSpeed(PlayingSoundID id, float speed) noexcept { }
void
playingSoundRelease(PlayingSoundID id) noexcept { }
