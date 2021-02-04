/***************************************
** Tsunagari Tile Engine              **
** viewport.cpp                       **
** Copyright 2011-2014 Michael Reiley **
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

#include "core/viewport.h"

#include "core/area.h"
#include "core/entity.h"
#include "core/vec.h"
#include "core/window.h"
#include "util/math2.h"

enum TrackingMode {
    TM_MANUAL,
    TM_FOLLOW_ENTITY,
};

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

    return {
        boundDimension(virtRes.x, areaWidth, pt.x, loopX),
        boundDimension(virtRes.y, areaHeight, pt.y, loopY),
    };
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
        return fvec2{cut, 0};
    }
    else {
        // Letterbox cuts off top-bottom.
        float cut = 1 - physAspect / virtAspect;
        return {0, cut};
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
    case TM_FOLLOW_ENTITY:
        _jumpToEntity(targete);
        break;
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
viewportTick(time_t) noexcept {
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
    return addLetterboxOffset(fvec2{0.0, 0.0});
}

fvec2
viewportGetScale() noexcept {
    fvec2 letterbox = getLetterbox();
    fvec2 physRes = {
        static_cast<float>(windowWidth()),
        static_cast<float>(windowHeight()),
    };

    return {
        physRes.x / virtRes.x * (1 - letterbox.x),
        physRes.y / virtRes.y * (1 - letterbox.y),
    };
}

fvec2
viewportGetPhysRes() noexcept {
    return {
        static_cast<float>(windowWidth()),
        static_cast<float>(windowHeight())
    };
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
