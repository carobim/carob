#ifndef SRC_CORE_DISPLAY_LIST_H_
#define SRC_CORE_DISPLAY_LIST_H_

#include "core/images.h"
#include "core/vec.h"
#include "util/compiler.h"
#include "util/vector.h"

struct DisplayItem {
    Image image;
    fvec3 destination;
};

struct DisplayList {
    bool loopX;
    bool loopY;

    fvec2 padding;
    fvec2 scale;
    fvec2 scroll;
    fvec2 size;

    Vector<DisplayItem> items;

    uint32_t colorOverlayARGB;
    bool paused;  // TODO: Move to colorOverlay & overlay.
};

void
displayListPresent(DisplayList* display) noexcept;

#endif  // SRC_CORE_DISPLAY_LIST_H_
