#ifndef SRC_AV_SDL2_SDL2_H_
#define SRC_AV_SDL2_SDL2_H_

#include "util/compiler.h"
#include "util/int.h"

extern "C" {

// SDL.h
int SDL_Init(U32) noexcept;
U32
SDL_WasInit(U32 flags) noexcept;
#define SDL_INIT_AUDIO 0x10
#define SDL_INIT_VIDEO 0x20

// SDL_audio.h
#define AUDIO_S16LSB 0x8010

// SDL_blendmode.h
typedef enum {
    SDL_BLENDMODE_NONE,
    SDL_BLENDMODE_BLEND,
} SDL_BlendMode;

// SDL_error.h
const char*
SDL_GetError() noexcept;

// SDL_scancode.h
//typedef enum {} SDL_Scancode;
typedef int SDL_Scancode;

// SDK_keycode.h
typedef I32 SDL_Keycode;
enum {
    SDLK_ESCAPE = '\033',
    SDLK_SPACE = ' ',
    SDLK_RIGHT = (1 << 30) | 79,
    SDLK_LEFT = (1 << 30) | 80,
    SDLK_DOWN = (1 << 30) | 81,
    SDLK_UP = (1 << 30) | 82,
    SDLK_LCTRL = (1 << 30) | 224,
    SDLK_LSHIFT = (1 << 30) | 225,
    SDLK_RCTRL = (1 << 30) | 228,
    SDLK_RSHIFT = (1 << 30) | 229,
};

// SDL_keyboard.h
typedef struct {
    SDL_Scancode scancode;
    SDL_Keycode sym;
    U16 mod;
    U32 unused;
} SDL_Keysym;

// SDL_events.h
typedef enum {
    SDL_QUIT = 0x100,
    SDL_KEYDOWN = 0x300,
    SDL_KEYUP = 0x301,
} SDL_EventType;
typedef struct {
    U32 type, timestamp, windowID;
    U8 state, repeat, padding2, padding3;
    SDL_Keysym keysym;
} SDL_KeyboardEvent;
typedef union {
    U32 type;
    SDL_KeyboardEvent key;
    U8 padding[56];
} SDL_Event;
int
SDL_PollEvent(SDL_Event*) noexcept;

// SDL_metal.h
void*
SDL_Metal_CreateView(void*) noexcept;
void*
SDL_Metal_GetLayer(void*) noexcept;

// SDL_pixels.h
#define SDL_PIXELFORMAT_RGBA8888 373694468
#define SDL_PIXELFORMAT_ABGR8888 376840196
#define SDL_PIXELFORMAT_RGBA32   SDL_PIXELFORMAT_ABGR8888  // When little endian

// SDL_rect.h
typedef struct {
    int x, y, w, h;
} SDL_Rect;

// SDL_rwops.h
typedef struct SDL_RWops SDL_RWops;
SDL_RWops*
SDL_RWFromMem(void*, int) noexcept;

// SDL_surface.h
struct SDL_Surface {
    U32 foo1;
    void* foo2;
    int w, h;
    int foo3;
    void* pixels;
};
void
SDL_FreeSurface(SDL_Surface*) noexcept;
SDL_Surface*
SDL_LoadBMP_RW(SDL_RWops*, int) noexcept;

// SDL_video.h
typedef struct SDL_Window SDL_Window;
typedef struct {
    U32 format;
    int w, h, refresh_rate;
    void* driverdata;
} SDL_DisplayMode;
typedef void* SDL_GLContext;
SDL_Window*
SDL_CreateWindow(const char*, int, int, int, int, U32) noexcept;
void
SDL_EnableScreenSaver(void) noexcept;
int
SDL_GetCurrentDisplayMode(int, SDL_DisplayMode*) noexcept;
int
SDL_GetWindowDisplayIndex(SDL_Window*) noexcept;
void
SDL_GetWindowSize(SDL_Window*, int*, int*) noexcept;
void
SDL_HideWindow(SDL_Window*) noexcept;
void
SDL_SetWindowTitle(SDL_Window*, const char*) noexcept;
void
SDL_ShowWindow(SDL_Window*) noexcept;
void*
SDL_GL_GetProcAddress(const char*) noexcept;
SDL_GLContext
SDL_GL_CreateContext(SDL_Window*) noexcept;
void
SDL_GL_SwapWindow(SDL_Window*) noexcept;
#define SDL_WINDOW_FULLSCREEN              0x00000001
#define SDL_WINDOW_OPENGL                  0x00000002
#define SDL_WINDOW_METAL                   0x20000000
#define SDL_WINDOW_HIDDEN                  0x00000008
#define SDL_WINDOWPOS_UNDEFINED_MASK       0x1FFF0000u
#define SDL_WINDOWPOS_UNDEFINED_DISPLAY(X) (SDL_WINDOWPOS_UNDEFINED_MASK | (X))
#define SDL_WINDOWPOS_UNDEFINED            SDL_WINDOWPOS_UNDEFINED_DISPLAY(0)

// SDL_render.h
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_RendererInfo {
    const char* name;
    U32 flags;
    U32 num_texture_formats;
    U32 texture_formats[16];
    int max_texture_width;
    int max_texture_height;
} SDL_RendererInfo;
SDL_Renderer*
SDL_CreateRenderer(SDL_Window*, int, U32) noexcept;
SDL_Texture*
SDL_CreateTexture(SDL_Renderer*, U32, int, int, int) noexcept;
SDL_Texture*
SDL_CreateTextureFromSurface(SDL_Renderer*, SDL_Surface*) noexcept;
void
SDL_DestroyTexture(SDL_Texture*) noexcept;
int
SDL_GetRendererInfo(SDL_Renderer*, SDL_RendererInfo*) noexcept;
int
SDL_QueryTexture(SDL_Texture*, U32*, int*, int*, int*) noexcept;
int
SDL_SetRenderTarget(SDL_Renderer*, SDL_Texture*) noexcept;
int
SDL_SetTextureBlendMode(SDL_Texture*, SDL_BlendMode) noexcept;
int
SDL_RenderClear(SDL_Renderer*) noexcept;
int
SDL_RenderCopy(SDL_Renderer*, SDL_Texture*, const SDL_Rect*,
               const SDL_Rect*) noexcept;
int
SDL_RenderFillRect(SDL_Renderer*, const SDL_Rect*) noexcept;
void
SDL_RenderPresent(SDL_Renderer*) noexcept;
int
SDL_SetRenderDrawBlendMode(SDL_Renderer*, SDL_BlendMode) noexcept;
int
SDL_SetRenderDrawColor(SDL_Renderer*, U8, U8, U8, U8) noexcept;
#define SDL_RENDERER_ACCELERATED   2
#define SDL_RENDERER_PRESENTVSYNC  4
#define SDL_RENDERER_TARGETTEXTURE 8
#define SDL_TEXTUREACCESS_TARGET   2

// SDL_image library
// SDL_image.h
SDL_Surface*
IMG_Load_RW(SDL_RWops*, int) noexcept;

// SDL_mixer library
// SDL_mixer.h
typedef struct Mix_Chunk Mix_Chunk;
typedef struct Mix_Music Mix_Music;
int
Mix_AllocateChannels(int) noexcept;
void
Mix_ChannelFinished(void (*)(int));
void
Mix_FreeChunk(Mix_Chunk*) noexcept;
void
Mix_FreeMusic(Mix_Music*) noexcept;
int
Mix_HaltChannel(int) noexcept;
int
Mix_HaltMusic() noexcept;
Mix_Music*
Mix_LoadMUS_RW(SDL_RWops*, int) noexcept;
Mix_Chunk*
Mix_LoadWAV_RW(SDL_RWops*, int) noexcept;
int
Mix_OpenAudio(int, U16, int, int) noexcept;
int
Mix_PausedMusic() noexcept;
void
Mix_Pause(int) noexcept;
void
Mix_PauseMusic() noexcept;
int
Mix_PlayingMusic() noexcept;
int
Mix_PlayChannelTimed(int, Mix_Chunk*, int, int) noexcept;
int
Mix_PlayMusic(Mix_Music*, int) noexcept;
void
Mix_Resume(int) noexcept;
void
Mix_ResumeMusic() noexcept;
int
Mix_SetPosition(int, I16, U8) noexcept;
int
Mix_Volume(int, int) noexcept;
int
Mix_VolumeMusic(int) noexcept;
#define MIX_DEFAULT_FORMAT AUDIO_S16LSB
#define Mix_PlayChannel(channel, chunk, loops) \
    Mix_PlayChannelTimed(channel, chunk, loops, -1)

}  // extern "C"

#endif  // SRC_AV_SDL2_SDL2_H_
