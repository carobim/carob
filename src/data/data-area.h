#ifndef SRC_DATA_DATA_AREA_H_
#define SRC_DATA_DATA_AREA_H_

#include "core/vec.h"
#include "data/inprogress.h"
#include "util/compiler.h"
#include "util/hashtable.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/vector.h"

class Area;
class Entity;

class DataArea {
 public:
    typedef void (DataArea::*TileScript)(Entity& triggeredBy, ivec3 tile);

 public:
    DataArea() noexcept : area(0) { }
    virtual ~DataArea() noexcept { }

    Area* area;  // borrowed reference

    virtual void
    onLoad() noexcept;
    virtual void
    onFocus() noexcept;
    virtual void
    onTick(Time dt) noexcept;
    virtual void
    onTurn() noexcept;

    // For scripts

    //! Play a sound with a 3% speed variation applied to it.
    void
    playSoundEffect(StringView sound) noexcept;

    void
    add(InProgress* inProgress) noexcept;

    // For engine
    void
    tick(Time dt) noexcept;
    void
    turn() noexcept;

    Hashmap<StringView, TileScript> scripts;

 private:
    DataArea(const DataArea&);
    DataArea&
    operator=(const DataArea&);

    Vector<InProgress*> inProgresses;
};

#endif  // SRC_DATA_DATA_AREA_H_
