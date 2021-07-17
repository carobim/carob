#include "core/tile.h"

#include "util/assert.h"
#include "util/compiler.h"

/*
 * TILESET
 */
int
TileSet::at(size_t x, size_t y) noexcept {
    size_t i = y * width + x;
    assert_(i < width * height);
    assert_(i < static_cast<size_t>(INT32_MAX));
    return firstGid + static_cast<int>(i);
}
