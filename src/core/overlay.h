#ifndef SRC_CORE_OVERLAY_H_
#define SRC_CORE_OVERLAY_H_

#include "core/entity.h"
#include "util/compiler.h"

class Overlay : public Entity {
 public:
    Overlay() noexcept { }
    virtual ~Overlay() noexcept { }

    void
    tick(Time dt) noexcept;

    void
    teleport(vicoord coord) noexcept;

    void
    drift(ivec2 xy) noexcept;
    void
    driftTo(ivec2 xy) noexcept;

 protected:
    void
    pickFacingForAngle() noexcept;
};

#endif  // SRC_CORE_OVERLAY_H_
