/*************************************
** Tsunagari Tile Engine            **
** walker.cpp                       **
** Copyright 2017-2021 Paul Merrill **
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

#include "pack/walker.h"

#include "os/os.h"
#include "util/compiler.h"
#include "util/queue.h"
#include "util/string-view.h"
#include "util/vector.h"

void
walk(Vector<StringView> paths,
     void* userData,
     void (*op)(void* userData, StringView path)) noexcept {
    Queue<String> files;

    for (StringView* path = paths.begin(); path != paths.end(); path++) {
        files.push(*path);
    }

    while (files.size) {
        String path = static_cast<String&&>(files.front());
        files.pop();

        // TODO: The call to isDir can be removed on Unix-like systems, which
        //       will save a stat(), as readdir() returns a file's type already.
        if (isDir(path)) {
            Vector<String> names = listDir(path);
            for (String* name = names.begin(); name != names.end(); name++) {
                String child;
                child << path << dirSeparator << *name;
                files.push(static_cast<String&&>(child));
            }
        }
        else {
            op(userData, path.view());
        }
    }
}
