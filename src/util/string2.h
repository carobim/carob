/***************************************
** Tsunagari Tile Engine              **
** string2.h                          **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2020 Paul Merrill   **
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

#ifndef SRC_UTIL_STRING2_H_
#define SRC_UTIL_STRING2_H_

#include "util/noexcept.h"
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
parseUInt(unsigned& out, String& s) noexcept;
bool
parseFloat(float& out, String& s) noexcept;

int
parseInt100(char* s) noexcept;

//! Split a string by a delimiter.
void
splitStr(Vector<StringView>& out, StringView str,
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

class Lines {
 public:
     // StringView is overwritten with each call.
     // StringView.size == 0 on empty.
     StringView
     operator++(int) noexcept;

 public:
    StringView file;
};

// Lines borrows file.
Lines
readLines(StringView file) noexcept;

template<char c>
class Tokens {
 public:
    // StringView is overwritten with each call.
    // StringView.data == 0 on empty.
    StringView
    operator++(int) noexcept {
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
    // Whenever buf.size == 0, we require buf.data == 0.
    StringView buf;
};

// Returned Tokens object borrows buf.
template<char c>
Tokens<c>
split(StringView buf) noexcept {
    Tokens<c> tokens;
    tokens.buf = buf;
    if (tokens.buf.size == 0) {
        tokens.buf.data = 0;
    }
    return tokens;
}

class FileStream {
 public:
    FileStream() noexcept;
    ~FileStream() noexcept;

    bool
    start(StringView path) noexcept;

    bool
    advance() noexcept;

 public:
    int fd;
    uint64_t rem;  // 0 when EOF

    String chunk;
};


#define CHUNK_SIZE (128 * 1024)

template<char delim>
class FileTokenStream {
 public:
    FileTokenStream() noexcept : offset(0) {}

    bool
    start(StringView path) noexcept {
        return file.start(path);
    }

    // StringView::data == 0 on end of file or I/O error.
    StringView
    operator++(int) noexcept {
        if (file.chunk.size == offset) {
            if (file.rem) {
                // Slow case: File has more data.
                //
                // This read starts at the beginning of a chunk. We will avoid
                // the use of joiner if the complete token ends within the
                // chunk.
                if (!file.advance()) {
                    // I/O error.
                    offset = file.chunk.size;
                    file.rem = 0;
                    return StringView();
                }
                offset = 0;
            }
            else {
                // Fast case: End of file.
                return StringView();
            }
        }
        size_t position = file.chunk.view().find(delim, offset);
        if (position != SV_NOT_FOUND) {
            // Fast case: The token is contained within a single chunk and ends
            //            before the end of the file. Don't use the joiner.
            size_t oldOffset = offset;
            offset = position + 1;  // 1 = delimiter
            return file.chunk.view().substr(oldOffset, position - oldOffset);
        }
        else if (file.rem == 0) {
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
                file.rem = 0;
                return StringView();
            }

            size_t position = file.chunk.view().find(delim);

            while (true) {
                if (position != SV_NOT_FOUND) {
                    // We found the end of the token within this chunk.
                    joiner << file.chunk.view().substr(0, position);
                    offset = position + 1;  // 1 = delimiter
                    return joiner.view();
                }
                else if (file.rem == 0) {
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
                        file.rem = 0;
                        return StringView();
                    }
                    position = file.chunk.view().find(delim);
                }
            }
        }
    }

 public:
    FileStream file;
    size_t offset;
    String joiner;
};

#endif  // SRC_UTIL_STRING2_H_
