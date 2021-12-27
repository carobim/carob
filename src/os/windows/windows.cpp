#include "windows.h"

#include "os/c.h"
#include "os/os.h"
#include "util/compiler.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

#pragma comment(                                                      \
        linker,                                                       \
        "\"/manifestdependency:type='win32' "                         \
        "name='Microsoft.Windows.Common-Controls' version='6.0.0.0' " \
        "processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma pack(push, 8)
extern "C" {
#define FORMAT_MESSAGE_FROM_SYSTEM    0x1000
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x200
#define FormatMessage                 FormatMessageA
#define ATTACH_PARENT_PROCESS         ((DWORD)-1)
#define CREATE_ALWAYS                 2
#define CreateDirectory               CreateDirectoryA
#define CreateFile                    CreateFileA
#define FILE_ATTRIBUTE_DIRECTORY      0x10
#define FILE_READ_ATTRIBUTES          0x80
#define FILE_READ_DATA                0x1
#define FILE_WRITE_DATA               0x2
#define FindFirstFile                 FindFirstFileA
#define FindNextFile                  FindNextFileA
#define FOREGROUND_BLUE               0x1
#define FOREGROUND_GREEN              0x2
#define FOREGROUND_INTENSITY          0x8
#define FOREGROUND_RED                0x4
#define GetFileAttributes             GetFileAttributesA
#define INVALID_FILE_ATTRIBUTES       ((DWORD)-1)
#define INVALID_HANDLE_VALUE          ((HANDLE)(LONG_PTR)-1)
#define MAX_PATH                      260
#define MB_OK                         0
#define MessageBox                    MessageBoxA
#define OPEN_EXISTING                 3
#define STD_OUTPUT_HANDLE             ((DWORD)-11)

typedef struct {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

typedef const char* va_list;

typedef struct {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[MAX_PATH];
    CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA, WIN32_FIND_DATA, *LPWIN32_FIND_DATAA;

WINBASEAPI BOOL WINAPI AttachConsole(DWORD) noexcept;
WINBASEAPI BOOL WINAPI CloseHandle(HANDLE) noexcept;
WINBASEAPI BOOL WINAPI
CreateDirectoryA(LPCSTR, void*) noexcept;
WINBASEAPI HANDLE WINAPI
CreateFileA(LPCSTR, DWORD, DWORD, void*, DWORD, DWORD, HANDLE) noexcept;
WINBASEAPI VOID WINAPI ExitProcess(UINT) noexcept;
WINBASEAPI HANDLE WINAPI FindFirstFileA(LPCSTR, LPWIN32_FIND_DATAA) noexcept;
WINBASEAPI BOOL WINAPI FindNextFileA(HANDLE, LPWIN32_FIND_DATAA) noexcept;
WINBASEAPI BOOL WINAPI FindClose(HANDLE) noexcept;
WINBASEAPI
DWORD
WINAPI
FormatMessageA(DWORD dwFlags,
               LPCVOID lpSource,
               DWORD dwMessageId,
               DWORD dwLanguageId,
               LPSTR lpBuffer,
               DWORD nSize,
               va_list* Arguments);
CRTIMP FILE*
freopen(const char*, const char*, FILE*) noexcept;
WINBASEAPI DWORD WINAPI GetFileAttributesA(LPCSTR) noexcept;
WINBASEAPI BOOL WINAPI
GetFileSizeEx(HANDLE, LARGE_INTEGER*) noexcept;
WINBASEAPI DWORD WINAPI GetLastError(VOID) noexcept;
WINBASEAPI HANDLE WINAPI GetStdHandle(DWORD) noexcept;
WINUSERAPI int WINAPI MessageBoxA(HWND, LPCSTR, LPCSTR, UINT) noexcept;
WINBASEAPI BOOL WINAPI
ReadFile(HANDLE, VOID*, DWORD, DWORD*, void*) noexcept;
WINBASEAPI BOOL WINAPI SetConsoleTextAttribute(HANDLE, WORD) noexcept;
WINBASEAPI BOOL WINAPI
WriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, void*) noexcept;
}  // extern "C"
#pragma pack(pop)


Filesize
getFileSize(StringView path) noexcept {
    HANDLE file = CreateFile(String(path).null(),
                             FILE_READ_ATTRIBUTES,
                             0,
                             0,
                             OPEN_EXISTING,
                             0,
                             0);
    if (file == INVALID_HANDLE_VALUE) {
        return FS_ERROR;
    }

    LARGE_INTEGER size;
    BOOL ok = GetFileSizeEx(file, &size);
    if (!ok) {
        printWin32Error();
        return FS_ERROR;
    }

    ok = CloseHandle(file);
    assert_(ok);

    return static_cast<Filesize>(size.QuadPart);
}

bool
writeFile(StringView path, U32 length, void* data) noexcept {
    HANDLE file = CreateFile(String(path).null(),
                             FILE_WRITE_DATA,
                             0,
                             0,
                             CREATE_ALWAYS,
                             0,
                             0);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD written;
    BOOL ok = WriteFile(file, data, length, &written, 0);
    if (!ok) {
        printWin32Error();
        CloseHandle(file);
        return false;
    }
    if (length != written) {
        printWin32Error();
        CloseHandle(file);
        return false;
    }

    CloseHandle(file);

    return true;
}

bool
writeFileVec(StringView path,
             U32 count,
             U32* lengths,
             void** datas) noexcept {
    HANDLE file = CreateFile(String(path).null(),
                             FILE_WRITE_DATA,
                             0,
                             0,
                             CREATE_ALWAYS,
                             0,
                             0);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    for (U32 i = 0; i < count; i++) {
        DWORD written;
        BOOL ok = WriteFile(file, datas[i], lengths[i], &written, 0);
        if (!ok) {
            printWin32Error();
            CloseHandle(file);
            return false;
        }
        if (lengths[i] != written) {
            CloseHandle(file);
            return false;
        }
    }

    CloseHandle(file);

    return true;
}

bool
isDir(StringView path) noexcept {
    DWORD attributes = GetFileAttributes(String(path).null());

    return (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY));
}

