/*************************************
** Tsunagari Tile Engine            **
** os.h                             **
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

#ifndef SRC_OS_OS_H_
#define SRC_OS_OS_H_

#include "util/int.h"
#include "util/io.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

// FIXME: Break up disk IO from terminal IO from process managment into separate
//        files.

extern const char dirSeparator;

typedef uint64_t Filesize;

#define FS_ERROR UINT64_MAX

Filesize
getFileSize(StringView path) noexcept;
bool
writeFile(StringView path, uint32_t length, void* data) noexcept;
bool
writeFileVec(StringView path,
             uint32_t count,
             uint32_t* lengths,
             void** datas) noexcept;
bool
isDir(StringView path) noexcept;
void
makeDirectory(StringView path) noexcept;
Vector<String>
listDir(StringView path) noexcept;
bool
readFile(StringView path, String& data) noexcept;

enum TermColor {
    TC_RESET,
    TC_GREEN,
    TC_YELLOW,
    TC_RED,
};

void
setTermColor(TermColor color, Output& out) noexcept;

void
exitProcess(int code) noexcept;

#ifdef _WIN32
#include "windows.h"
#endif

#endif  // SRC_OS_OS_H_
