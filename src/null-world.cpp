#include "data/data-area.h"
#include "data/data-world.h"

static DataArea nullArea;

DataArea*
dataWorldArea(StringView) noexcept {
    return &nullArea;
}

StringView dataWorldName = "Null World";
StringView dataWorldAuthor = "Paul Merrill";
StringView dataWorldVersion = "1";

enum MoveMode dataWorldMoveMode = TILE;
fvec2 dataWorldViewportResolution = {240, 160};
I32 dataWorldInputPersistDelayInitial = 300;
I32 dataWorldInputPersistDelayConsecutive = 100;
StringView dataWorldStartArea = "null-area.json";
StringView dataWorldPlayerFile = "null-player.json";
StringView dataWorldPlayerStartPhase = "down";
vicoord dataWorldStartCoords = {0, 0, 0};

StringView dataWorldDatafile = "./null.world";

bool
dataWorldInit() noexcept {
    return true;
}
