#include "data/data-area.h"
#include "data/data-world.h"

static DataArea nullArea;

DataArea*
dataWorldArea(StringView) noexcept {
    return &nullArea;
}

enum MoveMode dataWorldMoveMode = TILE;
fvec2 dataWorldViewportResolution = {240, 160};
StringView dataWorldStartArea = "null-area.json";
StringView dataWorldPlayerFile = "null-player.json";
StringView dataWorldPlayerStartPhase = "down";
vicoord dataWorldStartCoords = {0, 0, 0};

StringView dataWorldDatafile = "./null.world";

void
dataWorldInit() noexcept { }
