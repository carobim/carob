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

// FIXME: Flush manually instead of automatically on every '\n'.

static void
flush(State& state) noexcept {
    String& buf = state.buf;
    char* data = buf.data;
    Size size = buf.size;

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
}

Output::Output(void* data) noexcept : data(data) { }

Output&
Output::operator<<(char x) noexcept {
    STATE.buf << x;
    if (x == '\n') {
        flush(STATE);
    }
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
    if (x.find('\n') != SV_NOT_FOUND) {
        flush(STATE);
    }
    return *this;
}

Output&
Output::operator<<(bool x) noexcept {
    if (x) {
        STATE.buf << StringView("true", 4);
    }
    else {
        STATE.buf << StringView("false", 5);
    }
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
