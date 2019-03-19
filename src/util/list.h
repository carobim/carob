/********************************
** Tsunagari Tile Engine       **
** list.h                      **
** Copyright 2019 Paul Merrill **
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

#ifndef SRC_UTIL_LIST_H_
#define SRC_UTIL_LIST_H_

#include "util/int.h"
#include "util/move.h"

template<typename T>
class List {
 private:
    struct Node;
    
    struct Links {
        Links* next;
        Links* prev;
        
        inline Node* toNode() {
            return reinterpret_cast<Node*>(this);
        }
        inline const Node* toNode() const {
            return reinterpret_cast<const Node*>(this);
        }
    };
    
    struct Node : public Links {
        T x;
        
        template<typename... Args>
        Node(Args&&... args) noexcept : x{forward_<Args>(args)...} {}
    };
    
 public:
    class Iterator {
        friend List;
        
     public:
        inline T& operator*() const {
            return links->toNode()->x;
        }
        inline T* operator->() const {
            return &links->toNode()->x;
        }
        inline void operator++() {
            links = links->next;
        }
        inline bool operator==(const Iterator& other) const {
            return links == other.links;
        }
        inline bool operator!=(const Iterator& other) const {
            return links != other.links;
        }
        
     private:
        Iterator(Links* links) : links(links) {}
        Links* links;
    };
    
    class ConstIterator {
        friend List;
        
     public:
        inline const T& operator*() const {
            return links->toNode()->x;
        }
        inline const T* operator->() const {
            return &links->toNode()->x;
        }
        inline void operator++() {
            links = links->next;
        }
        inline bool operator==(const ConstIterator& other) const {
            return links == other.links;
        }
        inline bool operator!=(const ConstIterator& other) const {
            return links != other.links;
        }
        
     private:
        ConstIterator(const Links* links) : links(links) {}
        const Links* links;
    };
    
    inline List() noexcept : n(0) {
        head.next = &head;
        head.prev = &head;
    }
    inline List(const List& other) noexcept {
        *this = other;
    }
    inline List(List&& other) noexcept {
        *this = move_(other);
    }
    ~List() noexcept {
        for (Iterator it = begin(); it != end(); it = erase(it)) {}
    }
    List& operator=(const List& other) {
        for (ConstIterator it = other.begin(); it != other.end(); ++it) {
            emplace_back(*it);
        }
    }
    List& operator=(List&& other) {
        this->~List();
        
        head = other.head;
        n = other.n;
        
        other.head.next = &other.head;
        other.head.prev = &other.head;
        other.n = 0;
        
        return *this;
    }
    
    inline Iterator      begin()       { return Iterator     (head.next); }
    inline ConstIterator begin() const { return ConstIterator(head.next); }
    inline Iterator      end()         { return Iterator     (&head);      }
    inline ConstIterator end()   const { return ConstIterator(&head);      }
    
    template<typename... Args>
    inline Iterator emplace_back(Args&&... args) {
        Node* node = new Node(forward_<Args>(args)...);
        node->next = &head;
        node->prev = head.prev;
        node->prev->next = node;
        head.prev = node;
        ++n;
        return Iterator(node);
    }
    inline Iterator erase(Iterator it) {
        Iterator next = it;
        ++next;
        delete it.links->toNode();
        --n;
        return next;
    }
    
    inline size_t size() {
        return n;
    }
    inline bool empty() {
        return n == 0;
    }
    
 private:
    Links head;
    size_t n;
};

#endif  // SRC_UTIL_LIST_H_
