/***************************************
** Tsunagari Tile Engine              **
** tile-grid.h                        **
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

#ifndef SRC_CORE_TILE_GRID_H_
#define SRC_CORE_TILE_GRID_H_

#include "core/vec.h"
#include "data/data-area.h"
#include "util/hashtable.h"
#include "util/string.h"
#include "util/vector.h"

class Entity;

// List of possible flags that can be attached to a tile.
//
// Flags are attached to tiles and denote special behavior for
// the tile they are bound to.
//
// see AreaTMX::splitTileFlags().

// Neither the player nor NPCs can walk here.
#define TILE_NOWALK (unsigned)(0x001)

// The player cannot walk here. NPCs can, though.
#define TILE_NOWALK_PLAYER ((unsigned)(0x002))

// NPCs cannot walk here. The player can, though.
#define TILE_NOWALK_NPC ((unsigned)(0x004))

// This Tile is an Exit. Please take appropriate action when entering this
// Tile, usually by transferring to another Area.
//
// This flag is not carried by actual Tiles, but can instead be flipped in an
// Entity's "exempt" flag which will be read elsewhere in the engine.
#define TILE_NOWALK_EXIT ((unsigned)(0x008))

// This Tile is at the edge of an Area. If you step here, please handle it
// appropriately.
//
// (Usually if one moves off a map bound, one will either transfer to another
// Area, or will be destroyed.)
//
// This flag is not carried by actual Tiles, but can instead be flipped in an
// Entity's "exempt" flag which will be read elsewhere in the engine.
#define TILE_NOWALK_AREA_BOUND ((unsigned)(0x016))


// Types of exits.
enum ExitDirection {
    // An Exit that is taken upon arriving at the Tile.
    EXIT_NORMAL,
    // An Exit that is taken when leaving in the upwards direction from a Tile.
    EXIT_UP,
    // An Exit that is taken when leaving in the downwards direction from a
    // Tile.
    EXIT_DOWN,
    // An Exit that is taken when leaving to the left from a Tile.
    EXIT_LEFT,
    // An Exit that is taken when leaving to the right from a Tile.
    EXIT_RIGHT,
    EXITS_LENGTH
};

// Tiles with an exit trigger attached can teleport the player to a new area in
// the World. The Exit struct contains the destination area and coordinates.
struct Exit {
    String area;
    vicoord coords;
};

typedef void (*TileScript)(Entity& triggeredBy, ivec3 tile);

struct EmptyFloat {
    static CONSTEXPR11 float
    value() {
        return FLT_MIN;
    }
};

struct EmptyIcoord {
    static CONSTEXPR11 ivec3
    value() {
        return IVEC3_MIN;
    }
};

class TileGrid {
 public:
    TileGrid() noexcept;

    int
    getTileType(ivec3 phys) noexcept;
    int
    getTileType(vicoord virt) noexcept;

    void
    setTileType(vicoord virt, int type) noexcept;

    //! Returns true if a Tile exists at the specified coordinate.
    bool
    inBounds(ivec3 phys) noexcept;
    bool
    inBounds(vicoord virt) noexcept;
    bool
    inBounds(fvec3 virt) noexcept;

 public:
    // Convert between virtual and physical map coordinates. Physical
    // coordinates are the physical indexes into the Tile vector. Layer depth
    // is represented by an arbirarily chosen integer in the physical system.
    // Virtual coordinates include the correct floating-point depth.
    vicoord
    phys2virt_vi(ivec3 phys) noexcept;
    fvec3
    phys2virt_r(ivec3 phys) noexcept;
    ivec3
    virt2phys(vicoord virt) noexcept;
    ivec3
    virt2phys(fvec3 virt) noexcept;
    fvec3
    virt2virt(vicoord virt) noexcept;
    vicoord
    virt2virt(fvec3 virt) noexcept;

    // Convert between virtual and physical map depths.
    int
    depthIndex(float depth) noexcept;
    float
    indexDepth(int idx) noexcept;

    // Gets the correct destination for an Entity wanting to move off of this
    // tile in <code>facing</code> direction.
    //
    // This call is necessary to handle layermod.
    //
    // @param area    the area containing this Tile
    // @param here    area-space coordinate for this Tile
    // @param facing  facing vector
    ivec3
    moveDest(ivec3 from, ivec2 facing) noexcept;

    // nullptr means not found
    Exit*
    exitAt(ivec3 from, ivec2 facing) noexcept;
    // nullptr means not found
    float*
    layermodAt(ivec3 from, ivec2 facing) noexcept;

 public:
    // 3-dimensional array of the tiles that make up the grid.
    Vector<int> graphics;

    enum LayerType {
        TILE_LAYER,
        OBJECT_LAYER
    };
    Vector<LayerType> layerTypes;

    // 3-dimensional length of map.
    ivec3 dim;

    // Pixel size for each tile in area. All tiles in a TileGrid must be the
    // same size.
    ivec2 tileDim;

    // Maps virtual float-point depths to an index in our map array.
    Hashmap<float, int, EmptyFloat> depth2idx;

    // Maps an index in our map array to a virtual float-point depth.
    Vector<float> idx2depth;

    bool loopX;
    bool loopY;

    Hashmap<ivec3, bool, EmptyIcoord> occupied;

    enum ScriptType {
        SCRIPT_TYPE_ENTER,
        SCRIPT_TYPE_LEAVE,
        SCRIPT_TYPE_USE,
        SCRIPT_TYPE_LAST
    };

    Hashmap<ivec3, DataArea::TileScript, EmptyIcoord> scripts[SCRIPT_TYPE_LAST];

    Hashmap<ivec3, unsigned, EmptyIcoord> flags;

    Hashmap<ivec3, Exit, EmptyIcoord> exits[EXITS_LENGTH];
    Hashmap<ivec3, float, EmptyIcoord> layermods[EXITS_LENGTH];

 private:
    TileGrid(const TileGrid&);
    void
    operator=(const TileGrid&);
};

#endif  // SRC_CORE_TILE_GRID_H_
