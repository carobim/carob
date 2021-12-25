#ifndef SRC_TILES_MUSIC_WORKER_H_
#define SRC_TILES_MUSIC_WORKER_H_

#include "util/compiler.h"
#include "util/string-view.h"

void
musicWorkerPlay(StringView path) noexcept;

void
musicWorkerStop() noexcept;

void
musicWorkerPause() noexcept;
void
musicWorkerResume() noexcept;

void
musicWorkerGarbageCollect() noexcept;

#endif  // SRC_TILES_MUSIC_WORKER_H_
