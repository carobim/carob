#ifndef SRC_OS_MAC_C_H_
#define SRC_OS_MAC_C_H_

#include "util/compiler.h"
#include "util/int.h"

extern "C" {

// sys/_types.h
typedef int64_t blkcnt_t;
typedef int32_t blksize_t;
typedef int32_t dev_t;
typedef uint32_t gid_t;
typedef uint64_t ino64_t;
typedef uint16_t mode_t;
// In sys/_type/_nlink_t.h on Xcode 12.5/macOS 11
typedef uint16_t nlink_t;
typedef int64_t off_t;
typedef uint32_t uid_t;
// In sys/_type/_iovec_t.h on Xcode 12.5/macOS 11
struct iovec {
    void* iov_base;
    size_t iov_len;
};
// In sys/_type/_timespec.h on Xcode 12.5/macOS 11
struct timespec {
    time_t tv_sec;
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
    uint64_t d_ino;
    uint64_t d_seekoff;
    uint16_t d_reclen;
    uint16_t d_namlen;
    uint8_t d_type;
    char d_name[__DARWIN_MAXPATHLEN];
};
#define DT_DIR 4
#define DT_REG 8

// sys/errno.h
int*
__error(void) noexcept;
#define errno (*__error())
#define EINTR 4

// sys/fcntl.h
int
open(const char*, int, ...) noexcept;
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_CREAT  0x200
#define O_TRUNC  0x400

// sys/mman.h
void*
mmap(void*, size_t, int, int, int, off_t) noexcept;
int
munmap(void*, size_t) noexcept;
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
    uint32_t st_flags;
    uint32_t st_gen;
    int32_t st_lspare;
    int64_t st_qspare[2];
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
sysctl(int*, unsigned int, void*, size_t*, void*, size_t) noexcept;
#define CTL_HW  6
#define HW_NCPU 3

// sys/uio.h
ssize_t
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
double
atan2(double, double) noexcept;
double
ceil(double) noexcept;
float
ceilf(float) noexcept;
double
cos(double) noexcept;
double
floor(double) noexcept;
double
sin(double) noexcept;
double
sqrt(double) noexcept;

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
size_t
fread(void*, size_t, size_t, FILE*) noexcept;
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
memchr(const void*, int, size_t) noexcept;
int
memcmp(const void*, const void*, size_t) noexcept;
void*
memcpy(void*, const void*, size_t) noexcept;
void*
memmem(const void*, size_t, const void*, size_t) noexcept;
void*
memmove(void*, const void*, size_t) noexcept;
void*
memset(void*, int, size_t) noexcept;
size_t
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
ssize_t
pread(int, void*, size_t, off_t) noexcept;
ssize_t
pwrite(int, const void*, size_t, off_t) noexcept;
ssize_t
read(int, void*, size_t) noexcept;
ssize_t
write(int, const void*, size_t) noexcept;

}  // extern "C"

#endif  // SRC_OS_MAC_C_H_