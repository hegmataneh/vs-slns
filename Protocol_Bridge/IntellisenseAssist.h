#pragma once

#if defined(INTELISENSE_BUILD) && INTELISENSE_BUILD + 0

// WARNING . please consider that these value come from deepseek so original value must be controlled by programmer him self

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long int64_t;
typedef unsigned long uint64_t;
typedef int8_t int_least8_t;
typedef uint8_t uint_least8_t;
typedef int16_t int_least16_t;
typedef uint16_t uint_least16_t;
typedef int32_t int_least32_t;
typedef uint32_t uint_least32_t;
typedef int64_t int_least64_t;
typedef uint64_t uint_least64_t;
typedef int8_t int_fast8_t;
typedef uint8_t uint_fast8_t;
typedef int16_t int_fast16_t;
typedef uint16_t uint_fast16_t;
typedef int32_t int_fast32_t;
typedef uint32_t uint_fast32_t;
typedef int64_t int_fast64_t;
typedef uint64_t uint_fast64_t;
typedef long intptr_t;
typedef unsigned long uintptr_t;
typedef int64_t intmax_t;
typedef uint64_t uintmax_t;
#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483648)
#define INT64_MIN (-9223372036854775808L)
#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807L
#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295U
#define UINT64_MAX 18446744073709551615UL
#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN
#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX
#define UINT_LEAST8_MAX UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX
#define INT_FAST8_MIN INT8_MIN
#define INT_FAST16_MIN INT16_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN
#define INT_FAST8_MAX INT8_MAX
#define INT_FAST16_MAX INT16_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX
#define UINT_FAST8_MAX UINT8_MAX
#define UINT_FAST16_MAX UINT16_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX
#define INTPTR_MIN INT64_MIN
#define INTPTR_MAX INT64_MAX
#define UINTPTR_MAX UINT64_MAX
#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX
#define UINTMAX_MAX UINT64_MAX
#define PTRDIFF_MIN INT64_MIN
#define PTRDIFF_MAX INT64_MAX
#define SIZE_MAX UINT64_MAX
#define WCHAR_MIN INT32_MIN
#define WCHAR_MAX INT32_MAX
#define WINT_MIN INT32_MIN
#define WINT_MAX INT32_MAX
#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX

//<pthread.h>
typedef unsigned long int pthread_t;  
typedef union { char __size[__SIZEOF_PTHREAD_ATTR_T]; long int __align; } pthread_attr_t;  
typedef union { char __size[__SIZEOF_PTHREAD_MUTEX_T]; long int __align; } pthread_mutex_t;  
typedef union { char __size[__SIZEOF_PTHREAD_MUTEXATTR_T]; long int __align; } pthread_mutexattr_t;  
typedef union { char __size[__SIZEOF_PTHREAD_COND_T]; long int __align; } pthread_cond_t;  
typedef union { char __size[__SIZEOF_PTHREAD_CONDATTR_T]; long int __align; } pthread_condattr_t;  
typedef union { char __size[__SIZEOF_PTHREAD_RWLOCK_T]; long int __align; } pthread_rwlock_t;  
typedef union { char __size[__SIZEOF_PTHREAD_RWLOCKATTR_T]; long int __align; } pthread_rwlockattr_t;  
typedef union { char __size[__SIZEOF_PTHREAD_BARRIER_T]; long int __align; } pthread_barrier_t;  
typedef union { char __size[__SIZEOF_PTHREAD_BARRIERATTR_T]; long int __align; } pthread_barrierattr_t;  
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);  
int pthread_join(pthread_t thread, void **retval);  
int pthread_detach(pthread_t thread);  
void pthread_exit(void *retval);  
pthread_t pthread_self(void);  
int pthread_equal(pthread_t t1, pthread_t t2);  
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);  
int pthread_mutex_lock(pthread_mutex_t *mutex);  
int pthread_mutex_unlock(pthread_mutex_t *mutex);  
int pthread_mutex_destroy(pthread_mutex_t *mutex);  
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);  
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);  
int pthread_cond_signal(pthread_cond_t *cond);  
int pthread_cond_broadcast(pthread_cond_t *cond);  
int pthread_cond_destroy(pthread_cond_t *cond);  

