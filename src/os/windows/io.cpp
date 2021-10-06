#include "os/windows/io.h"

#include "os/windows/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"

#define FILE_READ_ATTRIBUTES     0x80
#define FILE_READ_DATA           0x01
#define FILE_SHARE_READ          0x01
#define FILE_SHARE_WRITE         0x02
#define FILE_WRITE_DATA          0x02
#define GENERIC_WRITE            0x120116L
#define INVALID_HANDLE_VALUE     ((HANDLE)-1)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#define CREATE_NEW               1
#define OPEN_EXISTING            3
#define TRUNCATE_EXISTING        5

extern "C" {
WINBASEAPI BOOL WINAPI CloseHandle(HANDLE) noexcept;
WINBASEAPI HANDLE WINAPI
CreateFileA(LPCSTR, DWORD, DWORD, void*, DWORD, DWORD, HANDLE) noexcept;
WINBASEAPI BOOL WINAPI GetFileSizeEx(HANDLE, PLARGE_INTEGER) noexcept;
WINBASEAPI BOOL WINAPI
ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, void*) noexcept;
WINBASEAPI BOOL WINAPI
SetEndOfFile(HANDLE hFile) noexcept;
WINBASEAPI DWORD WINAPI SetFilePointer(HANDLE, LONG, PLONG, DWORD) noexcept;
WINBASEAPI BOOL WINAPI
WriteConsoleA(HANDLE, const VOID*, DWORD, LPDWORD, LPVOID) noexcept;
WINBASEAPI BOOL WINAPI
WriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, void*) noexcept;
}  // extern "C"

File::File(StringView path) noexcept {
    handle = CreateFileA(String(path).null(),
                         FILE_READ_ATTRIBUTES | FILE_READ_DATA,
                         0,
                         0,
                         OPEN_EXISTING,
                         0,
                         0);
    if (handle == INVALID_HANDLE_VALUE) {
        // GetLastError(); for more specific information if we want
        return;
    }

    LARGE_INTEGER size;
    int ok = GetFileSizeEx(handle, &size);
    if (!ok) {
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
        return;
    }

    rem = static_cast<Size>(size.QuadPart);
}

File::File(File&& other) noexcept : handle(other.handle), rem(other.rem) {
    other.handle = INVALID_HANDLE_VALUE;
}

File::~File() noexcept {
    if (handle != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(handle)) {
            // GetLastError();
            assert_(false);
        }
    }
}

File::operator bool() noexcept {
    return handle != INVALID_HANDLE_VALUE;
}

bool
File::read(void* buf, Size len) noexcept {
    unsigned long numRead;
    if (!ReadFile(handle, buf, static_cast<DWORD>(len), &numRead, 0)) {
        // GetLastError();
        assert_(false);
        return false;
    }
    if (numRead != len) {
        rem = 0;
        return false;
    }
    rem -= numRead;
    return true;
}

bool
File::readOffset(void* buf, Size len, Size offset) noexcept {
    assert_(len <= static_cast<Size>(INT32_MAX));
    DWORD position = SetFilePointer(handle, static_cast<LONG>(len), 0, 0);
    if (position == INVALID_SET_FILE_POINTER) {
        // GetLastError();
        assert_(false);
        return false;
    }

    return read(buf, len);
}

FileWriter::FileWriter(StringView path) noexcept {
    handle = CreateFileA(String(path).null(),
                         GENERIC_WRITE,
                         0,
                         0,
                         CREATE_NEW | TRUNCATE_EXISTING,
                         0,
                         0);
}

FileWriter::~FileWriter() noexcept {
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
    }
}

// Whether the file was opened successfully.
FileWriter::operator bool() noexcept {
    return handle != INVALID_HANDLE_VALUE;
}

bool
FileWriter::resize(Size size) noexcept {
    assert_(size <= static_cast<Size>(INT32_MAX));
    DWORD position = SetFilePointer(handle, static_cast<LONG>(size), 0, 0);
    if (position == INVALID_SET_FILE_POINTER) {
        // GetLastError();
        assert_(false);
        return false;
    }

    BOOL ok = SetEndOfFile(handle);
    if (!ok) {
        // GetLastError();
        assert_(false);
        return false;
    }

    return true;
}

bool
FileWriter::writeOffset(const void* buf, Size len, Size offset) noexcept {
    assert_(len <= static_cast<Size>(INT32_MAX));
    DWORD position = SetFilePointer(handle, static_cast<LONG>(offset), 0, 0);
    if (position == INVALID_SET_FILE_POINTER) {
        // GetLastError();
        assert_(false);
        return false;
    }

    DWORD written;
    BOOL ok = WriteFile(handle, buf, static_cast<DWORD>(len), &written, 0);
    if (!ok) {
        //printWin32Error();
        assert_(false);
        return false;
    }
    if (len != written) {
        //printWin32Error();
        assert_(false);
        return false;
    }

    return true;
}

#define UNINITIALIZED ((HANDLE)-2)

HANDLE con = UNINITIALIZED;

static void
openConsole() noexcept {
    con = CreateFileA("CONOUT$",
                      GENERIC_WRITE,
                      FILE_SHARE_WRITE,
                      0,
                      OPEN_EXISTING,
                      0,
                      0);
    // if (con == INVALID_HANDLE_VALUE) {
    //     GetLastError();
    // }
}

bool
writeStdout(const char* buf, Size len) noexcept {
    if (con == UNINITIALIZED) {
        openConsole();
    }
    if (con == INVALID_HANDLE_VALUE) {
        return false;
    }

    assert_(len <= static_cast<Size>(INT32_MAX));

    DWORD written;
    if (!WriteConsoleA(con, buf, static_cast<DWORD>(len), &written, 0)) {
        // GetLastError();
        assert_(false);
        return false;
    }
    if (written != len) {
        // GetLastError();
        assert_(false);
        return false;
    }
    return true;
}

bool
writeStderr(const char* buf, Size len) noexcept {
    return writeStdout(buf, len);
}
