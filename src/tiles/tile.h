#ifndef SRC_CORE_TILE_H_
#define SRC_CORE_TILE_H_

#include "util/compiler.h"
#include "util/int.h"

struct TileSet {
    U32 firstGid;
    Size width;
    Size height;

    U32
    at(Size x, Size y) const noexcept;
};

#endif  // SRC_CORE_TILE_H_
