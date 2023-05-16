#ifndef SRC_UTIL_IO_H_
#define SRC_UTIL_IO_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

struct Flush { };

class Output {
 public:
    Output(void* data) noexcept;

    Output&
    operator<<(char x) noexcept;
    Output&
    operator<<(const char* x) noexcept;
    Output&
    operator<<(StringView x) noexcept;

    Output&
    operator<<(bool b) noexcept;
    Output&
    operator<<(int i) noexcept;
    Output&
    operator<<(unsigned int u) noexcept;
    Output&
    operator<<(long l) noexcept;
    Output&
    operator<<(unsigned long ul) noexcept;
    Output&
    operator<<(long long ll) noexcept;
    Output&
    operator<<(unsigned long long ull) noexcept;
    Output&
    operator<<(float f) noexcept;

    Output& operator<<(Flush) noexcept;

 public:
    // Platform dependent.
    void* data;
};

struct Flusher {
    Flusher(Output& stream) noexcept;
    ~Flusher() noexcept;

    Output& stream;
};

extern Output sout;
extern Output serr;

#endif  // SRC_UTIL_IO_H_
