#include "core/tile.h"

#include "util/assert.h"
#include "util/compiler.h"

/*
 * TILESET
 */
U32
TileSet::at(Size x, Size y) const noexcept {
    Size i = y * width + x;
    assert_(i < width * height);
    assert_(i < static_cast<Size>(INT32_MAX));
    return firstGid + static_cast<int>(i);
}
