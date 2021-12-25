#ifndef SRC_CORE_JSONS_H_
#define SRC_CORE_JSONS_H_

#include "util/compiler.h"
#include "util/json.h"
#include "util/string-view.h"

JsonDocument
loadJson(StringView path) noexcept;

#endif  // SRC_CORE_JSONS_H_
