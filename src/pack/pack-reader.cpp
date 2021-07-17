/*************************************
** Tsunagari Tile Engine            **
** pack-reader.cpp                  **
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

#include "pack/pack-reader.h"

#include "os/c.h"
#include "os/io.h"
#include "pack/layout.h"
#include "util/compiler.h"
#include "util/hashtable.h"
#include "util/int.h"
#include "util/new.h"

struct PackReader {
    PackReader(File file) noexcept
        : file(static_cast<File&&>(file)),
          lookupsConstructed(false) { }

    File file;

    HeaderSection header;
    BlobMetadata* metadata;
    char* paths;

    bool lookupsConstructed;
    Hashmap<StringView, uint32_t> lookups;
};

static void
constructLookups(PackReader* r) noexcept {
    for (uint32_t i = 0; i < r->header.blobCount; i++) {
        BlobMetadata meta = r->metadata[i];
        StringView path(r->paths + meta.pathOffset, meta.pathSize);
        r->lookups[path] = i;
    }
}

PackReader*
makePackReader(StringView path) noexcept {
    File file(path);
    if (!file) {
        return 0;
    }
    if (file.rem < sizeof(HeaderSection)) {
        return 0;
    }

    HeaderSection header;
    file.read(&header, sizeof(HeaderSection));
    if (memcmp(header.magic, PACK_MAGIC, sizeof(header.magic)) != 0) {
        return 0;
    }
    if (header.version != PACK_VERSION) {
        return 0;
    }

    if (file.rem < sizeof(BlobMetadata) * header.blobCount) {
        return 0;
    }
    BlobMetadata* metadata = xmalloc(BlobMetadata, header.blobCount);
    file.read(metadata, sizeof(BlobMetadata) * header.blobCount);

    uint32_t pathsSize = 0;
    for (size_t i = 0; i < header.blobCount; i++) {
        pathsSize += metadata[i].pathSize;
    }

    if (file.rem < pathsSize) {
        free(metadata);
        return 0;
    }
    char* paths = xmalloc(char, pathsSize);
    file.read(paths, pathsSize);

    PackReader* r = new PackReader(static_cast<File&&>(file));

    r->header = header;
    r->metadata = metadata;
    r->paths = paths;

    return r;
}

void
destroyReader(PackReader* r) noexcept {
    delete r;
}

uint32_t
readerSize(PackReader* r) noexcept {
    return r->header.blobCount;
}

uint32_t
readerIndex(PackReader* r, StringView path) noexcept {
    if (!r->lookupsConstructed) {
        r->lookupsConstructed = true;
        constructLookups(r);
    }

    Hashmap<StringView, uint32_t>::iterator it = r->lookups.find(path);
    if (it == r->lookups.end()) {
        return BLOB_NOT_FOUND;
    }
    else {
        return it->value;
    }
}

BlobDetails
readerDetails(PackReader* r, uint32_t index) noexcept {
    BlobMetadata meta = r->metadata[index];

    StringView path(r->paths + meta.pathOffset, meta.pathSize);
    uint32_t size = meta.uncompressedSize;

    return {
            path,
            size,
    };
}

bool
readerRead(PackReader* r, void* buf, uint32_t index) noexcept {
    BlobMetadata meta = r->metadata[index];

    uint32_t size = meta.compressedSize;
    uint32_t offset = r->header.dataOffset + meta.dataOffset;

    return r->file.readOffset(buf, size, offset);
}
