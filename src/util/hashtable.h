/********************************
** Tsunagari Tile Engine       **
** hashtable.h                 **
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

#ifndef SRC_UTIL_HASHTABLE_H_
#define SRC_UTIL_HASHTABLE_H_

#include "util/assert.h"
#include "util/hash.h"
#include "util/int.h"
#include "util/noexcept.h"
#include "util/vector.h"

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
        Key key;
        Value value;
    };

    struct iterator {
        iterator(const iterator& other) noexcept
                : hm(other.hm), idx(other.idx) {}

        bool
        operator==(const iterator& other) const noexcept {
            return other.hm == hm && other.idx == idx;
        }
        bool
        operator!=(const iterator& other) const noexcept {
            return !(other == *this);
        }

        void
        operator++() noexcept {
            ++idx;
            advancePastEmpty();
        }

        Entry& operator*() const noexcept { return hm->buckets[idx]; }
        Entry* operator->() const noexcept { return &hm->buckets[idx]; }

     private:
        explicit iterator(Hashmap* hm) noexcept : hm(hm) { advancePastEmpty(); }
        explicit iterator(Hashmap* hm, size_t idx) noexcept
                : hm(hm), idx(idx) {}

        void
        advancePastEmpty() noexcept {
            while (idx < hm->buckets.size &&
                   hm->buckets[idx].key == E::value()) {
                ++idx;
            }
        }

        Hashmap* hm = nullptr;
        size_t idx = 0;
        friend Hashmap;
    };

 public:
    // FIXME: Starting size should be 0.
    //        Make reserve() check for 0.
    Hashmap(size_t bucket_count = 8) noexcept {
        size_t pow2 = 1;
        while (pow2 < bucket_count) {
            pow2 <<= 1;
        }

        // buckets.resize(pow2);
        // FIXME: Can this be made more efficient?
        buckets.reserve(pow2);
        for (size_t i = 0; i < pow2; i++) {
            buckets.push_back(Entry{E::value(), Value()});
        }
    }

    // Operators
    void
    operator=(const Hashmap& other) = delete;

    // Iterators
    iterator
    begin() noexcept {
        return iterator(this);
    }
    iterator
    end() noexcept {
        return iterator(this, buckets.size);
    }

    // Capacity
    bool
    empty() noexcept {
        return size() == 0;
    }

    size_t
    size() noexcept {
        return count;
    }

    // Write
    void
    erase(iterator it) noexcept {
        size_t bucket = it.idx;
        for (size_t idx = probeNext(bucket);; idx = probeNext(idx)) {
            if (buckets[idx].key == E::value()) {
                buckets[bucket].key = E::value();
                count--;
                return;
            }
            size_t ideal = keyToIdx(buckets[idx].key);
            if (diff(bucket, ideal) < diff(idx, ideal)) {
                // Swap. Bucket is closer to ideal than idx.
                buckets[bucket] = buckets[idx];
                bucket = idx;
            }
        }
    }
    void
    erase(const Key& x) noexcept {
        auto it = find(x);
        if (it != end()) {
            erase(it);
        }
    }

    // Read
    Value& operator[](const Key& key) noexcept {
        assert_(key != E::value());  // Empty key shouldn't be used.
        reserve(count + 1);
        for (size_t idx = keyToIdx(key);; idx = probeNext(idx)) {
            if (buckets[idx].key == E::value()) {
                buckets[idx].value = Value();
                buckets[idx].key = key;
                count++;
                return buckets[idx].value;
            }
            else if (buckets[idx].key == key) {
                return buckets[idx].value;
            }
        }
    }
    Value& operator[](Key&& key) noexcept {
        assert_(key != E::value());  // Empty key shouldn't be used.
        reserve(count + 1);
        for (size_t idx = keyToIdx(key);; idx = probeNext(idx)) {
            if (buckets[idx].key == E::value()) {
                buckets[idx].value = Value();
                buckets[idx].key = static_cast<Key&&>(key);
                count++;
                return buckets[idx].value;
            }
            else if (buckets[idx].key == key) {
                return buckets[idx].value;
            }
        }
    }

    template<typename K>
    iterator
    find(const K& x) noexcept {
        assert_(x != E::value());  // Empty key shouldn't be used.
        for (size_t idx = keyToIdx(x);; idx = probeNext(idx)) {
            if (buckets[idx].key == E::value()) {
                return end();
            }
            if (buckets[idx].key == x) {
                return iterator(this, idx);
            }
        }
    }

    template<typename K>
    Value&
    at(const Key& x) noexcept {
        iterator it = find(x);
        assert_(it != end());
        return it->value;
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
    contains(const K& x) noexcept {
        return find(x) != end();
    }

    // Utility
    void
    reserve(size_t count) noexcept {
        if (count * 2 > buckets.size) {
            rehash(count * 2);
        }
    }

 private:
    Hashmap(Hashmap& other, size_t bucket_count) noexcept
            : Hashmap(bucket_count) {
        for (iterator it = other.begin(); it != other.end(); ++it) {
            operator[](it->key) = it->value;
        }
    }

    // Hash policy
    void
    rehash(size_t count) noexcept {
        // FIXME: Support count = 0.
        Hashmap other(*this, count * 2);

        buckets = static_cast<Vector<Entry>&&>(other.buckets);
        count = other.count;

        // TODO: Use move constructors, not copy constructors.
        // FIXME: Do not use a Vector as backing.
        /*
        for (auto it = other.begin(); it != other.end(); ++it) {
            insert(*it);
        }
        */
    }

    template<typename K>
    size_t
    keyToIdx(const K& key) noexcept {
        const size_t mask = buckets.size - 1;
        return hash_(key) & mask;
    }

    size_t
    probeNext(size_t idx) noexcept {
        const size_t mask = buckets.size - 1;
        return (idx + 1) & mask;
    }

    size_t
    diff(size_t a, size_t b) noexcept {
        const size_t mask = buckets.size - 1;
        return (buckets.size + a - b) & mask;
    }

 private:
    // FIXME: Only need three fields, not four. Replace Vector.
    Vector<Entry> buckets;
    size_t count = 0;
};

#endif  // SRC_UTIL_HASHTABLE_H_