//stddef.h
typedef __SIZE_TYPE__ size_t;  
typedef __PTRDIFF_TYPE__ ptrdiff_t;  
typedef __WCHAR_TYPE__ wchar_t;  
#define NULL ((void *)0)  
#define offsetof(type, member) __builtin_offsetof(type, member)  

//#include <errno.h>
extern int errno;  
#define EPERM 1  
#define ENOENT 2  
#define ESRCH 3  
#define EINTR 4  
#define EIO 5  
#define ENXIO 6  
#define E2BIG 7  
#define ENOEXEC 8  
#define EBADF 9  
#define ECHILD 10  
#define EAGAIN 11  
#define ENOMEM 12  
#define EACCES 13  
#define EFAULT 14  
#define ENOTBLK 15  
#define EBUSY 16  
#define EEXIST 17  
#define EXDEV 18  
#define ENODEV 19  
#define ENOTDIR 20  
#define EISDIR 21  
#define EINVAL 22  
#define ENFILE 23  
#define EMFILE 24  
#define ENOTTY 25  
#define ETXTBSY 26  
#define EFBIG 27  
#define ENOSPC 28  
#define ESPIPE 29  
#define EROFS 30  
#define EMLINK 31  
#define EPIPE 32  
#define EDOM 33  
#define ERANGE 34  

#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define ENOTBLK 15
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ETXTBSY 26
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDOM 33
#define ERANGE 34
#define EDEADLK 35
#define ENAMETOOLONG 36
#define ENOLCK 37
#define ENOSYS 38
#define ENOTEMPTY 39
#define ELOOP 40
#define EWOULDBLOCK EAGAIN
#define ENOMSG 42
#define EIDRM 43
#define ECHRNG 44
#define EL2NSYNC 45
#define EL3HLT 46
#define EL3RST 47
#define ELNRNG 48
#define EUNATCH 49
#define ENOCSI 50
#define EL2HLT 51
#define EBADE 52
#define EBADR 53
#define EXFULL 54
#define ENOANO 55
#define EBADRQC 56
#define EBADSLT 57
#define EDEADLOCK EDEADLK
#define EBFONT 59
#define ENOSTR 60
#define ENODATA 61
#define ETIME 62
#define ENOSR 63
#define ENONET 64
#define ENOPKG 65
#define EREMOTE 66
#define ENOLINK 67
#define EADV 68
#define ESRMNT 69
#define ECOMM 70
#define EPROTO 71
#define EMULTIHOP 72
#define EDOTDOT 73
#define EBADMSG 74
#define EOVERFLOW 75
#define ENOTUNIQ 76
#define EBADFD 77
#define EREMCHG 78
#define ELIBACC 79
#define ELIBBAD 80
#define ELIBSCN 81
#define ELIBMAX 82
#define ELIBEXEC 83
#define EILSEQ 84
#define ERESTART 85
#define ESTRPIPE 86
#define EUSERS 87
#define ENOTSOCK 88
#define EDESTADDRREQ 89
#define EMSGSIZE 90
#define EPROTOTYPE 91
#define ENOPROTOOPT 92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP 95
#define EPFNOSUPPORT 96
#define EAFNOSUPPORT 97
#define EADDRINUSE 98
#define EADDRNOTAVAIL 99
#define ENETDOWN 100
#define ENETUNREACH 101
#define ENETRESET 102
#define ECONNABORTED 103
#define ECONNRESET 104
#define ENOBUFS 105
#define EISCONN 106
#define ENOTCONN 107
#define ESHUTDOWN 108
#define ETOOMANYREFS 109
#define ETIMEDOUT 110
#define ECONNREFUSED 111
#define EHOSTDOWN 112
#define EHOSTUNREACH 113
#define EALREADY 114
#define EINPROGRESS 115
#define ESTALE 116
#define EUCLEAN 117
#define ENOTNAM 118
#define ENAVAIL 119
#define EISNAM 120
#define EREMOTEIO 121
#define EDQUOT 122
#define ENOMEDIUM 123
#define EMEDIUMTYPE 124
#define ECANCELED 125
#define ENOKEY 126
#define EKEYEXPIRED 127
#define EKEYREVOKED 128
#define EKEYREJECTED 129
#define EOWNERDEAD 130
#define ENOTRECOVERABLE 131
#define ERFKILL 132
#define EHWPOISON 133
#define ENOTSUP EOPNOTSUPP

