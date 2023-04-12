#ifndef SRC_OS_MAC_C_H_
#define SRC_OS_MAC_C_H_

#include "util/compiler.h"
#include "util/int.h"

extern "C" {

// sys/_types.h
typedef I64 blkcnt_t;
typedef I32 blksize_t;
typedef I32 dev_t;
typedef U32 gid_t;
typedef U64 ino64_t;
typedef U16 mode_t;
// In sys/_type/_nlink_t.h on Xcode 12.5/macOS 11
typedef U16 nlink_t;
typedef I64 off_t;
typedef U32 uid_t;
// In sys/_type/_iovec_t.h on Xcode 12.5/macOS 11
struct iovec {
    void* iov_base;
    Size iov_len;
};
// In sys/_type/_timespec.h on Xcode 12.5/macOS 11
struct timespec {
    Time tv_sec;
    long tv_nsec;
};

// sys/_pthread/_pthread_types.h
#define __PTHREAD_COND_SIZE__  40
#define __PTHREAD_MUTEX_SIZE__ 56
#define __PTHREAD_SIZE__       8176
struct pthread_cond_t {
    long __sig;
    char __opaque[__PTHREAD_COND_SIZE__];
};
struct pthread_mutex_t {
    long __sig;
    char __opaque[__PTHREAD_MUTEX_SIZE__];
};
struct _pthread_t {
    long __sig;
    struct __darwin_pthread_handler_rec* __cleanup_stack;
    char __opaque[__PTHREAD_SIZE__];
};
typedef _pthread_t* pthread_t;

// sys/cdefs.h
#if defined(__i386__)
#    error unknown C types on 32-bit XNU
#elif defined(__x86_64__)
#    define __DARWIN_ALIAS_I(sym) __asm("_" #sym "$INODE64")
#    define __DARWIN_INODE64(sym) __asm("_" #sym "$INODE64")
#else
#    define __DARWIN_ALIAS_I(sym)
#    define __DARWIN_INODE64(sym)
#endif

// sys/dirent.h
#define __DARWIN_MAXPATHLEN 1024
struct dirent {
    U64 d_ino;
    U64 d_seekoff;
    U16 d_reclen;
    U16 d_namlen;
    U8 d_type;
    char d_name[__DARWIN_MAXPATHLEN];
};
#define DT_DIR 4
#define DT_REG 8
#define DT_WHT 14

// sys/errno.h
int*
__error(void) noexcept;
#define errno (*__error())
#define EINTR 4

// sys/fcntl.h
int
open(const char*, int, ...) noexcept;
#define O_RDONLY    0x0000000
#define O_WRONLY    0x0000001
#define O_NONBLOCK  0x0000004
#define O_CREAT     0x0000200
#define O_TRUNC     0x0000400
#define O_DIRECTORY 0x0100000
#define O_CLOEXEC   0x1000000

// sys/mman.h
void*
mmap(void*, Size, int, int, int, off_t) noexcept;
int
munmap(void*, Size) noexcept;
#define MAP_FAILED ((void*)-1)
#define MAP_SHARED 0x1
#define PROT_READ  0x1

// sys/stat.h
struct stat {
    dev_t st_dev;
    mode_t st_mode;
    nlink_t st_nlink;
    ino64_t st_ino;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    struct timespec st_atimespec;
    struct timespec st_mtimespec;
    struct timespec st_ctimespec;
    struct timespec st_birthtimespec;
    off_t st_size;
    blkcnt_t st_blocks;
    blksize_t st_blksize;
    U32 st_flags;
    U32 st_gen;
    I32 st_lspare;
    I64 st_qspare[2];
};
int
fstat(int, struct stat*) noexcept __DARWIN_INODE64(fstat);
int
mkdir(const char*, mode_t) noexcept;
int
stat(const char*, struct stat*) noexcept __DARWIN_INODE64(stat);
// In sys/_types/_s_ifmt.h on Xcode 12.5/macOS 11
#define S_IFMT     0170000
// In sys/_types/_s_ifmt.h on Xcode 12.5/macOS 11
#define S_IFDIR    0040000
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)

