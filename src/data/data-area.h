#ifndef SRC_DATA_DATA_AREA_H_
#define SRC_DATA_DATA_AREA_H_

#include "data/action.h"
#include "tiles/vec.h"
#include "util/compiler.h"
#include "util/hashvector.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/vector.h"

class Area;
class Entity;

//! Play a sound with a 3% speed variation applied to it.
void
playSoundEffect(StringView sound) noexcept;

class DataArea {
 public:
    DataArea() noexcept { }
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

    void
    add(struct Action action) noexcept;

    // For engine
    void
    tick(Time dt) noexcept;
    void
    turn() noexcept;

    HashVector<void (*)(DataArea*, Entity* triggeredBy, ivec3 tile) noexcept>
            scripts;

 private:
    DataArea(const DataArea&);
    DataArea&
    operator=(const DataArea&);

    Vector<struct Action> actions;
};

#endif  // SRC_DATA_DATA_AREA_H_