/* stdio.h */
#define EOF (-1)
#define BUFSIZ 8192
#define FILENAME_MAX 4096
#define FOPEN_MAX 16
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
typedef struct _IO_FILE FILE;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);
int fgetc(FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int getc(FILE *stream);
int getchar(void);
char *gets(char *s);
int putc(int c, FILE *stream);
int putchar(int c);
int puts(const char *s);
int ungetc(int c, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);
int fflush(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void clearerr(FILE *stream);
int remove(const char *filename);
int rename(const char *old, const char *new);
FILE *fopen(const char *path, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE *stream);
int fclose(FILE *fp);
int fileno(FILE *stream);

/* stdlib.h */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 2147483647
typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void abort(void);
void exit(int status);
int atexit(void (*function)(void));
int system(const char *command);
char *getenv(const char *name);
int rand(void);
void srand(unsigned int seed);
int abs(int j);
long labs(long j);
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
long int strtol(const char *nptr, char **endptr, int base);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
double strtod(const char *nptr, char **endptr);

/* string.h */
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
size_t strlen(const char *s);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *str, const char *delim);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strerror(int errnum);

/* sys/socket.h */
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define AF_INET 2
#define AF_INET6 10
#define PF_INET AF_INET
#define PF_INET6 AF_INET6
#define INADDR_ANY ((in_addr_t) 0x00000000)
struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};
struct sockaddr_in {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
};
struct in_addr {
    in_addr_t s_addr;
};
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

/* arpa/inet.h */
in_addr_t inet_addr(const char *cp);
char *inet_ntoa(struct in_addr in);
int inet_aton(const char *cp, struct in_addr *inp);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int inet_pton(int af, const char *src, void *dst);
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

/* unistd.h */
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int unlink(const char *pathname);
pid_t fork(void);
pid_t getpid(void);
pid_t getppid(void);
unsigned int sleep(unsigned int seconds);
int pipe(int pipefd[2]);
char *getcwd(char *buf, size_t size);
int chdir(const char *path);

/* stdarg.h */
typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l) __builtin_va_arg(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_copy(d,s) __builtin_va_copy(d,s)

/* sys/time.h */
struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv, const struct timezone *tz);

/* sys/select.h */

#define FD_SETSIZE 1024
typedef unsigned long fd_mask;
typedef struct {
    unsigned long fds_bits[FD_SETSIZE / (8 * sizeof(long))];
} fd_set;
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
void FD_CLR(int fd, fd_set *set);
int FD_ISSET(int fd, fd_set *set);
void FD_SET(int fd, fd_set *set);
void FD_ZERO(fd_set *set);

/* sys/types.h */
typedef int pid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef long off_t;
typedef unsigned long size_t;
typedef long ssize_t;
typedef int mode_t;
typedef unsigned long ino_t;
typedef unsigned long dev_t;
typedef long time_t;
typedef unsigned int socklen_t;

/* time.h */

#define CLOCKS_PER_SEC 1000000
#define TIME_UTC 1
typedef long clock_t;
typedef long time_t;
struct tm { int tm_sec; int tm_min; int tm_hour; int tm_mday; int tm_mon; int tm_year; int tm_wday; int tm_yday; int tm_isdst; };
struct timespec { time_t tv_sec; long tv_nsec; };
clock_t clock(void);
time_t time(time_t *timer);
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm *timeptr);
struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer);
char *asctime(const struct tm *timeptr);
char *ctime(const time_t *timer);
size_t strftime(char * restrict s, size_t maxsize, const char * restrict format, const struct tm * restrict timeptr);
int timespec_get(struct timespec *ts, int base);

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

typedef unsigned short sa_family_t;
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
struct in_addr { in_addr_t s_addr; };

/* Standard types and constants */
typedef unsigned int size_t;
typedef int ptrdiff_t;
typedef unsigned char bool;
#define true 1
#define false 0
#define NULL ((void*)0)

