#include "util/assert.h"
#include "util/compiler.h"
#include "util/vector.h"

void
testUtilVector() noexcept {
    Vector<int> v;

    v.push(1);
    v.push(2);
    v.push(3);

    // Erasing the last element must shrink the vector.
    v.eraseUnordered(2);
    assert_(v.size == 2);
    assert_(v[0] == 1);
    assert_(v[1] == 2);

    // Erasing a non-last element moves the last element into its place.
    v.eraseUnordered(0);
    assert_(v.size == 1);
    assert_(v[0] == 2);

    // Erasing the only element empties the vector.
    v.eraseUnordered(0);
    assert_(v.size == 0);
}
