/*************************************
** Tsunagari Tile Engine            **
** layout.h                         **
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

#ifndef SRC_PACK_LAYOUT_H_
#define SRC_PACK_LAYOUT_H_

#include "util/constexpr.h"
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
static constexpr uint8_t PACK_MAGIC[8] = {84, 115, 117, 110, 97, 103, 97, 114};

static constexpr uint8_t PACK_VERSION = 2;

struct HeaderSection {
    uint8_t magic[8];
    uint8_t version;
    uint8_t unused[7];

    uint32_t blobCount;

    uint32_t metadataOffset;
    uint32_t pathsOffset;
    uint32_t dataOffset;
};

enum BlobCompressionType { BLOB_COMPRESSION_NONE };

struct BlobMetadata {
    // Offset into paths section.
    uint32_t pathOffset;
    uint32_t pathSize;

    // Offset into data section.
    uint32_t dataOffset;
    uint32_t uncompressedSize;
    uint32_t compressedSize;

    uint8_t compressionType;
    uint8_t unused[3];
};

#endif  // SRC_PACK_LAYOUT_H_
