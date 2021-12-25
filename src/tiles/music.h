#ifndef SRC_CORE_MUSIC_H_
#define SRC_CORE_MUSIC_H_

#include "util/compiler.h"
#include "util/string-view.h"

/**
 * State manager for currently playing music. Continuously controls which music
 * will play. Immediately upon entering an Area, the currently playing music is
 * stopped and the new music started and will play in a loop.
 *
 * When switching to a new Area with the same music as the previous Area, the
 * music is left alone, if possible.
 *
 * When a new music is played, the pause state of the previous music is
 * dropped.
 */

//! If the music path has changed, start playing it.
void
musicPlay(StringView path) noexcept;

//! Stop playing music. To begin again, set a new intro or loop.
void
musicStop() noexcept;

//! Pause playback of music.
void
musicPause() noexcept;
//! Resume playback of music.
void
musicResume() noexcept;

//! Free music not recently played.
void
musicGarbageCollect() noexcept;

#endif  // SRC_CORE_MUSIC_H_
