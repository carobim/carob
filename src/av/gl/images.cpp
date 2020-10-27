/*************************************
** Tsunagari Tile Engine            **
** images.cpp                       **
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

#include "core/images.h"

#include "av/sdl2/error.h"
#include "av/sdl2/sdl2.h"
#include "av/sdl2/window.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/window.h"
#include "util/hashvector.h"
#include "util/int.h"
#include "util/noexcept.h"
#include "util/optional.h"
#include "util/string-view.h"
#include "util/string.h"

//
// Reusable GL [ES] code
//

// OpenGL 1.2+
// https://github.com/KhronosGroup/OpenGL-Registry/blob/master/api/GL/glext.h
//
// OpenGL ES 2
// https://github.com/KhronosGroup/OpenGL-Registry/blob/master/api/GLES2/gl2.h
//
// OpenGL ES 2 extensions
// https://github.com/KhronosGroup/OpenGL-Registry/blob/master/api/GLES2/gl2ext.h
//
// OpenGL 4.6 core + some sanctioned 0extensions
// https://github.com/KhronosGroup/OpenGL-Registry/blob/master/api/GL/glcorearb.h
//
// OpenGL extensions
// https://github.com/KhronosGroup/OpenGL-Registry/tree/469e33a47301b42b9f26e0ccfdfa4eb9e77f7f64/extensions

#if defined(_WIN32)  // and if not static linking
#define APIENTRY __stdcall
#define APICALL __declspec(dllimport)
#else
#define APIENTRY
#define APICALL
#endif

static void checkError(StringView call) noexcept;

typedef unsigned GLbitfield;
typedef uint8_t GLboolean;
typedef char GLchar;
typedef unsigned GLenum;
typedef int GLint;
typedef float GLfloat;
typedef int GLsizei;
typedef ssize_t GLsizeiptr;
typedef uint8_t GLubyte;
typedef unsigned GLuint;

typedef GLint Attribute;
typedef GLuint Buffer;
typedef GLuint Program;
typedef GLuint Shader;
typedef GLuint Texture;
typedef GLint Uniform;
typedef GLuint VertexBuffer;

// TODO: Figure out noexcept?
typedef GLenum (APIENTRY* GlGetErrorProc)();
static APICALL GlGetErrorProc glGetError;

#define GLFN_VOID_1(rt, fn, t1) \
    typedef rt (APIENTRY* fn##Proc)(t1); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a) noexcept { \
        fn(a); \
        checkError(#fn); \
    }

#define GLFN_VOID_2(rt, fn, t1, t2) \
    typedef rt (APIENTRY* fn##Proc)(t1, t2); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a, t2 b) noexcept { \
        fn(a, b); \
        checkError(#fn); \
    }

#define GLFN_VOID_3(rt, fn, t1, t2, t3) \
    typedef rt (APIENTRY* fn##Proc)(t1, t2, t3); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a, t2 b, t3 c) noexcept { \
        fn(a, b, c); \
        checkError(#fn); \
    }

#define GLFN_VOID_4(rt, fn, t1, t2, t3, t4) \
    typedef rt (APIENTRY* fn##Proc)(t1, t2, t3, t4); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a, t2 b, t3 c, t4 d) noexcept { \
        fn(a, b, c, d); \
        checkError(#fn); \
    }

#define GLFN_VOID_6(rt, fn, t1, t2, t3, t4, t5, t6) \
    typedef rt (APIENTRY* fn##Proc)(t1, t2, t3, t4, t5, t6); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a, t2 b, t3 c, t4 d, t5 e, t6 f) noexcept { \
        fn(a, b, c, d, e, f); \
        checkError(#fn); \
    }

#define GLFN_VOID_9(rt, fn, t1, t2, t3, t4, t5, t6, t7, t8, t9) \
    typedef rt (APIENTRY* fn##Proc)(t1, t2, t3, t4, t5, t6, t7, t8, t9); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a, t2 b, t3 c, t4 d, t5 e, t6 f, t7 g, t8 h, t9 i) noexcept { \
        fn(a, b, c, d, e, f, g, h, i); \
        checkError(#fn); \
    }

#define GLFN_RETURN_0(rt, fn) \
    typedef rt (APIENTRY* fn##Proc)(); \
    static APICALL fn##Proc fn; \
    static rt fn##_() noexcept { \
        rt x = fn(); \
        checkError(#fn); \
        return x; \
    }

#define GLFN_RETURN_1(rt, fn, t1) \
    typedef rt (APIENTRY* fn##Proc)(t1); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a) noexcept { \
        rt x = fn(a); \
        checkError(#fn); \
        return x; \
    }

#define GLFN_RETURN_2(rt, fn, t1, t2) \
    typedef rt (APIENTRY* fn##Proc)(t1, t2); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a, t2 b) noexcept { \
        rt x = fn(a, b); \
        checkError(#fn); \
        return x; \
    }

GLFN_VOID_1(void, glActiveTexture, GLenum)
GLFN_VOID_2(void, glAttachShader, Program, Shader)
GLFN_VOID_2(void, glBindBuffer, GLenum, Buffer)
GLFN_VOID_2(void, glBindTexture, GLenum, Texture)
GLFN_VOID_4(void, glBufferData, GLenum, GLsizeiptr, const void*, GLenum)
GLFN_VOID_1(void, glClear, GLbitfield)
GLFN_VOID_4(void, glClearColor, GLfloat, GLfloat, GLfloat, GLfloat)
GLFN_VOID_1(void, glCompileShader, Shader)
GLFN_RETURN_0(Program, glCreateProgram)
GLFN_RETURN_1(Shader, glCreateShader, GLenum)
GLFN_VOID_3(void, glDrawArrays, GLenum, GLint, GLsizei)
GLFN_VOID_1(void, glEnableVertexAttribArray, Attribute)
GLFN_VOID_2(void, glGenBuffers, GLsizei, Buffer*)
GLFN_VOID_2(void, glGenTextures, GLsizei, Texture*)
GLFN_RETURN_2(Attribute, glGetAttribLocation, Program, const GLchar*)
GLFN_VOID_4(void, glGetProgramInfoLog, Program, GLsizei, GLsizei*, GLchar*)
GLFN_VOID_3(void, glGetProgramiv, Program, GLenum, GLint*)
GLFN_VOID_4(void, glGetShaderInfoLog, Shader, GLsizei, GLsizei*, GLchar*)
GLFN_VOID_3(void, glGetShaderiv, Shader, GLenum, GLint*)
GLFN_RETURN_1(const GLubyte*, glGetString, GLenum)
GLFN_RETURN_2(Uniform, glGetUniformLocation, Program, const GLchar*)
GLFN_VOID_1(void, glLinkProgram, Program)
GLFN_VOID_4(void, glShaderSource, Shader, GLsizei, const GLchar* const*,
                                  const GLint*)
GLFN_VOID_9(void, glTexImage2D, GLenum, GLint, GLint, GLsizei, GLsizei, GLint,
                                GLenum, GLenum, const void*)
GLFN_VOID_3(void, glTexParameteri, GLenum, GLenum, GLint)
GLFN_VOID_9(void, glTexSubImage2D, GLenum, GLint, GLint, GLint, GLsizei,
                                   GLsizei, GLenum, GLenum, const void*)
GLFN_VOID_2(void, glUniform1i, Uniform, GLint)
GLFN_VOID_1(void, glUseProgram, Program)
GLFN_VOID_6(void, glVertexAttribPointer, Buffer, GLint, GLenum, GLboolean,
                                         GLsizei, const void*)
GLFN_VOID_4(void, glViewport, GLint, GLint, GLsizei, GLsizei)

#define GL_FALSE                          0x0000
#define GL_NO_ERROR                       0x0000
#define GL_TRIANGLES                      0x0004
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_TEXTURE_2D                     0x0DE1
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_FLOAT                          0x1406
#define GL_RGBA                           0x1908
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_LINEAR                         0x2601
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_COLOR_BUFFER_BIT               0x4000
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE0                       0x84C0
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

static const StringView
getErrorName(GLenum error) noexcept {
    switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:
            return "Unknown GL error";
    }
}

static void
customError(StringView call, StringView msg) noexcept {
    logFatal("GL", String() << call << ": " << msg);
}

static void
enumError(StringView call) noexcept {
    GLenum error = glGetError();
    StringView errorName = getErrorName(error);

    customError(call, errorName);
}

static void
checkError(StringView call) noexcept {
    if (glGetError()) {
        enumError(call);
    }
}

static void
getProcAddress(void** fnAddr, const char* symbolName) noexcept {
    // TODO: Remove SDL usage.
    *fnAddr = SDL_GL_GetProcAddress(symbolName);
    if (*fnAddr == 0) {
        logFatal("GL", String() << "Couldn't load GL function: " << symbolName);
    }
}

static void
compileShader(Shader shader) noexcept {
    glCompileShader_(shader);

    GLint status;
    glGetShaderiv_(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        GLchar* log = static_cast<GLchar*>(malloc(32768));
        glGetShaderInfoLog_(shader, 32768, 0, log);

        customError("glCompileShader",
                    String() << "Could not compile shader: " << log);
    }
}

static const char*
getString(GLenum name) noexcept {
    const GLubyte* str = glGetString_(name);
    if (str == 0) {
        customError("glGetString", "Returned NULL");
    }

    return reinterpret_cast<const char*>(str);
}

static void
linkProgram(Program program) noexcept {
    glLinkProgram_(program);

    GLint status;
    glGetProgramiv_(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        GLchar* log = static_cast<GLchar*>(malloc(32768));
        glGetProgramInfoLog_(program, 32768, 0, log);

        customError("glLinkProgram",
                    String() << "Could not link program: " << log);
    }
}

static GLuint
makeShader(GLenum type, const char* source) noexcept {
    Shader shader = glCreateShader_(type);
    if (shader == 0) {
        customError("glCreateShader", "Returned NULL");
    }

    const GLchar* sources[] = { static_cast<const GLchar*>(source) };
    glShaderSource_(shader, 1, sources, 0);

    compileShader(shader);

    return shader;
}

static GLuint
makeProgram(const char* vertexSource, const char* fragmentSource) noexcept {
    Program program = glCreateProgram_();

    glAttachShader_(program, makeShader(GL_VERTEX_SHADER, vertexSource));
    glAttachShader_(program, makeShader(GL_FRAGMENT_SHADER, fragmentSource));

    linkProgram(program);

    return program;
}

//
// Tsunagari-specific code
//

static const char*
vertexSource =
    "#version 110\n"
    "\n"
    "attribute vec3 aPosition;\n"
    "varying vec2 vTexCoord;\n"
    "\n"
    "void main() {\n"
    //"    vTexCoord = (aPosition + 1.0) / 2.0;\n"
    "    vTexCoord = vec2(0, 0);\n"
    "    gl_Position = vec4(aPosition, 1.0);\n"
    "}\n";

static const char*
fragmentSource =
    "#version 110\n"
    "\n"
    "varying vec2 vTexCoord;\n"
    "uniform sampler2D tAtlas;\n"
    "\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(tAtlas, vTexCoord);\n"
    "    gl_FragColor = vec4(1, 1, 1, 1);\n"
    "}\n";

#define ATLAS_WIDTH 2048
#define ATLAS_HEIGHT 512

static HashVector<TiledImage> images;
static size_t atlasUsed = 0;

static Attribute aPosition;
//static Attribute aTexCoord;
//static Uniform uResolution;
static Uniform uAtlas;
static VertexBuffer vbPosition = 1;
//static VertexBuffer vbTexCoord;
static Texture tAtlas;

static bool printed = false;

static void
printVersion() noexcept {
    if (printed) {
        return;
    }
    printed = true;

    logInfo("GL", String() << "Vendor: " << getString(GL_VENDOR));
    logInfo("GL", String() << "Renderer: "<< getString(GL_RENDERER));
    logInfo("GL", String() << "Version: " << getString(GL_VERSION) );
    logInfo("GL", String() << "Shading language version: "
                           << getString(GL_SHADING_LANGUAGE_VERSION));
    //logInfo("GL", String() << "Extensions: " << getString(GL_EXTENSIONS));
}

#define loadFunction(fn) getProcAddress((void**)&fn, #fn)

void
imageInit() noexcept {
    TimeMeasure m("Constructed OpenGL renderer");

    if (SDL_GL_CreateContext(sdl2Window) == 0) {
        sdlDie("SDL", "SDL_GL_CreateContext");
    }

    loadFunction(glActiveTexture);
    loadFunction(glAttachShader);
    loadFunction(glBindBuffer);
    loadFunction(glBindTexture);
    loadFunction(glBufferData);
    loadFunction(glClear);
    loadFunction(glClearColor);
    loadFunction(glCompileShader);
    loadFunction(glCreateProgram);
    loadFunction(glCreateShader);
    loadFunction(glDrawArrays);
    loadFunction(glEnableVertexAttribArray);
    loadFunction(glGenBuffers);
    loadFunction(glGenTextures);
    loadFunction(glGetAttribLocation);
    loadFunction(glGetError);
    loadFunction(glGetProgramInfoLog);
    loadFunction(glGetProgramiv);
    loadFunction(glGetShaderInfoLog);
    loadFunction(glGetShaderiv);
    loadFunction(glGetString);
    loadFunction(glGetUniformLocation);
    loadFunction(glLinkProgram);
    loadFunction(glShaderSource);
    loadFunction(glTexImage2D);
    loadFunction(glTexParameteri);
    loadFunction(glTexSubImage2D);
    loadFunction(glUniform1i);
    loadFunction(glUseProgram);
    loadFunction(glVertexAttribPointer);
    loadFunction(glViewport);

    Program program = makeProgram(vertexSource, fragmentSource);
    glUseProgram_(program);

    aPosition = glGetAttribLocation_(program, "aPosition");
    uAtlas = glGetUniformLocation_(program, "tAtlas");

    float width = static_cast<float>(windowWidth());
    float height = static_cast<float>(windowHeight());

    float positions[] = {
        0, 0, 0,
        0.5, 0, 0,
        0, height, 0,

        0, height, 0,
        0.5, 0, 0,
        0.5, height, 0,
    };

    /*
    float positions[] = {
        0, 0, 0,
        1, 0, 0,
        0, 1, 0,

        0, 1, 0,
        1, 0, 0,
        1, 1, 0,
    };
    */

    glBindBuffer_(GL_ARRAY_BUFFER, vbPosition);
    glBufferData_(GL_ARRAY_BUFFER, sizeof(positions), positions,
                  GL_STATIC_DRAW);

    glVertexAttribPointer_(aPosition, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray_(aPosition);

    glUniform1i_(uAtlas, 0);

    //glUniform2f_(uResolution, width, height);
    //glViewport_(0, 0, windowWidth(), windowHeight());

    //glViewport_(0, 0, 1, 1);

    glGenTextures_(1, &tAtlas);
    glActiveTexture_(GL_TEXTURE0);
    glBindTexture_(GL_TEXTURE_2D, tAtlas);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D_(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, 0);

    const uint8_t data[] = { 255, 255, 255, 255 };
    glTexImage2D_(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, &data);
}

