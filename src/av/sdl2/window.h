/**********************************
** Tsunagari Tile Engine         **
** window.h                      **
** Copyright 2016 Paul Merrill   **
**********************************/

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

#ifndef SRC_AV_SDL2_WINDOW_H_
#define SRC_AV_SDL2_WINDOW_H_

#include <chrono>
#include <map>
#include <string>

#include <SDL.h>

#include "core/window.h"

class SDL2GameWindow : public GameWindow {
 public:
    SDL2GameWindow();
    ~SDL2GameWindow() = default;

    bool init();

    unsigned width() const;

    unsigned height() const;

    void setCaption(const std::string& caption);

    void mainLoop();
    void handleEvents();
    void handleEvent(const SDL_Event& event);

    void drawRect(double x1, double x2, double y1, double y2,
            uint32_t argb);

    void scale(double x, double y);
    void translate(double x, double y);
    void clip(double x, double y, double width, double height);

    void close();

    std::chrono::time_point<std::chrono::steady_clock> start;

 private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};

#endif  // SRC_AV_SDL2_WINDOW_H_
