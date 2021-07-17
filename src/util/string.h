#ifndef SRC_UTIL_STRING_H_
#define SRC_UTIL_STRING_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

class String {
 public:
    char* data;
    size_t size;
    size_t capacity;

 public:
    String() noexcept;
    String(const char* s) noexcept;
    String(StringView s) noexcept;
    String(const String& s) noexcept;
    String(String&& s) noexcept;
    ~String() noexcept;

    void
    operator=(const char* s) noexcept;
    void
    operator=(StringView s) noexcept;

    void
    operator=(const String& s) noexcept;
    void
    operator=(String&& s) noexcept;

    char&
    operator[](size_t i) noexcept;

    String&
    operator<<(char c) noexcept;
    String&
    operator<<(const char* s) noexcept;
    String&
    operator<<(StringView s) noexcept;

    String&
    operator<<(bool b) noexcept;
    String&
    operator<<(int i) noexcept;
    String&
    operator<<(unsigned int u) noexcept;
    String&
    operator<<(long l) noexcept;
    String&
    operator<<(unsigned long ul) noexcept;
    String&
    operator<<(long long ll) noexcept;
    String&
    operator<<(unsigned long long ull) noexcept;
    String&
    operator<<(float f) noexcept;

    void
    resize(size_t n) noexcept;
    void
    reserve(size_t n) noexcept;
    void
    clear() noexcept;
    void
    reset() noexcept;

    operator StringView() const noexcept;
    StringView
    view() const noexcept;

    const char*
    null() noexcept;
};

bool
operator<(const String& a, const String& b) noexcept;
bool
operator>(const String& a, const String& b) noexcept;

size_t
hash_(const String& s) noexcept;

#endif  // SRC_UTIL_STRING_H_
