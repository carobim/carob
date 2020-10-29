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

#include "os/mapped-file.h"
#include "util/hashtable.h"
#include "util/int.h"
#include "util/move.h"
#include "util/new.h"
#include "util/noexcept.h"
#include "util/optional.h"

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

Unique<PackReader>
PackReader::fromFile(StringView path) noexcept {
    Optional<MappedFile> maybeFile = MappedFile::fromPath(path);
    if (!maybeFile) {
        return Unique<PackReader>();
    }

    MappedFile file = move_(*maybeFile);

    size_t offset = 0;

    HeaderBlock* header = file.at<HeaderBlock*>(offset);
    offset += sizeof(*header);

    if (memcmp(header->magic, PACK_MAGIC, sizeof(header->magic)) != 0) {
        return Unique<PackReader>();
    }

    if (header->version != PACK_VERSION) {
        return Unique<PackReader>();
    }

    PackReaderImpl* reader =
            static_cast<PackReaderImpl*>(malloc(sizeof(PackReaderImpl)));
    new (reader) PackReaderImpl;

    reader->file = move_(file);
    reader->header = header;

    BlobIndex blobCount = header->blobCount;

    reader->pathOffsets = reader->file.at<PathOffset*>(offset);
    offset += (blobCount + 1) * sizeof(PathOffset);

    reader->paths = reader->file.at<char*>(offset);
    offset += header->pathsBlockSize;

    reader->metadatas = reader->file.at<BlobMetadata*>(offset);
    offset += blobCount * sizeof(BlobMetadata);

    reader->dataOffsets = reader->file.at<uint32_t*>(offset);
    // offset += blobCount * sizeof(uint64_t);

    return Unique<PackReader>(reader);
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

    auto it = lookups.find(path);
    if (it == lookups.end()) {
        return BLOB_NOT_FOUND;
    }
    else {
        return it.value();
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
    return file.at<void*>(dataOffsets[index]);
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
