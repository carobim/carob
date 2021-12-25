#ifndef SRC_TILES_DISPLAY_LIST_H_
#define SRC_TILES_DISPLAY_LIST_H_

#include "tiles/images.h"
#include "tiles/vec.h"
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

    U32 colorOverlayARGB;
    bool paused;  // TODO: Move to colorOverlay & overlay.
};

void
displayListPresent(DisplayList* display) noexcept;

#endif  // SRC_TILES_DISPLAY_LIST_H_
