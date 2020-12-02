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
parseInt(int& out, StringView s) noexcept;
bool
parseUInt(unsigned& out, String& s) noexcept;
bool
parseUInt(unsigned& out, StringView s) noexcept;
bool
parseFloat(float& out, String& s) noexcept;
bool
parseFloat(float& out, StringView s) noexcept;

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
parseRange(int& lo, int& hi, StringView format) noexcept;

/**
 * Parse ranges of integers separated by commas.
 * Can take things such as "5-7,2,12-18".
 */
bool
parseRanges(Vector<int>& out, StringView format) noexcept;

#endif  // SRC_UTIL_STRING2_H_
