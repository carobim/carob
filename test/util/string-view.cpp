#include "util/assert.h"
#include "util/compiler.h"
#include "util/string-view.h"

void
testUtilStringView() noexcept {
    assert_(StringView("Hello, world!").size == 13);
}
