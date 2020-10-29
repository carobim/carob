/*************************************
** Tsunagari Tile Engine            **
** pool.h                           **
** Copyright 2019-2020 Paul Merrill **
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

#ifndef SRC_UTIL_POOL_H_
#define SRC_UTIL_POOL_H_

#include "util/int.h"
#include "util/new.h"
#include "util/noexcept.h"

#define POOL_END UINT32_MAX

// Pool
//
// Simple growable array. All memory is uninitialized: constructors and
// destructors are never called on entries. Can delete entries anywhere in the
// array in O(1) time.
//
// Suggested to use exclusively with POD types.
template<typename T>
class Pool {
 private:
    typedef uint32_t Link;

 public:
    Pool() : storage(0), allocated(0), nextFree(POOL_END) {}
    ~Pool() { free(reinterpret_cast<char*>(storage)); }

    // Returns an unconstructed piece of memory.
    uint32_t
    allocate() noexcept {
        if (nextFree == POOL_END) {
            grow();
        }
        uint32_t id = nextFree;
        nextFree = asLink(nextFree);
        return id;
    }

    // Releases memory without destructing the object within.
    void
    release(uint32_t i) noexcept {
        assert_(0 <= i && i < allocated);
        asLink(i) = nextFree;
        nextFree = i;
    }

    T& operator[](uint32_t i) noexcept {
        assert_(0 <= i && i < allocated);
        return storage[i];
    }

 private:
    Pool(const Pool&) {}

    Link&
    asLink(uint32_t i) noexcept {
        return *reinterpret_cast<Link*>(reinterpret_cast<char*>(&storage[i]));
    }

    void
    grow() noexcept {
        uint32_t newAllocated = allocated == 0 ? 4 : allocated * 2;

        T* newStorage = reinterpret_cast<T*>(malloc(sizeof(T) * newAllocated));
        memcpy(newStorage, storage, sizeof(T) * allocated);
        free(reinterpret_cast<char*>(storage));

        nextFree = allocated;
        storage = newStorage;

        for (uint32_t i = allocated; i < newAllocated - 1; i++) {
            asLink(i) = i + 1;
        }
        asLink(newAllocated - 1) = POOL_END;

        allocated = newAllocated;
    }

    T* storage;
    uint32_t allocated;
    Link nextFree;
};

#endif  // SRC_UTIL_POOL_H_
