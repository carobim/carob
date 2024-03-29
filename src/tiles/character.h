#ifndef SRC_TILES_CHARACTER_H_
#define SRC_TILES_CHARACTER_H_

#include "tiles/entity.h"
#include "tiles/vec.h"
#include "util/compiler.h"
#include "util/int.h"

class Area;
struct Exit;

class Character : public Entity {
 public:
    Character() noexcept;
    virtual ~Character() noexcept { }

    virtual void
    tick(Time dt) noexcept;
    virtual void
    turn() noexcept;

    virtual void
    destroy() noexcept;

    //! Retrieve position within Area.
    ivec3
    getTileCoords_i() noexcept;
    vicoord
    getTileCoords_vi() noexcept;

    //! Set location within Area.
    void
    setTileCoords(I32 x, I32 y) noexcept;
    void
    setTileCoords(ivec3 phys) noexcept;
    void
    setTileCoords(vicoord virt) noexcept;
    void
    setTileCoords(fvec3 virt) noexcept;

    void
    setArea(Area* area, vicoord position) noexcept;

    //! Initiate a movement within the Area.
    virtual void
    moveByTile(ivec2 delta) noexcept;

 protected:
    //! Indicates which coordinate we will move into if we proceed in
    //! direction specified.
    ivec3
    moveDest(ivec2 facing) noexcept;

    //! Returns true if we can move in the desired direction.
    bool
    canMove(ivec3 dest) noexcept;

    bool
    nowalked(ivec3 phys) noexcept;

    void
    arrived() noexcept;

    void
    leaveTile() noexcept;
    void
    leaveTile(ivec3 phys) noexcept;
    void
    enterTile() noexcept;
    void
    enterTile(ivec3 phys) noexcept;

    void
    runTileExitScript() noexcept;
    void
    runTileEntryScript() noexcept;

 protected:
    U32 nowalkFlags;
    U32 nowalkExempt;

    fvec3 fromCoord;
    Exit* destExit;
};

#endif  // SRC_TILES_CHARACTER_H_
