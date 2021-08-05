#ifndef SRC_OS_MAC_GUI_H_
#define SRC_OS_MAC_GUI_H_

#include "util/compiler.h"
#include "util/string-view.h"

/**
 * Sets the current working directory to the "Resources" folder in the .app
 * package.
 */
void
macSetWorkingDirectory() noexcept;

/**
 * Displays a message box window.
 */
void
macMessageBox(StringView title, StringView msg) noexcept;

#endif  // SRC_OS_MAC_GUI_H_
