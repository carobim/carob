#ifndef SRC_CORE_ALGORITHM_H_
#define SRC_CORE_ALGORITHM_H_

#include "util/compiler.h"

template<class Container, class Predicate>
void
erase_if(Container& container, Predicate pred) noexcept {
    Size i = 0;
    while (i < container.size) {
        if (pred(container[i])) {
            container.erase(i);
        }
        else {
            i++;
        }
    }
}

#endif
