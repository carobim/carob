#ifndef SRC_UTIL_HASHVECTOR_H_
#define SRC_UTIL_HASHVECTOR_H_

#include "os/c.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"

// HashVector
//
// Simple array index-able by a key's hash, but without storing a copy of the
// key.
//
// Notes:
//   - Breaks if there is a hash collision.
//   - Does not run move constructors during array growth.
template<typename Value>
class HashVector {
 public:
    HashVector() noexcept : storage(0), allocated(0), used(0) { }
    ~HashVector() noexcept {
        for (Entry* e = storage; e < storage + used; e++) {
            e->value.~Value();
        }
        free(storage);
    }

    Value&
    allocate(U32 hash) noexcept {
        if (used == allocated) {
            grow();
        }

        Entry* e = &storage[used++];
        new (&e->value) Value();
        e->hash = hash;

        return e->value;
    }

    Value*
    find(U32 hash) noexcept {
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
        U32 newAllocated = allocated == 0 ? 4 : allocated * 2;

        Entry* newStorage = xmalloc(Entry, newAllocated);
        memcpy(newStorage, storage, sizeof(Entry) * allocated);
        free(storage);

        storage = newStorage;
        allocated = newAllocated;
    }

    struct Entry {
        U32 hash;
        Value value;
    };

    Entry* storage;
    U32 allocated;
    U32 used;
};

#endif  // SRC_UTIL_HASHVECTOR_H_
