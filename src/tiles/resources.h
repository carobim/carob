#ifndef SRC_CORE_RESOURCES_H_
#define SRC_CORE_RESOURCES_H_

#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"

// Provides data and resource extraction for a World.
// Each World comes bundled with associated data.

// Load a resource from the file at the given path.
bool
resourceLoad(StringView path, String& data) noexcept;

#endif  // SRC_CORE_RESOURCES_H_