void
makeDirectory(StringView path) noexcept {
    BOOL ok = CreateDirectory(String(path).null(), 0);
    assert_(ok);
}

Vector<String>
listDir(StringView path) noexcept {
    Vector<String> files;

    String query = path;
    query << "\\*";

    WIN32_FIND_DATA data;
    HANDLE find = FindFirstFile(query.null(), &data);

    if (find == INVALID_HANDLE_VALUE) {
        return Vector<String>();
    }

    do {
        if (data.cFileName[0] == '.' && data.cFileName[1] == '\0' ||
            data.cFileName[1] == '.') {
            continue;
        }

        files.push(String(data.cFileName));
    } while (FindNextFile(find, &data));

    FindClose(find);

    return files;
}

bool
readFile(StringView path, String& data) noexcept {
    Filesize size_ = getFileSize(path);
    if (size_ == FS_ERROR) {
        return false;
    }

    if (size_ > UINT32_MAX) {
        return false;
    }

    DWORD size = static_cast<DWORD>(size_);

    HANDLE file = CreateFile(String(path).null(),
                             FILE_READ_DATA,
                             0,
                             0,
                             OPEN_EXISTING,
                             0,
                             0);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    data.reserve(size + 1);
    data.resize(size);

    DWORD read;
    BOOL ok = ReadFile(file, data.data, size, &read, 0);
    if (!CloseHandle(file)) {
        printWin32Error();
        return false;
    }
    if (!ok) {
        printWin32Error();
        return false;
    }
    if (read != size) {
        printWin32Error();
        return false;
    }

    // Add NUL terminator.
    data.data[size] = 0;

    return true;
}

void
setTermColor(TermColor color, Output& /*out*/) noexcept {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == INVALID_HANDLE_VALUE) {
        return;
    }

    WORD attribute;

    switch (color) {
    default:
    case TC_RESET:
        attribute = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
    case TC_GREEN:
        attribute = FOREGROUND_GREEN;
        break;
    case TC_YELLOW:
        attribute = FOREGROUND_RED | FOREGROUND_GREEN;
        break;
    case TC_RED:
        attribute = FOREGROUND_RED;
        break;
    }

    BOOL ok = SetConsoleTextAttribute(out, attribute);
    assert_(ok);
}

void
wFixConsole(void) noexcept {
    // if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    //    freopen("CONOUT$", "wb", stdout);  // Attach STDOUT.
    //    freopen("CONOUT$", "wb", stderr);  // Attach STDERR.
    //}
}

void
wMessageBox(StringView title, StringView text) noexcept {
    // FIXME: Try to get the window's native handle instead of passing nullptr.
    MessageBox(0, String(text).null(), String(title).null(), MB_OK);
}

void
exitProcess(int code) noexcept {
    ExitProcess(code);
    unreachable;
}

#ifndef NDEBUG
void
printWin32Error(void) noexcept {
    DWORD error = GetLastError();

    char buf[512];

    DWORD size = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            0,
            error,
            0,
            buf,
            sizeof(buf) / sizeof(TCHAR),
            0);
    if (size == 0) {
        // Error.
        return;
    }

    // size might be count of TCHAR written to buf

    serr << StringView(buf, size) << '\n';
}
#endif
