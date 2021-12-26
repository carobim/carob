#ifndef SRC_OS_UNIX_MAPPED_FILE_H_
#define SRC_OS_UNIX_MAPPED_FILE_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

struct MappedFile {
    char* data;
    Size size;
    I32 fd;
};

bool
makeMappedFile(MappedFile& file, StringView path) noexcept;
void
destroyMappedFile(MappedFile file) noexcept;

#endif  // SRC_OS_UNIX_MAPPED_FILE_H_
