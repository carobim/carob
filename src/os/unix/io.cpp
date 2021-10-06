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
File::read(void* buf, Size len) noexcept {
    SSize nbytes = ::read(fd, buf, len);
    if (nbytes < 0) {
        // errno set
        return false;
    }
    if (static_cast<Size>(nbytes) != len) {
        return false;
    }
    rem -= nbytes;
    return true;
}

bool
File::readOffset(void* buf, Size len, Size offset) noexcept {
    SSize nbytes = pread(fd, buf, len, offset);
    if (nbytes < 0) {
        // errno set
        return false;
    }
    if (static_cast<Size>(nbytes) != len) {
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
FileWriter::resize(Size size) noexcept {
    return ftruncate(fd, size) == 0;
}

bool
FileWriter::writeOffset(const void* buf, Size len, Size offset) noexcept {
    return pwrite(fd, buf, len, offset) == static_cast<SSize>(len);
}

bool
writeStdout(const char* buf, Size len) noexcept {
    return write(1, buf, len) == static_cast<SSize>(len);
}

bool
writeStderr(const char* buf, Size len) noexcept {
    return write(2, buf, len) == static_cast<SSize>(len);
}
