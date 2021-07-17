#ifndef SRC_CORE_TILE_H_
#define SRC_CORE_TILE_H_

#include "util/compiler.h"
#include "util/int.h"

struct TileSet {
    int firstGid;
    size_t width;
    size_t height;

    int
    at(size_t x, size_t y) noexcept;
};

#endif  // SRC_CORE_TILE_H_
