#ifndef SRC_AV_SDL2_ERROR_H_
#define SRC_AV_SDL2_ERROR_H_

#include "util/compiler.h"
#include "util/string-view.h"

void
sdlError(StringView system, StringView function) noexcept;
void
sdlDie(StringView system, StringView function) noexcept;

#endif  // SRC_AV_SDL2_ERROR_H_
