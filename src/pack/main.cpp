#include "os/os.h"
#include "pack/pack-reader.h"
#include "pack/pack-writer.h"
#include "pack/walker.h"
#include "util/compiler.h"
#include "util/int.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string.h"

static String exe;
static bool verbose = false;

static void
usage() noexcept {
    String msg;
    msg << "usage: " << exe
        << " create [-v] <output-archive> [input-file]...\n"
           "       "
        << exe
        << " list <input-archive>\n"
           "       "
        << exe << " extract [-v] <input-archive>\n";
    serr << msg;
}

struct CreateArchiveContext {
    PackWriter* pack;
};

static void
addFile(CreateArchiveContext* ctx, StringView path) noexcept {
    String data;
    bool ok = readFile(path, data);

    if (!ok) {
        if (verbose) {
            sout << "Skipped " << path << ": file not found\n";
        }
        return;
    }

    if (verbose) {
        sout << "Added " << path << ": " << data.size << " bytes\n";
    }

    // Write the file path to the pack file with '/' instead of '\\' on Windows.
    String standardizedPath;

#if DIR_SEPARATOR != '/'
    standardizedPath = path;

    for (Size i = 0; i < path.size; i++) {
        if (standardizedPath[i] == DIR_SEPARATOR) {
            standardizedPath[i] = '/';
        }
    }

    path = standardizedPath;
#endif

    packWriterAddBlob(ctx->pack, path, static_cast<U32>(data.size), data.data);

    data.reset();  // Don't delete data pointer.
}

static void
addFileCallback(void* data, StringView path) noexcept {
    CreateArchiveContext* ctx = reinterpret_cast<CreateArchiveContext*>(data);
    addFile(ctx, path);
}

static bool
createArchive(StringView archivePath, Vector<StringView> paths) noexcept {
    CreateArchiveContext ctx;
    ctx.pack = makePackWriter();

    walk(static_cast<Vector<StringView>&&>(paths), &ctx, addFileCallback);

    bool ok = packWriterWriteToFile(ctx.pack, archivePath);

    if (verbose) {
        sout << "Wrote to " << archivePath << '\n';
    }

    destroyPackWriter(ctx.pack);
    return ok;
}

static bool
listArchive(StringView archivePath) noexcept {
    PackReader* pack = makePackReader(archivePath);
    if (!pack) {
        serr << exe << ": " << archivePath << ": not found\n";

        destroyReader(pack);
        return false;
    }

    String output;

    U32 numEntries = readerSize(pack);
    for (U32 i = 0; i < numEntries; i++) {
        BlobDetails details = readerDetails(pack, i);
        StringView path = details.path;
        U32 size = details.size;

        // Print file paths with '\\' Windows.
        String standardizedPath;

#if DIR_SEPARATOR != '/'
        standardizedPath = path;

        for (Size j = 0; j < path.size; j++) {
            if (standardizedPath[j] == '/') {
                standardizedPath[j] = DIR_SEPARATOR;
            }
        }

        path = standardizedPath;
#endif

        output << path << ": " << size << " bytes\n";
    }

    sout << output;

    destroyReader(pack);
    return true;
}

static bool
getParentPath(StringView path, StringView& parent) noexcept {
    StringPosition sep = path.rfind(DIR_SEPARATOR);
    if (sep == SV_NOT_FOUND) {
        return false;
    }
    else {
        parent = path.substr(0, sep);
        return true;
    }
}

static void
createDirs(StringView path) noexcept {
    StringView parentPath;
    if (getParentPath(path, parentPath)) {
        // Make sure parentPath's parent exists.
        createDirs(parentPath);

        makeDirectory(parentPath);
    }
}

static void
putFile(StringView path, U32 size, void* data) noexcept {
    createDirs(path);

    // TODO: Propagate error up.
    writeFile(path, size, data);
}

static bool
extractArchive(StringView archivePath) noexcept {
    PackReader* pack = makePackReader(archivePath);
    if (!pack) {
        serr << exe << ": " << archivePath << ": not found\n";

        destroyReader(pack);
        return false;
    }

    U32 numEntries = readerSize(pack);

    U32 maxSize = 0;
    for (U32 i = 0; i < numEntries; i++) {
        BlobDetails details = readerDetails(pack, i);
        U32 size = details.size;
        maxSize = size > maxSize ? size : maxSize;
    }

    String standardizedPath;
    void* buf = malloc(maxSize);

    for (U32 i = 0; i < numEntries; i++) {
        BlobDetails details = readerDetails(pack, i);
        StringView path = details.path;
        U32 size = details.size;

        // Change file paths to use '\\' on Windows.
#if DIR_SEPARATOR != '/'
        standardizedPath = path;

        for (Size j = 0; j < path.size; j++) {
            if (standardizedPath[j] == '/') {
                standardizedPath[j] = DIR_SEPARATOR;
            }
        }

        path = standardizedPath;
#endif

        if (verbose) {
            sout << "Extracting " << path << ": " << size << " bytes\n";
        }

        readerRead(pack, buf, i);

        putFile(path, size, buf);
    }

    free(buf);
    destroyReader(pack);
    return true;
}

I32
main(I32 argc, char* argv[]) noexcept {
    Flusher f1(sout);
    Flusher f2(serr);

    exe = argv[0];
    StringPosition dir = exe.view().rfind(DIR_SEPARATOR);
    if (dir == SV_NOT_FOUND) {
        exe = exe.view().substr(dir + 1);
    }

    if (argc == 1) {
        usage();
        return 0;
    }
    if (argc == 2) {
        usage();
        return 1;
    }

    StringView command = argv[1];
    Vector<StringView> args;

    for (I32 i = 2; i < argc; i++) {
        args.push(argv[i]);
    }

    I32 exitCode;

    if (command == "create") {
        if (args.size > 0 && args[0] == "-v") {
            verbose = true;
            args.erase(0);
        }

        if (args.size < 2) {
            usage();
            return 1;
        }

        // The first argument is the archive, the rest are files to add to it.
        StringView archivePath = args[0];
        args.erase(0);

        return createArchive(archivePath,
                             static_cast<Vector<StringView>&&>(args))
                       ? 0
                       : 1;
    }
    else if (command == "list") {
        verbose = true;

        if (args.size != 1) {
            usage();
            return 1;
        }

        exitCode = listArchive(args[0]) ? 0 : 1;
    }
    else if (command == "extract") {
        if (args.size > 0 && args[0] == "-v") {
            verbose = true;
            args.erase(0);
        }

        if (args.size != 1) {
            usage();
            return 1;
        }

        exitCode = extractArchive(args[0]) ? 0 : 1;
    }
    else {
        usage();
        return 1;
    }

    return exitCode;
}
