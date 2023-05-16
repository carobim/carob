#include "os/windows/mapped-file.h"

#include "os/c.h"
#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"

extern "C" {
typedef struct {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

WINBASEAPI BOOL WINAPI
CloseHandle(HANDLE hObject) noexcept;
WINBASEAPI HANDLE WINAPI
CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
            HANDLE hTemplateFile) noexcept;
WINBASEAPI HANDLE WINAPI
CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                   DWORD flProtect, DWORD dwMaximumSizeHigh,
                   DWORD dwMaximumSizeLow, LPCSTR lpName) noexcept;
WINBASEAPI LPVOID WINAPI
MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess,
              DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
              SIZE_T dwNumberOfBytesToMap) noexcept;
WINBASEAPI BOOL WINAPI
UnmapViewOfFile(LPCVOID lpBaseAddress) noexcept;

#define CreateFile        CreateFileA
#define CreateFileMapping CreateFileMappingA

#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_MAP_READ         SECTION_MAP_READ
#define GENERIC_READ          0x80000000L
#define INVALID_HANDLE_VALUE  ((HANDLE)(LONG_PTR)-1)
#define OPEN_EXISTING         3
#define PAGE_READONLY         0x02
#define SECTION_MAP_READ      0x0004
}

bool
makeMappedFile(MappedFile& file, StringView path) noexcept {
    HANDLE hFile = CreateFile(String(path).null(), GENERIC_READ, 0, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    HANDLE mapping = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
    if (mapping == 0) {
        CloseHandle(hFile);
        return false;
    }

    void* data = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (data == 0) {
        CloseHandle(mapping);
        CloseHandle(hFile);
        return false;
    }

    file.data = static_cast<char*>(data);
    file.mapping = mapping;
    file.file = hFile;
    return true;
}

void
destroyMappedFile(MappedFile file) noexcept {
    if (file.data) {
        UnmapViewOfFile(static_cast<void*>(file.data));
    }
    if (file.mapping) {
        CloseHandle(file.mapping);
    }
    if (file.file != INVALID_HANDLE_VALUE) {
        CloseHandle(file.file);
    }
}
