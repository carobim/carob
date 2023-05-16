#include "pack/walker.h"

#include "os/os.h"
#include "util/compiler.h"
#include "util/queue.h"
#include "util/string-view.h"
#include "util/vector.h"

void
walk(Vector<StringView> paths, void* userData,
     void (*op)(void* userData, StringView path)) noexcept {
    Queue<String> files;

    for (StringView* path = paths.begin(); path != paths.end(); path++)
        files.push(*path);

    while (files.size) {
        String path = static_cast<String&&>(files.front());
        files.pop();

        // TODO: The call to isDir can be removed on Unix-like systems, which
        //       will save a stat(), as readdir() returns a file's type already.
        if (isDir(path)) {
            Vector<String> names = listDir(path);
            for (String* name = names.begin(); name != names.end(); name++) {
                String child;
                child << path << DIR_SEPARATOR << *name;
                files.push(static_cast<String&&>(child));
            }
        }
        else {
            op(userData, path.view());
        }
    }
}
