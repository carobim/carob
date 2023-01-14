#ifndef HTML_H_
#define HTML_H_

#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"

/**
 * Perform HTML string character escaping.
 *
 * Appends to the output string, does not overwrite.
 */
void
htmlEscape(String& out, StringView in) noexcept;

#endif
