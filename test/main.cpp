#include "util/compiler.h"
#include "util/io.h"

void
testUtilStringView() noexcept;

I32
main() noexcept {
    Flusher f1(sout);
    Flusher f2(serr);

    testUtilStringView();

    return 0;
}
