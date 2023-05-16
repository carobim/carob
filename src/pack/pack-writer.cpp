#include "pack/pack-writer.h"

#include "os/io.h"
#include "pack/file-type.h"
#include "pack/layout.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/math2.h"
#include "util/sort.h"
#include "util/string.h"
#include "util/vector.h"

typedef U32 BlobSize;
typedef U32 PathOffset;

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
    Blob blob = {path, size, data};
    writer->blobs.push(blob);
}

bool
packWriterWriteToFile(PackWriter* writer, StringView path) noexcept {
    bool ok = false;

    Vector<Blob>& blobs = writer->blobs;
    U32 blobCount = static_cast<U32>(blobs.size);

    //
    // Compute metadata.
    //

    // Sort blobs by size (smallest first).
    sortA(blobs);

    BlobMetadata* metadataSection = xmalloc(BlobMetadata, blobCount);

    PathOffset nextPathOffset = 0;
    U32 nextDataOffset = 0;

    for (U32 i = 0; i < blobCount; i++) {
        Blob& blob = blobs[i];

        BlobMetadata meta;
        meta.pathOffset = nextPathOffset;
        meta.pathSize = static_cast<U32>(blob.path.size);
        meta.dataOffset = nextDataOffset;
        meta.uncompressedSize = blob.size;
        meta.compressedSize = blob.size;
        meta.compressionType = BLOB_COMPRESSION_NONE;

        metadataSection[i] = meta;

        nextPathOffset += static_cast<U32>(blob.path.size);
        nextDataOffset += align64(blob.size);
    }

    //
    // Compute paths.
    //

    String pathsSection;
    pathsSection.reserve(nextPathOffset);

    for (Blob* blob = blobs.begin(); blob != blobs.end(); blob++) {
        pathsSection << blob->path;
    }

    //
    // Compute header.
    //

    U32 offset = 0;

    U32 headerOffset = offset;
    U32 headerSize = static_cast<U32>(sizeof(HeaderSection));
    offset += headerSize;

    U32 metadataOffset = offset;
    U32 metadataSize = static_cast<U32>(sizeof(BlobMetadata)) * blobCount;
    offset += metadataSize;

    U32 pathsOffset = offset;
    U32 pathsSize = nextPathOffset;
    offset += pathsOffset;

    offset = align64(offset);
    U32 dataOffset = offset;
    U32 dataSize = nextDataOffset;

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

    for (U32 i = 0; i < blobCount; i++) {
        BlobMetadata& meta = metadataSection[i];
        Blob& blob = blobs[i];

        f.writeOffset(blob.data,
                      meta.uncompressedSize,
                      dataOffset + meta.dataOffset);
    }

    ok = true;
err:
    free(metadataSection);
    return ok;
}
