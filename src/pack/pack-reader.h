#ifndef SRC_PACK_PACK_READER_H_
#define SRC_PACK_PACK_READER_H_

#include "util/int.h"
#include "util/compiler.h"
#include "util/string-view.h"

#define BLOB_NOT_FOUND UINT32_MAX

struct BlobDetails {
    StringView path;
    uint32_t size;
};

struct PackReader;

PackReader*
makePackReader(StringView path) noexcept;
void
destroyReader(PackReader* r) noexcept;

uint32_t
readerSize(PackReader* r) noexcept;
uint32_t
readerIndex(PackReader* r, StringView path) noexcept;
BlobDetails
readerDetails(PackReader* r, uint32_t index) noexcept;

bool
readerRead(PackReader* r, void* buf, uint32_t index) noexcept;

#endif  // SRC_PACK_PACK_READER_H_
