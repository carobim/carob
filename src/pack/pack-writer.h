#ifndef SRC_PACK_PACK_WRITER_H_
#define SRC_PACK_PACK_WRITER_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

typedef struct PackWriter PackWriter;

PackWriter*
makePackWriter() noexcept;

void
destroyPackWriter(PackWriter* writer) noexcept;

void
packWriterAddBlob(PackWriter* writer,
                  StringView path,
                  U32 size,
                  const void* data) noexcept;

bool
packWriterWriteToFile(PackWriter* writer, StringView path) noexcept;

#endif  // SRC_PACK_PACK_WRITER_H_
