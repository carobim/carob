/*************************************
** Tsunagari Tile Engine            **
** io.cpp                           **
** Copyright 2020-2021 Paul Merrill **
*************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#include "os/windows/io.h"

#include "os/windows-c.h"
#include "util/assert.h"
#include "util/int.h"
#include "util/noexcept.h"
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

    rem = static_cast<size_t>(size.QuadPart);
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
File::read(void* buf, size_t len) noexcept {
    unsigned long numRead;
    if (!ReadFile(handle, buf, len, &numRead, 0)) {
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
File::readOffset(void* buf, size_t len, size_t offset) noexcept {
    DWORD position = SetFilePointer(handle, len, 0, 0);
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
FileWriter::resize(size_t size) noexcept {
    DWORD position = SetFilePointer(handle, size, 0, 0);
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
FileWriter::writeOffset(const void* buf, size_t len, size_t offset) noexcept {
    DWORD position = SetFilePointer(handle, offset, 0, 0);
    if (position == INVALID_SET_FILE_POINTER) {
        // GetLastError();
        assert_(false);
        return false;
    }

    DWORD written;
    BOOL ok = WriteFile(handle, buf, len, &written, 0);
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
writeStdout(const char* buf, size_t len) noexcept {
    if (con == UNINITIALIZED) {
        openConsole();
    }
    if (con == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD written;
    if (!WriteConsoleA(con, buf, len, &written, 0)) {
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
writeStderr(const char* buf, size_t len) noexcept {
    return writeStdout(buf, len);
}
