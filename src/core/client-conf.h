#ifndef SRC_CORE_CLIENT_CONF_H_
#define SRC_CORE_CLIENT_CONF_H_

#include "core/log.h"
#include "core/vec.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

//! Engine-wide user-confurable values.

//! Game Movement Mode
enum MoveMode { TURN, TILE, NOTILE };

extern LogVerbosity confVerbosity;
extern MoveMode confMoveMode;
extern ivec2 confWindowSize;
extern bool confFullscreen;
extern int confMusicVolume;
extern int confSoundVolume;
extern Time confCacheTTL;
extern int confPersistInit;
extern int confPersistCons;

bool
confParse(StringView filename) noexcept;

#endif  // SRC_CORE_CLIENT_CONF_H_
