#include "tiles/window.h"

#include "os/os.h"
#include "tiles/world.h"
#include "util/compiler.h"

Keys windowKeysDown = 0;

void
windowEmitKeyDown(Key key) noexcept {
    bool wasDown = !!(windowKeysDown & key);

    windowKeysDown |= key;

    if (windowKeysDown & KEY_ESCAPE &&
        (windowKeysDown & KEY_LEFT_SHIFT || windowKeysDown & KEY_RIGHT_SHIFT)) {
        windowClose();
        exitProcess(0);
    }

    if (!wasDown) {
        worldButtonDown(key);
    }
}

void
windowEmitKeyUp(Key key) noexcept {
    bool wasDown = !!(windowKeysDown & key);

    windowKeysDown &= ~key;

    if (wasDown) {
        worldButtonUp(key);
    }
}
