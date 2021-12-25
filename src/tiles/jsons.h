#ifndef SRC_TILES_JSONS_H_
#define SRC_TILES_JSONS_H_

#include "util/compiler.h"
#include "util/json.h"
#include "util/string-view.h"

JsonDocument
loadJson(StringView path) noexcept;

#endif  // SRC_TILES_JSONS_H_
