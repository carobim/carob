/*************************************
** Tsunagari Tile Engine            **
** gosu.h                           **
** Copyright 2019-2020 Paul Merrill **
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

#ifndef SRC_AV_GOSU_GOSU_H_
#define SRC_AV_GOSU_GOSU_H_

#include <math.h>

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#define SRC_OS_C_H_
#define SRC_UTIL_INT_H_
#define SRC_UTIL_NEW_H_

#include "util/constexpr.h"
#include "util/noexcept.h"

namespace Gosu {

    // Gosu/Buttons.h
    enum ButtonName {
        KB_ESCAPE = 41,
        KB_SPACE = 44,
        KB_LEFT = 80,
        KB_RIGHT = 79,
        KB_UP = 82,
        KB_DOWN = 81,
        KB_LEFT_SHIFT = 225,
        KB_RIGHT_SHIFT = 229,
        KB_LEFT_CONTROL = 224,
        KB_RIGHT_CONTROL = 228,

        KB_HIGHEST = KB_RIGHT_SHIFT,
    };

    // Gosu/Color.h
    class Color {
     public:
        Color(uint32_t rep) noexcept : rep(rep) {}

        uint32_t rep;
    };

    // Gosu/GraphicsBase.h
    enum AlphaMode { AM_DEFAULT };
    enum ImageFlags {
        IF_TILEABLE_LEFT = 1 << 1,
        IF_TILEABLE_TOP = 1 << 2,
        IF_TILEABLE_RIGHT = 1 << 3,
        IF_TILEABLE_BOTTOM = 1 << 4,
        IF_TILEABLE = IF_TILEABLE_LEFT | IF_TILEABLE_TOP | IF_TILEABLE_RIGHT |
                      IF_TILEABLE_BOTTOM,
        IF_RETRO = 1 << 5
    };
    typedef double ZPos;
    typedef std::array<double, 16> Transform;
    Transform
    translate(double, double) noexcept;
    Transform
    scale(double, double, double, double) noexcept;

    // Gosu/Graphics.h
    class Graphics {
     public:
        static void
        clip_to(double,
                double,
                double,
                double,
                const std::function<void()>&) noexcept;
        static void
        transform(const Transform&, const std::function<void()>&) noexcept;
        static void
        draw_quad(double,
                  double,
                  Color,
                  double,
                  double,
                  Color,
                  double,
                  double,
                  Color,
                  double,
                  double,
                  Color,
                  ZPos,
                  AlphaMode) noexcept;

     private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

    // Gosu/Input.h
    typedef unsigned Button;

    // Gosu/IO.h
    class Resource {
     public:
        virtual ~Resource() noexcept = default;
        virtual size_t
        size() noexcept = 0;
        virtual void resize(size_t) noexcept = 0;
        virtual void
        read(size_t, size_t, void*) noexcept = 0;
        virtual void
        write(size_t, size_t, const void*) noexcept = 0;
    };
    class Reader {
     public:
        Reader(Resource* res, size_t pos) noexcept : res(res), pos(pos) {}

        Resource* res;
        size_t pos;
    };

    // Gosu/Window.h
    class Window {
     public:
        Window(unsigned, unsigned, bool, double) noexcept;
        virtual ~Window() noexcept;
        unsigned
        width() const noexcept;
        unsigned
        height() const noexcept;
        void
        set_caption(const std::string&) noexcept;
        virtual void
        show() noexcept;
        virtual bool
        tick() noexcept;
        virtual void
        close() noexcept;
        virtual void
        update() noexcept {}
        virtual void
        draw() noexcept {}
        virtual bool
        needs_redraw() const noexcept {
            return true;
        }
        virtual bool
        needs_cursor() const noexcept {
            return false;
        }
        virtual void
        lose_focus() noexcept {}
        virtual void
        release_memory() noexcept {}
        virtual void button_down(Button) noexcept;
        virtual void
        button_up(Button) noexcept {}
        virtual void
        drop(/* const std::string& filename */) noexcept {}
        virtual void
        touch_began(/* Touch touch */) noexcept {}
        virtual void
        touch_moved(/* Touch touch */) noexcept {}
        virtual void
        touch_ended(/* Touch touch */) noexcept {}
        virtual void
        touch_cancelled(/* Touch touch */) noexcept {}
        Graphics&
        graphics() noexcept;

     private:
        struct Impl;
        const std::unique_ptr<Impl> pimpl;
    };
    unsigned
    screen_width(Window*) noexcept;
    unsigned
    screen_height(Window*) noexcept;

    // Gosu/Bitmap.h
    class Bitmap {
     public:
        Bitmap() : w(0), h(0) {}

        unsigned w, h;
        std::vector<Color> pixels;
    };
    void
    load_image_file(Bitmap&, Reader) noexcept;

    // Gosu/Image.h
    class ImageData {
     public:
        virtual ~ImageData() noexcept;
    };
    class Image {
     public:
        Image(const Image&) noexcept =
                default;  // Not in Gosu, but should be okay.

        explicit Image(const Bitmap&, unsigned) noexcept;
        Image(const Bitmap&,
              unsigned,
              unsigned,
              unsigned,
              unsigned,
              unsigned) noexcept;
        unsigned
        width() const noexcept;
        unsigned
        height() const noexcept;
        void
        draw(double, double, ZPos, double, double, Color c, AlphaMode mode)
                const noexcept;

     private:
        std::shared_ptr<ImageData> data;
    };
    std::vector<Image>
    load_tiles(const Bitmap&, int, int, unsigned) noexcept;

}  // namespace Gosu

#endif  // SRC_AV_GOSU_GOSU_H_
