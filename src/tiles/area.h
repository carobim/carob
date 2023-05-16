#ifndef SRC_TILES_AREA_H_
#define SRC_TILES_AREA_H_

#include "tiles/animation.h"
#include "tiles/tile-grid.h"
#include "tiles/tile.h"
#include "tiles/vec.h"
#include "tiles/window.h"
#include "util/compiler.h"
#include "util/hashtable.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

class AreaJSON;
class Character;
class DataArea;
struct DisplayList;
class Entity;
class Overlay;
class Player;

//! An Area represents one map, or screen, in a World.
/*!
    The Area class manages a three-dimensional structure of Tiles and a set
    of Entities.

    The game's Viewport must be "focused" on an Area. Only one Area can be
    focused on at a time.

    The viewport will not scroll past the edge of an Area. (At least as of
    June 2012. :)
*/
class Area {
 public:
    Area() noexcept;

    //! Prepare game state for this Area to be in focus.
    void
    focus() noexcept;

    //! Processes keyboard input, calling the Player object when necessary.
    void
    buttonDown(Key key) noexcept;
    void
    buttonUp(Key key) noexcept;

    //! Renders all visible Tiles and Entities within this Area.
    void
    draw(DisplayList* display) noexcept;

    //! If false, drawing might be skipped. Saves CPU cycles when idle.
    bool
    needsRedraw() noexcept;

    //! Inform the Area that a redraw is needed.
    void
    requestRedraw() noexcept;

    /**
     * Update the game state within this Area as if dt milliseconds had
     * passed since the last call. Updates Entities, runs scripts, and
     * checks for Tile animation updates.
     */
    void
    tick(Time dt) noexcept;

    /**
     * Updates Entities, runs scripts, and checks for Tile animation
     * updates.
     */
    void
    turn() noexcept;

    U32
    getColorOverlay() const noexcept;
    void
    setColorOverlay(U8 a, U8 r, U8 g, U8 b) noexcept;

    TileSet*
    getTileSet(StringView imagePath) noexcept;

    //! Returns a physical cubic range of Tiles that are visible on-screen.
    //! Takes actual map size into account.
    icube
    visibleTiles() noexcept;

    //! Returns true if a Tile exists at the specified coordinate.
    bool
    inBounds(Entity* ent) noexcept;

    // Create an NPC and insert it into the Area. Returns a borrowed ref.
    Character*
    spawnNPC(StringView descriptor_, vicoord coord, StringView phase) noexcept;
    // Create an Overlay and insert it into the Area. Returns a borrowed ref.
    Overlay*
    spawnOverlay(StringView descriptor_,
                 vicoord coord,
                 StringView phase) noexcept;

    DataArea*
    getDataArea() noexcept;

    void
    runScript(TileGrid::ScriptType type,
              ivec3 tile,
              Entity* triggeredBy) noexcept;

 public:
    TileGrid grid;

    bool ok;

 protected:
    //! Calculate frame to show for each type of tile
    void
    drawTiles(DisplayList* display, icube& tiles, I32 z) noexcept;
    void
    drawEntities(DisplayList* display, icube& tiles, I32 z) noexcept;

 protected:
    Hashmap<String, TileSet> tileSets;

    Vector<Animation> tileGraphics;
    Vector<bool> checkedForAnimation;
    Vector<bool> tilesAnimated;

    Vector<Character*> characters;
    Vector<Overlay*> overlays;

    bool beenFocused;
    bool redraw;
    U32 colorOverlayARGB;

    DataArea* dataArea;

    Player* player;

    // The following contain filenames such that they may be loaded lazily.
    String descriptor;

    String name;
    String author;
    String musicPath;

    friend class AreaJSON;
};

#endif  // SRC_TILES_AREA_H_
