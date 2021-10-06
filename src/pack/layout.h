#ifndef SRC_PACK_LAYOUT_H_
#define SRC_PACK_LAYOUT_H_

#include "util/compiler.h"
#include "util/int.h"

// Pack file layout:
//
//   Header                           [struct]
//   Metadata                         [struct array]
//   Paths                            [string pool, 1-byte alignment]
//   Data                             [byte-buffer pool, 8-byte alignment]
//   EOF
//
//
// Paths are stored one after the other with no NUL terminating byte or other
// spacers between them.
//
// Data are stored one after the other with padding so that each is aligned on
// an 8-byte boundary.

// Version history:
//   (1) Initial version.
//   (2) Coalesce metadata into one section. Align data to boundary.

//                                       "T   s    u    n    a   g    a   r"
static constexpr U8 PACK_MAGIC[8] = {84, 115, 117, 110, 97, 103, 97, 114};

static constexpr U8 PACK_VERSION = 2;

struct HeaderSection {
    U8 magic[8];
    U8 version;
    U8 unused[7];

    U32 blobCount;

    U32 metadataOffset;
    U32 pathsOffset;
    U32 dataOffset;
};

enum BlobCompressionType { BLOB_COMPRESSION_NONE };

struct BlobMetadata {
    // Offset into paths section.
    U32 pathOffset;
    U32 pathSize;

    // Offset into data section.
    U32 dataOffset;
    U32 uncompressedSize;
    U32 compressedSize;

    U8 compressionType;
    U8 unused[3];
};

#endif  // SRC_PACK_LAYOUT_H_
