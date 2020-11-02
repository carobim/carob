/***************************************
** Tsunagari Tile Engine              **
** sounds.h                           **
** Copyright 2011-2014 Michael Reiley **
** Copyright 2011-2020 Paul Merrill   **
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

#ifndef SRC_CORE_SOUNDS_H_
#define SRC_CORE_SOUNDS_H_

#include "util/int.h"
#include "util/markable.h"
#include "util/string-view.h"

typedef Markable<int, -1> SoundID;
typedef Markable<int, -1> PlayingSoundID;

//
// Sounds
//

SoundID
soundLoad(StringView path) noexcept;

// Free destroyed Sounds that were not recently played.
void
soundsPrune(time_t latestPermissibleUse) noexcept;

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

#endif  // SRC_CORE_SOUNDS_H_
