/*************************************
** Tsunagari Tile Engine            **
** images.cpp                       **
** Copyright 2016-2021 Paul Merrill **
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
#include "core/client-conf.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/window.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/hashvector.h"
#include "util/int.h"
#include "util/random.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/transform.h"
#include "util/vector.h"

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
typedef float GLclampf;
typedef double GLdouble;
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

#define GLFN_VOID_0(rt, fn) \
    typedef rt (APIENTRY* fn##Proc)(); \
    static APICALL fn##Proc fn; \
    static rt fn##_() noexcept { \
        fn(); \
        checkError(#fn); \
    }

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
GLFN_VOID_2(void, glAlphaFunc, GLenum, GLclampf)
GLFN_VOID_2(void, glAttachShader, Program, Shader)
GLFN_VOID_2(void, glBindBuffer, GLenum, Buffer)
GLFN_VOID_2(void, glBindTexture, GLenum, Texture)
GLFN_VOID_2(void, glBlendFunc, GLenum, GLenum)
GLFN_VOID_4(void, glBufferData, GLenum, GLsizeiptr, const void*, GLenum)
GLFN_VOID_1(void, glClear, GLbitfield)
GLFN_VOID_4(void, glClearColor, GLfloat, GLfloat, GLfloat, GLfloat)
GLFN_VOID_1(void, glCompileShader, Shader)
GLFN_RETURN_0(Program, glCreateProgram)
GLFN_RETURN_1(Shader, glCreateShader, GLenum)
GLFN_VOID_1(void, glDisable, GLenum)
GLFN_VOID_3(void, glDrawArrays, GLenum, GLint, GLsizei)
GLFN_VOID_1(void, glEnable, GLenum)
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
GLFN_VOID_4(void, glUniformMatrix4fv, Uniform, GLsizei, GLboolean,
                                      const GLfloat*)
GLFN_VOID_1(void, glUseProgram, Program)
GLFN_VOID_6(void, glVertexAttribPointer, Buffer, GLint, GLenum, GLboolean,
                                         GLsizei, const void*)
GLFN_VOID_4(void, glViewport, GLint, GLint, GLsizei, GLsizei)

#define GL_FALSE                          0x0000
#define GL_NO_ERROR                       0x0000
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_DEPTH_BUFFER_BIT               0x0100
#define GL_NOTEQUAL                       0x0205
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_DEPTH_TEST                     0x0B71
#define GL_ALPHA_TEST                     0x0BC0
#define GL_BLEND                          0x0BE2
#define GL_TEXTURE_2D                     0x0DE1
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_PROJECTION                     0x1701
#define GL_RGBA                           0x1908
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_COLOR_BUFFER_BIT               0x4000
#define GL_BGRA                           0x80E1
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
enumError(StringView call, GLenum error) noexcept {
    StringView errorName = getErrorName(error);

    customError(call, errorName);
}

static void
checkError(StringView call) noexcept {
    GLenum error = glGetError();
    if (error) {
        enumError(call, error);
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

#define ATLAS_WIDTH 2048
#define ATLAS_HEIGHT 512

static HashVector<TiledImage> images;
static size_t atlasUsed = 0;

Texture tAtlas;

#define Z_NEAR_MAX "1024.0"
#define Z_FAR_MAX "-1024.0"

static const char*
imageVertexSource =
    "#version 110\n"
    "\n"
    "uniform mat4 uProjection;\n"
    "attribute vec3 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "\n"
    "void main() {\n"
    "    float near = " Z_NEAR_MAX ";\n"
    "    float far = " Z_FAR_MAX ";\n"
    "    float z = (aPosition.z - near) / (far - near);\n"
    "    vTexCoord = aTexCoord;\n"
    "    gl_Position = uProjection * vec4(aPosition.xy, z, 1.0);\n"
    "}\n";

static const char*
imageFragmentSource =
    "#version 110\n"
    "\n"
    "uniform sampler2D uAtlas;\n"
    "varying vec2 vTexCoord;\n"
    "\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(uAtlas, vTexCoord);\n"
    "}\n";

struct ImageVertex {
    fvec3 position;
    fvec2 texCoord;
};

struct ImageProgram {
    Program program;
    Uniform uAtlas;
    Uniform uProjection;
    Attribute aPosition;
    Attribute aTexCoord;

    Vector<ImageVertex> attributes;
};

static const char*
rectVertexSource =
    "#version 110\n"
    "\n"
    "uniform mat4 uProjection;\n"
    "attribute vec3 aPosition;\n"
    "attribute vec4 aColor;\n"
    "varying vec4 vColor;\n"
    "\n"
    "void main() {\n"
    "    float near = " Z_NEAR_MAX ";\n"
    "    float far = " Z_FAR_MAX ";\n"
    "    float z = (aPosition.z - near) / (far - near);\n"
    "    vColor = aColor;\n"
    "    gl_Position = uProjection * vec4(aPosition.xy, z, 1.0);\n"
    "}\n";

static const char*
rectFragmentSource =
    "#version 110\n"
    "\n"
    "varying vec4 vColor;\n"
    "\n"
    "void main() {\n"
    "    gl_FragColor = vColor;\n"
    "}\n";

struct RectVertex {
    fvec3 position;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct RectProgram {
    Program program;
    Uniform uProjection;
    Attribute aColor;
    Attribute aPosition;

    Vector<RectVertex> attributes;
};

static VertexBuffer vertexBuffer = 1;
static ImageProgram ip;
static RectProgram rp;

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
    loadFunction(glAlphaFunc);
    loadFunction(glAttachShader);
    loadFunction(glBindBuffer);
    loadFunction(glBindTexture);
    loadFunction(glBlendFunc);
    loadFunction(glBufferData);
    loadFunction(glClear);
    loadFunction(glClearColor);
    loadFunction(glCompileShader);
    loadFunction(glCreateProgram);
    loadFunction(glCreateShader);
    loadFunction(glDisable);
    loadFunction(glDrawArrays);
    loadFunction(glEnable);
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
    loadFunction(glUniformMatrix4fv);
    loadFunction(glUseProgram);
    loadFunction(glVertexAttribPointer);
    loadFunction(glViewport);

    glEnable_(GL_BLEND);
    glBlendFunc_(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable_(GL_DEPTH_TEST);

    // Deprecated in OpenGL 3.0 and replaced in 3.1 with the "discard" GLSL
    // operation.
    // TODO: Test for availability with GL_ALPHA_TEST.
    glEnable_(GL_ALPHA_TEST);
    glAlphaFunc_(GL_NOTEQUAL, 0.0f);

    glGenTextures_(1, &tAtlas);
    glActiveTexture_(GL_TEXTURE0);
    glBindTexture_(GL_TEXTURE_2D, tAtlas);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D_(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, 0);

    ip.program = makeProgram(imageVertexSource, imageFragmentSource);
    ip.uAtlas = glGetUniformLocation_(ip.program, "uAtlas");
    ip.uProjection = glGetUniformLocation_(ip.program, "uProjection");
    ip.aPosition = glGetAttribLocation_(ip.program, "aPosition");
    ip.aTexCoord = glGetAttribLocation_(ip.program, "aTexCoord");
    glUseProgram_(ip.program);
    glUniform1i_(ip.uAtlas, 0);

    rp.program = makeProgram(rectVertexSource, rectFragmentSource);
    rp.aColor = glGetAttribLocation_(rp.program, "aColor");
    rp.aPosition = glGetAttribLocation_(rp.program, "aPosition");
    rp.uProjection = glGetUniformLocation_(rp.program, "uProjection");
}

static TiledImage*
load(StringView path) noexcept {
    TiledImage& tiles = images.allocate(hash_(path));
    tiles = {};

    String r;
    if (!resourceLoad(path, r)) {
        // Error logged.
        return 0;
    }

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r.data)),
                          static_cast<int>(r.size));

    int x = atlasUsed;
    int y = 0;
    int width;
    int height;

    {
        TimeMeasure m(String() << "Constructed " << path << " as image");

        //SDL_Surface* surface = IMG_Load_RW(ops, 1);
        SDL_Surface* surface = SDL_LoadBMP_RW(ops, 1);
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
        glTexSubImage2D_(
            GL_TEXTURE_2D,  // target
            0,  // level
            x,  // xoffset
            y,  // yoffset
            width,  // width
            height,  // height
            GL_BGRA,  // format
            GL_UNSIGNED_BYTE,  // type
            surface->pixels  // data
        );
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
imageDraw(Image image, float x, float y, float z) noexcept {
    fvec2 trans = sdl2Translation;
    fvec2 scale = sdl2Scaling;

    float yTop    = scale.y * (trans.y + y);
    float yBottom = scale.y * (trans.y + y + image.height);
    float xLeft   = scale.x * (trans.x + x);
    float xRight  = scale.x * (trans.x + x + image.width);

    size_t offset = ip.attributes.size;

    const size_t QUAD_COORDS = 6;

    if (ip.attributes.capacity == 0) {
        ip.attributes.reserve(QUAD_COORDS * 8);
    }
    else if (ip.attributes.size + QUAD_COORDS > ip.attributes.capacity) {
        ip.attributes.reserve(ip.attributes.capacity * 2);
    }
    ip.attributes.size += QUAD_COORDS;

    float tWidth = ATLAS_WIDTH;
    float tHeight = ATLAS_HEIGHT;

    float vTop    =  image.y                 / tHeight;
    float vBottom = (image.y + image.height) / tHeight;
    float uLeft   =  image.x                 / tWidth;
    float uRight  = (image.x + image.width)  / tWidth;

    ip.attributes[offset+0] = { {xLeft,  yBottom, z}, {uLeft,  vBottom} };
    ip.attributes[offset+1] = { {xRight, yBottom, z}, {uRight, vBottom} };
    ip.attributes[offset+2] = { {xLeft,  yTop,    z}, {uLeft,  vTop   } };

    ip.attributes[offset+3] = { {xRight, yBottom, z}, {uRight, vBottom} };
    ip.attributes[offset+4] = { {xLeft,  yTop,    z}, {uLeft,  vTop   } };
    ip.attributes[offset+5] = { {xRight, yTop,    z}, {uRight, vTop   } };
}

TiledImage
tilesLoad(StringView path,
          uint32_t tileWidth,
          uint32_t tileHeight,
          uint32_t numAcross,
          uint32_t numHigh) noexcept {
    TiledImage* tiles = images.find(hash_(path));

    if (!tiles) {
        tiles = load(path);

        assert_(tiles->image.width == tileWidth * numAcross);
        assert_(tiles->image.height == tileHeight * numHigh);

        tiles->tileWidth = tileWidth;
        tiles->tileHeight = tileHeight;
        tiles->numTiles = numAcross * numHigh;
    }

    return *tiles;
}

void
tilesRelease(TiledImage tiles) noexcept {}

Image
tileAt(TiledImage tiles, uint32_t index) noexcept {
    assert_(TILES_VALID(tiles));

    Image image = tiles.image;

    return {
        image.texture,
        image.x + tiles.tileWidth * index % image.width,
        image.y + tiles.tileWidth * index / image.width * tiles.tileHeight,
        tiles.tileWidth,
        tiles.tileHeight,
    };
}

void
imagesPrune(time_t latestPermissibleUse) noexcept {}

void
imageDrawRect(float left, float right, float top, float bottom, float z,
              uint32_t argb) noexcept {
    fvec2 trans = sdl2Translation;
    fvec2 scale = sdl2Scaling;

    float yTop    = scale.y * (trans.y + top);
    float yBottom = scale.y * (trans.y + bottom);
    float xLeft   = scale.x * (trans.x + left);
    float xRight  = scale.x * (trans.x + right);

    size_t offset = rp.attributes.size;

    const size_t QUAD_COORDS = 6;

    if (rp.attributes.capacity == 0) {
        rp.attributes.reserve(QUAD_COORDS * 8);
    }
    else if (rp.attributes.size + QUAD_COORDS > rp.attributes.capacity) {
        rp.attributes.reserve(rp.attributes.capacity * 2);
    }
    rp.attributes.size += QUAD_COORDS;

    uint8_t a = (argb >> 24) & 0xFF;
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >>  8) & 0xFF;
    uint8_t b = (argb >>  0) & 0xFF;

    rp.attributes[offset+0] = { {xLeft,  yBottom, z}, r, g, b, a };
    rp.attributes[offset+1] = { {xRight, yBottom, z}, r, g, b, a };
    rp.attributes[offset+2] = { {xLeft,  yTop,    z}, r, g, b, a };

    rp.attributes[offset+3] = { {xRight, yBottom, z}, r, g, b, a };
    rp.attributes[offset+4] = { {xLeft,  yTop,    z}, r, g, b, a };
    rp.attributes[offset+5] = { {xRight, yTop,    z}, r, g, b, a };
}

void
imageStartFrame() noexcept {
    // FIXME: Uses lots of CPU on macOS. Replace with adding black borders
    //        around play area.
    glClearColor_(0, 0, 0, 1);
    glClear_(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static Transform
getOrtho() noexcept {
    float ww = static_cast<float>(confWindowSize.x);
    float wh = static_cast<float>(confWindowSize.y);

    return transformMultiply(transformScale(2.0f / ww, -2.0f / wh), transformTranslate(-1, 1));
}

void
imageFlushImages() noexcept {
    if (ip.attributes.size == 0) {
        return;
    }

    glUseProgram_(ip.program);

    glBindBuffer_(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData_(GL_ARRAY_BUFFER, ip.attributes.size * sizeof(ImageVertex),
                  ip.attributes.data, GL_STATIC_DRAW);

    glEnableVertexAttribArray_(ip.aPosition);
    glEnableVertexAttribArray_(ip.aTexCoord);

    glVertexAttribPointer_(ip.aPosition, 3, GL_FLOAT, false,
                           sizeof(ImageVertex),
                           &static_cast<ImageVertex*>(0)->position);
    glVertexAttribPointer_(ip.aTexCoord, 2, GL_FLOAT, false,
                           sizeof(ImageVertex),
                           &static_cast<ImageVertex*>(0)->texCoord);

    glUniformMatrix4fv_(ip.uProjection, 1, false, getOrtho().m);

    glEnable_(GL_DEPTH_TEST);
    glDrawArrays_(GL_TRIANGLES, 0, ip.attributes.size);

    ip.attributes.size = 0;
}

void
imageFlushRects() noexcept {
    if (rp.attributes.size == 0) {
        return;
    }

    glUseProgram_(rp.program);

    glBindBuffer_(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData_(GL_ARRAY_BUFFER, rp.attributes.size * sizeof(RectVertex),
                  rp.attributes.data, GL_STATIC_DRAW);

    glEnableVertexAttribArray_(rp.aPosition);
    glEnableVertexAttribArray_(rp.aColor);

    glVertexAttribPointer_(rp.aPosition, 3, GL_FLOAT, false,
                           sizeof(RectVertex),
                           &static_cast<RectVertex*>(0)->position);
    glVertexAttribPointer_(rp.aColor, 4, GL_UNSIGNED_BYTE, true,
                           sizeof(RectVertex),
                           &static_cast<RectVertex*>(0)->r);

    glUniformMatrix4fv_(rp.uProjection, 1, false, getOrtho().m);

    glDisable_(GL_DEPTH_TEST);
    glDrawArrays_(GL_TRIANGLES, 0, rp.attributes.size);

    rp.attributes.size = 0;
}

void
imageEndFrame() noexcept {
    SDL_GL_SwapWindow(sdl2Window);
}
