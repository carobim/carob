/*************************************
** Tsunagari Tile Engine            **
** music.cpp                        **
** Copyright 2016-2020 Paul Merrill **
*************************************/

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

#include "av/sdl2/error.h"
#include "av/sdl2/sdl2.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/music-worker.h"
#include "core/resources.h"
#include "util/assert.h"
#include "util/hashvector.h"
#include "util/int.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"

struct Song {
    // The Mix_Music needs the music data to be kept around for its lifetime.
    StringView fileContent;

    Mix_Music* mix;
};

static bool initalized = false;
static int paused = 0;
static HashVector<Song> songs;
static uint32_t songHash = 0;
static Song* song = 0;

static Song*
load(StringView path) noexcept {
    Song& newSong = songs.allocate(hash_(path));
    newSong.mix = 0;

    StringView r;
    if (!resourceLoad(path, r)) {
        // Error logged.
        return 0;
    }

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r.data)),
                          static_cast<int>(r.size));

    TimeMeasure m(String() << "Constructed " << path << " as music");
    Mix_Music* mix = Mix_LoadMUS_RW(ops, 1);

    if (!mix) {
        sdlDie("SDL2", String() << "Failed to load music: " << path);
        return 0;
    }

    // We need to keep the memory around, so put it in a struct.
    newSong.fileContent = r;
    newSong.mix = mix;

    return &newSong;
}

static void
init() noexcept {
    if (initalized) {
        return;
    }

    initalized = true;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        {
            TimeMeasure m("Initialized the SDL2 audio subsystem");
            if (SDL_Init(SDL_INIT_AUDIO) < 0) {
                sdlDie("SDL2Music", "SDL_Init(SDL_INIT_AUDIO)");
            }
        }

        {
            TimeMeasure m("Opened an audio device");
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
                sdlDie("SDL2Music", "Mix_OpenAudio");
            }
        }
    }
}

void
musicWorkerPlay(StringView path) noexcept {
    init();

    uint32_t pathHash = hash_(path);

    if (songHash == pathHash) {
        return;
    }

    paused = 0;

    if (song && !Mix_PausedMusic()) {
        Mix_HaltMusic();
    }

    if (path.size == 0) {
        songHash = 0;
        song = 0;
        return;
    }

    song = songs.find(hash_(path));
    if (!song) {
        song = load(path);
    }
    if (!song || !song->mix) {
        songHash = 0;
        song = 0;
        return;
    }

    songHash = pathHash;
    song = song;

    TimeMeasure m(String() << "Playing " << path);
    Mix_PlayMusic(song->mix, -1);
}

void
musicWorkerStop() noexcept {
    init();

    paused = 0;

    if (song) {
        songHash = 0;
        song = 0;
        Mix_HaltMusic();
    }
}

void
musicWorkerPause() noexcept {
    init();

    if (paused == 0 && song) {
        Mix_PauseMusic();
    }

    paused++;
}

void
musicWorkerResume() noexcept {
    init();

    if (!paused) {
        return;
    }

    paused--;

    if (paused == 0 && song) {
        Mix_ResumeMusic();
    }
}

void
musicWorkerGarbageCollect() noexcept {}
