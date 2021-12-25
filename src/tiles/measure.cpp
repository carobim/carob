#include "tiles/measure.h"

#include "tiles/log.h"
#include "util/compiler.h"

#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
#    include "util/hashtable.h"

// sys/kdebug_signpost.h
extern "C" {
int kdebug_signpost_start(U32, Size, Size, Size, Size) noexcept;
int kdebug_signpost_end(U32, Size, Size, Size, Size) noexcept;
}

static U32 nextSignpost = 0;
static Hashmap<String, U32> signposts;

static U32
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
