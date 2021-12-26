#ifndef SRC_TILES_SOUNDS_H_
#define SRC_TILES_SOUNDS_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/markable.h"
#include "util/string-view.h"

typedef Markable<I32, -1> SoundID;
typedef Markable<I32, -1> PlayingSoundID;

//
// Sounds
//

SoundID
soundLoad(StringView path) noexcept;

// Free destroyed Sounds that were not recently played.
void
soundsPrune(Time latestPermissibleUse) noexcept;

//
// Sound
//

PlayingSoundID
soundPlay(SoundID sid) noexcept;

void
soundRelease(SoundID sid) noexcept;

//
// PlayingSound
//

// Whether the sound is currently playing.
bool
playingSoundIsPlaying(PlayingSoundID psid) noexcept;
// Stop playing the sound. SoundInstances cannot resume from stop().
// Create a new one to play again. Calls destroy().
void
playingSoundStop(PlayingSoundID psid) noexcept;

// Between 0.0 (silence) and 1.0 (full).
void
playingSoundVolume(PlayingSoundID psid, float volume) noexcept;
// 1.0 is normal speed.
void
playingSoundSpeed(PlayingSoundID psid, float speed) noexcept;

// Release the resources used by this PlayingSound.
void
playingSoundRelease(PlayingSoundID psid) noexcept;

#endif  // SRC_TILES_SOUNDS_H_
