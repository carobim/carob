#include "core/overlay.h"

#include "core/area.h"
#include "core/client-conf.h"
#include "util/compiler.h"

void
Overlay::tick(Time dt) noexcept {
    Entity::tick(dt);
    moveTowardDestination(dt);
}

void
Overlay::teleport(vicoord coord) noexcept {
    r = area->grid.virt2virt(coord);
    redraw = true;
}

void
Overlay::drift(ivec2 xy) noexcept {
    ivec2 dest = {static_cast<int>(r.x) + xy.x, static_cast<int>(r.y) + xy.y};
    driftTo(dest);
}

void
Overlay::driftTo(ivec2 xy) noexcept {
    fvec3 destCoord = {static_cast<float>(xy.x), static_cast<float>(xy.y), r.z};
    setDestinationCoordinate(destCoord);

    pickFacingForAngle();
    moving = true;
    setAnimationMoving();

    // Movement happens in Entity::moveTowardDestination() during tick().
}

void
Overlay::pickFacingForAngle() noexcept {
    // TODO
}
