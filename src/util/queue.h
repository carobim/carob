#ifndef SRC_UTIL_QUEUE_H_
#define SRC_UTIL_QUEUE_H_

#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"

template<typename T>
class Queue {
 public:
    Queue() noexcept : data(0), offset(0), size(0), capacity(0) { }
    ~Queue() noexcept {
        // Items at head of ring buffer.
        for (Size i = offset; i < offset + size && i < capacity; i++) {
            data[i].~T();
        }
        // Items at tail of ring buffer.
        for (Size i = 0;
             offset + size > capacity && i < offset + size - capacity; i++) {
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
    operator[](Size i) noexcept {
        assert_(i < size);
        return data[(offset + i) % capacity];
    }

 private:
    void
    resize() noexcept {
        Size newCapacity = capacity ? capacity * 2 : 4;
        T* newData = xmalloc(T, newCapacity);

        // Realign data items at offset 0 in the new array.
        for (Size i = offset; i < offset + size && i < capacity; i++) {
            new (&newData[i - offset]) T(static_cast<T&&>(data[i]));
            data[i].~T();
        }
        for (Size i = 0;
             offset + size > capacity && i < offset + size - capacity; i++) {
            new (&newData[i + capacity - offset]) T(static_cast<T&&>(data[i]));
            data[i].~T();
        }

        free(data);

        data = newData;
        offset = 0;
        capacity = newCapacity;
    }

 public:
    T* data;
    Size offset;
    Size size;
    Size capacity;
};

#endif  // SRC_UTIL_QUEUE_H_
