#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "data/data-world.h"
#include "os/mutex.h"
#include "pack/pack-reader.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"

static Mutex mutex;
static PackReader* pack = 0;

static bool
openPackFile() noexcept {
    if (pack) {
        return true;
    }

    StringView path = dataWorldDatafile;

    // TimeMeasure m("Opened " + path);

    pack = makePackReader(path);
    if (!pack) {
        logFatal("PackResources",
                 String() << path << ": could not open archive");
        return false;
    }

    return true;
}

static String
getFullPath(StringView path) noexcept {
    return String() << dataWorldDatafile << "/" << path;
}

bool
resourceLoad(StringView path, String& data) noexcept {
    LockGuard lock(mutex);

    if (!openPackFile()) {
        return false;
    }

    uint32_t index = readerIndex(pack, path);

    if (index == BLOB_NOT_FOUND) {
        logErr("PackResources",
               String() << getFullPath(path) << ": file missing");
        return false;
    }

    BlobDetails details = readerDetails(pack, index);
    uint32_t size = details.size;

    // Will it fit in memory?
    if (size > static_cast<uint32_t>(INT32_MAX)) {
        logErr("PackResources",
               String() << getFullPath(path) << ": file too large");
        return false;
    }

    if (data.capacity < static_cast<size_t>(size) + 1) {
        data.reserve(static_cast<size_t>(size) + 1);
    }

    bool ok = readerRead(pack, data.data, index);
    assert_(ok);
    (void)ok;

    data.size = size;
    data.data[size] = 0;
    return true;
}
