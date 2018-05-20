/*************************************
** Tsunagari Tile Engine            **
** jsons.h                          **
** Copyright 2016-2018 Paul Merrill **
*************************************/

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

#ifndef SRC_CORE_JSONS_H_
#define SRC_CORE_JSONS_H_

#include <string>

#include "util/rc.h"
#include "util/unique.h"
#include "util/vector.h"

class JSONArray;
class JSONObject;

typedef Unique<const JSONArray> JSONArrayPtr;
typedef Unique<const JSONObject> JSONObjectPtr;
typedef Rc<const JSONObject> JSONObjectRef;

class JSONObject {
 public:
    JSONObject(const JSONObject& other) = delete;

    virtual ~JSONObject() = default;

    virtual vector<std::string> names() const = 0;

    virtual bool hasBool(const std::string& name) const = 0;
    virtual bool hasInt(const std::string& name) const = 0;
    virtual bool hasUnsigned(const std::string& name) const = 0;
    virtual bool hasDouble(const std::string& name) const = 0;
    virtual bool hasString(const std::string& name) const = 0;
    virtual bool hasObject(const std::string& name) const = 0;
    virtual bool hasArray(const std::string& name) const = 0;

    virtual bool hasStringDouble(const std::string& name) const = 0;

    virtual bool boolAt(const std::string& name) const = 0;
    virtual int intAt(const std::string& name) const = 0;
    virtual int intAt(const std::string& name, int lowerBound, int upperBound)
        const = 0;
    virtual unsigned unsignedAt(const std::string& name) const = 0;
    virtual double doubleAt(const std::string& name) const = 0;
    virtual std::string stringAt(const std::string& name) const = 0;
    virtual JSONObjectPtr objectAt(const std::string& name) const = 0;
    virtual JSONArrayPtr arrayAt(const std::string& name) const = 0;

    virtual double stringDoubleAt(const std::string& name) const = 0;

 protected:
    JSONObject() = default;
};

class JSONArray {
 public:
    JSONArray(const JSONArray& other) = delete;

    virtual ~JSONArray() = default;

    virtual size_t size() const = 0;

    virtual bool isBool(size_t index) const = 0;
    virtual bool isInt(size_t index) const = 0;
    virtual bool isUnsigned(size_t index) const = 0;
    virtual bool isDouble(size_t index) const = 0;
    virtual bool isString(size_t index) const = 0;
    virtual bool isObject(size_t index) const = 0;
    virtual bool isArray(size_t index) const = 0;

    virtual bool boolAt(size_t index) const = 0;
    virtual int intAt(size_t index) const = 0;
    virtual unsigned unsignedAt(size_t index) const = 0;
    virtual double doubleAt(size_t index) const = 0;
    virtual std::string stringAt(size_t index) const = 0;
    virtual JSONObjectPtr objectAt(size_t index) const = 0;
    virtual JSONArrayPtr arrayAt(size_t index) const = 0;

 protected:
    JSONArray() = default;
};

class JSONs {
 public:
    //! Acquire the global JSONs object.
    static JSONs& instance();

    JSONs(const JSONs&) = delete;
    JSONs(JSONs&&) = delete;
    JSONs& operator=(const JSONs&) = delete;
    JSONs& operator=(JSONs&&) = delete;

    virtual ~JSONs() = default;

    //! Load a JSON document.
    virtual JSONObjectRef load(const std::string& path) = 0;

    //! Parse a document from the outside world.
    static JSONObjectPtr parse(std::string data);

    //! Free JSON documents not recently used.
    virtual void garbageCollect() = 0;

 protected:
    JSONs() = default;
};

#endif  // SRC_CORE_JSONS_H_
