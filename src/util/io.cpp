#include "util/io.h"

#include "os/c.h"
#include "os/io.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"

struct State {
    State(bool err) : err(err) { }

    bool err;
    String buf;
};

#define STATE (*reinterpret_cast<State*>(data))

static State outstate(false);
static State errstate(true);

Output sout(&outstate);
Output serr(&errstate);

static void
flush(State& state) noexcept {
    String& buf = state.buf;
    char* data = buf.data;
    Size size = buf.size;

    if (size == 0)
        return;
    if (state.err)
        writeStderr(data, size);
    else
        writeStdout(data, size);
    buf.size -= size;

    // The below is the old code, and it does not handle flushing without a newline.

    /*
    if (size == 0) {
        return;
    }
    if (data[size - 1] == '\n') {
        if (state.err) {
            writeStderr(data, size);
        }
        else {
            writeStdout(data, size);
        }
        buf.clear();
    }
    else {
        Size len = state.buf.view().rfind('\n') + 1;
        if (state.err) {
            writeStderr(data, len);
        }
        else {
            writeStdout(data, len);
        }
        memmove(data, data + len, size - len);
        buf.size -= len;
    }
    */
}

Output::Output(void* data) noexcept : data(data) { }

Output&
Output::operator<<(char x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(const char* x) noexcept {
    *this << StringView(x, strlen(x));
    return *this;
}

Output&
Output::operator<<(StringView x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(bool x) noexcept {
    if (x)
        STATE.buf << StringView("true", 4);
    else
        STATE.buf << StringView("false", 5);
    return *this;
}

Output&
Output::operator<<(int x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(unsigned int x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(long x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(unsigned long x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(long long x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(unsigned long long x) noexcept {
    STATE.buf << x;
    return *this;
}

Output&
Output::operator<<(float x) noexcept {
    STATE.buf << x;
    return *this;
}

static char* hex = "0123456789abcdef";

Output&
Output::operator<<(void* ptr) noexcept {
    STATE.buf.ensure(2 + SIZE / 4);
    char* buf = STATE.buf.data;
    Size& size = STATE.buf.size;
    buf[size++] = '0';
    buf[size++] = 'x';
    Size i = reinterpret_cast<Size>(ptr);
#if SIZE == 64
    buf[size++] = hex[(i >> 60) & 0xf];
    buf[size++] = hex[(i >> 56) & 0xf];
    buf[size++] = hex[(i >> 52) & 0xf];
    buf[size++] = hex[(i >> 48) & 0xf];
    buf[size++] = hex[(i >> 44) & 0xf];
    buf[size++] = hex[(i >> 40) & 0xf];
    buf[size++] = hex[(i >> 36) & 0xf];
    buf[size++] = hex[(i >> 32) & 0xf];
#endif
    buf[size++] = hex[(i >> 28) & 0xf];
    buf[size++] = hex[(i >> 24) & 0xf];
    buf[size++] = hex[(i >> 20) & 0xf];
    buf[size++] = hex[(i >> 16) & 0xf];
    buf[size++] = hex[(i >> 12) & 0xf];
    buf[size++] = hex[(i >> 8) & 0xf];
    buf[size++] = hex[(i >> 4) & 0xf];
    buf[size++] = hex[(i >> 0) & 0xf];
    return *this;
}

Output&
Output::operator<<(Flush) noexcept {
    flush(STATE);
    return *this;
}

Flusher::Flusher(Output& stream) noexcept : stream(stream) { }

Flusher::~Flusher() noexcept {
    stream << Flush();
}
