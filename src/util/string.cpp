#include "util/string.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/fnv.h"
#include "util/new.h"

static Size
grow1(Size current) noexcept {
    return current < 4 ? 4 : current * 2;
}

static Size
growN(Size current, Size addition) noexcept {
    Size newSize = current == 0 ? 4 : current * 2;
    while (newSize < current + addition)
        newSize *= 2;
    return newSize;
}

String::String() noexcept : data(0), size(0), capacity(0) { }

String::String(const char* s) noexcept {
    if (s == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        Size len = strlen(s);
        // FIXME: Choose better size.
        // Make it a valid C-string.
        data = xmalloc(char, len + 1);
        size = len;
        capacity = len + 1;
        memcpy(data, s, len);
        data[len] = 0;
    }
}

String::String(StringView s) noexcept {
    if (s.size == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        // FIXME: Choose better size.
        data = xmalloc(char, s.size);
        size = capacity = s.size;
        memcpy(data, s.data, s.size);
    }
}

String::String(const String& s) noexcept {
    if (s.capacity == 0) {
        data = 0;
        size = capacity = 0;
    }
    else {
        data = xmalloc(char, s.capacity);
        size = s.size;
        capacity = s.capacity;
        memcpy(data, s.data, size);
    }
}

String::String(String&& s) noexcept {
    data = s.data;
    size = s.size;
    capacity = s.capacity;
    s.data = 0;
    s.size = s.capacity = 0;
}

String::~String() noexcept {
    free(data);
}

void
String::operator=(const char* s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(StringView s) noexcept {
    clear();
    *this << s;
}

void
String::operator=(const String& s) noexcept {
    assert_(this != &s);
    clear();
    *this << s;
}

void
String::operator=(String&& s) noexcept {
    assert_(this != &s);
    free(data);
    data = s.data;
    size = s.size;
    capacity = s.capacity;
    s.data = 0;
    s.size = s.capacity = 0;
}

char&
String::operator[](Size i) noexcept {
    assert_(i < size);
    return data[i];
}

String&
String::operator<<(char c) noexcept {
    if (size == capacity)
        reserve(grow1(size));
    data[size++] = c;
    return *this;
}

String&
String::operator<<(const char* s) noexcept {
    // TODO: Use strncpy without knowing the size. If the string has more
    //       characters, then do a strlen & growN & memcpy like below.
    Size len = strlen(s);
    if (capacity < size + len)
        reserve(growN(size, len));
    memcpy(data + size, s, len);
    size += len;
    return *this;
}

String&
String::operator<<(StringView s) noexcept {
    if (capacity < size + s.size)
        reserve(growN(size, s.size));
    memcpy(data + size, s.data, s.size);
    size += s.size;
    return *this;
}

String&
String::operator<<(bool b) noexcept {
    if (capacity < size + 5)
        reserve(growN(size, 5));
    if (b) {
        memcpy(data + size, "true", 4);
        size += 4;
    }
    else {
        memcpy(data + size, "false", 5);
        size += 5;
    }
    return *this;
}

String&
String::operator<<(int i) noexcept {
    // Minus sign & 10 digits.
    if (capacity < size + 11)
        reserve(growN(size, 11));

    if (i < 0) {
        if (i == INT32_MIN) {
            memcpy(data + size, "-2147483648", 11);
            size += 11;
            return *this;
        }

        data[size++] = '-';
        i = -i;
    }

    char buf[10];
    char* p = buf;

    do {
        *p++ = static_cast<char>(i % 10) + '0';
        i /= 10;
    } while (i > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(unsigned int u) noexcept {
    if (capacity < size + 10)
        reserve(growN(size, 10));

    char buf[10];
    char* p = buf;

    do {
        *p++ = static_cast<char>(u % 10) + '0';
        u /= 10;
    } while (u > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(long l) noexcept {
#if MSVC
    return *this << static_cast<int>(l);
#else
    return *this << static_cast<long long>(l);
#endif
}

String&
String::operator<<(unsigned long ul) noexcept {
#if MSVC
    return *this << static_cast<unsigned int>(ul);
#else
    return *this << static_cast<unsigned long long>(ul);
#endif
}

String&
String::operator<<(long long ll) noexcept {
    if (capacity < size + 20)
        reserve(growN(size, 20));

    if (ll < 0) {
        if (ll == INT64_MIN) {
            memcpy(data + size, "-9223372036854775808", 20);
            size += 20;
            return *this;
        }

        data[size++] = '-';
        ll = -ll;
    }

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(ll % 10) + '0';
        ll /= 10;
    } while (ll > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(unsigned long long ull) noexcept {
    if (capacity < size + 20)
        reserve(growN(size, 20));

    char buf[20];
    char* p = buf;

    do {
        *p++ = static_cast<char>(ull % 10) + '0';
        ull /= 10;
    } while (ull > 0);

    do {
        data[size++] = *--p;
    } while (p != buf);

    return *this;
}

String&
String::operator<<(float f) noexcept {
    char buf[64];
    sprintf(buf, "%f", f);
    return *this << buf;
}

void
String::reserve(Size n) noexcept {
    assert_(capacity < n);
    char* newData = xmalloc(char, n);  // TODO: Use realloc
    memmove(newData, data, size);
    data = newData;
    capacity = n;
}

void
String::ensure(Size n) noexcept {
    if (capacity < size + n)
        reserve(growN(size, n));
}

void
String::resize(Size n) noexcept {
    if (capacity < n)
        reserve(n);
    size = n;
}

void
String::clear() noexcept {
    size = 0;
}

void
String::reset() noexcept {
    data = 0;
    size = capacity = 0;
}

String::operator StringView() const noexcept {
    return StringView(data, size);
}

StringView
String::view() const noexcept {
    return StringView(data, size);
}

const char*
String::null() noexcept {
    if (size == capacity)
        reserve(grow1(size));
    data[size] = 0;
    return data;
}

bool
operator<(const String& a, const String& b) noexcept {
    return a.view() < b.view();
}

bool
operator>(const String& a, const String& b) noexcept {
    return a.view() > b.view();
}

Size
hash_(const String& s) noexcept {
    return fnvHash(s.data, s.size);
}
