#ifndef SRC_DATA_DATA_WORLD_H_
#define SRC_DATA_DATA_WORLD_H_

class DataArea;

#include "core/client-conf.h"
#include "core/vec.h"
#include "util/compiler.h"
#include "util/string-view.h"

//! After the engine has booted, initialize the world.
bool
dataWorldInit() noexcept;

DataArea*
dataWorldArea(StringView areaName) noexcept;

// Engine parameters set by world's author.
extern StringView dataWorldName;
extern StringView dataWorldAuthor;
extern StringView dataWorldVersion;

extern enum MoveMode dataWorldMoveMode;
extern fvec2 dataWorldViewportResolution;
extern int dataWorldInputPersistDelayInitial;
extern int dataWorldInputPersistDelayConsecutive;
extern StringView dataWorldStartArea;
extern StringView dataWorldPlayerFile;
extern StringView dataWorldPlayerStartPhase;
extern vicoord dataWorldStartCoords;

extern StringView dataWorldDatafile;

#endif  // SRC_DATA_DATA_WORLD_H_
