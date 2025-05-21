#include "os/windows/io.h"

#include "os/windows/c.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"

extern "C" {
// WinBase.h
#define INVALID_HANDLE_VALUE     ((HANDLE)-1)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#define CREATE_NEW               1
#define CREATE_ALWAYS            2
#define OPEN_EXISTING            3
#define TRUNCATE_EXISTING        5
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
WINBASEAPI BOOL WINAPI CloseHandle(HANDLE) noexcept;
WINBASEAPI HANDLE WINAPI
CreateFileA(LPCSTR, DWORD, DWORD, void*, DWORD, DWORD, HANDLE) noexcept;
WINBASEAPI DWORD WINAPI
FormatMessageA(DWORD, LPCVOID, DWORD, DWORD, LPSTR, DWORD, char*) noexcept;
WINBASEAPI BOOL WINAPI GetFileSizeEx(HANDLE, PLARGE_INTEGER) noexcept;
WINBASEAPI DWORD WINAPI
GetLastError(void);
WINBASEAPI BOOL WINAPI
ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, void*) noexcept;
WINBASEAPI BOOL WINAPI
SetEndOfFile(HANDLE hFile) noexcept;
WINBASEAPI DWORD WINAPI SetFilePointer(HANDLE, LONG, PLONG, DWORD) noexcept;
WINBASEAPI BOOL WINAPI
WriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, void*) noexcept;

// WinCon.h
WINBASEAPI BOOL WINAPI
WriteConsoleA(HANDLE, const VOID*, DWORD, LPDWORD, LPVOID) noexcept;

// WinNT.h
#define LANG_ENGLISH             0x09
#define SUBLANG_ENGLISH_US       0x01
#define MAKELANGID(p, s)         ((((U16)(s)) << 10) | (U16)(p))
#define GENERIC_READ             0x80000000L
#define GENERIC_WRITE            0x40000000L
#define FILE_READ_DATA           0x01
#define FILE_WRITE_DATA          0x02
#define FILE_READ_ATTRIBUTES     0x80
#define FILE_SHARE_READ          0x01
#define FILE_SHARE_WRITE         0x02
}  // extern "C"

static void
printWin32Error(DWORD error = 0) noexcept {
#if DEBUG
    if (error == 0)
        error = GetLastError();
    char buf[1024];
    DWORD size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, error, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), buf, 1024, 0);
    if (size == 0)
        serr << "Error calling FormatMessage: " << error << '\n';
    else {
        StringView msg(buf, size);
        if (msg.endsWith("\r\n"))
            msg.size -= 2;
        serr << "Win32 error (" << error << "): \"" << msg << "\"\n";
    }
    assert_(false);
#endif
}

File::File(StringView path) noexcept {
    handle =
        CreateFileA(String(path).null(), GENERIC_READ,
                    0, 0, OPEN_EXISTING, 0, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        printWin32Error();
        return;
    }

    LARGE_INTEGER size;
    I32 ok = GetFileSizeEx(handle, &size);
    if (!ok) {
        printWin32Error();
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
            printWin32Error();
            assert_(false);
        }
    }
}

File::operator bool() noexcept {
    return handle != INVALID_HANDLE_VALUE;
}

bool
File::read(void* buf, Size len) noexcept {
    DWORD numRead;
    if (!ReadFile(handle, buf, static_cast<DWORD>(len), &numRead, 0)) {
        printWin32Error();
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
        printWin32Error();
        assert_(false);
        return false;
    }

    return read(buf, len);
}

FileWriter::FileWriter(StringView path) noexcept {
    handle = CreateFileA(String(path).null(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        printWin32Error();
    }
}

FileWriter::~FileWriter() noexcept {
    if (handle != INVALID_HANDLE_VALUE)
        if (!CloseHandle(handle))
            printWin32Error();
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
        printWin32Error();
        assert_(false);
        return false;
    }

    BOOL ok = SetEndOfFile(handle);
    if (!ok) {
        printWin32Error();
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
        printWin32Error();
        assert_(false);
        return false;
    }

    DWORD written;
    BOOL ok = WriteFile(handle, buf, static_cast<DWORD>(len), &written, 0);
    if (!ok) {
        printWin32Error();
        assert_(false);
        return false;
    }
    if (len != written) {
        printWin32Error();
        assert_(false);
        return false;
    }

    return true;
}

#define UNINITIALIZED ((HANDLE)-2)

HANDLE con = UNINITIALIZED;

static void
openConsole() noexcept {
    con = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, 0,
                      OPEN_EXISTING, 0, 0);
    if (con == INVALID_HANDLE_VALUE) {
        printWin32Error();
    }
}

bool
writeStdout(const char* buf, Size len) noexcept {
    if (con == UNINITIALIZED)
        openConsole();
    if (con == INVALID_HANDLE_VALUE)
        return false;

    assert_(len <= static_cast<Size>(INT32_MAX));

    DWORD written;
    if (!WriteConsoleA(con, buf, static_cast<DWORD>(len), &written, 0)) {
        printWin32Error();
        assert_(false);
        return false;
    }
    if (written != len) {
        printWin32Error();
        assert_(false);
        return false;
    }
    return true;
}

bool
writeStderr(const char* buf, Size len) noexcept {
    return writeStdout(buf, len);
}
