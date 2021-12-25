#ifndef SRC_AV_SDL2_WINDOW_H_
#define SRC_AV_SDL2_WINDOW_H_

#include "av/sdl2/sdl2.h"
#include "tiles/vec.h"
#include "util/compiler.h"

extern SDL_Window* sdl2Window;
extern fvec2 sdl2Translation;
extern fvec2 sdl2Scaling;

void
imageStartFrame() noexcept;
void
imageEndFrame() noexcept;

#endif  // SRC_AV_SDL2_WINDOW_H_
