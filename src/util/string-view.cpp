#include "util/string-view.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/fnv.h"

StringView::StringView() noexcept : data(0), size(0) { }
StringView::StringView(const char* data) noexcept
    : data(data),
      size(strlen(data)) { }
StringView::StringView(const char* data, Size size) noexcept
    : data(data),
      size(size) { }
StringView::StringView(const StringView& s) noexcept
    : data(s.data),
      size(s.size) { }

void
StringView::operator=(const StringView& s) noexcept {
    data = s.data;
    size = s.size;
}

char
StringView::operator[](Size i) noexcept {
    return data[i];
}

const char*
StringView::begin() const noexcept {
    return data;
}
const char*
StringView::end() const noexcept {
    return data + size;
}

StringPosition
StringView::find(char needle) const noexcept {
    // Necessary check because GCC 8.3.0 assumes that the first parameter to
    // memchr cannot be 0, which can propagate a `data != 0` constraint up to
    // callers, which is a constraint that is not always true. E.g., it would
    // be nice to call this function with .data = 0 and .size = 0 and get a
    // SV_NOT_FOUND as a result and then later do a check against `.data == 0`,
    // but with GCC's assumption, that check will be optimized out.
    if (data == 0)
        return SV_NOT_FOUND;

    char* result = static_cast<char*>(memchr(data, needle, size));
    if (result == 0)
        return SV_NOT_FOUND;
    return StringPosition(result - data);
}

StringPosition
StringView::find(char needle, Size start) const noexcept {
    if (data == 0)
        return SV_NOT_FOUND;

    char* result =
        static_cast<char*>(memchr(data + start, needle, size - start));
    if (result == 0)
        return SV_NOT_FOUND;
    return StringPosition(result - data);
}

StringPosition
StringView::find(StringView needle) const noexcept {
    if (data == 0)
        return SV_NOT_FOUND;

    char* result =
        static_cast<char*>(memmem(data, size, needle.data, needle.size));
    if (result == 0)
        return SV_NOT_FOUND;
    return StringPosition(result - data);
}

StringPosition
StringView::find(StringView needle, Size start) const noexcept {
    assert_(size >= start);

    if (data == 0)
        return SV_NOT_FOUND;

    char* result = static_cast<char*>(
        memmem(data + start, size - start, needle.data, needle.size));
    if (result == 0)
        return SV_NOT_FOUND;
    return StringPosition(result - data);
}

StringPosition
StringView::rfind(char needle) const noexcept {
    if (size == 0)
        return SV_NOT_FOUND;

    Size i = size;
    do {
        i--;
        if (data[i] == needle)
            return StringPosition(i);
    } while (i > 0);

    return SV_NOT_FOUND;
}

StringPosition
StringView::rfind(StringView needle) const noexcept {
    if (data == 0 || needle.size > size)
        return SV_NOT_FOUND;
    if (needle.size == 0)
        return 0;

    Size i = size - needle.size;
    do {
        if (data[i] == needle[0]) {
            if (substr(i, needle.size) == needle)
                return StringPosition(i);
        }
    } while (i-- > 0);

    return SV_NOT_FOUND;
}

bool
StringView::startsWith(StringView needle) const noexcept {
    if (size < needle.size)
        return false;
    return memcmp(data, needle.data, needle.size) == 0;
}
bool
StringView::endsWith(StringView needle) const noexcept {
    if (size < needle.size)
        return false;
    return memcmp(data + size - needle.size, needle.data, needle.size) == 0;
}

StringView
StringView::substr(const Size from) const noexcept {
    assert_(from <= this->size);
    return StringView(data + from, size - from);
}
StringView
StringView::substr(const Size from, const Size span) const noexcept {
    assert_(from <= size);
    assert_(from + span <= size);
    return StringView(data + from, span);
}

bool
operator==(const StringView& a, const StringView& b) noexcept {
    return (a.size == b.size) && memcmp(a.data, b.data, a.size) == 0;
}

bool
operator!=(const StringView& a, const StringView& b) noexcept {
    return !(a == b);
}

bool
operator>(const StringView& a, const StringView& b) noexcept {
    Size s = a.size < b.size ? a.size : b.size;
    const char* ad = a.data;
    const char* bd = b.data;

    while (s--) {
        if (*ad != *bd)
            return *ad > *bd;
        ad++;
        bd++;
    }
    if (a.size != b.size)
        return a.size > b.size;
    return false;
}

bool
operator<(const StringView& a, const StringView& b) noexcept {
    Size s = a.size < b.size ? a.size : b.size;
    const char* ad = a.data;
    const char* bd = b.data;

    while (s--) {
        if (*ad != *bd)
            return *ad < *bd;
        ad++;
        bd++;
    }
    if (a.size != b.size)
        return a.size < b.size;
    return false;
}

Size
hash_(StringView s) noexcept {
    return fnvHash(s.data, s.size);
}

Size
hash_(const char* s) noexcept {
    return hash_(StringView(s));
}
