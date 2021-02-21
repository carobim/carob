/***************************************
** Tsunagari Tile Engine              **
** string2.cpp                        **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2021 Paul Merrill   **
***************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#include "util/string2.h"

#include "os/c.h"
#include "os/os.h"
#include "util/constexpr.h"
#include "util/int.h"
#include "util/io.h"
#include "util/math2.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"

#define CHUNK_SIZE (128 * 1024)

/**
 * Matches regex /\s*-?\d+/
 */
bool
isInteger(StringView s) noexcept {
#define SPACE 0
#define SIGN  1
#define DIGIT 2

    int state = SPACE;

    for (size_t i = 0; i < s.size; i++) {
        char c = s.data[i];
        if (state == SPACE) {
            if (c == ' ')
                continue;
            else
                state++;
        }
        if (state == SIGN) {
            state++;
            if (c == '-')
                continue;
        }
        if (state == DIGIT) {
            if ('0' <= c && c <= '9')
                continue;
            else
                return false;
        }
    }
    return true;

#undef SPACE
#undef SIGN
#undef DIGIT
}

/**
 * Matches regex /\s*-?\d+\.?\d* /   [sic: star-slash ends comment]
 */
bool
isDecimal(StringView s) noexcept {
#define SPACE  0
#define SIGN   1
#define DIGIT  2
#define DOT    3
#define DIGIT2 4

    int state = SPACE;

    for (size_t i = 0; i < s.size; i++) {
        char c = s.data[i];
        switch (state) {
        case SPACE:
            if (c == ' ')
                continue;
            else
                state++;
        case SIGN:
            state++;
            if (c == '-')
                continue;
        case DIGIT:
            if ('0' <= c && c <= '9')
                continue;
            else
                state++;
        case DOT:
            state++;
            if (c == '.')
                continue;
            else
                return false;
        case DIGIT2:
            if ('0' <= c && c <= '9')
                continue;
            else
                return false;
        }
    }
    return true;

#undef SPACE
#undef SIGN
#undef DIGIT
#undef DOT
#undef DIGIT2
}

/**
 * Matches "5-7,2,12-14" no whitespace.
 */
bool
isRanges(StringView s) noexcept {
#define SIGN  0
#define DIGIT 1
#define DASH  3
#define COMMA 4

    bool dashed = false;

    int state = SIGN;

    for (size_t i = 0; i < s.size; i++) {
        char c = s.data[i];
        switch (state) {
        case SIGN:
            state++;
            if (c == '-' || c == '+')
                break;
        case DIGIT:
            if ('0' <= c && c <= '9')
                break;
            state++;
        case DASH:
            state++;
            if (c == '-') {
                if (dashed)
                    return false;
                dashed = true;
                state = SIGN;
                break;
            }
        case COMMA:
            state++;
            if (c == ',') {
                dashed = false;
                state = SIGN;
                break;
            }
            return false;
        }
    }
    return true;

#undef SIGN
#undef DIGIT
#undef DASH
#undef COMMA
}

static inline char
tolower(char c) noexcept {
    if ('A' <= c && c <= 'Z') {
        return c + 'a' - 'A';
    }
    else {
        return c;
    }
}

bool
iequals(StringView a, StringView b) noexcept {
    if (a.size != b.size) {
        return false;
    }
    for (size_t i = 0; i < a.size; i++) {
        if (tolower(a.data[i]) != tolower(b.data[i])) {
            return false;
        }
    }
    return true;
}

static const StringView true_ = "true";
static const StringView yes = "yes";
static const StringView on = "on";

static const StringView false_ = "false";
static const StringView no = "no";
static const StringView off = "off";

bool
parseBool(bool& out, StringView s) noexcept {
    if (iequals(s, true_) || iequals(s, yes) || iequals(s, on) || s == "1") {
        out = true;
        return true;
    }

    if (iequals(s, false_) || iequals(s, no) || iequals(s, off) || s == "0") {
        out = false;
        return true;
    }

    return false;
}

bool
parseUInt(unsigned& out, String& s) noexcept {
    errno = 0;

    char* end;
    unsigned long ul = strtoul(s.null(), &end, 10);

    if (end != s.data + s.size) {
        return false;
    }

    if (errno != 0) {
        // Overflow.
        return false;
    }
    if (ul > UINT32_MAX) {
        // Overflow.
        return false;
    }

    out = static_cast<unsigned>(ul);
    return true;
}

bool
parseInt(int& out, String& s) noexcept {
    errno = 0;

    char* end;
    long l = strtol(s.null(), &end, 10);

    if (end != s.data + s.size) {
        return false;
    }

    if (errno != 0) {
        // Overflow.
        return false;
    }
    if (l > static_cast<long>(UINT32_MAX)) {
        // Overflow.
        return false;
    }

    out = static_cast<int>(l);
    return true;
}

bool
parseFloat(float& out, String& s) noexcept {
    errno = 0;

    char* end;
    out = static_cast<float>(strtod(s.null(), &end));

    if (end != s.data + s.size) {
        return false;
    }

    if (errno != 0) {
        // Overflow.
        return false;
    }

    return true;
}

