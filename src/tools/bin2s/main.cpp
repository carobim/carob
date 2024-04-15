#include "os/io.h"
#include "os/os.h"
#include "util/assert.h"
#include "util/math2.h"
#include "util/io.h"
#include "util/string-view.h"
#include "util/string2.h"

#define GCC_OR_LLVM 0
#define APPLE_LLVM 1

// #define ASM GCC_OR_LLVM
#define ASM APPLE_LLVM

String
makeCIdent(StringView path) noexcept {
    Size pos = path.find('/');
    if (pos != SV_NOT_FOUND)
        path = path.substr(pos + 1);
#ifdef MSVC
    pos = path.find('\\');
    if (pos != SV_NOT_FOUND)
        path = path.substr(pos + 1);
#endif
    String s;
    for (char c : path) {
        if (c == '-' || c == '.')
            c = '_';
        if (c == '_' || isAlNum(c))
            s << c;
    }
    return s;
}

int
main(int argc, char** argv) noexcept {
    Flusher f1(sout);
    Flusher f2(serr);

    if (argc != 2)
        return 1;
    StringView ifn = argv[1];
    String ofn = String() << ifn << ".s";

    String id;
    bool ok = readFile(ifn, id);
    assert_(ok);

    String ident = makeCIdent(ifn);

    int align = 4;

    String od;

#if ASM == GCC_OR_LLVM
    od << "\t.section .rodata." << ident << ", \"a\"\n";
    od << "\t.balign " << align << '\n';
    od << "\t.global " << ident << '\n';
    od << ident << ":\n";
#else
    od << "\t.const_data\n";
    od << "\t.balign " << align << '\n';
    od << "\t.global _" << ident << '\n';
    od << '_' << ident << ":\n";
#endif

    for (Size i = 0; i < id.size; i += 16) {
        od << "\t.byte ";
        Size end = min(i + 16, id.size);
        for (Size j = i; j < end; j++) {
            if (i != j)
                od << ',';
            od << (U32)(U8)id[j];
        }
        od << '\n';
    }

    ok = writeFile(ofn, od.size, od.data);
    assert_(ok);

    sout << "extern I8 " << ident << "[];\n";
    sout << "#define " << ident << "_size " << id.size << '\n';
    return 0;
}
