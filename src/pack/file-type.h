#ifndef SRC_PACK_FILE_TYPE_H_
#define SRC_PACK_FILE_TYPE_H_

#include "util/compiler.h"
#include "util/string-view.h"

enum FileType { FT_TEXT, FT_UNKNOWN, FT_MEDIA };

FileType
determineFileType(StringView path) noexcept;

#endif  // SRC_PACK_FILE_TYPE_H_
