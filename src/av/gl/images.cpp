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

#include "av/sdl2/sdl2.h"
#include "av/sdl2/window.h"
#include "core/log.h"
#include "util/int.h"
#include "util/noexcept.h"
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
typedef char GLchar;
typedef unsigned GLenum;
typedef int GLint;
typedef float GLfloat;
typedef int GLsizei;
typedef uint8_t GLubyte;
typedef unsigned GLuint;

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

#define GLFN_RETURN_1(rt, fn, t1) \
    typedef rt (APIENTRY* fn##Proc)(t1); \
    static APICALL fn##Proc fn; \
    static rt fn##_(t1 a) noexcept { \
        rt x = fn(a); \
        checkError(#fn); \
        return x; \
    }

GLFN_VOID_1  (void, glClear,               GLbitfield)
GLFN_VOID_4  (void, glClearColor,          GLfloat, GLfloat, GLfloat, GLfloat)
GLFN_VOID_1  (void, glCompileShader,       GLuint)
GLFN_RETURN_1(GLuint, glCreateShader,      GLenum)
GLFN_VOID_4  (void, glGetShaderInfoLog,    GLuint, GLsizei, GLsizei*, GLchar*)
GLFN_VOID_3  (void, glGetShaderiv,         GLuint, GLenum, GLint*)
GLFN_RETURN_1(const GLubyte*, glGetString, GLenum)
GLFN_VOID_4  (void, glShaderSource,        GLuint,
                                           GLsizei,
                                           const GLchar* const*,
                                           const GLint*)

#define GL_FALSE                          0
#define GL_NO_ERROR                       0
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_COLOR_BUFFER_BIT               0x4000
#define GL_COMPILE_STATUS                 0x8B81
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
compileShader(GLuint shader) noexcept {
    glCompileShader_(shader);

    GLint status;
    glGetShaderiv_(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        GLchar* infoLog = static_cast<GLchar*>(malloc(32768));
        glGetShaderInfoLog_(shader, 32768, 0, infoLog);

        customError("glCompileShader",
                    String() << "Could not compile shader: " << infoLog);
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

static GLuint
loadShader(GLenum type, const char* source) noexcept {
    GLuint shader = glCreateShader_(type);
    if (shader == 0) {
        customError("glCreateShader", "Returned NULL");
    }
    const GLchar* sources[] = {
        static_cast<const GLchar*>(source)
    };
    glShaderSource_(shader, 1, sources, 0);
    compileShader(shader);
    return shader;
}

//
// Tsunagari-specific code
//

typedef GLint Attribute;
typedef GLint VertexUniform;
typedef GLint FragmentUniform;
typedef GLuint VertexBuffer;
typedef GLuint Texture;

static Attribute aPosition;
static Attribute aTexcoord;
static VertexUniform vuResolution;
static FragmentUniform fuAtlasTexture;
static VertexBuffer vbPosition;
static VertexBuffer vbTexcoord;
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

static TiledImage global = { { &global, 0, 0, 1, 1 }, 1, 1, 1000 };

#define loadFunction(fn) getProcAddress((void**)&fn, #fn)

void
imageInit() noexcept {
    SDL_GL_CreateContext(sdl2Window);

    loadFunction(glClear);
    loadFunction(glClearColor);
    loadFunction(glCompileShader);
    loadFunction(glCreateShader);
    loadFunction(glGetError);
    loadFunction(glGetShaderInfoLog);
    loadFunction(glGetShaderiv);
    loadFunction(glGetString);
    loadFunction(glShaderSource);
}

Image
imageLoad(StringView path) noexcept {
    return global.image;
}

void
imageRelease(Image image) noexcept {}

void
imageDraw(Image image, float x, float y, float z) noexcept {}

TiledImage
tilesLoad(StringView path, uint32_t tileWidth, uint32_t tileHeight) noexcept {
    printVersion();
    return global;
}

void
tilesRelease(TiledImage tiles) noexcept {}

Image
tileAt(TiledImage tiles, uint32_t index) noexcept {
    return global.image;
}

void
imagesPrune(time_t latestPermissibleUse) noexcept {}

void
imageDrawRect(float x1, float x2, float y1, float y2, uint32_t argb) noexcept {
}

void
imageStartFrame() noexcept {
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void
imageEndFrame() noexcept {
    SDL_GL_SwapWindow(sdl2Window);
}
