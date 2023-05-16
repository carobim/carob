#include "tiles/sounds.h"

#include "av/sdl2/error.h"
#include "av/sdl2/sdl2.h"
#include "os/mutex.h"
#include "tiles/resources.h"
#include "tiles/world.h"
#include "util/compiler.h"
#include "util/hashtable.h"
#include "util/int.h"
#include "util/markable.h"
#include "util/measure.h"
#include "util/pool.h"
#include "util/vector.h"

struct SDL2Sound {
    int numUsers;
    Time lastUse;

    String frames;     // Audio frames.
    Mix_Chunk* chunk;  // Decoding configuration.
};

static bool
operator==(SDL2Sound a, SDL2Sound b) noexcept {
    return a.numUsers == b.numUsers && a.lastUse == b.lastUse &&
           a.frames == b.frames && a.chunk == b.chunk;
}

struct SDL2PlayingSound {
    bool playing;
    bool inUse;

    int channel;
};

static Hashmap<String, SoundID> soundIDs;
static Pool<SDL2Sound> soundPool;
static Pool<SDL2PlayingSound> playingSoundPool;

// Map from SDL2 channel to PlayingSoundID for SDL2_mixer callbacks.
static Vector<int> playingChannels;

static Mutex channelMutex;

static void
channelFinished(int channel) noexcept {
    LockGuard guard(channelMutex);

    int psid = playingChannels[channel];
    SDL2PlayingSound& ps = playingSoundPool[psid];
    ps.playing = false;
    if (!ps.inUse) {
        playingSoundPool.release(psid);
    }
}

static void
init() noexcept {
    static bool initialized = false;

    if (initialized) {
        return;
    }
    initialized = true;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        {
            TimeMeasure m("Initialized the SDL2 audio subsystem");
            if (SDL_Init(SDL_INIT_AUDIO) < 0) {
                sdlDie("Sounds", "SDL_Init(SDL_INIT_AUDIO)");
            }
        }

        {
            TimeMeasure m("Opened an audio device");
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
                sdlDie("Sounds", "Mix_OpenAudio");
            }
        }
    }

    Mix_ChannelFinished(channelFinished);

    playingChannels.resize(Mix_AllocateChannels(-1));
}

static SDL2Sound
makeSound(StringView path) noexcept {
    String r;
    if (!resourceLoad(path, r)) {
        // Error logged.
        return SDL2Sound();
    }

    SDL_RWops* ops =
        SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r.data)),
                      static_cast<int>(r.size));

    Mix_Chunk* chunk;

    {
        TimeMeasure m(String() << "Constructed " << path << " as sound");
        chunk = Mix_LoadWAV_RW(ops, true);
    }

    if (chunk == 0) {
        sdlError("Sounds", String() << "Mix_LoadWAV(" << path << ")");
        return SDL2Sound();
    }

    return SDL2Sound{1, 0, static_cast<String&&>(r), chunk};
}

SoundID
soundLoad(StringView path) noexcept {
    init();

    SoundID* cachedId = soundIDs.tryAt(path);
    if (cachedId) {
        int sid = **cachedId;
        SDL2Sound& sound = soundPool[sid];
        sound.numUsers += 1;
        return SoundID(sid);
    }

    SDL2Sound sound = makeSound(path);
    if (sound == SDL2Sound()) {
        soundIDs[path] = mark;
        return mark;
    }

    int sid = soundPool.allocate();
    new (&soundPool[sid].frames) String();
    soundPool[sid] = sound;

    soundIDs[path] = sid;

    return SoundID(sid);
}

void
soundsPrune(Time latestPermissibleUse) noexcept {
    // Mix_FreeChunk(chunk);
}

PlayingSoundID
soundPlay(SoundID sid) noexcept {
    if (!sid) {
        return mark;
    }

    SDL2Sound sound = soundPool[*sid];

    int channel = Mix_PlayChannel(-1, sound.chunk, 0);
    if (channel == -1) {
        // Maybe there are too many sounds playing at once right now.
        return mark;
    }

    (void)Mix_Volume(channel, 255);

    LockGuard guard(channelMutex);

    int psid = playingSoundPool.allocate();
    playingSoundPool[psid] = SDL2PlayingSound{true, true, channel};
    playingChannels[channel] = psid;
    return PlayingSoundID(psid);
}

void
soundRelease(SoundID sid) noexcept {
    if (!sid) {
        return;
    }

    SDL2Sound& sound = soundPool[*sid];

    sound.numUsers -= 1;
    assert_(sound.numUsers >= 0);

    if (sound.numUsers == 0) {
        sound.lastUse = worldTime();
    }
}

bool
playingSoundIsPlaying(PlayingSoundID psid) noexcept {
    if (!psid) {
        return false;
    }

    LockGuard guard(channelMutex);

    SDL2PlayingSound ps = playingSoundPool[*psid];
    return ps.playing;
}

void
playingSoundStop(PlayingSoundID psid) noexcept {
    if (!psid) {
        return;
    }

    SDL2PlayingSound& ps = playingSoundPool[*psid];

    LockGuard guard(channelMutex);

    assert_(ps.playing);
    ps.playing = false;

    (void)Mix_HaltChannel(ps.channel);
}

void
playingSoundVolume(PlayingSoundID psid, float volume) noexcept {
    if (!psid) {
        return;
    }

    LockGuard guard(channelMutex);

    SDL2PlayingSound ps = playingSoundPool[*psid];

    (void)Mix_Volume(ps.channel, static_cast<int>(volume * 128));
}

void
playingSoundSpeed(PlayingSoundID psid, float speed) noexcept {
    // No-op. SDL2 doesn't support changing playback rate.
}

void
playingSoundRelease(PlayingSoundID psid) noexcept {
    if (!psid) {
        return;
    }

    LockGuard guard(channelMutex);

    SDL2PlayingSound& ps = playingSoundPool[*psid];

    assert_(ps.inUse);
    ps.inUse = false;

    if (!ps.playing) {
        playingSoundPool.release(*psid);
    }
}
