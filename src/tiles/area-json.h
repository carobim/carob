#ifndef SRC_TILES_AREA_JSON_H_
#define SRC_TILES_AREA_JSON_H_

#include "util/compiler.h"
#include "util/string-view.h"

class Area;
class Player;

Area*
makeAreaFromJSON(Player* player, StringView filename) noexcept;

#endif  // SRC_TILES_AREA_JSON_H_
