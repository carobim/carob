#include "os/unix/mapped-file.h"

#include "os/c.h"
#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"

bool
makeMappedFile(MappedFile& file, StringView path) noexcept {
    I32 fd = open(String(path).null(), O_RDONLY);
    if (fd == -1) {
        return false;
    }

    struct stat st;
    if (fstat(fd, &st)) {
        close(fd);
        return false;
    }

    if (st.st_size == 0) {
        close(fd);
        return false;
    }

    Size size = static_cast<Size>(st.st_size);
    void* data = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        close(fd);
        return false;
    }

    // FIXME: Close the fd now or at destruction?
    file.data = static_cast<char*>(data);
    file.size = size;
    file.fd = fd;
    return true;
}

void
destroyMappedFile(MappedFile file) noexcept {
    if (file.data != MAP_FAILED) {
        munmap(file.data, file.size);
        close(file.fd);
    }
}
