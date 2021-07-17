#ifndef SRC_PACK_WALKER_H_
#define SRC_PACK_WALKER_H_

#include "util/compiler.h"
#include "util/string-view.h"
#include "util/vector.h"

// Recursively walk the file system under the following paths, calling op on
// each regular file (non-directory) found.
void
walk(Vector<StringView> paths,
     void* userData,
     void (*op)(void* userData, StringView path)) noexcept;

#endif  // SRC_PACK_WALKER_H_
