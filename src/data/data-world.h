/***************************************
** Tsunagari Tile Engine              **
** data-world.h                       **
** Copyright 2014      Michael Reiley **
** Copyright 2014-2020 Paul Merrill   **
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

#ifndef SRC_DATA_DATA_WORLD_H_
#define SRC_DATA_DATA_WORLD_H_

class DataArea;

#include "core/client-conf.h"
#include "core/vec.h"
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

extern enum Conf::MovementMode dataWorldMoveMode;
extern rvec2 dataWorldViewportResolution;
extern int dataWorldInputPersistDelayInitial;
extern int dataWorldInputPersistDelayConsecutive;
extern StringView dataWorldStartArea;
extern StringView dataWorldPlayerFile;
extern StringView dataWorldPlayerStartPhase;
extern vicoord dataWorldStartCoords;

extern StringView dataWorldDatafile;

#endif  // SRC_DATA_DATA_WORLD_H_