// sys/sysctl.h
int
sysctl(int*, unsigned int, void*, Size*, void*, Size) noexcept;
#define CTL_HW  6
#define HW_NCPU 3

// sys/uio.h
SSize
writev(int, const struct iovec*, int) noexcept;

// _stdio.h
struct FILE;

// dirent.h
struct DIR {
    int __dd_fd;
    long __dd_loc;
    long __dd_size;
    char* __dd_buf;
    int __dd_len;
    long __dd_seek;
    long __padding;
    int __dd_flags;
    pthread_mutex_t __dd_lock;
    struct _telldir* __dd_td;
};
int
closedir(DIR*) noexcept;
DIR*
opendir(const char*) noexcept __DARWIN_ALIAS_I(opendir);
struct dirent*
readdir(DIR*) noexcept __DARWIN_INODE64(readdir);

// math.h
float
atan2f(float, float) noexcept;
float
ceilf(float) noexcept;
float
cosf(float) noexcept;
float
floorf(float) noexcept;
float
sinf(float) noexcept;
float
sqrtf(float) noexcept;

// pthread/pthread.h
// pthread/pthread_impl.h
#define _PTHREAD_MUTEX_SIG_init 0x32AAABA7
#define PTHREAD_MUTEX_INITIALIZER      \
    {                                  \
        _PTHREAD_MUTEX_SIG_init, { 0 } \
    }
#define _PTHREAD_COND_SIG_init 0x3CB0B1BB
#define PTHREAD_COND_INITIALIZER      \
    {                                 \
        _PTHREAD_COND_SIG_init, { 0 } \
    }
int
pthread_mutex_destroy(pthread_mutex_t*) noexcept;
int
pthread_mutex_lock(pthread_mutex_t*) noexcept;
int
pthread_mutex_unlock(pthread_mutex_t*) noexcept;
int
pthread_cond_destroy(pthread_cond_t*) noexcept;
int
pthread_cond_signal(pthread_cond_t*) noexcept;
int
pthread_cond_broadcast(pthread_cond_t*) noexcept;
int
pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*) noexcept;
int
pthread_create(pthread_t*, const void*, void* (*)(void*), void*) noexcept;
int
pthread_join(pthread_t, void**) noexcept;

// stdio.h
int
fclose(FILE*) noexcept;
FILE*
fopen(const char*, const char*) noexcept;
int
fprintf(FILE*, const char*, ...) noexcept;
Size
fread(void*, Size, Size, FILE*) noexcept;
int
printf(const char*, ...) noexcept;
int
sprintf(char*, const char*, ...) noexcept;
extern FILE* __stdinp;
extern FILE* __stdoutp;
extern FILE* __stderrp;
#define stdin  __stdinp
#define stdout __stdoutp
#define stderr __stderrp

// stdlib.h
int
rand() noexcept;
void
srand(unsigned) noexcept;
double
strtod(const char*, char**) noexcept;
long
strtol(const char*, char**, int) noexcept;
unsigned long
strtoul(const char*, char**, int) noexcept;

// string.h
void*
memchr(const void*, int, Size) noexcept;
int
memcmp(const void*, const void*, Size) noexcept;
void*
memcpy(void*, const void*, Size) noexcept;
void*
memmem(const void*, Size, const void*, Size) noexcept;
void*
memmove(void*, const void*, Size) noexcept;
void*
memset(void*, int, Size) noexcept;
Size
strlen(char const*) noexcept;

// time.h
enum clockid_t {
    CLOCK_UPTIME_RAW = 8,
};
int
clock_gettime(clockid_t, struct timespec*) noexcept;

// unistd.h
int
close(int) noexcept;
void
_exit(int) noexcept __attribute__((noreturn));
int
ftruncate(int, off_t) noexcept;
int
isatty(int) noexcept;
SSize
pread(int, void*, Size, off_t) noexcept;
SSize
pwrite(int, const void*, Size, off_t) noexcept;
SSize
read(int, void*, Size) noexcept;
SSize
write(int, const void*, Size) noexcept;

}  // extern "C"

#endif  // SRC_OS_MAC_C_H_
