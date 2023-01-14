#include "net/html.h"

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"

void
htmlEscape(String& out, StringView in) noexcept {
    for (Size i = 0; i < in.size; i++) {
        char c = in[i];
        if (c == '&') {
            out << "&amp;";
        }
        else if (c == '<') {
            out << "&lt;";
        }
        else if (c == '>') {
            out << "&gt;";
        }
        else if (c == '"') {
            out << "&quot;";
        }
        else if (c == '\'') {
            out << "&#x27;";
        }
        else {
            out << c;
        }
    }
}
