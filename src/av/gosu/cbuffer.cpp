/***************************************
** Tsunagari Tile Engine              **
** cbuffer.cpp                        **
** Copyright 2011-2015 Michael Reiley **
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

#include "av/gosu/cbuffer.h"

#include "os/c.h"
#include "util/assert.h"

size_t
GosuCBuffer::size() noexcept {
    return size_;
}

void
GosuCBuffer::resize(size_t) noexcept {
    assert_(false);
}

void
GosuCBuffer::read(size_t offset, size_t length, void* destBuffer) noexcept {
    memcpy(destBuffer, data_ + offset, length);
}

void
GosuCBuffer::write(size_t, size_t, const void*) noexcept {
    assert_(false);
}
