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
    Hashmap<StringView, U32> lookups;
};

static void
constructLookups(PackReader* r) noexcept {
    for (U32 i = 0; i < r->header.blobCount; i++) {
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

    U32 pathsSize = 0;
    for (Size i = 0; i < header.blobCount; i++) {
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

U32
readerSize(PackReader* r) noexcept {
    return r->header.blobCount;
}

U32
readerIndex(PackReader* r, StringView path) noexcept {
    if (!r->lookupsConstructed) {
        r->lookupsConstructed = true;
        constructLookups(r);
    }

    Hashmap<StringView, U32>::iterator it = r->lookups.find(path);
    if (it == r->lookups.end()) {
        return BLOB_NOT_FOUND;
    }
    else {
        return it->value;
    }
}

BlobDetails
readerDetails(PackReader* r, U32 index) noexcept {
    BlobMetadata meta = r->metadata[index];

    StringView path(r->paths + meta.pathOffset, meta.pathSize);
    U32 size = meta.uncompressedSize;

    BlobDetails details = {path, size};
    return details;
}

bool
readerRead(PackReader* r, void* buf, U32 index) noexcept {
    BlobMetadata meta = r->metadata[index];

    U32 size = meta.compressedSize;
    U32 offset = r->header.dataOffset + meta.dataOffset;

    return r->file.readOffset(buf, size, offset);
}
