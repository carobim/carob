/********************************
** Tsunagari Tile Engine       **
** io.cpp                      **
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

#include "os/unix/io.h"

#include "os/c.h"
#include "util/int.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"

File::File(StringView path) noexcept {
    fd = open(String(path).null(), O_RDONLY);
    if (fd < 0) {
        // errno set
        return;
    }

    struct stat status;
    if (fstat(fd, &status)) {
        // errno set
        close(fd);
        fd = -1;
        return;
    }

    rem = status.st_size;
}

File::~File() noexcept {
    if (fd >= 0) {
        if (close(fd)) {
            // errno set
        }
    }
}

File::operator bool() noexcept {
    return fd >= 0;
}

bool
File::read(void* buf, size_t len) noexcept {
    ssize_t nbytes = ::read(fd, buf, len);
    if (nbytes < 0) {
        // errno set
        return false;
    }
    if (nbytes != len) {
        return false;
    }
    rem -= nbytes;
    return true;
}

bool
writeStdout(const char* buf, size_t len) noexcept {
    ssize_t written = write(1, buf, len);
    if (written < 0) {
        // errno set
        return false;
    }
    if (written != len) {
        return false;
    }
    return true;
}

bool
writeStderr(const char* buf, size_t len) noexcept {
    ssize_t written = write(2, buf, len);
    if (written < 0) {
        // errno set
        return false;
    }
    if (written != len) {
        return false;
    }
    return true;
}
