#include "data/data-area.h"
#include "data/data-world.h"

static DataArea nullArea;

DataArea*
dataWorldArea(StringView) noexcept {
    return &nullArea;
}

enum MoveMode dataWorldMoveMode = TILE;
fvec2 dataWorldViewportResolution = {240, 160};
StringView dataWorldStartArea = "area.json";
StringView dataWorldPlayerFile = "player.json";
StringView dataWorldPlayerStartPhase = "down";
vicoord dataWorldStartCoords = {0, 0, 0};

StringView dataWorldDatafile = "./demo.world";
