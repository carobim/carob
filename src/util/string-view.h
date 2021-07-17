#ifndef SRC_UTIL_STRING_VIEW_H_
#define SRC_UTIL_STRING_VIEW_H_

#include "util/compiler.h"
#include "util/int.h"

typedef size_t StringPosition;

#define SV_NOT_FOUND SIZE_MAX

class StringView {
 public:
    const char* data;
    size_t size;

 public:
    StringView() noexcept;
    StringView(const char* data) noexcept;
    template<size_t N>
    StringView(const char (&data)[N]) noexcept : data(data),
                                                 size(N) { }
    StringView(const char* data, size_t size) noexcept;
    StringView(const StringView& s) noexcept;

    void
    operator=(const StringView& s) noexcept;

    char
    operator[](size_t i) noexcept;

    const char*
    begin() const noexcept;
    const char*
    end() const noexcept;

    StringPosition
    find(char needle) const noexcept;
    StringPosition
    find(char needle, size_t start) const noexcept;
    StringPosition
    find(StringView needle) const noexcept;
    StringPosition
    find(StringView needle, size_t start) const noexcept;
    StringPosition
    rfind(char needle) const noexcept;

    StringView
    substr(const size_t from) const noexcept;
    StringView
    substr(const size_t from, const size_t span) const noexcept;
};

bool
operator==(const StringView& a, const StringView& b) noexcept;
bool
operator!=(const StringView& a, const StringView& b) noexcept;
bool
operator>(const StringView& a, const StringView& b) noexcept;
bool
operator<(const StringView& a, const StringView& b) noexcept;

size_t
hash_(StringView s) noexcept;
size_t
hash_(const char* s) noexcept;

#endif  // SRC_UTIL_STRING_VIEW_H_
