#ifndef NDEBUG

#    include "util/assert.h"

#    include "os/os.h"
#    include "util/compiler.h"
#    include "util/io.h"

#    if MSVC
extern "C" __declspec(dllimport) I32 __stdcall IsDebuggerPresent() noexcept;
void __cdecl __debugbreak();  // Cannot be noexcept.
#    endif

#    ifdef __APPLE__
// sys/_types/_mach_port_t.h
typedef U32 mach_port_t;

// mach/arm/exception.h
// mach/i386/exception.h
#        define EXC_TYPES_COUNT 14

// mach/arm/kern_return.h
// mach/i386/kern_return.h
typedef I32 kern_return_t;

// mach/thread_status.h
typedef I32 thread_state_flavor_t;
typedef I32* thread_state_flavor_array_t;

// mach/exception_types.h
typedef U32 exception_mask_t;
typedef I32 exception_behavior_t;
typedef exception_behavior_t* exception_behavior_array_t;
typedef exception_mask_t* exception_mask_array_t;
typedef thread_state_flavor_t* exception_flavor_array_t;
#        define EXC_BREAKPOINT (1 << 6)

// mach/kern_return.h
#        define KERN_SUCCESS 0

// mach/mach_init.h
extern mach_port_t mach_task_self_;
#        define mach_task_self() mach_task_self_

// mach/mach_types.h
typedef mach_port_t task_t;
typedef mach_port_t exception_handler_t;
typedef exception_handler_t* exception_handler_array_t;

// mach/message.h
typedef U32 mach_msg_type_number_t;

// mach/port.h
#        define MACH_PORT_VALID(name) ((name) != 0 && (name) != ((U32)~0))

// mach/task.h
extern "C" kern_return_t
task_get_exception_ports(task_t, exception_mask_t, exception_mask_array_t,
                         mach_msg_type_number_t*, exception_handler_array_t,
                         exception_behavior_array_t, exception_flavor_array_t)
                        noexcept;

static bool
isDebuggerPresent(void) {
    mach_msg_type_number_t count = 0;
    exception_mask_t masks[EXC_TYPES_COUNT];
    mach_port_t ports[EXC_TYPES_COUNT];
    exception_behavior_t behaviors[EXC_TYPES_COUNT];
    thread_state_flavor_t flavors[EXC_TYPES_COUNT];

    exception_mask_t mask = EXC_BREAKPOINT;
    kern_return_t result =
        task_get_exception_ports(mach_task_self(), mask, masks, &count, ports,
                                 behaviors, flavors);
    if (result == KERN_SUCCESS) {
        for (mach_msg_type_number_t portIndex = 0;
             portIndex < count;
             portIndex++) {
            if (MACH_PORT_VALID(ports[portIndex])) {
                return true;
            }
        }
    }
    return false;
}
#    endif

#    ifdef __linux__
enum ptrace_request {
    PTRACE_TRACEME = 0,
    PTRACE_DETACH = 17
};
extern "C" long
ptrace(enum ptrace_request, ...) noexcept;

static bool
isDebuggerPresent(void) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
        return true;
    }
    else {
        ptrace(PTRACE_DETACH, 0, 0, 0);
        return false;
    }
}
#    endif

static bool
haveDebugger() noexcept {
    static I32 result = 2;

    if (result == 2) {
#    if defined(_WIN32)
        result = IsDebuggerPresent();
#    elif defined(__APPLE__)
        result = isDebuggerPresent();
#    elif defined(__linux__)
        result = isDebuggerPresent();
#    elif defined(__FreeBSD__)
        // FIXME: Check to see if ptrace(2) works.
        result = false;
#    elif defined(__NetBSD__)
        // FIXME: Check to see if ptrace(2) works.
        result = false;
#    else
#        error not sure how to check for debugger
#    endif
    }

    return result != 0;
}

static void
triggerDebugger() noexcept {
#    if defined(_WIN32)
    __debugbreak();
#    elif defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__("int3");
//#elif defined(__ARMCC_VERSION)
//    __breakpoint(42)
#    elif defined(__thumb__)
    __asm__ __volatile__(".inst 0xde01");
#    elif defined(__aarch64__)
    __asm__ __volatile__(".inst 0xd4200000");
#    elif defined(__arm__)
    __asm__ __volatile__(".inst 0xe7f001f0");
#    else
#        warn not sure how to trap for debugger
#    endif
}

void
assert__(const char* func,
         const char* file,
         I32 line,
         const char* expr) noexcept {
    if (haveDebugger()) {
        triggerDebugger();
    }
    else {
        sout << "Assertion failed: " << expr << ", function " << func
             << ", file " << file << ", line " << line << "\n";

        exitProcess(127);
    }
}

void
debugger() noexcept {
    if (haveDebugger()) {
        triggerDebugger();
    }
}

#endif
