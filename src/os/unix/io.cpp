#include "os/unix/io.h"

#include "os/c.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"

File::File(StringView path) noexcept {
    fd = open(String(path).null(), O_RDONLY);
    if (fd < 0) {
        // errno set
        return;
    }

    struct stat status;
    if (fstat(fd, &status)) {
        // errno set
        close(fd);
        fd = -1;
        return;
    }

    rem = status.st_size;
}

File::File(File&& other) noexcept : fd(other.fd), rem(other.rem) {
    other.fd = 0;
    other.rem = 0;
}

File::~File() noexcept {
    if (fd >= 0) {
        if (close(fd)) {
            // errno set
        }
    }
}

File::operator bool() noexcept {
    return fd >= 0;
}

bool
File::read(void* buf, size_t len) noexcept {
    ssize_t nbytes = ::read(fd, buf, len);
    if (nbytes < 0) {
        // errno set
        return false;
    }
    if (static_cast<size_t>(nbytes) != len) {
        return false;
    }
    rem -= nbytes;
    return true;
}

bool
File::readOffset(void* buf, size_t len, size_t offset) noexcept {
    ssize_t nbytes = pread(fd, buf, len, offset);
    if (nbytes < 0) {
        // errno set
        return false;
    }
    if (static_cast<size_t>(nbytes) != len) {
        return false;
    }
    return true;
}

FileWriter::FileWriter(StringView path) noexcept {
    fd = open(String(path).null(), O_CREAT | O_TRUNC | O_WRONLY, 0666);
}

FileWriter::~FileWriter() noexcept {
    if (fd >= 0) {
        close(fd);
    }
}

// Whether the file was opened successfully.
FileWriter::operator bool() noexcept {
    return fd >= 0;
}

bool
FileWriter::resize(size_t size) noexcept {
    return ftruncate(fd, size) == 0;
}

bool
FileWriter::writeOffset(const void* buf, size_t len, size_t offset) noexcept {
    return pwrite(fd, buf, len, offset) == static_cast<ssize_t>(len);
}

bool
writeStdout(const char* buf, size_t len) noexcept {
    return write(1, buf, len) == static_cast<ssize_t>(len);
}

bool
writeStderr(const char* buf, size_t len) noexcept {
    return write(2, buf, len) == static_cast<ssize_t>(len);
}
