/********************************
** Tsunagari Tile Engine       **
** pack.h                      **
** Copyright 2016 Paul Merrill **
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

#ifndef SRC_RESOURCES_PACK_H_
#define SRC_RESOURCES_PACK_H_

#include <stdlib.h>

#include <memory>
#include <mutex>

#include "core/resources.h"
#include "pack/pack-file.h"

struct Free {
    void operator() (void* ptr) {
        free(ptr);
    }
};

class PackResource : public Resource {
 public:
    PackResource(void* data, size_t size);
    ~PackResource() = default;

    const void* data() const;
    size_t size() const;

 private:
    std::unique_ptr<void, Free> _data;
    size_t _size;
};

class PackResources : public Resources {
 public:
    PackResources();
    ~PackResources() = default;

    void init();

    std::unique_ptr<Resource> load(const std::string& path);

 private:
    PackResources(const PackResources&) = delete;
    PackResources& operator=(const PackResources&) = delete;

    bool initialized;
    std::mutex mutex;
    std::unique_ptr<PackReader> pack;
};

#endif  // SRC_RESOURCES_PACK_H_