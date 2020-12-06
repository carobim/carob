/********************************
** Tsunagari Tile Engine       **
** io.h                        **
** Copyright 2020 Paul Merrill **
********************************/

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

#ifndef SRC_OS_UNIX_IO_H_
#define SRC_OS_UNIX_IO_H_

#include "util/int.h"
#include "util/noexcept.h"
#include "util/string-view.h"

class File {
 public:
    File(StringView path) noexcept;
    ~File() noexcept;

    // Whether the file was opened successfully.
    operator bool() noexcept;

    bool
    read(void* buf, size_t len) noexcept;

 public:
    int fd;
    size_t rem;  // 0 when EOF
};

bool
writeStdout(const char* buf, size_t len) noexcept;
bool
writeStderr(const char* buf, size_t len) noexcept;

#endif  // SRC_OS_UNIX_IO_H_
