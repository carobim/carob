#ifndef SRC_TILES_ENTITY_H_
#define SRC_TILES_ENTITY_H_

#include "tiles/animation.h"
#include "tiles/images.h"
#include "tiles/vec.h"
#include "util/compiler.h"
#include "util/function.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

class Animation;
class Area;
struct DisplayList;

enum SetPhaseResult { PHASE_NOTFOUND, PHASE_NOTCHANGED, PHASE_CHANGED };

// An Entity represents one 'thing' that will be rendered to the screen.
//
// An Entity might be a dynamic game object such as a monster, NPC, or
// item.  Entity can handle animated images that cycle through their
// frames over time. It also has the capacity to switch between a couple
// different images on demand.
//
// For example, you might have a Entity for a player character with
// animated models for walking in each possible movement direction (up,
// down, left, right) along with static standing-still images for each
// direction.
class Entity {
 public:
    Entity() noexcept;
    virtual ~Entity() noexcept;

    // Entity initializer
    virtual bool
    init(StringView descriptor, StringView initialPhase) noexcept;

    // Entity destroyer.
    virtual void
    destroy() noexcept;

    void
    draw(DisplayList* display) noexcept;
    bool
    needsRedraw(icube& visiblePixels) noexcept;
    bool
    isDead() noexcept;

    virtual void
    tick(Time dt) noexcept;
    virtual void
    turn() noexcept;

    // Normalize each of the X-Y axes into [-1, 0, or 1] and saves value
    // to 'facing'.
    void
    setFacing(ivec2 facing) noexcept;

    StringView
    getFacing() noexcept;

    // Change the graphic. Returns true if it was changed to something
    // different.
    bool
    setPhase(StringView name) noexcept;

    ivec2
    getImageSize() noexcept;

    void
    setAnimationStanding() noexcept;
    void
    setAnimationMoving() noexcept;


    // The offset from the upper-left of the Area to the upper-left of the
    // Tile the Entity is standing on.
    fvec3
    getPixelCoord() noexcept;


    // Gets the Entity's current Area.
    Area*
    getArea() noexcept;

    // Specifies the Area object this entity will ask when looking for
    // nearby Tiles. Doesn't change x,y,z position.
    virtual void
    setArea(Area* area) noexcept;


    // Gets speed in pixels per second.
    float
    getSpeedInPixels() noexcept;
    // Gets speed in tiles per second.
    float
    getSpeedInTiles() noexcept;


    virtual void
    setFrozen(bool b) noexcept;


    struct OnTickFn {
        void (*fn)(void* data, Time);
        void* data;
    };
    typedef Function OnTurnFn;

    void
    attach(OnTickFn fn) noexcept;
    void
    attach(OnTurnFn fn) noexcept;

    // Script hooks.
    // ScriptRef tickScript, turnScript, tileEntryScript,
    //            tileExitScript;


 protected:
    // Precalculate various drawing measurements.
    void
    calcDraw() noexcept;

    // Gets a string describing a direction.
    StringView
    directionStr(ivec2 facing) noexcept;

    enum SetPhaseResult
    _setPhase(StringView name) noexcept;

    void
    setDestinationCoordinate(fvec3 destCoord) noexcept;

    void
    moveTowardDestination(Time dt) noexcept;

    // arrived() is called when an Entity arrives at its destination.  If
    // it is ordered to begin moving again from within arrived(), then the
    // Entity’s graphics will appear as if it never stopped moving.
    virtual void
    arrived() noexcept;

 public:
    // Set to true if the Entity was destroyed this tick.
    bool dead;

    // Set to true if the Entity wants the screen to be redrawn.
    bool redraw;

    // Pointer to Area this Entity is located on.
    Area* area;
    // Real x,y position: hold partial pixel transversal
    fvec3 r;
    // Drawing offset to center entity on tile.
    fvec3 doff;

    String descriptor;

    bool frozen;

    float tilesPerSecond;
    float pixelsPerSecond;

    // True if currently moving to a new coordinate in an Area.
    bool moving;

    fvec3 destCoord;
    float angleToDest;

    ivec2 imgsz;
    Animation* phase;
    String phaseName;
    ivec2 facing;

    Animation phaseStance;
    Animation phaseDown;
    Animation phaseLeft;
    Animation phaseUp;
    Animation phaseRight;
    Animation phaseMovingUp;
    Animation phaseMovingRight;
    Animation phaseMovingDown;
    Animation phaseMovingLeft;

    //  sounds["step"] = "sounds/player_step.oga"
    String soundPathStep;

    Vector<OnTickFn> onTickFns;
    Vector<OnTurnFn> onTurnFns;
};

#endif  // SRC_TILES_ENTITY_H_
