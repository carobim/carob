#ifndef SRC_CORE_TILE_H_
#define SRC_CORE_TILE_H_

#include "util/compiler.h"
#include "util/int.h"

struct TileSet {
    int firstGid;
    Size width;
    Size height;

    int
    at(Size x, Size y) noexcept;
};

#endif  // SRC_CORE_TILE_H_
