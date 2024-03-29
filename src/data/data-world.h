#ifndef SRC_DATA_DATA_WORLD_H_
#define SRC_DATA_DATA_WORLD_H_

class DataArea;

#include "tiles/client-conf.h"
#include "tiles/vec.h"
#include "util/compiler.h"
#include "util/string-view.h"

//! After the engine has booted, initialize the world.
void
dataWorldInit() noexcept;

DataArea*
dataWorldArea(StringView areaName) noexcept;

// Engine parameters set by world's author.
extern enum MoveMode dataWorldMoveMode;
extern fvec2 dataWorldViewportResolution;
extern StringView dataWorldStartArea;
extern StringView dataWorldPlayerFile;
extern StringView dataWorldPlayerStartPhase;
extern vicoord dataWorldStartCoords;

extern StringView dataWorldDatafile;

#endif  // SRC_DATA_DATA_WORLD_H_
