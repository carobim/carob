/*************************************
** Tsunagari Tile Engine            **
** align.h                          **
** Copyright 2019-2021 Paul Merrill **
*************************************/

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

#ifndef SRC_UTIL_ALIGN_H_
#define SRC_UTIL_ALIGN_H_

template<typename T>
struct Align {
#if (defined(_MSC_VER) && _MSC_VER >= 1900) || defined(__clang__) || (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ >= 9)
    alignas(alignof(T)) char storage[sizeof(T)];
#elif defined(_MSC_VER) && defined(_WIN64)
    __declspec(align(8)) char storage[sizeof(T)];
#elif defined(_MSC_VER)
    __declspec(align(4)) char storage[sizeof(T)];
#elif defined(__x86_64__)
    __attribute__((aligned(8))) char storage[sizeof(T)];
#elif defined(__i386__)
    __attribute__((aligned(4))) char storage[sizeof(T)];
#endif
};

#endif  // SRC_UTIL_ALIGN_H_
