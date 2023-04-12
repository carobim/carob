#include "os/c.h"
#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

Vector<String>
listDir(StringView path) noexcept {
    DIR* dir;
    struct dirent* entry;
    Vector<String> names;

    if ((dir = opendir(String(path).null())) == 0) {
        return names;
    }

    // FIXME: Replace with reentrant function calls.
    while ((entry = readdir(dir)) != 0) {
        if (entry->d_ino == 0) {
            // Ignore unlinked files.
            continue;
        }
        if (entry->d_name[0] == '.') {
            // Ignore hidden files and directories.
            continue;
        }
        if ((entry->d_type & (DT_DIR | DT_REG)) == 0) {
            // Ignore odd files.
            continue;
        }
        names.push(entry->d_name);
    }

    closedir(dir);

    return names;
}
