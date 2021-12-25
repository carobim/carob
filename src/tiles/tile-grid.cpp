#include "tiles/tile-grid.h"

#include "tiles/log.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/math2.h"
#include "util/string.h"

static int
ivec2_to_dir(ivec2 v) noexcept {
    switch (v.x) {
    case -1:
        return v.y == 0 ? EXIT_LEFT : -1;
    case 0:
        switch (v.y) {
        case -1:
            return EXIT_UP;
        case 0:
            return EXIT_NORMAL;
        case 1:
            return EXIT_DOWN;
        default:
            return -1;
        }
        break;
    case 1:
        return v.y == 0 ? EXIT_RIGHT : -1;
    default:
        return -1;
    }
}

TileGrid::TileGrid() noexcept : loopX(false), loopY(false) {
    dim.x = dim.y = dim.z = 0;
    tileDim.x = tileDim.y = 0;
}

int
TileGrid::getTileType(ivec3 phys) noexcept {
    int idx = (phys.z * dim.y + phys.y) * dim.x + phys.x;
    return graphics[idx];
}

int
TileGrid::getTileType(vicoord virt) noexcept {
    return getTileType(virt2phys(virt));
}

void
TileGrid::setTileType(vicoord virt, int type) noexcept {
    ivec3 phys = virt2phys(virt);

    int idx = (phys.z * dim.y + phys.y) * dim.x + phys.x;
    graphics[idx] = type;
}

bool
TileGrid::inBounds(ivec3 phys) noexcept {
    return (loopX || (0 <= phys.x && phys.x < dim.x)) &&
           (loopY || (0 <= phys.y && phys.y < dim.y)) &&
           (0 <= phys.z && phys.z < dim.z);
}

bool
TileGrid::inBounds(vicoord virt) noexcept {
    return inBounds(virt2phys(virt));
}

bool
TileGrid::inBounds(fvec3 virt) noexcept {
    return inBounds(virt2phys(virt));
}


vicoord
TileGrid::phys2virt_vi(ivec3 phys) noexcept {
    vicoord virt = {phys.x, phys.y, indexDepth(phys.z)};
    return virt;
}

fvec3
TileGrid::phys2virt_r(ivec3 phys) noexcept {
    fvec3 virt = {static_cast<float>(phys.x * tileDim.x),
                  static_cast<float>(phys.y * tileDim.y),
                  indexDepth(phys.z)};
    return virt;
}

ivec3
TileGrid::virt2phys(vicoord virt) noexcept {
    ivec3 phys = {static_cast<int>(virt.x),
                  static_cast<int>(virt.y),
                  depthIndex(virt.z)};
    return phys;
}

ivec3
TileGrid::virt2phys(fvec3 virt) noexcept {
    ivec3 phys = {static_cast<int>(virt.x) / tileDim.x,
                  static_cast<int>(virt.y) / tileDim.y,
                  depthIndex(virt.z)};
    return phys;
}

fvec3
TileGrid::virt2virt(vicoord virt) noexcept {
    fvec3 v = {static_cast<float>(virt.x * tileDim.x),
               static_cast<float>(virt.y * tileDim.y),
               virt.z};
    return v;
}

vicoord
TileGrid::virt2virt(fvec3 virt) noexcept {
    vicoord v = {static_cast<int>(virt.x) / tileDim.x,
                  static_cast<int>(virt.y) / tileDim.y,
                  virt.z};
    return v;
}

int
TileGrid::depthIndex(float depth) noexcept {
    assert_(depth2idx.contains(depth) && "Attempt to access invalid layer");
    return depth2idx[depth];
}

float
TileGrid::indexDepth(int idx) noexcept {
    assert_(0 <= idx && idx <= dim.z);
    return idx2depth[(Size)idx];
}

ivec3
TileGrid::moveDest(ivec3 from, ivec2 facing) noexcept {
    ivec3 dest = from;
    dest.x += facing.x;
    dest.y += facing.y;

    float* layermod = layermodAt(from, facing);
    if (layermod) {
        vicoord virt = {dest.x, dest.y, *layermod};
        dest = virt2phys(virt);
    }
    return dest;
}

Exit*
TileGrid::exitAt(ivec3 from, ivec2 facing) noexcept {
    int idx = ivec2_to_dir(facing);
    return idx == -1 ? 0 : exits[idx].tryAt(from);
}

float*
TileGrid::layermodAt(ivec3 from, ivec2 facing) noexcept {
    int idx = ivec2_to_dir(facing);
    return idx == -1 ? 0 : layermods[idx].tryAt(from);
}
