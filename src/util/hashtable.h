#ifndef SRC_UTIL_HASHTABLE_H_
#define SRC_UTIL_HASHTABLE_H_

// Original source downloaded from: https://github.com/rigtorp/HashMap

/*
 * Copyright (c) 2017 Erik Rigtorp <erik@rigtorp.se>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "util/assert.h"
#include "util/compiler.h"
#include "util/hash.h"
#include "util/int.h"
#include "util/math2.h"
#include "util/new.h"

/*
 * HashMap
 *
 * A high performance hash map. Uses open addressing with linear probing.
 *
 * Advantages:
 *   - Predictable performance. Doesn't use the allocator unless load factor
 *     grows beyond 50%. Linear probing ensures cache efficency.
 *   - Deletes items by rearranging items and marking slots as empty instead of
 *     marking items as deleted. This keeps performance high when there is a
 *     high rate of churn (many paired inserts and deletes) since otherwise
 *     most slots would be marked deleted and probing would end up scanning
 *     most of the table.
 *
 * Disadvantages:
 *   - Significant performance degradation at high load factors.
 *   - Maximum load factor hard coded to 50%, memory inefficient.
 *   - Memory is not reclaimed on erase.
 */

// Ripe for specialization in the case of primitives (which may not initialize
// correctly) or where having a default value as a key is necessary.
template<typename T>
struct Empty {
    static T
    value() noexcept {
        return T();
    }
};

template<typename Key, typename Value, typename E = Empty<Key>>
class Hashmap {
 public:
    struct Entry {
        Entry() : key(E::value()), value() { }

        Key key;
        Value value;
    };

    struct iterator {
        iterator(const iterator& other) noexcept : h(other.h), i(other.i) { }

        bool
        operator==(iterator other) noexcept {
            assert_(h == other.h);
            return i == other.i;
        }
        bool
        operator!=(iterator other) noexcept {
            assert_(h == other.h);
            return i != other.i;
        }

        void
        operator++() noexcept {
            i++;
            skipEmpties();
        }

        Entry&
        operator*() noexcept {
            return h->data[i];
        }
        Entry*
        operator->() noexcept {
            return &h->data[i];
        }

     private:
        explicit iterator(Hashmap* h) noexcept : h(h), i(0) { skipEmpties(); }
        explicit iterator(Hashmap* h, U32 i) noexcept : h(h), i(i) { }

        void
        skipEmpties() noexcept {
            while (i < h->capacity && h->data[i].key == E::value()) {
                i++;
            }
        }

        Hashmap* h;
        U32 i;
        friend class Hashmap;
    };

 public:
    Hashmap(U32 bucketCount = 0) noexcept {
        size = 0;
        capacity = 0;
        data = 0;

        if (bucketCount) {
            capacity = pow2(bucketCount);
            data = static_cast<Entry*>(malloc(sizeof(Entry) * capacity));
            for (Size i = 0; i < capacity; i++) {
                new (data + i) Entry;
            }
        }
    }

    ~Hashmap() noexcept {
        for (Size i = 0; i < capacity; i++) {
            data[i].~Entry();
        }
        free(data);
    }

    // Iterators
    iterator
    begin() noexcept {
        return iterator(this);
    }
    iterator
    end() noexcept {
        return iterator(this, capacity);
    }

    // Read/write
    Value&
    operator[](const Key& key) noexcept {
        assert_(key != E::value());  // Empty key shouldn't be used.
        reserve(size + 1);
        for (U32 idx = keyToIdx(key);; idx = probe(idx)) {
            if (data[idx].key == E::value()) {
                data[idx].value = Value();
                data[idx].key = key;
                size++;
                return data[idx].value;
            }
            else if (data[idx].key == key) {
                return data[idx].value;
            }
        }
    }
    Value&
    operator[](Key&& key) noexcept {
        assert_(key != E::value());  // Empty key shouldn't be used.
        reserve(size + 1);
        for (U32 idx = keyToIdx(key);; idx = probe(idx)) {
            if (data[idx].key == E::value()) {
                data[idx].value = Value();
                data[idx].key = static_cast<Key&&>(key);
                size++;
                return data[idx].value;
            }
            else if (data[idx].key == key) {
                return data[idx].value;
            }
        }
    }

    // Write
    void
    erase(iterator it) noexcept {
        U32 bucket = it.i;
        for (U32 idx = probe(bucket);; idx = probe(idx)) {
            if (data[idx].key == E::value()) {
                data[bucket].key = E::value();
                size--;
                return;
            }
            U32 ideal = keyToIdx(data[idx].key);
            if (diff(bucket, ideal) < diff(idx, ideal)) {
                // Swap. Bucket is closer to ideal than idx.
                data[bucket] = data[idx];
                bucket = idx;
            }
        }
    }
    void
    erase(const Key& k) noexcept {
        iterator it = find(k);
        assert_(it != end());
        erase(it);
    }
    void
    clear() noexcept {
        for (Size i = 0; i < capacity; i++) {
            data[i] = Entry();
        }
        size = 0;
    }

    // Read
    template<typename K>
    iterator
    find(const K& k) noexcept {
        assert_(k != E::value());  // Empty key shouldn't be used.
        if (size == 0) {
            return end();
        }
        for (U32 idx = keyToIdx(k);; idx = probe(idx)) {
            if (data[idx].key == E::value()) {
                return end();
            }
            if (data[idx].key == k) {
                return iterator(this, idx);
            }
        }
    }
    template<typename K>
    Value*
    tryAt(const K& key) noexcept {
        iterator it = find(key);
        if (it == end()) {
            return 0;
        }
        else {
            return &it->value;
        }
    }
    template<typename K>
    bool
    contains(const K& k) noexcept {
        return find(k) != end();
    }

    // Utility
    void
    reserve(U32 size_) noexcept {
        // Only allow a 50% load limit.
        if (size_ * 2 > capacity) {
            U32 pow = pow2(size_ * 2);
            rehash(pow > 8 ? pow : 8);
        }
    }

 private:
    void
    operator=(const Hashmap& other);

    void
    rehash(U32 newCapacity) noexcept {
        // Must be power of 2.
        assert_((newCapacity & (newCapacity - 1)) == 0);

        U32 oldCapacity = capacity;
        Entry* oldData = data;

        size = 0;
        capacity = newCapacity;
        data = static_cast<Entry*>(malloc(sizeof(Entry) * capacity));

        for (U32 i = 0; i < capacity; i++) {
            new (data + i) Entry;
        }

        for (U32 i = 0; i < oldCapacity; i++) {
            Key& key = oldData[i].key;
            Value& value = oldData[i].value;

            if (key != E::value()) {
                operator[](static_cast<Key&&>(key)) =
                    static_cast<Value&&>(value);
            }

            oldData[i].~Entry();
        }

        free(oldData);
    }

    template<typename K>
    U32
    keyToIdx(const K& key) noexcept {
        U32 mask = capacity - 1;
        return hash_(key) & mask;
    }

    U32
    probe(U32 idx) noexcept {
        U32 mask = capacity - 1;
        return (idx + 1) & mask;
    }

    U32
    diff(U32 a, U32 b) noexcept {
        U32 mask = capacity - 1;
        return (capacity + a - b) & mask;
    }

 public:
    U32 size;
    U32 capacity;
    Entry* data;
};

#endif  // SRC_UTIL_HASHTABLE_H_
