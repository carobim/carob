// Original source downloaded from: https://github.com/vivkin/gason
// Original source downloaded from: https://github.com/azadkuh/gason--

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Ivan Vashchaev
 * Copyright (c) 2014 Amir Zamani
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

// TODO: Look into performance improvements in:
//       https://github.com/strangenaiad/gason

#include "util/json.h"

#include "util/compiler.h"
#include "util/int.h"
#include "util/new.h"
#include "util/string-view.h"
#include "util/string.h"

#define JSON_ZONE_SIZE  4096
#define JSON_STACK_SIZE 32

JsonAllocator::JsonAllocator() noexcept : head(0) { }
JsonAllocator::JsonAllocator(JsonAllocator&& other) noexcept
    : head(other.head) {
    other.head = 0;
}
JsonAllocator::~JsonAllocator() noexcept {
    deallocate();
}

void*
JsonAllocator::allocate(Size size) noexcept {
    size = (size + 7) & ~7;

    if (head && head->used + size <= JSON_ZONE_SIZE) {
        char* p = reinterpret_cast<char*>(head) + head->used;
        head->used += size;
        return p;
    }

    Size allocSize = sizeof(Zone) + size;
    Zone* zone = static_cast<Zone*>(
        malloc(allocSize <= JSON_ZONE_SIZE ? JSON_ZONE_SIZE : allocSize));
    if (zone == 0)
        return 0;
    zone->used = allocSize;
    if (allocSize <= JSON_ZONE_SIZE || head == 0) {
        zone->next = head;
        head = zone;
    }
    else {
        zone->next = head->next;
        head->next = zone;
    }
    return reinterpret_cast<char*>(zone) + sizeof(Zone);
}

void
JsonAllocator::deallocate() noexcept {
    while (head) {
        Zone* next = head->next;
        free(head);
        head = next;
    }
}

static inline bool
isspace(char c) noexcept {
    return c == ' ' || (c >= '\t' && c <= '\r');
}

static inline bool
isdelim(char c) noexcept {
    return c == ',' || c == ':' || c == ']' || c == '}' || isspace(c) || !c;
}

static inline bool
isdigit(char c) noexcept {
    return c >= '0' && c <= '9';
}

static inline bool
isxdigit(char c) noexcept {
    return (c >= '0' && c <= '9') || ((c & ~' ') >= 'A' && (c & ~' ') <= 'F');
}

static inline I32
char2int(char c) noexcept {
    if (c <= '9')
        return c - '0';
    return (c & ~' ') - 'A' + 10;
}

static double
string2double(char* s, char** endptr) noexcept {
    char ch = *s;
    if (ch == '-')
        ++s;

    double result = 0;
    while (isdigit(*s))
        result = (result * 10) + (*s++ - '0');

    if (*s == '.') {
        ++s;

        double fraction = 1;
        while (isdigit(*s)) {
            fraction *= 0.1;
            result += (*s++ - '0') * fraction;
        }
    }

    if (*s == 'e' || *s == 'E') {
        ++s;

        double base = 10;
        if (*s == '+') {
            ++s;
        }
        else if (*s == '-') {
            ++s;
            base = 0.1;
        }

        U32 exponent = 0;
        while (isdigit(*s))
            exponent = (exponent * 10) + (*s++ - '0');

        double power = 1;
        for (; exponent; exponent >>= 1, base *= base)
            if (exponent & 1)
                power *= base;

        result *= power;
    }

    *endptr = s;
    return ch == '-' ? -result : result;
}

static inline JsonNode*
insertAfter(JsonNode* tail, JsonNode* node) noexcept {
    if (!tail)
        return node->next = node;
    node->next = tail->next;
    tail->next = node;
    return node;
}

static inline JsonValue
listToValue(JsonTag tag, JsonNode* tail) noexcept {
    if (tail) {
        JsonNode* head = tail->next;
        tail->next = 0;
        return JsonValue(tag, head);
    }
    return JsonValue(tag, 0);
}

