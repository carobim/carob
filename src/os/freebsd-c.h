#ifndef SRC_OS_FREEBSD_C_H_
#define SRC_OS_FREEBSD_C_H_

#include "util/int.h"

// sys/_iovec.h
// sys/_timespec.h
// sys/_types.h
// sys/types.h
extern "C" {
typedef I64 blkcnt_t;
typedef I32 blksize_t;
typedef I32 clockid_t;
typedef U64 dev_t;
typedef U32 fflags_t;
typedef U32 gid_t;
typedef U64 ino_t;
typedef U16 mode_t;
typedef U64 nlink_t;
typedef I64 off_t;
typedef U32 uid_t;
struct iovec {
    void* iov_base;
    Size iov_len;
};
struct timespec {
    Time tv_sec;
    long tv_nsec;
};
}

// sys/_pthreadtypes.h
extern "C" {
struct pthread;
struct pthread_cond;
struct pthread_mutex;
typedef struct pthread* pthread_t;
typedef struct pthread_cond* pthread_cond_t;
typedef struct pthread_mutex* pthread_mutex_t;
}

// sys/dirent.h
extern "C" {
struct dirent {
    ino_t d_fileno;
    off_t d_off;
    U16 d_reclen;
    U8 d_type;
    U8 d_pad0;
    U16 d_namlen;
    U16 d_pad1;
    char d_name[256];
};
#define DT_DIR 4
#define DT_REG 8
}

// errno.h
extern "C" {
int*
__error() noexcept;
#define errno (*__error())
#define EINTR 4
}

// fcntl.h
extern "C" {
int
open(const char*, int, ...) noexcept;
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_CREAT  0x0200
#define O_TRUNC  0x0400
}

// sys/mman.h
extern "C" {
void*
mmap(void*, Size, int, int, int, off_t) noexcept;
int
munmap(void*, Size) noexcept;
#define MAP_FAILED ((void*)-1)
#define MAP_SHARED 0x0001
#define PROT_READ  0x01
}

// sys/stat.h
extern "C" {
struct stat {
    dev_t st_dev;
    ino_t st_ino;
    nlink_t st_nlink;
    mode_t st_mode;
    I16 st_padding0;
    uid_t st_uid;
    gid_t st_gid;
    I32 st_padding1;
    dev_t st_rdev;
#ifdef __i386__
    I32 st_atim_ext;
#endif
    struct timespec st_atim;
#ifdef __i386__
    I32 st_mtim_ext;
#endif
    struct timespec st_mtim;
#ifdef __i386__
    I32 st_ctim_ext;
#endif
    struct timespec st_ctim;
#ifdef __i386__
    I32 st_btim_ext;
#endif
    struct timespec st_birthtim;
    off_t st_size;
    blkcnt_t st_blocks;
    blksize_t st_blksize;
    fflags_t st_flags;
    U64 st_gen;
    U64 st_spare[10];
};
int
fstat(int, struct stat*) noexcept;
int
mkdir(const char*, mode_t) noexcept;
int
stat(const char*, struct stat*) noexcept;
#define S_IFMT     0170000
#define S_IFDIR    0040000
#define S_ISDIR(m) (((m)&0170000) == 0040000)
}

// sys/uio.h
extern "C" {
SSize
writev(int, const struct iovec*, int) noexcept;
}

// dirent.h
extern "C" {
typedef struct _dirdesc DIR;
int
closedir(DIR*) noexcept;
DIR*
opendir(const char*) noexcept;
struct dirent*
readdir(DIR*) noexcept;
#define d_ino d_fileno
}

// math.h
#define atan2f __builtin_atan2f
#define ceilf __builtin_ceilf
#define cosf __builtin_cosf
#define floorf __builtin_floorf
#define sinf __builtin_sinf
#define sqrtf __builtin_sqrtf

// pthread.h
extern "C" {
#define PTHREAD_MUTEX_INITIALIZER { 0 }
#define PTHREAD_COND_INITIALIZER  { 0 }
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
}

// stdio.h
extern "C" {
struct __sFILE;
typedef struct __sFILE FILE;
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
}

// stdlib.h
extern "C" {
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
}

// string.h
extern "C" {
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
}

// time.h
extern "C" {
int
clock_gettime(clockid_t, struct timespec*) noexcept;
int
nanosleep(const struct timespec*, struct timespec*) noexcept;
#define CLOCK_MONOTONIC 4
}

// unistd.h
extern "C" {
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
#define _SC_NPROCESSORS_ONLN 58
}

#endif  // SRC_OS_FREEBSD_C_H_
