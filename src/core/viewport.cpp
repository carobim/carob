/***************************************
** Tsunagari Tile Engine              **
** viewport.cpp                       **
** Copyright 2011-2014 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

static Viewport globalViewport;

Viewport&
Viewport::instance() noexcept {
    return globalViewport;
}

Viewport::Viewport() noexcept : off{0, 0}, mode(TM_MANUAL), area(nullptr) {}

void
Viewport::setSize(rvec2 virtRes) noexcept {
    this->virtRes = virtRes;

    // Calculate or recalculate the aspect ratio.
    double width = (double)GameWindow::width();
    double height = (double)GameWindow::height();
    aspectRatio = width / height;
}

void
Viewport::tick(time_t) noexcept {
    update();
}

void
Viewport::turn() noexcept {
    update();
}

rvec2
Viewport::getMapOffset() const noexcept {
    return off;
}

rvec2
Viewport::getLetterboxOffset() const noexcept {
    return addLetterboxOffset(rvec2{0.0, 0.0});
}

rvec2
Viewport::getScale() const noexcept {
    rvec2 letterbox = getLetterbox();
    rvec2 physRes =
            rvec2{(double)GameWindow::width(), (double)GameWindow::height()};

    return rvec2{physRes.x / virtRes.x * (1 - letterbox.x),
                 physRes.y / virtRes.y * (1 - letterbox.y)};
}

rvec2
Viewport::getPhysRes() const noexcept {
    return rvec2{(double)GameWindow::width(), (double)GameWindow::height()};
}

rvec2
Viewport::getVirtRes() const noexcept {
    return virtRes;
}

// Immediatly center render offset. Stop any tracking.
void
Viewport::jumpToPt(ivec2 pt) noexcept {
    jumpToPt(rvec2{(double)pt.x, (double)pt.y});
}

void
Viewport::jumpToPt(rvec2 pt) noexcept {
    mode = TM_MANUAL;
    off = offsetForPt(pt);
}

void
Viewport::jumpToEntity(const Entity* e) noexcept {
    mode = TM_MANUAL;  // API implies mode change.
    _jumpToEntity(e);
}


// Continuously follow.
void
Viewport::trackEntity(const Entity* e) noexcept {
    mode = TM_FOLLOW_ENTITY;
    targete = e;
    update();
}


void
Viewport::setArea(const Area* a) noexcept {
    area = a;
}


void
Viewport::update() noexcept {
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
Viewport::_jumpToEntity(const Entity* e) noexcept {
    rcoord pos = e->getPixelCoord();
    ivec2 td = area->grid.getTileDimensions();
    rvec2 center = rvec2{pos.x + td.x / 2, pos.y + td.y / 2};
    off = offsetForPt(center);
}


rvec2
Viewport::getLetterbox() const noexcept {
    rvec2 physRes = getPhysRes();
    double physAspect = physRes.x / physRes.y;
    double virtAspect = virtRes.x / virtRes.y;

    if (physAspect > virtAspect) {
        // Letterbox cuts off left-right.
        double cut = 1 - virtAspect / physAspect;
        return rvec2{cut, 0};
    }
    else {
        // Letterbox cuts off top-bottom.
        double cut = 1 - physAspect / virtAspect;
        return rvec2{0, cut};
    }
}

rvec2
Viewport::offsetForPt(rvec2 pt) const noexcept {
    return boundToArea(centerOn(pt));
}

rvec2
Viewport::centerOn(rvec2 pt) const noexcept {
    return pt - virtRes / 2;
}

rvec2
Viewport::boundToArea(rvec2 pt) const noexcept {
    icoord ad = area->grid.dim;
    ivec2 td = area->grid.tileDim;
    double areaWidth = ad.x * td.x;
    double areaHeight = ad.y * td.y;
    bool loopX = area->grid.loopX;
    bool loopY = area->grid.loopY;

    return rvec2{boundDimension(virtRes.x, areaWidth, pt.x, loopX),
                 boundDimension(virtRes.y, areaHeight, pt.y, loopY)};
}

double
Viewport::boundDimension(double screen, double area, double pt, bool loop) const
        noexcept {
    // Since looping areas continue without bound, this is a no-op.
    if (loop) {
        return pt;
    }

    // If the Area is smaller than the screen, center the Area. Otherwise,
    // allow the screen to move to the edge of the Area, but not past.
    double wiggleRoom = area - screen;
    return wiggleRoom <= 0 ? wiggleRoom / 2 : bound(pt, 0.0, wiggleRoom);
}

rvec2
Viewport::addLetterboxOffset(rvec2 pt) const noexcept {
    rvec2 physRes = getPhysRes();
    rvec2 letterbox = getLetterbox();
    return pt - letterbox * physRes / 2;
}