static bool
parse(char* s, JsonValue* value, JsonAllocator& allocator) noexcept {
    JsonNode* tails[JSON_STACK_SIZE];
    JsonTag tags[JSON_STACK_SIZE];
    char* keys[JSON_STACK_SIZE];
    JsonValue o;
    SSize pos = -1;
    bool separator = true;
    JsonNode* node;
    char* endptr = s;

    while (*s) {
        while (isspace(*s)) {
            ++s;
            if (!*s)
                break;
        }
        endptr = s++;
        switch (*endptr) {
        case '-':
            if (!isdigit(*s) && *s != '.') {
                endptr = s;
                return false;
            }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            o = JsonValue(string2double(endptr, &s));
            if (!isdelim(*s)) {
                endptr = s;
                return false;
            }
            break;
        case '"':
            o = JsonValue(JSON_STRING, s);
            for (char* it = s; *s; ++it, ++s) {
                I32 c = *it = *s;
                if (c == '\\') {
                    c = *++s;
                    switch (c) {
                    case '\\':
                    case '"':
                    case '/': *it = c; break;
                    case 'b': *it = '\b'; break;
                    case 'f': *it = '\f'; break;
                    case 'n': *it = '\n'; break;
                    case 'r': *it = '\r'; break;
                    case 't': *it = '\t'; break;
                    case 'u':
                        c = 0;
                        for (I32 i = 0; i < 4; ++i) {
                            if (isxdigit(*++s)) {
                                c = c * 16 + char2int(*s);
                            }
                            else {
                                endptr = s;
                                return false;
                            }
                        }
                        if (c < 0x80) {
                            *it = c;
                        }
                        else if (c < 0x800) {
                            *it++ = 0xC0 | (c >> 6);
                            *it = 0x80 | (c & 0x3F);
                        }
                        else {
                            *it++ = 0xE0 | (c >> 12);
                            *it++ = 0x80 | ((c >> 6) & 0x3F);
                            *it = 0x80 | (c & 0x3F);
                        }
                        break;
                    default: endptr = s; return false;
                    }
                }
                else if (static_cast<U32>(c) < ' ' || c == '\x7F') {
                    endptr = s;
                    return false;
                }
                else if (c == '"') {
                    *it = 0;
                    ++s;
                    break;
                }
            }
            if (!isdelim(*s)) {
                endptr = s;
                return false;
            }
            break;
        case 't':
            if (!(s[0] == 'r' && s[1] == 'u' && s[2] == 'e' && isdelim(s[3])))
                return false;
            o = JsonValue(JSON_TRUE);
            s += 3;
            break;
        case 'f':
            if (!(s[0] == 'a' && s[1] == 'l' && s[2] == 's' && s[3] == 'e' &&
                  isdelim(s[4]))) {
                return false;
            }
            o = JsonValue(JSON_FALSE);
            s += 4;
            break;
        case 'n':
            if (!(s[0] == 'u' && s[1] == 'l' && s[2] == 'l' && isdelim(s[3])))
                return false;
            o = JsonValue(JSON_NULL);
            s += 3;
            break;
        case ']':
            if (pos == -1)
                return false;
            if (tags[pos] != JSON_ARRAY)
                return false;
            o = listToValue(JSON_ARRAY, tails[pos--]);
            break;
        case '}':
            if (pos == -1)
                return false;
            if (tags[pos] != JSON_OBJECT)
                return false;
            if (keys[pos] != 0)
                return false;
            o = listToValue(JSON_OBJECT, tails[pos--]);
            break;
        case '[':
            if (++pos == JSON_STACK_SIZE)
                return false;
            tails[pos] = 0;
            tags[pos] = JSON_ARRAY;
            keys[pos] = 0;
            separator = true;
            continue;
        case '{':
            if (++pos == JSON_STACK_SIZE)
                return false;
            tails[pos] = 0;
            tags[pos] = JSON_OBJECT;
            keys[pos] = 0;
            separator = true;
            continue;
        case ':':
            if (separator || keys[pos] == 0)
                return false;
            separator = true;
            continue;
        case ',':
            if (separator || keys[pos] != 0)
                return false;
            separator = true;
            continue;
        case '\0': continue;
        default: return false;
        }

        separator = false;

        if (pos == -1) {
            endptr = s;
            *value = o;
            return true;
        }

        if (tags[pos] == JSON_OBJECT) {
            if (!keys[pos]) {
                if (o.getTag() != JSON_STRING)
                    return false;
                keys[pos] = o.toCString();
                continue;
            }
            if ((node = reinterpret_cast<JsonNode*>(
                     allocator.allocate(sizeof(JsonNode)))) == 0) {
                return false;
            }
            tails[pos] = insertAfter(tails[pos], node);
            tails[pos]->key = keys[pos];
            keys[pos] = 0;
        }
        else {
            if ((node = reinterpret_cast<JsonNode*>(allocator.allocate(
                     sizeof(JsonNode) - sizeof(char*)))) == 0) {
                return false;
            }
            tails[pos] = insertAfter(tails[pos], node);
        }
        tails[pos]->value = o;
    }
    return false;
}

JsonValue
JsonValue::operator[](StringView key) noexcept {
    for (JsonNode* node = toNode(); node != 0; node = node->next)
        if (key == node->key)
            return node->value;

    return JsonValue();
}

void
JsonAllocator::operator=(JsonAllocator&& other) noexcept {
    head = other.head;
    other.head = 0;
}

JsonDocument::JsonDocument() noexcept : ok(false) { }

JsonDocument::JsonDocument(String text) noexcept
    : text(static_cast<String&&>(text)) {
    this->text << '\0';
    ok = parse(this->text.data, &root, allocator);
}

JsonDocument::JsonDocument(JsonDocument&& other) noexcept {
    root = other.root;
    ok = other.ok;
    text = static_cast<String&&>(other.text);
    allocator.head = other.allocator.head;

    other.ok = false;
    other.allocator.head = 0;
}

JsonDocument::~JsonDocument() noexcept {
    allocator.deallocate();
}
