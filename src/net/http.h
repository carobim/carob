#ifndef HTTP_H_
#define HTTP_H_

#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"

struct Header {
    StringView name;
    StringView value;
};

struct FormData {
    StringView name;
    StringView value;
};

struct Http;

Http*
httpMake(bool verbose = false) noexcept;
void
httpDestroy(Http* self) noexcept;

void
httpSetCookie(Http* self, String cookie) noexcept;

StringView
httpGet(Http* self, String* response, StringView url, Header* headers,
        Size headersSize) noexcept;

StringView
httpForm(Http* self, String* response, StringView url, Header* headers,
         Size headersSize, FormData* data, Size dataSize) noexcept;

#endif
