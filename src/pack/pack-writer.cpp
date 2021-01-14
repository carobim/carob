/*************************************
** Tsunagari Tile Engine            **
** pack-writer.cpp                  **
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

#include "pack/pack-writer.h"

#include "os/io.h"
#include "os/os.h"
#include "pack/file-type.h"
#include "pack/layout.h"
#include "pack/pack-reader.h"
#include "util/int.h"
#include "util/math2.h"
#include "util/move.h"
#include "util/noexcept.h"
#include "util/sort.h"
#include "util/string.h"
#include "util/vector.h"

typedef uint32_t BlobSize;
typedef uint32_t PathOffset;

struct Blob {
    String path;
    BlobSize size;
    const void* data;
};

static bool
operator<(const Blob& a, const Blob& b) noexcept {
    FileType typeA = determineFileType(a.path);
    FileType typeB = determineFileType(b.path);
    if (typeA < typeB) {
        return true;
    }
    else if (typeA > typeB) {
        return false;
    }
    else {
        return a.path < b.path;
    }
}


struct PackWriter {
    Vector<Blob> blobs;
};

PackWriter*
makePackWriter() noexcept {
    PackWriter* writer = new PackWriter();
    return writer;
}

void
destroyPackWriter(PackWriter* writer) noexcept {
    delete writer;
}

void
packWriterAddBlob(PackWriter* writer, StringView path, BlobSize size,
        const void* data) noexcept {
    writer->blobs.push_back({path, size, data});
}

bool
packWriterWriteToFile(PackWriter* writer, StringView path) noexcept {
    bool ok = false;

    Vector<Blob>& blobs = writer->blobs;
    uint32_t blobCount = static_cast<uint32_t>(blobs.size);

    //
    // Compute metadata.
    //

    // Sort blobs by size (smallest first).
    sortA(blobs);

    BlobMetadata* metadataSection = xmalloc(BlobMetadata, blobCount);

    PathOffset nextPathOffset = 0;
    uint32_t nextDataOffset = 0;

    for (uint32_t i = 0; i < blobCount; i++) {
        Blob& blob = blobs[i];

        BlobMetadata meta;
        meta.pathOffset = nextPathOffset;
        meta.pathSize = static_cast<uint32_t>(blob.path.size);
        meta.dataOffset = nextDataOffset;
        meta.uncompressedSize = blob.size;
        meta.compressedSize = blob.size;
        meta.compressionType = BLOB_COMPRESSION_NONE;

        metadataSection[i] = meta;

        nextPathOffset += static_cast<uint32_t>(blob.path.size);
        nextDataOffset += align64(blob.size);
    }

    //
    // Compute paths.
    //

    String pathsSection;
    pathsSection.reserve(nextPathOffset);

    for (Blob& blob : blobs) {
        pathsSection << blob.path;
    }

    //
    // Compute header.
    //

    uint32_t offset = 0;

    uint32_t headerOffset = offset;
    uint32_t headerSize = sizeof32(HeaderSection);
    offset += headerSize;

    uint32_t metadataOffset = offset;
    uint32_t metadataSize = sizeof32(BlobMetadata) * blobCount;
    offset += metadataSize;

    uint32_t pathsOffset = offset;
    uint32_t pathsSize = nextPathOffset;
    offset += pathsOffset;

    offset = align64(offset);
    uint32_t dataOffset = offset;
    uint32_t dataSize = nextDataOffset;

    HeaderSection headerSection = {
            {PACK_MAGIC[0],
             PACK_MAGIC[1],
             PACK_MAGIC[2],
             PACK_MAGIC[3],
             PACK_MAGIC[4],
             PACK_MAGIC[5],
             PACK_MAGIC[6],
             PACK_MAGIC[7]},

            PACK_VERSION,
            {0, 0, 0, 0, 0, 0, 0},

            blobCount,

            pathsOffset,
            metadataOffset,
            dataOffset,
    };

    //
    // Write file to disk.
    //

    FileWriter f(path);
    if (!f) {
        goto err;
    }

    f.resize(dataOffset + dataSize);

    f.writeOffset(&headerSection, headerSize, headerOffset);
    f.writeOffset(metadataSection, metadataSize, metadataOffset);
    f.writeOffset(pathsSection.data, pathsSize, pathsOffset);

    for (uint32_t i = 0; i < blobCount; i++) {
        BlobMetadata& meta = metadataSection[i];
        Blob& blob = blobs[i];

        f.writeOffset(blob.data, meta.uncompressedSize,
                      dataOffset + meta.dataOffset);
    }

    ok = true;
err:
    free(metadataSection);
    return ok;
}
