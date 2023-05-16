#include "util/cpu.h"

#include "util/compiler.h"
#include "util/int.h"

// Original source downloaded from: https://github.com/google/cpu_features
// Apache License 2.0

#if defined(__i386__) || defined(__x86_64__)
inline static bool
isBitSet(U32 reg, U32 bit) noexcept {
    return (reg >> bit) & 0x1;
}

// A struct to hold the result of a call to cpuid.
struct Leaf {
    U32 eax, ebx, ecx, edx;
};

#    if defined(CLANG) || defined(GCC)
static struct Leaf
getCpuidLeaf(U32 leafId, int subleaf) noexcept {
    struct Leaf leaf;
    __asm__ volatile(
            "cpuid"
            : "=a"(leaf.eax), "=b"(leaf.ebx), "=c"(leaf.ecx), "=d"(leaf.edx)
            : "a"(leafId), "b"(0), "c"(subleaf), "d"(0));
    return leaf;
}
#    elif defined(MSVC)
static struct Leaf
getCpuidLeaf(U32 leafId, int subleaf) noexcept {
    struct Leaf leaf;
    int data[4];
    __cpuidex(data, leafId, subleaf);
    leaf.eax = data[0];
    leaf.ebx = data[1];
    leaf.ecx = data[2];
    leaf.edx = data[3];
    return leaf;
}
#    else
#        error unimplemented
#    endif

static const struct Leaf EMPTY_LEAF = {};

static struct Leaf
safeCpuIdEx(U32 maxLeaf, U32 leafId, int subleaf) noexcept {
    if (leafId <= maxLeaf) {
        return getCpuidLeaf(leafId, subleaf);
    }
    else {
        return EMPTY_LEAF;
    }
}

struct Leaves {
    U32 maxLeaf;
    //struct Leaf leaf0;   // Root
    struct Leaf leaf1;  // Family, Model, Stepping
    //struct Leaf leaf2;   // Intel cache info + features
    struct Leaf leaf7;  // Features
    //struct Leaf leaf71;  // Features
    //U32 maxLeafExt;
    //struct Leaf leaf80000000;  // Root for extended leaves
    //struct Leaf leaf80000001;  // AMD features features and cache
    //struct Leaf leaf80000002;  // brand string
    //struct Leaf leaf80000003;  // brand string
    //struct Leaf leaf80000004;  // brand string
};

static struct Leaves
readLeaves(void) noexcept {
    struct Leaf leaf0 = getCpuidLeaf(0, 0);
    U32 maxLeaf = leaf0.eax;
    //struct Leaf leaf_80000000 = getCpuidLeaf(0x80000000, 0);
    //U32 maxLeafExt = leaf_80000000.eax;
    struct Leaves leaves;
    leaves.maxLeaf = maxLeaf;
    //leaves.leaf0 = leaf0;
    leaves.leaf1 = safeCpuIdEx(maxLeaf, 0x00000001, 0);
    //leaves.leaf2 = safeCpuIdEx(maxLeaf, 0x00000002, 0);
    leaves.leaf7 = safeCpuIdEx(maxLeaf, 0x00000007, 0);
    //leaves.leaf71 = safeCpuIdEx(maxLeaf, 0x00000007, 1);
    //leaves.maxLeafExt = maxLeafExt;
    //leaves.leaf80000000 = leaf_80000000;
    //leaves.leaf80000001 = safeCpuIdEx(maxLeafExt, 0x80000001, 0);
    //leaves.leaf80000002 = safeCpuIdEx(maxLeafExt, 0x80000002, 0);
    //leaves.leaf80000003 = safeCpuIdEx(maxLeafExt, 0x80000003, 0);
    //leaves.leaf80000004 = safeCpuIdEx(maxLeafExt, 0x80000004, 0);
    return leaves;
}

#    define MASK_XMM 0x2
#    define MASK_YMM 0x4

static bool
hasMask(U32 value, U32 mask) noexcept {
    return (value & mask) == mask;
}

// Checks that operating system saves and restores xmm registers during context
// switches.
static bool
hasXmmOsXSave(U32 xcr0Eax) noexcept {
    return hasMask(xcr0Eax, MASK_XMM);
}

// Checks that operating system saves and restores ymm registers during context
// switches.
static bool
hasYmmOsXSave(U32 xcr0Eax) noexcept {
    return hasMask(xcr0Eax, MASK_XMM | MASK_YMM);
}

#    if defined(CLANG) || defined(GCC)
static U32
getXCR0Eax(void) noexcept {
    U32 eax, edx;
    /* named form of xgetbv not supported on OSX, so must use byte form, see:
     https://github.com/asmjit/asmjit/issues/78
   */
    __asm(".byte 0x0F, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c"(0));
    return eax;
}
#    elif defined(MSVC)
static U32
getXCR0Eax(void) noexcept {
    return (U32)_xgetbv(0);
}
#    else
#        error unimplemented
#    endif

struct X86Features
getCpu() noexcept {
    struct X86Features features = {};
    Leaves leaves = readLeaves();

    struct Leaf leaf1 = leaves.leaf1;
    struct Leaf leaf7 = leaves.leaf7;
    //struct Leaf leaf71 = leaves.leaf71;
    bool haveXsave = isBitSet(leaf1.ecx, 26);
    bool haveOsxsave = isBitSet(leaf1.ecx, 27);
    bool haveXcr0 = haveXsave && haveOsxsave;

    features.popcnt = isBitSet(leaf1.ecx, 23);
    features.rdrnd = isBitSet(leaf1.ecx, 30);

    if (haveXcr0) {
        U32 xcr0Eax = getXCR0Eax();
        bool sseRegisters = hasXmmOsXSave(xcr0Eax);
        bool avxRegisters = hasYmmOsXSave(xcr0Eax);
        if (sseRegisters) {
            features.sse4_1 = isBitSet(leaf1.ecx, 19);
            features.sse4_2 = isBitSet(leaf1.ecx, 20);
        }
        if (avxRegisters) {
            features.avx = isBitSet(leaf1.ecx, 28);
            features.avx2 = isBitSet(leaf7.ebx, 5);
        }
    }
    else {
        // TODO: Handle this case. It is on pre-AVX architectures.
        unreachable;
    }
    return features;
}
#elif defined(__aarch64__)
// Nothing.
#else
// Nothing. What is this?
#endif
