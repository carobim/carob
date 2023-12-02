#include "util/assert.h"
#include "util/compiler.h"
#include "util/string2.h"

void
testUtilString2() noexcept {
    I32 i;
    StringView rest;

    //
    // parse32
    //
    assert_(parseI32(&i, 0, "0") && i == 0);
    assert_(!parseI32(&i, 0, ""));
    assert_(!parseI32(&i, 0, "-"));
    assert_(!parseI32(&i, 0, "a"));
    assert_(!parseI32(&i, 0, "-a"));
    assert_(parseI32(&i, 0, "-100") && i == -100);
    assert_(!parseI32(&i, 0, "1a"));
    assert_(parseI32(&i, &rest, "1a") && rest == "a");
}
