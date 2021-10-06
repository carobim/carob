#ifndef SRC_PACK_PACK_READER_H_
#define SRC_PACK_PACK_READER_H_

#include "util/int.h"
#include "util/compiler.h"
#include "util/string-view.h"

#define BLOB_NOT_FOUND UINT32_MAX

struct BlobDetails {
    StringView path;
    U32 size;
};

struct PackReader;

PackReader*
makePackReader(StringView path) noexcept;
void
destroyReader(PackReader* r) noexcept;

U32
readerSize(PackReader* r) noexcept;
U32
readerIndex(PackReader* r, StringView path) noexcept;
BlobDetails
readerDetails(PackReader* r, U32 index) noexcept;

bool
readerRead(PackReader* r, void* buf, U32 index) noexcept;

#endif  // SRC_PACK_PACK_READER_H_