int
parseInt100(char* s) noexcept {
    int i = atoi(s);
    return bound(i, 0, 100);
}

void
splitStr(Vector<StringView>& out,
         StringView input,
         StringView delimiter) noexcept {
    size_t i = 0;

    for (StringPosition pos = input.find(delimiter); pos != SV_NOT_FOUND;
         pos = input.find(delimiter, i)) {
        out.push_back(input.substr(i, pos - i));
        i = pos + delimiter.size;
    }

    if (input.size != i) {
        out.push_back(input.substr(i));
    }
}

/**
 * Parse ranges of integers.
 * Can take things such as "5-7".
 */
bool
parseRange(int& lo, int& hi, StringView range, String& buf) noexcept {
    StringPosition dash = range.find('-');

    if (dash == SV_NOT_FOUND) {
        return false;
    }

    StringView rngstart = range.substr(0, dash);
    StringView rngend = range.substr(dash + 1);

    buf.clear();
    buf = rngstart;
    if (!parseInt(lo, buf)) {
        return false;
    }

    buf.clear();
    buf = rngend;
    if (!parseInt(hi, buf)) {
        return false;
    }

    if (lo > hi) {
        return false;
    }

    return true;
}

bool
parseRanges(Vector<int>& out, StringView format) noexcept {
    Vector<StringView> tokens;
    splitStr(tokens, format, ",");

    String buf;
    for (StringView range : tokens) {
        StringPosition dash = range.find('-');

        if (dash == SV_NOT_FOUND) {
            buf.clear();
            buf = range;

            int i;
            if (!parseInt(i, buf)) {
                return false;
            }

            out.push_back(i);
        }
        else {
            StringView rngstart = range.substr(0, dash);
            StringView rngend = range.substr(dash + 1);

            buf.clear();
            buf = rngstart;
            int start;
            if (!parseInt(start, buf)) {
                return false;
            }

            buf.clear();
            buf = rngend;
            int end;
            if (!parseInt(end, buf)) {
                return false;
            }

            if (start > end) {
                return false;
            }

            for (int i = start; i <= end; i++) {
                out.push_back(i);
            }
        }
    }

    return true;
}

FileStream::FileStream(StringView path) noexcept : file(path) {
    size_t bufsz = file.rem < CHUNK_SIZE ? file.rem : CHUNK_SIZE;
    chunk.reserve(bufsz);
}

FileStream::operator bool() noexcept {
    return file;
}

bool
FileStream::advance() noexcept {
    chunk.size = file.rem < CHUNK_SIZE ? file.rem : CHUNK_SIZE;
    return file.read(chunk.data, chunk.size);
}

ReadLines::ReadLines(StringView path) noexcept : file(path), offset(0) { }

ReadLines::operator bool() noexcept {
    return file;
}

// StringView::data == 0 on end of file or I/O error.
StringView
ReadLines::next() noexcept {
    if (file.chunk.size == offset) {
        if (file.file.rem) {
            // Slow case: File has more data.
            //
            // This read starts at the beginning of a chunk. We will avoid
            // the use of joiner if the complete token ends within the
            // chunk.
            if (!file.advance()) {
                // I/O error.
                offset = file.chunk.size;
                file.file.rem = 0;
                return StringView();
            }
            offset = 0;
        }
        else {
            // Fast case: End of file.
            return StringView();
        }
    }
    size_t position = file.chunk.view().find('\n', offset);
    if (position != SV_NOT_FOUND) {
        // Fast case: The token is contained within a single chunk and ends
        //            before the end of the file. Don't use the joiner.
        size_t oldOffset = offset;
        offset = position + 1;  // 1 = delimiter
        return file.chunk.view().substr(oldOffset, position - oldOffset);
    }
    else if (file.file.rem == 0) {
        // Fast case: The token ends at the end of the file. Don't use the
        // joiner.
        size_t oldOffset = offset;
        offset = file.chunk.size;
        return StringView(file.chunk.view().substr(oldOffset));
    }
    else {
        // Slow case: The token spans two or more chunks.

        // To begin with, start a joiner with the last of the current
        // chunk.
        joiner = file.chunk.view().substr(offset);

        // Start reading chunks until we find the end of the token within
        // one.
        if (!file.advance()) {
            // I/O error.
            offset = file.chunk.size;
            file.file.rem = 0;
            return StringView();
        }

        size_t position = file.chunk.view().find('\n');

        while (true) {
            if (position != SV_NOT_FOUND) {
                // We found the end of the token within this chunk.
                joiner << file.chunk.view().substr(0, position);
                offset = position + 1;  // 1 = delimiter
                return joiner.view();
            }
            else if (file.file.rem == 0) {
                // The file has ended. The token must end here.
                joiner << file.chunk;
                offset = file.chunk.size;
                return joiner.view();
            }
            else {
                // We searched the entire chunk but the token has not ended
                // yet.
                joiner << file.chunk;
                if (!file.advance()) {
                    // I/O error.
                    offset = file.chunk.size;
                    file.file.rem = 0;
                    return StringView();
                }
                position = file.chunk.view().find('\n');
            }
        }
    }
}