/* File I/O */
int fclose(FILE *stream);
int feof(FILE *stream);
int fflush(FILE *stream);
int fgetc(FILE *stream);
int fgetpos(FILE *stream, fpos_t *pos);
char *fgets(char *s, int n, FILE *stream);
FILE *fopen(const char *filename, const char *mode);
int fprintf(FILE *stream, const char *format, ...);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long int offset, int whence);
int fsetpos(FILE *stream, const fpos_t *pos);
long int ftell(FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/* Memory management */
void *aligned_alloc(size_t alignment, size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

/* String manipulation */
void *memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
size_t strlen(const char *s);
char *strncat(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

/* Mathematics */
double ceil(double x);
double fabs(double x);
double floor(double x);
double fmod(double x, double y);
double pow(double x, double y);
double sqrt(double x);

/* Utility */
void abort(void);
int atexit(void (*func)(void));
void exit(int status);
char *getenv(const char *name);
int system(const char *command);

/* Signal handling */
void (*signal(int sig, void (*func)(int)))(int);
int raise(int sig);

/* Time */
time_t time(time_t *timer);
double difftime(time_t time1, time_t time0);
struct tm *localtime(const time_t *timer);
size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr);

/* System */
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
pid_t getpid(void);
unsigned sleep(unsigned seconds);



typedef long fpos_t;
struct __sbuf { unsigned char *_base; int _size; };
typedef struct __sFILE { unsigned char *_p; int _r; int _w; short _flags; short _file; struct __sbuf _bf; int _lbfsize; void *_cookie; int (*_close)(void *); int (*_read)(void *, char *, int); fpos_t (*_seek)(void *, fpos_t, int); int (*_write)(void *, const char *, int); struct __sbuf _ub; unsigned char *_up; int _ur; unsigned char _ubuf[3]; unsigned char _nbuf[1]; struct __sbuf _lb; int _blksize; fpos_t _offset; } FILE;


typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;
typedef int register_t;
typedef int __cpu_mask;
typedef __cpu_mask cpu_set_t;
typedef unsigned long __fd_mask;
typedef struct { __fd_mask __fds_bits[1024 / (8 * sizeof(__fd_mask))]; } fd_set;
typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __time_t;
typedef long int __blksize_t;
typedef long int __blkcnt_t;
typedef __off_t off_t;
typedef __ssize_t ssize_t;
typedef __time_t time_t;
typedef __uid_t uid_t;
typedef __gid_t gid_t;
typedef __pid_t pid_t;
typedef __key_t key_t;
typedef __clockid_t clockid_t;
typedef __timer_t timer_t;
typedef __useconds_t useconds_t;
typedef __suseconds_t suseconds_t;
typedef unsigned long size_t;
typedef long int ssize_t;
typedef long int __syscall_slong_t;
typedef unsigned long int __syscall_ulong_t;
typedef int __sig_atomic_t;
typedef __intptr_t intptr_t;
typedef __uintptr_t uintptr_t;
typedef __blkcnt_t blkcnt_t;
typedef __fsblkcnt_t fsblkcnt_t;
typedef __fsfilcnt_t fsfilcnt_t;
typedef __id_t id_t;
typedef __ino_t ino_t;
typedef __mode_t mode_t;
typedef __nlink_t nlink_t;
typedef __clock_t clock_t;
typedef __blksize_t blksize_t;
typedef int64_t quad_t;
typedef uint64_t u_quad_t;
typedef int64_t int64_t;
typedef uint64_t uint64_t;
typedef int32_t int32_t;
typedef uint32_t uint32_t;
typedef int16_t int16_t;
typedef uint16_t uint16_t;
typedef int8_t int8_t;
typedef uint8_t uint8_t;
typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef int intptr_t;
typedef unsigned int uintptr_t;
typedef long int intmax_t;
typedef unsigned long int uintmax_t;
typedef long int int_fast8_t;
typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned long int uint_fast8_t;
typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
typedef long int int_least8_t;
typedef long int int_least16_t;
typedef long int int_least32_t;
typedef long int int_least64_t;
typedef unsigned long int uint_least8_t;
typedef unsigned long int uint_least16_t;
typedef unsigned long int uint_least32_t;
typedef unsigned long int uint_least64_t;
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long int int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;
typedef int intptr_t;
typedef unsigned int uintptr_t;
typedef __loff_t loff_t;
typedef __ino64_t ino64_t;
typedef __off64_t off64_t;
typedef int __socklen_t;
typedef unsigned int __socklen_t;
typedef unsigned short sa_family_t;
typedef unsigned short in_port_t;
typedef unsigned long int socklen_t;
typedef unsigned int speed_t;
typedef unsigned long tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int __rlim_t;
typedef unsigned int __rlim64_t;
typedef unsigned int rlim_t;
typedef unsigned int rlim64_t;
typedef unsigned long int __priority_which_t;
typedef int __which_t;
typedef unsigned long int __who_t;
typedef unsigned long int __which_t;
typedef unsigned long int __who_t;
typedef unsigned long int __priority_t;
typedef unsigned long int __nice_t;
typedef unsigned long int __id_t;
typedef unsigned long int __key_t;
typedef unsigned long int __clock_t;
typedef unsigned long int __timer_t;
typedef unsigned long int __useconds_t;
typedef unsigned long int __suseconds_t;
typedef unsigned long int __blkcnt64_t;
typedef unsigned long int __fsblkcnt64_t;
typedef unsigned long int __fsfilcnt64_t;
typedef unsigned long int __ino64_t;
typedef unsigned long int __off64_t;
typedef unsigned long int __loff_t;
typedef unsigned long int __qaddr_t;
typedef unsigned long int __swblk_t;
typedef unsigned long int __swblk_t;
typedef unsigned long int __daddr_t;
typedef unsigned long int __caddr_t;
typedef unsigned long int __time64_t;
typedef unsigned long int __socklen_t;
typedef unsigned long int __fd_mask;
typedef unsigned long int __ipc_pid_t;
typedef unsigned long int __uid32_t;
typedef unsigned long int __gid32_t;
typedef unsigned long int __ino64_t;
typedef unsigned long int __mode_t;
typedef unsigned long int __nlink_t;
typedef unsigned long int __off64_t;
typedef unsigned long int __pid_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned long int __blkcnt_t;
typedef unsigned long int __blkcnt64_t;
typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;
typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;
typedef unsigned long int __id_t;
typedef unsigned long int __clockid_t;
typedef unsigned long int __timer_t;
typedef unsigned long int __useconds_t;
typedef unsigned long int __suseconds_t;
typedef unsigned long int __socklen_t;
typedef unsigned long int __sigset_t;
typedef unsigned long int __fd_mask;
typedef unsigned long int __ipc_pid_t;
typedef unsigned long int __key_t;
typedef unsigned long int __size_t;
typedef unsigned long int __ssize_t;
typedef unsigned long int __ptrdiff_t;
typedef unsigned long int __wchar_t;
typedef unsigned long int __wint_t;
typedef unsigned long int __dev_t;
typedef unsigned long int __uid_t;
typedef unsigned long int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __mode_t;
typedef unsigned long int __nlink_t;
typedef unsigned long int __off_t;
typedef unsigned long int __pid_t;
typedef unsigned long int __ssize_t;
typedef unsigned long int __time_t;
typedef unsigned long int __blksize_t;
typedef unsigned long int __blkcnt_t;
typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __id_t;
typedef unsigned long int __clock_t;
typedef unsigned long int __clockid_t;
typedef unsigned long int __timer_t;
typedef unsigned long int __useconds_t;
typedef unsigned long int __suseconds_t;
typedef unsigned long int __sigset_t;
typedef unsigned long int __fd_mask;
typedef unsigned long int __ipc_pid_t;
typedef unsigned long int __key_t;

#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_RAW 3
#define SOCK_RDM 4
#define SOCK_SEQPACKET 5
#define SOCK_DCCP 6
#define SOCK_PACKET 10
#define SOCK_CLOEXEC 02000000
#define SOCK_NONBLOCK 04000
#define SOL_SOCKET 1
#define SO_DEBUG 1
#define SO_REUSEADDR 2
#define SO_TYPE 3
#define SO_ERROR 4
#define SO_DONTROUTE 5
#define SO_BROADCAST 6
#define SO_SNDBUF 7
#define SO_RCVBUF 8
#define SO_KEEPALIVE 9
#define SO_OOBINLINE 10
#define SO_NO_CHECK 11
#define SO_PRIORITY 12
#define SO_LINGER 13
#define SO_BSDCOMPAT 14
#define SO_REUSEPORT 15
#define SO_PASSCRED 16
#define SO_PEERCRED 17
#define SO_RCVLOWAT 18
#define SO_SNDLOWAT 19
#define SO_RCVTIMEO 20
#define SO_SNDTIMEO 21
#define SO_SECURITY_AUTHENTICATION 22
#define SO_SECURITY_ENCRYPTION_TRANSPORT 23
#define SO_SECURITY_ENCRYPTION_NETWORK 24
#define SO_BINDTODEVICE 25
#define SO_ATTACH_FILTER 26
#define SO_DETACH_FILTER 27
#define SO_PEERNAME 28
#define SO_TIMESTAMP 29
#define SCM_TIMESTAMP SO_TIMESTAMP
#define SO_ACCEPTCONN 30
#define SO_PEERSEC 31
#define SO_PASSSEC 34
#define SO_TIMESTAMPNS 35
#define SCM_TIMESTAMPNS SO_TIMESTAMPNS
#define SO_MARK 36
#define SO_TIMESTAMPING 37
#define SCM_TIMESTAMPING SO_TIMESTAMPING
#define SO_PROTOCOL 38
#define SO_DOMAIN 39
#define SO_RXQ_OVFL 40
#define SO_WIFI_STATUS 41
#define SCM_WIFI_STATUS SO_WIFI_STATUS
#define SO_PEEK_OFF 42
#define SO_NOFCS 43
#define SO_LOCK_FILTER 44
#define SO_SELECT_ERR_QUEUE 45
#define SO_BUSY_POLL 46
#define SO_MAX_PACING_RATE 47
#define SO_BPF_EXTENSIONS 48
#define SO_INCOMING_CPU 49
#define SO_ATTACH_BPF 50
#define SO_DETACH_BPF SO_DETACH_FILTER
#define SO_ATTACH_REUSEPORT_CBPF 51
#define SO_ATTACH_REUSEPORT_EBPF 52
#define SO_CNX_ADVICE 53
#define SCM_TIMESTAMPING_OPT_STATS 54
#define SO_MEMINFO 55
#define SO_INCOMING_NAPI_ID 56
#define SO_COOKIE 57
#define SCM_TIMESTAMPING_PKTINFO 58
#define SO_PEERGROUPS 59
#define SO_ZEROCOPY 60
#define SO_TXTIME 61
#define SCM_TXTIME SO_TXTIME
#define SO_BINDTOIFINDEX 62
#define SO_DETACH_REUSEPORT_BPF 68
#define AF_UNSPEC 0
#define AF_UNIX 1
#define AF_LOCAL 1
#define AF_INET 2
#define AF_AX25 3
#define AF_IPX 4
#define AF_APPLETALK 5
#define AF_NETROM 6
#define AF_BRIDGE 7
#define AF_ATMPVC 8
#define AF_X25 9
#define AF_INET6 10
#define AF_ROSE 11
#define AF_DECnet 12
#define AF_NETBEUI 13
#define AF_SECURITY 14
#define AF_KEY 15
#define AF_NETLINK 16
#define AF_ROUTE 16
#define AF_PACKET 17
#define AF_ASH 18
#define AF_ECONET 19
#define AF_ATMSVC 20
#define AF_RDS 21
#define AF_SNA 22
#define AF_IRDA 23
#define AF_PPPOX 24
#define AF_WANPIPE 25
#define AF_LLC 26
#define AF_IB 27
#define AF_MPLS 28
#define AF_CAN 29
#define AF_TIPC 30
#define AF_BLUETOOTH 31
#define AF_IUCV 32
#define AF_RXRPC 33
#define AF_ISDN 34
#define AF_PHONET 35
#define AF_IEEE802154 36
#define AF_CAIF 37
#define AF_ALG 38
#define AF_NFC 39
#define AF_VSOCK 40
#define AF_KCM 41
#define AF_QIPCRTR 42
#define AF_SMC 43
#define AF_XDP 44
#define PF_UNSPEC AF_UNSPEC
#define PF_UNIX AF_UNIX
#define PF_LOCAL AF_LOCAL
#define PF_INET AF_INET
#define PF_AX25 AF_AX25
#define PF_IPX AF_IPX
#define PF_APPLETALK AF_APPLETALK
#define PF_NETROM AF_NETROM
#define PF_BRIDGE AF_BRIDGE
#define PF_ATMPVC AF_ATMPVC
#define PF_X25 AF_X25
#define PF_INET6 AF_INET6
#define PF_ROSE AF_ROSE
#define PF_DECnet AF_DECnet
#define PF_NETBEUI AF_NETBEUI
#define PF_SECURITY AF_SECURITY
#define PF_KEY AF_KEY
#define PF_NETLINK AF_NETLINK
#define PF_ROUTE AF_ROUTE
#define PF_PACKET AF_PACKET
#define PF_ASH AF_ASH
#define PF_ECONET AF_ECONET
#define PF_ATMSVC AF_ATMSVC
#define PF_RDS AF_RDS
#define PF_SNA AF_SNA
#define PF_IRDA AF_IRDA
#define PF_PPPOX AF_PPPOX
#define PF_WANPIPE AF_WANPIPE
#define PF_LLC AF_LLC
#define PF_IB AF_IB
#define PF_MPLS AF_MPLS
#define PF_CAN AF_CAN
#define PF_TIPC AF_TIPC
#define PF_BLUETOOTH AF_BLUETOOTH
#define PF_IUCV AF_IUCV
#define PF_RXRPC AF_RXRPC
#define PF_ISDN AF_ISDN
#define PF_PHONET AF_PHONET
#define PF_IEEE802154 AF_IEEE802154
#define PF_CAIF AF_CAIF
#define PF_ALG AF_ALG
#define PF_NFC AF_NFC
#define PF_VSOCK AF_VSOCK
#define PF_KCM AF_KCM
#define PF_QIPCRTR AF_QIPCRTR
#define PF_SMC AF_SMC
#define PF_XDP AF_XDP
#define MSG_OOB 0x01
#define MSG_PEEK 0x02
#define MSG_DONTROUTE 0x04
#define MSG_CTRUNC 0x08
#define MSG_PROXY 0x10
#define MSG_TRUNC 0x20
#define MSG_DONTWAIT 0x40
#define MSG_EOR 0x80
#define MSG_WAITALL 0x100
#define MSG_FIN 0x200
#define MSG_SYN 0x400
#define MSG_CONFIRM 0x800
#define MSG_RST 0x1000
#define MSG_ERRQUEUE 0x2000
#define MSG_NOSIGNAL 0x4000
#define MSG_MORE 0x8000
#define MSG_WAITFORONE 0x10000
#define MSG_BATCH 0x40000
#define MSG_ZEROCOPY 0x4000000
#define MSG_FASTOPEN 0x20000000
#define MSG_CMSG_CLOEXEC 0x40000000
struct sockaddr { sa_family_t sa_family; char sa_data[14]; };
struct sockaddr_storage { sa_family_t ss_family; char __ss_padding[128 - sizeof(sa_family_t)]; __ss_aligntype __ss_align; };
struct linger { int l_onoff; int l_linger; };
struct msghdr { void *msg_name; socklen_t msg_namelen; struct iovec *msg_iov; size_t msg_iovlen; void *msg_control; size_t msg_controllen; int msg_flags; };
struct cmsghdr { size_t cmsg_len; int cmsg_level; int cmsg_type; };
#define CMSG_DATA(cmsg) ((unsigned char *)((struct cmsghdr *)(cmsg) + 1))
#define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr(mhdr, cmsg)
#define CMSG_FIRSTHDR(mhdr) ((size_t)(mhdr)->msg_controllen >= sizeof(struct cmsghdr) ? (struct cmsghdr *)(mhdr)->msg_control : NULL)
#define CMSG_ALIGN(len) (((len) + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#define SCM_RIGHTS 1
#define SCM_CREDENTIALS 2
#define SCM_SECURITY 3
int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int sv[2]);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int shutdown(int sockfd, int how);

#endif
