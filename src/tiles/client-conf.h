#ifndef SRC_TILES_CLIENT_CONF_H_
#define SRC_TILES_CLIENT_CONF_H_

#include "tiles/log.h"
#include "tiles/vec.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

//! Engine-wide user-configurable values.

//! Game Movement Mode
enum MoveMode { TURN, TILE, NOTILE };

extern MoveMode confMoveMode;
extern ivec2 confWindowSize;
extern bool confFullscreen;

void
confParse(StringView filename) noexcept;

#endif  // SRC_TILES_CLIENT_CONF_H_
