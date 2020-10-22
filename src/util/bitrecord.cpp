/***************************************
** Tsunagari Tile Engine              **
** bitrecord.cpp                      **
** Copyright 2011-2014 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

#include "bitrecord.h"

#include "util/noexcept.h"

BitRecord::BitRecord(size_t length) noexcept {
    states.resize(length);
    for (size_t i = 0; i < length; i++) {
        states[i] = false;
    }
}

bool& BitRecord::operator[](size_t idx) noexcept {
    return states[idx];
}

Vector<size_t>
BitRecord::diff(BitRecord& other) noexcept {
    Vector<size_t> changes;

    for (size_t i = 0; i < states.size(); i++) {
        if (states[i] != other.states[i]) {
            changes.push_back(i);
        }
    }
    return changes;
}
