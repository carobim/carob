#ifndef SRC_OS_UNIX_IO_H_
#define SRC_OS_UNIX_IO_H_

#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"

class File {
 public:
    File(StringView path) noexcept;
    File(File&& other) noexcept;
    ~File() noexcept;

    // Whether the file was opened successfully.
    operator bool() noexcept;

    bool
    read(void* buf, Size len) noexcept;

    bool
    readOffset(void* buf, Size len, Size offset) noexcept;

 public:
    I32 fd;
    Size rem;  // 0 when EOF

 private:
    void
    operator=(const File&) noexcept;
};

class FileWriter {
 public:
    FileWriter(StringView path) noexcept;
    ~FileWriter() noexcept;

    // Whether the file was opened successfully.
    operator bool() noexcept;

    bool
    resize(Size size) noexcept;

    bool
    writeOffset(const void* buf, Size len, Size offset) noexcept;

 public:
    I32 fd;
};

// Returns false on error. nread == 0 on EOF.
bool
readStdin(void* buf, Size len, Size* nread) noexcept;

bool
writeStdout(const void* buf, Size len) noexcept;
bool
writeStderr(const void* buf, Size len) noexcept;

#endif  // SRC_OS_UNIX_IO_H_