static TiledImage*
load(StringView path) noexcept {
    TiledImage& tiles = images.allocate(hash_(path));
    tiles = {};

    Optional<StringView> r = Resources::load(path);
    if (!r) {
        // Error logged.
        return 0;
    }

    // FIXME: Do this at the resource level.
    //assert_(r->size < INT32_MAX);

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r->data)),
                          static_cast<int>(r->size));

    int x = atlasUsed;
    int y = 0;
    int width;
    int height;

    {
        TimeMeasure m(String() << "Constructed " << path << " as image");

        SDL_Surface* surface = IMG_Load_RW(ops, 1);
        //SDL_Surface* surface = SDL_LoadBMP_RW(ops, 1);
        if (!surface) {
            logFatal("SDL2", String() << "Invalid image: " << path);
            return 0;
        }

        width = surface->w;
        height = surface->h;

        // Rectangle packing algorithm:
        //
        // Copy surface into the atlas to the right of the previous image,
        // or at the left edge, if there was no previous image.
        /*
        glTexSubImage2D_(
            GL_TEXTURE_2D,  // target
            0,  // level
            x, // xoffset
            y, // yoffset
            width,  // width
            height,  // height
            GL_RGBA,  // format
            GL_UNSIGNED_BYTE,  // type
            surface->pixels  // data
        );
        */
        /*
        glTexImage2D_(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            surface->pixels
        );
        */

        SDL_FreeSurface(surface);
    }

    tiles.image = {
        reinterpret_cast<void*>(tAtlas),
        static_cast<uint32_t>(x),
        static_cast<uint32_t>(y),
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };

    atlasUsed += static_cast<uint32_t>(width);

    return &tiles;
}

