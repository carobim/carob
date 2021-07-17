#include "core/measure.h"

#include "core/log.h"
#include "util/compiler.h"

#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
#    include "util/hashtable.h"

// sys/kdebug_signpost.h
extern "C" {
int kdebug_signpost_start(uint32_t,
                          size_t,
                          size_t,
                          size_t,
                          size_t) noexcept;
int kdebug_signpost_end(uint32_t,
                        size_t,
                        size_t,
                        size_t,
                        size_t) noexcept;
}

static uint32_t nextSignpost = 0;
static Hashmap<String, uint32_t> signposts;

static uint32_t
getSignpost(String description) noexcept {
    if (signposts.find(description) != signposts.end()) {
        return signposts[description];
    }
    else {
        logInfo("Measure",
                String() << description << " is signpost " << nextSignpost);
        signposts[static_cast<String&&>(description)] = nextSignpost;
        return nextSignpost++;
    }
}
#endif  // defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)

TimeMeasure::TimeMeasure(String description) noexcept {
    this->description = static_cast<String&&>(description);
    start = chronoNow();
#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
    signpost = getSignpost(this->description);
    kdebug_signpost_start(signpost, 0, 0, 0, 0);
#endif
}

TimeMeasure::~TimeMeasure() noexcept {
#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
    kdebug_signpost_end(signpost, 0, 0, 0, 0);
#endif

    Nanoseconds end = chronoNow();

    Nanoseconds elapsed = end - start;
    logInfo("Measure",
            description << " took " << ns_to_s_d(elapsed) << " seconds");
}
