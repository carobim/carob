#ifndef SRC_OS_WINDOWS_MAPPED_FILE_H_
#define SRC_OS_WINDOWS_MAPPED_FILE_H_

#include "os/c.h"
#include "util/compiler.h"
#include "util/string-view.h"

struct MappedFile {
    char* data;
    HANDLE mapping;
    HANDLE file;
};

bool
makeMappedFile(MappedFile& file, StringView path) noexcept;
void
destroyMappedFile(MappedFile file) noexcept;

#endif  // SRC_OS_WINDOWS_MAPPED_FILE_H_
