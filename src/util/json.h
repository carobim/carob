/********************************
** Tsunagari Tile Engine       **
** json.h                      **
** Copyright 2020 Paul Merrill **
********************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

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

#ifndef SRC_UTIL_JSON_H_
#define SRC_UTIL_JSON_H_

//#include "util/assert.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"

#define JSON_VALUE_NAN_MASK 0x7FF8000000000000ULL
#define JSON_VALUE_PAYLOAD_MASK 0x00007FFFFFFFFFFFULL
#define JSON_VALUE_TAG_MASK 0xF
#define JSON_VALUE_TAG_SHIFT 47

enum JsonTag {
    JSON_NUMBER = 0,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL = 0xF
};

struct JsonNode;

union JsonValue {
    uint64_t ival;
    double fval;

    inline JsonValue(double x) noexcept : fval(x) {}
    inline JsonValue(JsonTag tag = JSON_NULL,
                     void* payload = nullptr) noexcept {
        // assert_(reinterpret_cast<size_t>(payload) <=
        // JSON_VALUE_PAYLOAD_MASK);
        ival = JSON_VALUE_NAN_MASK |
               (static_cast<uint64_t>(tag) << JSON_VALUE_TAG_SHIFT) |
               reinterpret_cast<size_t>(payload);
    }

    inline bool
    isNull() noexcept {
        return getTag() == JSON_NULL;
    }

    inline bool
    isNumber() noexcept {
        return getTag() == JSON_NUMBER;
    }

    inline bool
    isBool() noexcept {
        JsonTag tag = getTag();
        return tag == JSON_TRUE || tag == JSON_FALSE;
    }

    inline bool
    isString() noexcept {
        return getTag() == JSON_STRING;
    }

    inline bool
    isNode() noexcept {
        JsonTag tag = getTag();
        return tag == JSON_ARRAY || tag == JSON_OBJECT;
    }

    inline bool
    isArray() noexcept {
        return getTag() == JSON_ARRAY;
    }

    inline bool
    isObject() noexcept {
        return getTag() == JSON_OBJECT;
    }

    inline bool
    isDouble() noexcept {
        return static_cast<int64_t>(ival) <=
               static_cast<int64_t>(JSON_VALUE_NAN_MASK);
    }

    inline JsonTag
    getTag() noexcept {
        return isDouble() ? JSON_NUMBER
                          : JsonTag((ival >> JSON_VALUE_TAG_SHIFT) &
                                    JSON_VALUE_TAG_MASK);
    }

    inline int
    toBool() noexcept {
        // assert_(isBool());
        return getTag() == JSON_TRUE;
    }

    inline int
    toInt() noexcept {
        // assert_(isDouble());
        return static_cast<int>(fval);
    }

    inline double
    toNumber() noexcept {
        // assert_(isDouble());
        return fval;
    }

    inline char*
    toCString() noexcept {
        // assert_(isString());
        return reinterpret_cast<char*>(getPayload());
    }

    inline StringView
    toString() noexcept {
        // assert_(isString());
        return StringView(reinterpret_cast<char*>(getPayload()));
    }

    inline JsonNode*
    toNode() noexcept {
        // assert_(isArray() || isObject());
        return reinterpret_cast<JsonNode*>(getPayload());
    }

    JsonValue operator[](StringView key) noexcept;

    inline uint64_t
    getPayload() noexcept {
        // assert_(!isDouble());
        return ival & JSON_VALUE_PAYLOAD_MASK;
    }
};

struct JsonNode {
    JsonValue value;
    JsonNode* next;
    char* key;
};

struct JsonIterator {
    JsonNode* node;

    JsonIterator(JsonNode* node) : node(node) {}

    inline void
    operator++() noexcept {
        node = node->next;
    }

    inline bool
    operator!=(JsonIterator& other) noexcept {
        return node != other.node;
    }

    inline JsonNode& operator*() noexcept { return *node; }

    inline JsonNode& operator->() noexcept { return *node; }
};

inline JsonIterator
begin(JsonValue object) noexcept {
    return JsonIterator(object.toNode());
}

inline JsonIterator
end(JsonValue) noexcept {
    return JsonIterator(nullptr);
}

struct JsonAllocator {
    inline JsonAllocator() noexcept : head(nullptr) {}
    inline JsonAllocator(JsonAllocator&& other) noexcept : head(other.head) {
        other.head = nullptr;
    }
    inline ~JsonAllocator() noexcept { deallocate(); }

    void
    operator=(JsonAllocator&& other) noexcept;

    JsonAllocator(const JsonAllocator&) = delete;
    void
    operator=(const JsonAllocator&) = delete;

    void*
    allocate(size_t size) noexcept;

    void
    deallocate() noexcept;

    struct Zone {
        Zone* next;
        size_t used;
    };

    Zone* head;
};

class JsonDocument {
 public:
    JsonDocument() noexcept : ok(false) {}
    JsonDocument(String text) noexcept;  // Destructively edits text.
    JsonDocument(JsonDocument&&) noexcept;
    ~JsonDocument() noexcept;

    JsonDocument(const JsonDocument&) = delete;
    void
    operator=(const JsonAllocator&) = delete;

 public:
    JsonValue root;
    bool ok;

 private:
    String text;
    JsonAllocator allocator;
};

#endif  // SRC_UTIL_JSON_H_
