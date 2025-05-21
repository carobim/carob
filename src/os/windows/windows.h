#ifndef SRC_OS_WINDOWS_WINDOWS_H_
#define SRC_OS_WINDOWS_WINDOWS_H_

#include "util/compiler.h"
#include "util/string-view.h"

/* Visual C++ ignorantly assumes that all programs will use either a console OR
 * a window. Our program needs a window and an optional console. When Carob
 * is run from the command line, this function forces Windows to reattach a
 * console to its process. Otherwise it does nothing.
 */
void
wFixConsole(void) noexcept;

// Simple wrapper to create a halting (modal) message box.
void
wMessageBox(StringView title, StringView text) noexcept;

#if DEBUG
void
printWin32Error(void) noexcept;
#else
#    define printWin32Error()
#endif

#endif  // SRC_OS_WINDOWS_WINDOWS_H_
