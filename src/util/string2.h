#ifndef SRC_UTIL_STRING2_H_
#define SRC_UTIL_STRING2_H_

#include "os/io.h"
#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

class String;

//! Returns true if the string contains only digits, whitespace, and minus.
bool
isInteger(StringView s) noexcept;

//! Returns true if the string contains only digits, whitespace, minus, and
//! period.
bool
isDecimal(StringView s) noexcept;

bool
isRanges(StringView s) noexcept;


//! Whether two strings are case-insensative equals.
bool
iequals(StringView a, StringView b) noexcept;

//! Return a bool from a "true"/"false" string.
bool
parseBool(bool& out, StringView s) noexcept;

bool
parseInt(int& out, String& s) noexcept;
bool
parseInt0(int* out, StringView s) noexcept;
bool
parseUInt(unsigned& out, String& s) noexcept;
bool
parseFloat(float& out, String& s) noexcept;

int
parseInt100(char* s) noexcept;

//! Split a string by a delimiter.
void
splitStr(Vector<StringView>& out,
         StringView str,
         StringView delimiter) noexcept;

/**
 * Parse ranges of integers.
 * Can take things such as "5-7".
 */
bool
parseRange(int& lo, int& hi, StringView format, String& buf) noexcept;

/**
 * Parse ranges of integers separated by commas.
 * Can take things such as "5-7,2,12-18".
 */
bool
parseRanges(Vector<int>& out, StringView format) noexcept;

template<char c>
class Splits {
 public:
    Splits(StringView b) noexcept {
        buf = b;
        if (buf.size == 0) {
            buf.data = 0;
        }
    }

    // StringView is overwritten with each call.
    // StringView.data == 0 on empty.
    StringView
    next() noexcept {
        const char* data = buf.data;
        size_t size = buf.find(c);
        if (size == SV_NOT_FOUND) {
            size = buf.size;
            buf.data = 0;
            buf.size = 0;
            return StringView(data, size);
        }
        else {
            buf.data += size + 1;
            buf.size -= size + 1;
            return StringView(data, size);
        }
    }

 public:
    // Borrowed.
    // Whenever buf.size == 0, we require buf.data == 0.
    StringView buf;
};

class FileStream {
 public:
    FileStream(StringView path) noexcept;

    // Whether file stream opened successfully.
    operator bool() noexcept;

    bool
    advance() noexcept;

 public:
    File file;
    String chunk;
};

class ReadLines {
 public:
    ReadLines(StringView path) noexcept;

    // Whether file opened successfully.
    operator bool() noexcept;

    // StringView::data == 0 on end of file or I/O error.
    StringView
    next() noexcept;

 public:
    FileStream file;
    size_t offset;
    String joiner;
};

#endif  // SRC_UTIL_STRING2_H_
