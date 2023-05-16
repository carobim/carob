#ifndef SRC_UTIL_POOL_H_
#define SRC_UTIL_POOL_H_

#include "os/c.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"

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
    typedef U32 Link;

 public:
    Pool() : storage(0), allocated(0), nextFree(POOL_END) { }
    ~Pool() { free(storage); }

    // Returns an unconstructed piece of memory.
    U32
    allocate() noexcept {
        if (nextFree == POOL_END)
            grow();
        U32 id = nextFree;
        nextFree = asLink(nextFree);
        return id;
    }

    // Releases memory without destructing the object within.
    void
    release(U32 i) noexcept {
        assert_(0 <= i && i < allocated);
        asLink(i) = nextFree;
        nextFree = i;
    }

    T&
    operator[](U32 i) noexcept {
        assert_(0 <= i && i < allocated);
        return storage[i];
    }

 private:
    Pool(const Pool&) { }

    Link&
    asLink(U32 i) noexcept {
        return *reinterpret_cast<Link*>(reinterpret_cast<char*>(&storage[i]));
    }

    void
    grow() noexcept {
        U32 newAllocated = allocated == 0 ? 4 : allocated * 2;

        T* newStorage = xmalloc(T, newAllocated);
        memcpy(newStorage, storage, sizeof(T) * allocated);
        free(reinterpret_cast<char*>(storage));

        nextFree = allocated;
        storage = newStorage;

        for (U32 i = allocated; i < newAllocated - 1; i++)
            asLink(i) = i + 1;
        asLink(newAllocated - 1) = POOL_END;

        allocated = newAllocated;
    }

    T* storage;
    U32 allocated;
    Link nextFree;
};

#endif  // SRC_UTIL_POOL_H_
