#ifndef SRC_OS_LINUX_LINUX_C_H_
#define SRC_OS_LINUX_LINUX_C_H_

// For additional copyright information pertaining to this file, please refer
// to musl's COPYRIGHT file.

// Note: Prefer sourcing types from musl.

#include "util/compiler.h"
#include "util/int.h"

// arch/x86_64/bits/stat.h
// arch/arm/bits/stat.h
// arch/i386/bits/stat.h
// include/alltypes.h.in
extern "C" {
typedef struct _IO_FILE FILE;
typedef I64 blkcnt_t;
typedef I64 off_t;
typedef int clockid_t;
#ifdef __aarch64__
// musl bits/alltypes.h
typedef int blksize_t;
#else
typedef long blksize_t;
#endif
typedef U64 dev_t;
typedef U64 ino_t;
struct iovec {
    void* iov_base;
    Size iov_len;
};
typedef unsigned gid_t;
#ifdef __aarch64__
// musl bits/alltypes.h
typedef unsigned int nlink_t;
#else
typedef unsigned long nlink_t;
#endif
typedef unsigned long pthread_t;
typedef unsigned mode_t;
struct timespec {
    Time tv_sec;
    long tv_nsec;
};
typedef unsigned uid_t;

// include/alltypes.h.in
#if __LONG_MAX__ == 0x7fffffffffffffffL
typedef volatile int pthread_cond_t[12];
typedef volatile int pthread_mutex_t[10];
#elif __LONG_MAX__ == 0x7fffffff
typedef volatile int pthread_cond_t[12];
// Is this 13 ints on gcc-4.5 w/libc6-2.13?
typedef volatile int pthread_mutex_t[6];
#else
#    error unknown system
#endif

// bits/errno.h
// errno.h
int*
__errno_location() noexcept;
#define errno (*__errno_location())
#define EINTR 4

// fcntl.h
int
open(const char*, int, ...) noexcept;
#define O_RDONLY 00
#define O_WRONLY 01
#define O_CREAT  0100
#define O_TRUNC  01000

// sys/mman.h
void*
mmap(void*, Size, int, int, int, off_t) noexcept;
int
munmap(void*, Size) noexcept;
#define MAP_FAILED ((void*)-1)
#define MAP_SHARED 0x01
#define PROT_READ  1

#if defined(__EMSCRIPTEN__)
// musl arch/emscripten/bits/stat.h
struct stat {
    dev_t st_dev;
    long __st_ino;
    mode_t st_mode;
    unsigned st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    int st_blocks;  // blkcnt_t = int
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    ino_t st_ino;
};
#elif defined(__aarch64__)
// arch/aarch64/bits/stat.h
struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    unsigned long __pad;
    off_t st_size;
    blksize_t st_blksize;
    int __pad2;
    blkcnt_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    unsigned __unused[2];
};
#elif defined(__x86_64__)
// arch/x86_64/bits/stat.h
struct stat {
    dev_t st_dev;
    ino_t st_ino;
    nlink_t st_nlink;
    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid;
    unsigned int __pad0;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    long __unused[3];
};
#elif __LONG_MAX__ == 0x7fffffff
// arch/arm/bits/stat.h
// arch/i386/bits/stat.h
struct stat {
    dev_t st_dev;
    int __st_dev_padding;
    long __st_ino_truncated;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    int __st_rdev_padding;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    ino_t st_ino;
};
#endif

// bits/stat.h
// sys/stat.h
int
fstat(int, struct stat*) noexcept;
int
mkdir(const char*, mode_t) noexcept;
int
stat(const char*, struct stat*) noexcept;
#define S_IFMT     0170000
#define S_IFDIR    0040000
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)

// sys/uio.h
SSize
writev(int, const struct iovec*, int) noexcept;

// dirent.h
struct dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[256];
};
typedef struct __dirstream DIR;
int
closedir(DIR*) noexcept;
DIR*
opendir(const char*) noexcept;
struct dirent*
readdir(DIR*) noexcept;
#define DT_DIR 4
#define DT_REG 8

// math.h
#define atan2f __builtin_atan2f
#define ceilf __builtin_ceilf
#define cosf __builtin_cosf
#define floorf __builtin_floorf
#define sinf __builtin_sinf
#define sqrtf __builtin_sqrtf

// pthread.h
#define PTHREAD_MUTEX_INITIALIZER { 0 }
#define PTHREAD_COND_INITIALIZER { 0 }
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
extern FILE* const stdin;
extern FILE* const stdout;
extern FILE* const stderr;

// stdlib.h
int
abs(int) noexcept;
void
exit(int) noexcept;
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
#define memcpy __builtin_memcpy
void*
memmem(const void*, Size, const void*, Size) noexcept;
void*
memmove(void*, const void*, Size) noexcept;
void*
memset(void*, int, Size) noexcept;
Size
strlen(char const*) noexcept;

// time.h
int
clock_gettime(clockid_t, struct timespec*) noexcept;
int
nanosleep(const struct timespec*, struct timespec*) noexcept;
#define CLOCK_MONOTONIC 1

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
long
sysconf(int) noexcept;
SSize
write(int, const void*, Size) noexcept;
#define _SC_NPROCESSORS_ONLN 84

}  // extern "C"

#endif  // SRC_OS_LINUX_C_H_
