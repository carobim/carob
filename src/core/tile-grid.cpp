/***************************************
** Tsunagari Tile Engine              **
** tile-grid.cpp                      **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2021 Paul Merrill   **
***************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#include "core/tile-grid.h"

#include "core/log.h"
#include "util/assert.h"
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

TileGrid::TileGrid() noexcept
        : dim({0, 0, 0}),
          tileDim({0, 0}),
          loopX(false),
          loopY(false) { }

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
    return vicoord{phys.x, phys.y, indexDepth(phys.z)};
}

fvec3
TileGrid::phys2virt_r(ivec3 phys) noexcept {
    return fvec3{static_cast<float>(phys.x * tileDim.x),
                 static_cast<float>(phys.y * tileDim.y),
                 indexDepth(phys.z)};
}

ivec3
TileGrid::virt2phys(vicoord virt) noexcept {
    return ivec3{static_cast<int>(virt.x),
                 static_cast<int>(virt.y),
                 depthIndex(virt.z)};
}

ivec3
TileGrid::virt2phys(fvec3 virt) noexcept {
    return ivec3{static_cast<int>(virt.x) / tileDim.x,
                 static_cast<int>(virt.y) / tileDim.y,
                 depthIndex(virt.z)};
}

fvec3
TileGrid::virt2virt(vicoord virt) noexcept {
    return fvec3{static_cast<float>(virt.x * tileDim.x),
                 static_cast<float>(virt.y * tileDim.y),
                 virt.z};
}

vicoord
TileGrid::virt2virt(fvec3 virt) noexcept {
    return vicoord{static_cast<int>(virt.x) / tileDim.x,
                   static_cast<int>(virt.y) / tileDim.y,
                   virt.z};
}

int
TileGrid::depthIndex(float depth) noexcept {
    assert_(depth2idx.contains(depth) && "Attempt to access invalid layer");
    return depth2idx[depth];
}

float
TileGrid::indexDepth(int idx) noexcept {
    assert_(0 <= idx && idx <= dim.z);
    return idx2depth[(size_t)idx];
}

ivec3
TileGrid::moveDest(ivec3 from, ivec2 facing) noexcept {
    ivec3 dest = from + ivec3{facing.x, facing.y, 0};

    float* layermod = layermodAt(from, facing);
    if (layermod) {
        dest = virt2phys(vicoord{dest.x, dest.y, *layermod});
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
