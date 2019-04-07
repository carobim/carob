/********************************
** Tsunagari Tile Engine       **
** os/windows-mapped-file.cpp  **
** Copyright 2019 Paul Merrill **
********************************/

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

#include "os/windows-mapped-file.h"

#include "os/windows-types.h"
#include "util/string.h"

extern "C" {
typedef struct {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

WINBASEAPI BOOL WINAPI CloseHandle(HANDLE hObject);
WINBASEAPI HANDLE WINAPI CreateFileA(LPCSTR lpFileName,
                                     DWORD dwDesiredAccess,
                                     DWORD dwShareMode,
                                     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                     DWORD dwCreationDisposition,
                                     DWORD dwFlagsAndAttributes,
                                     HANDLE hTemplateFile);
WINBASEAPI HANDLE WINAPI
CreateFileMappingA(HANDLE hFile,
                   LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                   DWORD flProtect,
                   DWORD dwMaximumSizeHigh,
                   DWORD dwMaximumSizeLow,
                   LPCSTR lpName);
WINBASEAPI LPVOID WINAPI MapViewOfFile(HANDLE hFileMappingObject,
                                       DWORD dwDesiredAccess,
                                       DWORD dwFileOffsetHigh,
                                       DWORD dwFileOffsetLow,
                                       SIZE_T dwNumberOfBytesToMap);
WINBASEAPI BOOL WINAPI UnmapViewOfFile(LPCVOID lpBaseAddress);

#define CreateFile CreateFileA
#define CreateFileMapping CreateFileMappingA

#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_MAP_READ SECTION_MAP_READ
#define GENERIC_READ 0x80000000L
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define OPEN_ALWAYS 4
#define PAGE_READONLY 0x02
#define SECTION_MAP_READ 0x0004
}

Optional<MappedFile>
MappedFile::fromPath(StringView path) {
    HANDLE file = CreateFile(String(path).null(),
                             GENERIC_READ,
                             0,
                             nullptr,
                             OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return Optional<MappedFile>();
    }

    HANDLE mapping =
            CreateFileMapping(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (mapping == nullptr) {
        CloseHandle(file);
        return Optional<MappedFile>();
    }

    void* data = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (data == nullptr) {
        CloseHandle(mapping);
        CloseHandle(file);
        return Optional<MappedFile>();
    }

    MappedFile m;
    m.file = file;
    m.mapping = mapping;
    m.data = static_cast<char*>(data);
    return Optional<MappedFile>(move_(m));
}

MappedFile::MappedFile()
        : file(INVALID_HANDLE_VALUE), mapping(nullptr), data(nullptr) {}

MappedFile::MappedFile(MappedFile&& other) {
    *this = move_(other);
}

MappedFile::~MappedFile() {
    if (data) {
        UnmapViewOfFile(static_cast<void*>(data));
    }
    if (mapping) {
        CloseHandle(mapping);
    }
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
    }
}

MappedFile&
MappedFile::operator=(MappedFile&& other) {
    file = other.file;
    mapping = other.mapping;
    data = other.data;
    other.file = INVALID_HANDLE_VALUE;
    other.mapping = nullptr;
    other.data = nullptr;
    return *this;
}