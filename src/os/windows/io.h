#ifndef SRC_OS_WINDOWS_IO_H_
#define SRC_OS_WINDOWS_IO_H_

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
    void* handle;
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
    void* handle;
};

// On Windows, these are identical and both write to the attached console, if
// it exists.
bool
writeStdout(const char* buf, Size len) noexcept;
bool
writeStderr(const char* buf, Size len) noexcept;

#endif  // SRC_OS_WINDOWS_IO_H_
