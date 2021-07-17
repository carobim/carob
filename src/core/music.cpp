#include "core/music.h"

#include "core/music-worker.h"
#include "util/compiler.h"
//#include "util/jobs.h"

void
musicPlay(StringView name) noexcept {
    // String name_ = name;
    // JobsEnqueue([name_]() { musicWorkerPlay(name_); });
    musicWorkerPlay(name);
}

void
musicStop() noexcept {
    // JobsEnqueue([]() { musicWorkerStop(); });
    musicWorkerStop();
}

void
musicPause() noexcept {
    // JobsEnqueue([]() { musicWorkerPause(); });
    musicWorkerPause();
}

void
musicResume() noexcept {
    // JobsEnqueue([]() { musicWorkerResume(); });
    musicWorkerResume();
}

void
musicGarbageCollect() noexcept {
    // JobsEnqueue([]() { musicWorkerGarbageCollect(); });
    musicWorkerGarbageCollect();
}
