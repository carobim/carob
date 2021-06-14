/*************************************
** Tsunagari Tile Engine            **
** display-list.cpp                 **
** Copyright 2018-2021 Paul Merrill **
*************************************/

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

#include "core/display-list.h"

#include "core/window.h"
#include "util/int.h"
#include "util/math2.h"

static void
pushLetterbox(DisplayList* display) noexcept {
    // Aspect ratio correction.
    fvec2 sz = display->size;
    fvec2 lb = -1.f * display->padding;

    // Map bounds.
    fvec2 scale = display->scale;
    fvec2 virtScroll = display->scroll;
    fvec2 padding = display->padding;

    fvec2 physScroll = -1.f * virtScroll * scale + padding;

    float x = lb.x;
    float y = lb.y;
    float width = sz.x - 2 * lb.x;
    float height = sz.y - 2 * lb.y;

    if (!display->loopX && physScroll.x > 0) {
        // Boxes on left-right.
        x = max(x, physScroll.x);
        width = min(width, sz.x - 2 * physScroll.x);
        height = min(height, sz.x);
    }
    if (!display->loopY && physScroll.y > 0) {
        // Boxes on top-bottom.
        y = max(y, physScroll.y);
        width = min(width, sz.x);
        height = min(height, sz.y - 2 * physScroll.y);
    }

    windowPushClip(x, y, width, height);
}

static void
popLetterbox() noexcept {
    windowPopClip();
}

void
displayListPresent(DisplayList* display) noexcept {
    pushLetterbox(display);

    // Zoom and pan the Area to fit on-screen.
    windowPushTranslate(-display->padding.x, -display->padding.y);
    windowPushScale(display->scale.x, display->scale.y);
    windowPushTranslate(-display->scroll.x, -display->scroll.y);

    for (DisplayItem& item : display->items) {
        imageDraw(item.image, item.destination.x, item.destination.y,
                  item.destination.z);
    }

    windowPopTranslate();
    windowPopScale();
    windowPopTranslate();

    if (display->colorOverlayARGB & 0xFF000000) {
        float ww = static_cast<float>(windowWidth());
        float wh = static_cast<float>(windowHeight());
        imageDrawRect(0, ww, 0, wh, display->colorOverlayARGB);
    }

    popLetterbox();

    if (display->paused) {
        float ww = static_cast<float>(windowWidth());
        float wh = static_cast<float>(windowHeight());
        imageDrawRect(0, ww, 0, wh, 0x7F000000);
        Image pauseInfo = imageLoad("resource/pause_overlay.bmp");
        if (IMAGE_VALID(pauseInfo)) {
            float iw = static_cast<float>(pauseInfo.width);
            float ih = static_cast<float>(pauseInfo.height);
            float top = 768.0f;
            imageDraw(pauseInfo, ww / 2 - iw / 2, wh / 2 - ih / 2, top);
        }
        imageRelease(pauseInfo);
    }
}
