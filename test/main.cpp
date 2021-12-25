#include "util/compiler.h"
#include "util/io.h"

void
testUtilStringView() noexcept;

int
main() noexcept {
    Flusher f1(sout);
    Flusher f2(serr);

    testUtilStringView();

    return 0;
}
