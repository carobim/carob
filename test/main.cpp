#include "util/compiler.h"
#include "util/io.h"

void
testUtilString2() noexcept;
void
testUtilStringView() noexcept;
void
testUtilVector() noexcept;

I32
main() noexcept {
    Flusher f1(sout);
    Flusher f2(serr);

    testUtilString2();
    testUtilStringView();
    testUtilVector();

    return 0;
}
