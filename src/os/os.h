#ifndef SRC_OS_OS_H_
#define SRC_OS_OS_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

// FIXME: Break up disk IO from terminal IO from process managment into separate
//        files.

extern const char dirSeparator;

typedef U64 Filesize;

#define FS_ERROR UINT64_MAX

Filesize
getFileSize(StringView path) noexcept;
bool
writeFile(StringView path, U32 length, void* data) noexcept;
bool
writeFileVec(StringView path, U32 count, U32* lengths,
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
    TC_RED
};

void
setTermColor(TermColor color, Output& out) noexcept;

void
exitProcess(int code) noexcept;

#if MSVC
#    include "windows.h"
#endif

#endif  // SRC_OS_OS_H_
