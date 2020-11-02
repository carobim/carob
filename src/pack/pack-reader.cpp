/*************************************
** Tsunagari Tile Engine            **
** pack-reader.cpp                  **
** Copyright 2016-2020 Paul Merrill **
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
#include "os/mapped-file.h"
#include "util/constexpr.h"
#include "util/hashtable.h"
#include "util/int.h"
#include "util/new.h"
#include "util/noexcept.h"

//                                       "T   s    u    n    a   g    a   r"
static constexpr uint8_t PACK_MAGIC[8] = {84, 115, 117, 110, 97, 103, 97, 114};

static constexpr uint8_t PACK_VERSION = 1;

struct HeaderBlock {
    uint8_t magic[8];
    uint8_t version;
    uint8_t unused[7];
    PackReader::BlobIndex blobCount;
    uint32_t pathOffsetsBlockOffset;
    uint32_t pathsBlockOffset;
    uint32_t pathsBlockSize;
    uint32_t metadataBlockOffset;
    uint32_t dataOffsetsBlockOffset;
};

typedef uint32_t PathOffset;

enum BlobCompressionType { BLOB_COMPRESSION_NONE };

struct BlobMetadata {
    PackReader::BlobSize uncompressedSize;
    PackReader::BlobSize compressedSize;
    BlobCompressionType compressionType;
};

class PackReaderImpl : public PackReader {
 public:
    ~PackReaderImpl() noexcept;

    BlobIndex
    size() noexcept;

    BlobIndex
    findIndex(StringView path) noexcept;

    StringView
    getBlobPath(BlobIndex index) noexcept;
    BlobSize
    getBlobSize(BlobIndex index) noexcept;
    void*
    getBlobData(BlobIndex index) noexcept;

 public:
    void
    constructLookups() noexcept;

 public:
    MappedFile file;

    // Pointers into `file`.
    HeaderBlock* header;
    PathOffset* pathOffsets;
    char* paths;
    BlobMetadata* metadatas;
    uint32_t* dataOffsets;

    bool lookupsConstructed = false;
    Hashmap<StringView, BlobIndex> lookups;
};

PackReader*
PackReader::fromFile(StringView path) noexcept {
    MappedFile file;
    if (!makeMappedFile(path, file)) {
        return 0;
    }
    char* data = file.data;

    size_t offset = 0;

    HeaderBlock* header = reinterpret_cast<HeaderBlock*>(data + offset);

    offset += sizeof(*header);

    if (memcmp(header->magic, PACK_MAGIC, sizeof(header->magic)) != 0) {
        destroyMappedFile(file);
        return 0;
    }

    if (header->version != PACK_VERSION) {
        destroyMappedFile(file);
        return 0;
    }

    PackReaderImpl* reader =
            static_cast<PackReaderImpl*>(malloc(sizeof(PackReaderImpl)));
    new (reader) PackReaderImpl;

    reader->file = file;
    reader->header = header;

    BlobIndex blobCount = header->blobCount;

    reader->pathOffsets = reinterpret_cast<PathOffset*>(data + offset);
    offset += (blobCount + 1) * sizeof(PathOffset);

    reader->paths = reinterpret_cast<char*>(data + offset);
    offset += header->pathsBlockSize;

    reader->metadatas = reinterpret_cast<BlobMetadata*>(data + offset);
    offset += blobCount * sizeof(BlobMetadata);

    reader->dataOffsets = reinterpret_cast<uint32_t*>(data + offset);
    // offset += blobCount * sizeof(uint64_t);

    return reader;
}

PackReaderImpl::~PackReaderImpl() noexcept {
    destroyMappedFile(file);
}

PackReader::BlobIndex
PackReaderImpl::size() noexcept {
    return header->blobCount;
}

PackReader::BlobIndex
PackReaderImpl::findIndex(StringView path) noexcept {
    if (!lookupsConstructed) {
        lookupsConstructed = true;
        constructLookups();
    }

    Hashmap<StringView, BlobIndex>::iterator it = lookups.find(path);
    if (it == lookups.end()) {
        return BLOB_NOT_FOUND;
    }
    else {
        return it->value;
    }
}

StringView
PackReaderImpl::getBlobPath(PackReader::BlobIndex index) noexcept {
    uint32_t begin = pathOffsets[index];
    uint32_t end = pathOffsets[index + 1];
    return StringView(paths + begin, end - begin);
}

PackReader::BlobSize
PackReaderImpl::getBlobSize(PackReader::BlobIndex index) noexcept {
    return metadatas[index].uncompressedSize;
}

void*
PackReaderImpl::getBlobData(PackReader::BlobIndex index) noexcept {
    return file.data + dataOffsets[index];
}

void
PackReaderImpl::constructLookups() noexcept {
    for (PackReader::BlobIndex i = 0; i < header->blobCount; i++) {
        uint32_t pathBegin = pathOffsets[i];
        uint32_t pathEnd = pathOffsets[i + 1];
        StringView blobPath(paths + pathBegin, pathEnd - pathBegin);
        lookups[blobPath] = i;
    }
}
