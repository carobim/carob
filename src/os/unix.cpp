/*************************************
** Tsunagari Tile Engine            **
** unix.cpp                         **
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

#include "os/c.h"
#include "os/os.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

const char dirSeparator = '/';

Filesize
getFileSize(StringView path) noexcept {
    struct stat status;
    if (stat(String(path).null(), &status)) {
        return FS_ERROR;
    }
    return static_cast<Filesize>(status.st_size);
}

bool
isDir(StringView path) noexcept {
    struct stat status;
    if (stat(String(path).null(), &status)) {
        return false;
    }
    return S_ISDIR(status.st_mode);
}

void
makeDirectory(StringView path) noexcept {
    mkdir(String(path).null(), 0777);
}

Vector<String>
listDir(StringView path) noexcept {
    DIR* dir = 0;
    struct dirent* entry = 0;
    Vector<String> names;

    if ((dir = opendir(String(path).null())) == 0) {
        return names;
    }

    // FIXME: Replace with reentrant function calls.
    while ((entry = readdir(dir)) != 0) {
        if (entry->d_ino == 0) {
            // Ignore unlinked files.
            continue;
        }
        if (entry->d_name[0] == '.') {
            // Ignore hidden files and directories.
            continue;
        }
        if ((entry->d_type & (DT_DIR | DT_REG)) == 0) {
            // Ignore odd files.
            continue;
        }
        names.push_back(entry->d_name);
    }

    closedir(dir);

    return names;
}

bool
writeFile(StringView path, uint32_t length, void* data) noexcept {
    int fd = open(String(path).null(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return false;
    }
    ssize_t written = write(fd, data, length);
    if (written != length) {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

bool
writeFileVec(StringView path,
             uint32_t count,
             uint32_t* lengths,
             void** datas) noexcept {
    int fd = open(String(path).null(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return false;
    }

    ssize_t total = 0;
    Vector<iovec> ios;

    ios.reserve(count);
    for (size_t i = 0; i < count; i++) {
        total += lengths[i];
        ios.push_back({datas[i], lengths[i]});
    }

    ssize_t written = writev(fd, ios.data, static_cast<int>(ios.size));
    if (written != total) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool
readFile(StringView path, String& data) noexcept {
    Filesize size = getFileSize(path);
    if (size == FS_ERROR) {
        return false;
    }

    String path_;
    path_.reserve(path.size + 1);
    path_ << path;

    int fd = open(path_.null(), O_RDONLY);
    if (fd < 0) {
        return false;
    }

    struct stat status;
    if (fstat(fd, &status)) {
        // errno set
        close(fd);
        return false;
    }

    size_t rem = status.st_size;
    data.reserve(size + 1);
    data.resize(size);

    ssize_t nbytes = read(fd, data.data, size);
    if (nbytes < 0 || nbytes != size) {
        close(fd);
        return false;
    }

    // Add NUL terminator.
    data.data[size] = 0;

    close(fd);
    return true;
}

static bool
isaTTY() noexcept {
#if defined(__EMSCRIPTEN__)
    return false;
#else
    static bool checked = false;
    static bool tty = false;

    if (!checked) {
        checked = true;
        tty = isatty(0) != 0;
    }
    return tty;
#endif
}

void
setTermColor(TermColor color, Output& out) noexcept {
    if (!isaTTY()) {
        return;
    }

    // VT100 terminal control codes from:
    //   http://www.termsys.demon.co.uk/vtansi.htm

    const char escape = 27;

    switch (color) {
    case TC_RESET:
        out << "\033[0m";
        break;
    case TC_GREEN:
        out << "\033[32m";
        break;
    case TC_YELLOW:
        out << "\033[33m";
        break;
    case TC_RED:
        out << "\033[31m";
        break;
    }
}

void
exitProcess(int code) noexcept {
    _exit(code);
}
