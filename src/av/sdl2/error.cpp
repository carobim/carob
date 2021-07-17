#include "av/sdl2/error.h"

#include "av/sdl2/sdl2.h"
#include "core/log.h"
#include "util/compiler.h"
#include "util/string.h"

void
sdlError(StringView system, StringView function) noexcept {
    String message = function;
    StringView err = SDL_GetError();
    if (err.size > 0) {
        message << ": " << err;
    }
    logErr(system, message);
}

void
sdlDie(StringView system, StringView function) noexcept {
    String message = function;
    StringView err = SDL_GetError();
    if (err.size > 0) {
        message << ": " << err;
    }
    logFatal(system, message);
}
