#include "tiles/viewport.h"

#include "tiles/area.h"
#include "tiles/entity.h"
#include "tiles/vec.h"
#include "tiles/window.h"
#include "util/compiler.h"
#include "util/math2.h"

enum TrackingMode { TM_MANUAL, TM_FOLLOW_ENTITY };

static float aspectRatio;
static fvec2 off = {0, 0};
static fvec2 virtRes;

static TrackingMode mode = TM_MANUAL;
static Area* viewportArea = 0;
static Entity* targete;

static fvec2
centerOn(fvec2 pt) noexcept {
    return pt - virtRes / 2.f;
}

static float
boundDimension(float screen, float area, float pt, bool loop) noexcept {
    // Since looping areas continue without bound, this is a no-op.
    if (loop) {
        return pt;
    }

    // If the Area is smaller than the screen, center the Area. Otherwise,
    // allow the screen to move to the edge of the Area, but not past.
    float wiggleRoom = area - screen;
    return wiggleRoom <= 0 ? wiggleRoom / 2 : bound(pt, 0.0f, wiggleRoom);
}

static fvec2
boundToArea(fvec2 pt) noexcept {
    ivec3 ad = viewportArea->grid.dim;
    ivec2 td = viewportArea->grid.tileDim;
    float areaWidth = static_cast<float>(ad.x * td.x);
    float areaHeight = static_cast<float>(ad.y * td.y);
    bool loopX = viewportArea->grid.loopX;
    bool loopY = viewportArea->grid.loopY;

    fvec2 bounds = {
            boundDimension(virtRes.x, areaWidth, pt.x, loopX),
            boundDimension(virtRes.y, areaHeight, pt.y, loopY),
    };
    return bounds;
}

static fvec2
offsetForPt(fvec2 pt) noexcept {
    return boundToArea(centerOn(pt));
}

static void
_jumpToEntity(Entity* e) noexcept {
    fvec3 pos = e->getPixelCoord();
    ivec2 td = viewportArea->grid.tileDim;
    fvec2 center = {pos.x + td.x / 2, pos.y + td.y / 2};
    off = offsetForPt(center);
}

//! Returns as a normalized vector the percentage of screen that should
//! be blanked to preserve the aspect ratio. It can also be thought of
//! as the correcting aspect ratio.
static fvec2
getLetterbox() noexcept {
    fvec2 physRes = viewportGetPhysRes();
    float physAspect = physRes.x / physRes.y;
    float virtAspect = virtRes.x / virtRes.y;

    if (physAspect > virtAspect) {
        // Letterbox cuts off left-right.
        float cut = 1 - virtAspect / physAspect;
        fvec2 letterbox = {cut, 0};
        return letterbox;
    }
    else {
        // Letterbox cuts off top-bottom.
        float cut = 1 - physAspect / virtAspect;
        fvec2 letterbox = {0, cut};
        return letterbox;
    }
}

static fvec2
addLetterboxOffset(fvec2 pt) noexcept {
    fvec2 physRes = viewportGetPhysRes();
    fvec2 letterbox = getLetterbox();
    return pt - letterbox * physRes / 2.f;
}

static void
update() noexcept {
    switch (mode) {
    case TM_MANUAL:
        // Do nothing.
        break;
    case TM_FOLLOW_ENTITY: _jumpToEntity(targete); break;
    };
}

void
viewportSetSize(fvec2 virtRes_) noexcept {
    virtRes = virtRes_;

    // Calculate or recalculate the aspect ratio.
    float width = static_cast<float>(windowWidth());
    float height = static_cast<float>(windowHeight());
    aspectRatio = width / height;
}

void
viewportTick(Time) noexcept {
    update();
}

void
viewportTurn() noexcept {
    update();
}

fvec2
viewportGetMapOffset() noexcept {
    return off;
}

fvec2
viewportGetLetterboxOffset() noexcept {
    fvec2 offset = {0.0f, 0.0f};
    return addLetterboxOffset(offset);
}

fvec2
viewportGetScale() noexcept {
    fvec2 letterbox = getLetterbox();
    fvec2 physRes = {
            static_cast<float>(windowWidth()),
            static_cast<float>(windowHeight()),
    };

    fvec2 scale = {
            physRes.x / virtRes.x * (1 - letterbox.x),
            physRes.y / virtRes.y * (1 - letterbox.y),
    };
    return scale;
}

fvec2
viewportGetPhysRes() noexcept {
    fvec2 resolution = {static_cast<float>(windowWidth()),
                        static_cast<float>(windowHeight())};
    return resolution;
}

fvec2
viewportGetVirtRes() noexcept {
    return virtRes;
}

// Immediatly center render offset. Stop any tracking.
void
viewportJumpToPt(fvec2 pt) noexcept {
    mode = TM_MANUAL;
    off = offsetForPt(pt);
}

void
viewportJumpToEntity(Entity* e) noexcept {
    mode = TM_MANUAL;  // API implies mode change.
    _jumpToEntity(e);
}


// Continuously follow.
void
viewportTrackEntity(Entity* e) noexcept {
    mode = TM_FOLLOW_ENTITY;
    targete = e;
    update();
}


void
viewportSetArea(Area* a) noexcept {
    viewportArea = a;
}
