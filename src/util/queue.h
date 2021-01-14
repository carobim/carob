/*************************************
** Tsunagari Tile Engine            **
** queue.h                          **
** Copyright 2020-2021 Paul Merrill **
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

#ifndef SRC_UTIL_QUEUE_H_
#define SRC_UTIL_QUEUE_H_

#include "util/assert.h"
#include "util/int.h"
#include "util/new.h"

template<typename T>
class Queue {
 public:
    Queue() noexcept : data(0), offset(0), size(0), capacity(0) {}
    ~Queue() noexcept {
        // Items at head of ring buffer.
        for (size_t i = offset; i < offset + size && i < capacity; i++) {
            data[i].~T();
        }
        // Items at tail of ring buffer.
        for (size_t i = 0;
             offset + size > capacity && i < offset + size - capacity;
             i++) {
            data[i].~T();
        }

        free(data);
    }

    // Write
    void
    push(T t) noexcept {
        if (size == capacity) {
            resize();
        }
        new (&data[(offset + size) % capacity]) T(static_cast<T&&>(t));
        size += 1;
    }
    void
    pop() noexcept {
        assert_(size);
        data[offset].~T();
        offset = (offset + 1) % capacity;
        size -= 1;
    }

    // Read
    T&
    front() noexcept {
        assert_(size);
        return data[offset];
    }
    T&
    operator[](size_t i) noexcept {
        assert_(i < size);
        return data[(offset + i) % capacity];
    }

 private:
    void
    resize() noexcept {
        size_t newCapacity = capacity ? capacity * 2 : 4;
        T* newData = xmalloc(T, newCapacity);

        // Realign data items at offset 0 in the new array.
        for (size_t i = offset; i < offset + size && i < capacity; i++) {
            new (&newData[i - offset]) T(static_cast<T&&>(data[i]));
            data[i].~T();
        }
        for (size_t i = 0;
             offset + size > capacity && i < offset + size - capacity;
             i++) {
            new (&newData[i + capacity - offset])
                T(static_cast<T&&>(data[i]));
            data[i].~T();
        }

        free(data);

        data = newData;
        offset = 0;
        capacity = newCapacity;
    }

 public:
    T* data;
    size_t offset;
    size_t size;
    size_t capacity;
};

#endif  // SRC_UTIL_QUEUE_H_
