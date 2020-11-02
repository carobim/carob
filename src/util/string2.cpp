/***************************************
** Tsunagari Tile Engine              **
** string2.cpp                        **
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

#include "util/string2.h"

#include "os/c.h"
#include "os/os.h"
#include "util/constexpr.h"
#include "util/int.h"
#include "util/math2.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"

/**
 * Matches regex /\s*-?\d+/
 */
bool
isInteger(StringView s) noexcept {
#define SPACE 0
#define SIGN 1
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
#define SPACE 0
#define SIGN 1
#define DIGIT 2
#define DOT 3
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
#define SIGN 0
#define DIGIT 1
#define DASH 3
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
parseUInt(unsigned& out, StringView s) noexcept {
    String s_(s);
    return parseUInt(out, s_);
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
parseInt(int& out, StringView s) noexcept {
    String s_(s);
    return parseInt(out, s_);
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

bool
parseFloat(float& out, StringView s) noexcept {
    String s_(s);
    return parseFloat(out, s_);
}

int
parseInt100(char* s) noexcept {
    int i = atoi(s);
    return bound(i, 0, 100);
}

Vector<StringView>
splitStr(StringView input, StringView delimiter) noexcept {
    Vector<StringView> strlist;
    size_t i = 0;

    for (StringPosition pos = input.find(delimiter); pos != SV_NOT_FOUND;
         pos = input.find(delimiter, i)) {
        strlist.push_back(input.substr(i, pos - i));
        i = pos + delimiter.size;
    }

    if (input.size != i) {
        strlist.push_back(input.substr(i));
    }
    return strlist;
}

bool
parseRanges(Vector<int>& out, StringView format) noexcept {
    for (StringView range : splitStr(format, ",")) {
        StringPosition dash = range.find('-');

        if (dash == SV_NOT_FOUND) {
            int i;
            if (!parseInt(i, range)) {
                return false;
            }

            out.push_back(i);
        }
        else {
            StringView rngstart = range.substr(0, dash);
            StringView rngend = range.substr(dash + 1);

            int start, end;
            if (!parseInt(start, rngstart) || !parseInt(end, rngend)) {
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
