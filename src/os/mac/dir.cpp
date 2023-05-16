#include "os/mac/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

// Available on Mac OS X Snow Leopard and above.
extern "C" SSize
__getdirentries64(int, void*, Size, off_t*) noexcept;

Vector<String>
listDir(StringView path) noexcept {
    Vector<String> names;

    int fd = open(String(path).null(),
                  O_CLOEXEC | O_DIRECTORY | O_NONBLOCK | O_RDONLY);
    if (fd == -1)
        return names;

    Size len = 8192;
    char* buf = static_cast<char*>(malloc(8192));
    SSize loc = 0;
    SSize size = 0;

    while (true) {
        if (loc >= size)
            loc = 0;

        if (loc == 0) {
            off_t position = 0;
            // TODO: Read last 4 bytes to get advance notice on EOF on 10.15.0+
            size = __getdirentries64(fd, buf, len, &position);
            if (size <= 0)
                break;
        }

        struct dirent* d = reinterpret_cast<struct dirent*>(buf + loc);
        assert_((reinterpret_cast<SSize>(d) & 3) == 0);
        assert_(d->d_reclen > 0 && d->d_reclen <= len + 1 - loc);
        assert_(d->d_ino != 0);  // Maybe deleted but not yet removed from dir?
        assert_(d->d_type != DT_WHT);

        loc += d->d_reclen;

        if (d->d_name[0] == '.') {
            // Ignore hidden files and directories.
            continue;
        }
        if ((d->d_type & (DT_DIR | DT_REG)) == 0) {
            // Ignore odd files.
            continue;
        }

        names.push(d->d_name);
    }

    free(buf);
    close(fd);

    return names;
}
