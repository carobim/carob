#if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || \
        defined(__NetBSD__)

#    include "util/new.h"

#    include "util/assert.h"
#    include "util/compiler.h"
#    include "util/int.h"

// Note: Do not add noexcept.
void*
operator new(size_t count) {
    return malloc(count);
}

// Note: Do not add noexcept.
void*
operator new[](size_t count) {
    return malloc(count);
}

void
operator delete(void* ptr) noexcept {
    free(ptr);
}

void
operator delete[](void* ptr) noexcept {
    free(ptr);
}

void
operator delete(void* ptr, size_t) noexcept {
    free(ptr);
}

void
operator delete[](void* ptr, size_t) noexcept {
    free(ptr);
}

// Clang and GCC have strict requirements for how __cxa_pure_virtual is defined:
//
//   (default): pointer
//   Debug: pointer
//   Release: function
//   RelWithDebInfo: function
//   MinSizeRel: function

#    ifdef RELEASE
extern "C" void
__cxa_pure_virtual() { }
#    else
void*
__cxa_pure_virtual = 0;
#    endif

#endif  // defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || \
        // defined(__NetBSD__)
