#ifndef SRC_UTIL_LIST_H_
#define SRC_UTIL_LIST_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/move.h"

template<typename T>
class List {
 private:
    struct Node;

    struct Links {
        Links* next;
        Links* prev;

        inline constexpr14 Node*
        toNode() noexcept {
            return reinterpret_cast<Node*>(this);
        }
        inline const constexpr14 Node*
        toNode() const noexcept {
            return reinterpret_cast<const Node*>(this);
        }
    };

    struct Node : public Links {
        T x;

        template<typename... Args>
        Node(Args&&... args) noexcept : x{forward_<Args>(args)...} { }
    };

 public:
    class Iterator {
        friend List;

     public:
        inline constexpr14 T&
        operator*() const noexcept {
            return links->toNode()->x;
        }
        inline constexpr14 T*
        operator->() const noexcept {
            return &links->toNode()->x;
        }
        inline void
        operator++() {
            links = links->next;
        }
        inline bool
        operator==(const Iterator& other) const noexcept {
            return links == other.links;
        }
        inline bool
        operator!=(const Iterator& other) const noexcept {
            return links != other.links;
        }

     private:
        constexpr11
        Iterator(Links* links) noexcept
            : links(links) { }
        Links* links;
    };

    class ConstIterator {
        friend List;

     public:
        inline const T&
        operator*() const noexcept {
            return links->toNode()->x;
        }
        inline const T*
        operator->() const noexcept {
            return &links->toNode()->x;
        }
        inline void
        operator++() noexcept {
            links = links->next;
        }
        inline bool
        operator==(const ConstIterator& other) const noexcept {
            return links == other.links;
        }
        inline bool
        operator!=(const ConstIterator& other) const noexcept {
            return links != other.links;
        }

     private:
        ConstIterator(const Links* links) noexcept : links(links) { }
        const Links* links;
    };

    inline constexpr14
    List() noexcept {
        head = {&head, &head};
        n = 0;
    }
    inline List(const List& other) noexcept {
        head = {&head, &head};
        n = 0;

        for (ConstIterator it = other.begin(); it != other.end(); ++it)
            emplace_back(*it);
    }
    inline List(List&& other) noexcept {
        if (other.empty()) {
            head = {&head, &head};
            n = 0;
            return;
        }

        head = other.head;
        head.next->prev = &head;
        head.prev->next = &head;
        n = other.n;

        other.head = {&other.head, &other.head};
        other.n = 0;
    }

    ~List() noexcept { clear(); }

    List&
    operator=(const List& other) noexcept {
        clear();

        for (ConstIterator it = other.begin(); it != other.end(); ++it)
            emplace_back(*it);
    }
    List&
    operator=(List&& other) noexcept {
        clear();

        if (other.empty())
            return *this;

        head = other.head;
        head.next->prev = &head;
        head.prev->next = &head;
        n = other.n;

        other.head = {&other.head, &other.head};
        other.n = 0;

        return *this;
    }

    inline constexpr14 Iterator
    begin() noexcept {
        return Iterator(head.next);
    }
    inline constexpr14 ConstIterator
    begin() const noexcept {
        return ConstIterator(head.next);
    }
    inline constexpr14 Iterator
    end() noexcept {
        return Iterator(&head);
    }
    inline constexpr14 ConstIterator
    end() const noexcept {
        return ConstIterator(&head);
    }

    template<typename... Args>
    inline Iterator
    emplace_back(Args&&... args) noexcept {
        Node* node = new Node(forward_<Args>(args)...);
        node->next = &head;
        node->prev = head.prev;
        node->prev->next = node;
        head.prev = node;
        ++n;
        return Iterator(node);
    }
    inline Iterator
    erase(Iterator it) noexcept {
        Links* prev = it.links->prev;
        Links* next = it.links->next;
        delete it.links->toNode();
        prev->next = next;
        next->prev = prev;
        n--;
        return Iterator(next);
    }
    inline void
    clear() noexcept {
        for (Iterator it = begin(); it != end(); it = erase(it)) {
        }
    }

    inline constexpr11 Size
    size() const noexcept {
        return n;
    }
    inline constexpr11 bool
    empty() const noexcept {
        return n == 0;
    }

 private:
    Links head;
    Size n;
};

#endif  // SRC_UTIL_LIST_H_
