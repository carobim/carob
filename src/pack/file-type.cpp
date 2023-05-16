#include "pack/file-type.h"

#include "util/compiler.h"
#include "util/string-view.h"

static const StringView textExtensions[] = {".json"};

static const StringView mediaExtensions[] = {".bmp", ".oga", ".png"};

FileType
determineFileType(StringView path) noexcept {
    StringPosition dot = path.rfind('.');
    if (dot == SV_NOT_FOUND)
        return FT_UNKNOWN;
    StringView extension = path.substr(dot);
    for (const StringView* text = &textExtensions[0];
         text !=
         &textExtensions[sizeof(textExtensions) / sizeof(textExtensions[0])];
         text++) {
        if (extension == *text)
            return FT_TEXT;
    }
    for (const StringView* text = &mediaExtensions[0];
         text !=
         &mediaExtensions[sizeof(mediaExtensions) / sizeof(mediaExtensions[0])];
         text++) {
        if (extension == *text)
            return FT_MEDIA;
    }
    return FT_UNKNOWN;
}
