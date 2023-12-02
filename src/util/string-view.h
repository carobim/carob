#ifndef SRC_UTIL_STRING_VIEW_H_
#define SRC_UTIL_STRING_VIEW_H_

#include "util/compiler.h"
#include "util/int.h"

typedef Size StringPosition;

#define SV_NOT_FOUND SIZE_MAX

class StringView {
 public:
    const char* data;
    Size size;

 public:
    StringView() noexcept;
    StringView(const char* data) noexcept;
    template<Size N>
    StringView(const char (&data)[N]) noexcept : data(data),
                                                 size(N) { }
    StringView(const char* data, Size size) noexcept;
    StringView(const StringView& s) noexcept;

    void
    operator=(const StringView& s) noexcept;

    char
    operator[](Size i) noexcept;

    const char*
    begin() const noexcept;
    const char*
    end() const noexcept;

    StringPosition
    find(char needle) const noexcept;
    StringPosition
    find(char needle, Size start) const noexcept;
    StringPosition
    find(StringView needle) const noexcept;
    StringPosition
    find(StringView needle, Size start) const noexcept;
    StringPosition
    rfind(char needle) const noexcept;
    StringPosition
    rfind(StringView needle) const noexcept;

    bool
    startsWith(StringView needle) const noexcept;
    bool
    endsWith(StringView needle) const noexcept;

    StringView
    substr(Size from) const noexcept;
    StringView
    substr(Size from, Size span) const noexcept;

    void
    ltrim() noexcept;
};

bool
operator==(const StringView& a, const StringView& b) noexcept;
bool
operator!=(const StringView& a, const StringView& b) noexcept;
bool
operator>(const StringView& a, const StringView& b) noexcept;
bool
operator<(const StringView& a, const StringView& b) noexcept;

Size
hash_(StringView s) noexcept;
Size
hash_(const char* s) noexcept;

#endif  // SRC_UTIL_STRING_VIEW_H_
