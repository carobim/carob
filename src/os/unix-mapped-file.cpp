/*************************************
** Tsunagari Tile Engine            **
** unix-mapped-file.cpp             **
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

#include "os/unix-mapped-file.h"

#include "os/c.h"
#include "util/string-view.h"
#include "util/string.h"

bool
makeMappedFile(MappedFile& file, StringView path) noexcept {
    int fd = open(String(path).null(), O_RDONLY);
    if (fd == -1) {
        return false;
    }

    struct stat st;
    if (fstat(fd, &st)) {
        close(fd);
        return false;
    }

    if (st.st_size == 0) {
        close(fd);
        return false;
    }

    size_t size = static_cast<size_t>(st.st_size);
    void* data = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        close(fd);
        return false;
    }

    // FIXME: Close the fd now or at destruction?
    file.data = static_cast<char*>(data);
    file.size = size;
    file.fd = fd;
    return true;
}

void
destroyMappedFile(MappedFile file) noexcept {
    if (file.data != MAP_FAILED) {
        munmap(file.data, file.size);
        close(file.fd);
    }
}
