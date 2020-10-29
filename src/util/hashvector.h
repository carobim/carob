/********************************
** Tsunagari Tile Engine       **
** hashvector.h                **
** Copyright 2020 Paul Merrill **
********************************/

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

#ifndef SRC_UTIL_HASHVECTOR_H_
#define SRC_UTIL_HASHVECTOR_H_

#include "os/c.h"
#include "util/int.h"
#include "util/new.h"
#include "util/noexcept.h"

// HashVector
//
// Simple array indexable by a key's hash, but without storing a copy of the
// key.
//
// Notes:
//   - Breaks if there is a hash collision.
//   - Does not run move constructors during array growth.
template<typename Value>
class HashVector {
 public:
    HashVector() noexcept : storage(0), allocated(0), used(0) {}
    ~HashVector() noexcept {
        for (Entry* e = storage; e < storage + used; e++) {
            e->value.~Value();
        }
        free(storage);
    }

    Value&
    allocate(uint32_t hash) noexcept {
        if (used == allocated) {
            grow();
        }

        Entry& e = storage[used++];
        e.hash = hash;

        return e.value;
    }

    Value*
    find(uint32_t hash) noexcept {
        for (Entry* e = storage; e < storage + used; e++) {
            if (e->hash == hash) {
                return &e->value;
            }
        }

        return 0;
    }

 protected:
    void
    grow() noexcept {
        uint32_t newAllocated = allocated == 0 ? 4 : allocated * 2;

        Entry* newStorage =
            reinterpret_cast<Entry*>(malloc(sizeof(Entry) * newAllocated));
        memcpy(newStorage, storage, sizeof(Entry) * allocated);
        free(storage);

        storage = newStorage;
        allocated = newAllocated;
    }

    struct Entry {
        uint32_t hash;
        Value value;
    };

    Entry* storage;
    uint32_t allocated;
    uint32_t used;
};

#endif  // SRC_UTIL_HASHVECTOR_H_
