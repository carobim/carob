#include "os/c.h"
#include "os/os.h"
#include "util/compiler.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

Filesize
getFileSize(StringView path) noexcept {
    struct stat status;
    if (stat(String(path).null(), &status)) {
        return FS_ERROR;
    }
    return static_cast<Filesize>(status.st_size);
}

bool
isDir(StringView path) noexcept {
    struct stat status;
    if (stat(String(path).null(), &status)) {
        return false;
    }
    return S_ISDIR(status.st_mode);
}

void
makeDirectory(StringView path) noexcept {
    mkdir(String(path).null(), 0777);
}

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

bool
writeFile(StringView path, U32 length, void* data) noexcept {
    int fd = open(String(path).null(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return false;
    }
    SSize written = write(fd, data, length);
    if (written != length) {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

bool
writeFileVec(StringView path, U32 count, U32* lengths,
             void** datas) noexcept {
    int fd = open(String(path).null(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return false;
    }

    SSize total = 0;
    Vector<iovec> ios;

    ios.reserve(count);
    for (Size i = 0; i < count; i++) {
        total += lengths[i];
        iovec io{datas[i], lengths[i]};
        ios.push(io);
    }

    SSize written = writev(fd, ios.data, static_cast<int>(ios.size));
    if (written != total) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool
readFile(StringView path, String& data) noexcept {
    String path_;
    path_.reserve(path.size + 1);
    path_ << path;

    int fd = open(path_.null(), O_RDONLY);
    if (fd < 0) {
        return false;
    }

    struct stat status;
    if (fstat(fd, &status)) {
        // errno set
        close(fd);
        return false;
    }

    Size size = status.st_size;
    data.reserve(size + 1);
    data.resize(size);

    SSize nbytes = read(fd, data.data, size);
    if (nbytes < 0 || static_cast<Size>(nbytes) != size) {
        close(fd);
        return false;
    }

    // Add NUL terminator.
    data.data[size] = 0;

    close(fd);
    return true;
}

static bool
isaTTY() noexcept {
#if defined(__EMSCRIPTEN__)
    return false;
#else
    static bool checked = false;
    static bool tty = false;

    if (!checked) {
        checked = true;
        tty = isatty(0) != 0;
    }
    return tty;
#endif
}

void
setTermColor(TermColor color, Output& out) noexcept {
    if (!isaTTY()) {
        return;
    }

    // VT100 terminal control codes from:
    //   http://www.termsys.demon.co.uk/vtansi.htm

    switch (color) {
    case TC_RESET:
        out << "\033[0m";
        break;
    case TC_GREEN:
        out << "\033[32m";
        break;
    case TC_YELLOW:
        out << "\033[33m";
        break;
    case TC_RED:
        out << "\033[31m";
        break;
    }
}

void
exitProcess(int code) noexcept {
    _exit(code);
    unreachable;
}