Image
imageLoad(StringView path) noexcept {
    TiledImage* tiles = images.find(hash_(path));

    if (!tiles) {
        tiles = load(path);
    }

    assert_(tiles);

    return tiles->image;
}

void
imageRelease(Image image) noexcept {}

void
imageDraw(Image image, float x, float y, float z) noexcept {}

// TODO:
// glMatrixMode(GL_PROJECTION | GL_MODELVIEW);
// glLoadIdentity();
// glOrtho(-320,320,240,-240,0,1);

TiledImage
tilesLoad(StringView path, uint32_t tileWidth, uint32_t tileHeight) noexcept {
    TiledImage* tiles = images.find(hash_(path));

    if (!tiles) {
        tiles = load(path);
        tiles->tileWidth = tileWidth;
        tiles->tileHeight = tileHeight;
        tiles->numTiles = (tiles->image.width / tileWidth) *
                          (tiles->image.height / tileHeight);
    }

    assert_(tiles);

    return *tiles;
}

void
tilesRelease(TiledImage tiles) noexcept {}

Image
tileAt(TiledImage tiles, uint32_t index) noexcept {
    assert_(TILES_VALID(tiles));

    Image image = tiles.image;

    return {
        tiles.image.texture,
        tiles.image.x + tiles.tileWidth * index % tiles.image.width,
        tiles.image.y + tiles.tileWidth * index / tiles.image.width *
                        tiles.tileHeight,
        tiles.tileWidth,
        tiles.tileHeight,
    };
}

void
imagesPrune(time_t latestPermissibleUse) noexcept {}

void
imageDrawRect(float x1, float x2, float y1, float y2, uint32_t argb) noexcept {
}

void
imageStartFrame() noexcept {
    glClearColor_(0, 0, 0, 1);
    glClear_(GL_COLOR_BUFFER_BIT);
}

void
imageEndFrame() noexcept {
    glDrawArrays_(GL_TRIANGLES, 0, 6);
    SDL_GL_SwapWindow(sdl2Window);
}
