#ifndef SRC_UTIL_VECTOR_H_
#define SRC_UTIL_VECTOR_H_

#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"

/* #include "os/c.h" */
extern "C" void*
memmove(void*, const void*, Size) noexcept;

// Does not call move constructors in its own move constructor or when growing.
template<typename X>
class Vector {
 public:
    X* data;
    Size size;
    Size capacity;

 public:
    Vector() noexcept : data(0), size(0), capacity(0) { }
    Vector(const Vector& other) noexcept {
        if (other.capacity == 0) {
            data = 0;
            size = capacity = 0;
        }
        else {
            data = xmalloc(X, other.capacity);
            size = other.size;
            capacity = other.capacity;
            for (Size i = 0; i < size; i++)
                new (data + i) X(other.data[i]);
        }
    }
    Vector(Vector&& other) noexcept {
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        other.data = 0;
        other.size = other.capacity = 0;
    }
    ~Vector() noexcept {
        for (Size i = 0; i < size; i++)
            data[i].~X();
        free(data);
    }

    void
    operator=(const Vector& other) noexcept {
        assert_(this != &other);
        for (Size i = 0; i < size; i++)
            data[i].~X();
        free(data);
        if (other.capacity == 0) {
            data = 0;
            size = capacity = 0;
        }
        else {
            data = xmalloc(X, other.capacity);
            size = other.size;
            capacity = other.capacity;
            for (Size i = 0; i < size; i++)
                new (data + i) X(other[i]);
        }
    }
    void
    operator=(Vector&& other) noexcept {
        assert_(this != &other);
        for (Size i = 0; i < size; i++)
            data[i].~X();
        free(data);
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        other.data = 0;
        other.size = other.capacity = 0;
    }

    X&
    operator[](Size i) noexcept {
        assert_(i < size);
        return data[i];
    }
    X*
    begin() noexcept {
        return data;
    }
    X*
    end() noexcept {
        return data + size;
    }

    void
    push(const X& x) noexcept {
        grow();
        new (data + size) X(x);
        size++;
    }
    void
    push(X&& x) noexcept {
        grow();
        new (data + size) X(static_cast<X&&>(x));
        size++;
    }
    void
    insert(Size i, const X& x) noexcept {
        // FIXME: Does not call move constructors.
        assert_(i <= size);
        grow();
        memmove(data + i, data + i + 1, sizeof(X) * (size - i));
        new (data + i) X(x);
        size++;
    }
    void
    insert(Size i, X&& x) noexcept {
        // FIXME: Does not call move constructors.
        assert_(i <= size);
        grow();
        memmove(data + i, data + i + 1, sizeof(X) * (size - i));
        new (data + i) X(static_cast<X&&>(x));
        size++;
    }
    void
    append(Size n, const X* xs) noexcept {
        assert_(n <= size);
        reserve(size + n);  // FIXME: Choose better size.
        for (Size i = 0; i < n; i++)
            new (data + size + i) X(xs[i]);
        size += n;
    }

    void
    pop() noexcept {
        assert_(size);
        size--;
        data[size].~X();
    }
    void
    erase(Size i) noexcept {
        assert_(i < size);
        for (Size j = i; j < size - 1; j++)
            data[j] = static_cast<X&&>(data[j + 1]);
        pop();
    }
    void
    eraseUnordered(Size i) noexcept {
        assert_(i < size);
        data[i].~X();
        if (i < size - 1) {
            data[i] = static_cast<X&&>(data[size - 1]);
            pop();
        }
    }

    // Calls move constructors (which empties the old objects), but not call
    // destructors on the just-moved objects since they are hopefully empty.
    void
    reserve(Size n) noexcept {
        assert_(n > capacity);
        X* newData = xmalloc(X, n);
        for (Size i = 0; i < size; i++)
            new (newData + i) X(static_cast<X&&>(data[i]));
        free(data);
        data = newData;
        capacity = n;
    }
    void
    resize(Size n) noexcept {
        if (n > capacity)
            reserve(n);
        if (n > size)
            for (Size i = size; i < n; i++)
                new (data + i) X;
        else if (n < size)
            for (Size i = n; i < size; i++)
                data[i].~X();
        size = n;
    }
    void
    grow() noexcept {
        if (size == capacity)
            reserve(size == 0 ? 4 : size * 2);  // FIXME: Choose better size.
    }
    void
    clear() noexcept {
        for (Size i = 0; i < size; i++)
            data[i].~X();
        size = 0;
    }
    void
    reset() noexcept {
        data = 0;
        size = capacity = 0;
    }
};

#endif  // SRC_UTIL_VECTOR_H_
