#ifndef OCONFIGURE_CONFIG_H
#define OCONFIGURE_CONFIG_H

/*
 * Hand-written configuration for building the lowdown library with
 * Microsoft Visual C.  The upstream configure script only knows about
 * POSIX systems, so this file replaces the generated config.h.
 */

#ifdef _MSC_VER

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <wchar.h>

/* POSIX type not provided by the Microsoft CRT. */
typedef intptr_t ssize_t;

/* GCC declaration attributes are meaningless to MSVC. */
#define __attribute__(...)

/* Byte order (only used by the bundled MD5 implementation). */
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#define BYTE_ORDER LITTLE_ENDIAN

/*
 * Availability of system interfaces.  Interfaces that lowdown does not
 * use are claimed as present so that compats.c does not try to compile
 * the POSIX-only implementations.
 */
#define HAVE_ARC4RANDOM 1
#define HAVE_B64_NTOP 1
#define HAVE_BLOWFISH 1
#define HAVE_CAPSICUM 0
#define HAVE_CRYPT 1
#define HAVE_CRYPT_NEWHASH 1
#define HAVE_ENDIAN_H 0
#define HAVE_ERR 1
#define HAVE_EXPLICIT_BZERO 1
#define HAVE_FTS 1
#define HAVE_GETEXECNAME 1
#define HAVE_GETPROGNAME 1
#define HAVE_INFTIM 1
#define HAVE_LANDLOCK 0
#define HAVE_MD5 0
#define HAVE_MEMMEM 0
#define HAVE_MEMRCHR 0
#define HAVE_MEMSET_S 1
#define HAVE_MKFIFOAT 1
#define HAVE_MKNODAT 1
#define HAVE_OSBYTEORDER_H 0
#define HAVE_PASSWORD_LEN 1
#define HAVE_PATH_MAX 1
#define HAVE_PLEDGE 0
#define HAVE_PROGRAM_INVOCATION_SHORT_NAME 0
#define HAVE_READPASSPHRASE 1
#define HAVE_REALLOCARRAY 0
#define HAVE_RECALLOCARRAY 1
#define HAVE_SANDBOX_INIT 0
#define HAVE_SCAN_SCALED 1
#define HAVE_SECCOMP_FILTER 0
#define HAVE_SETRESGID 1
#define HAVE_SETRESUID 1
#define HAVE_SHA2 1
#define HAVE_SHA2_H 1
#define HAVE_SOCK_NONBLOCK 0
#define HAVE_STRLCAT 0
#define HAVE_STRLCPY 0
#define HAVE_STRNDUP 0
#define HAVE_STRNLEN 1
#define HAVE_STRTONUM 0
#define HAVE_SYS_BYTEORDER_H 0
#define HAVE_SYS_CDEFS_H 0
#define HAVE_SYS_ENDIAN_H 0
#define HAVE_SYS_MKDEV_H 0
#define HAVE_SYS_QUEUE 1
#define HAVE_SYS_SYSMACROS_H 0
#define HAVE_SYS_TREE 0
#define HAVE_SYSTRACE 0
#define HAVE_TERMIOS 0
#define HAVE_TIMINGSAFE_BCMP 1
#define HAVE_UNVEIL 0
#define HAVE_WAIT_ANY 0
#define HAVE___PROGNAME 0

/* MD5, used by the diff interface when the system has no md5.h. */
#define MD5_BLOCK_LENGTH 64
#define MD5_DIGEST_LENGTH 16
#define MD5_DIGEST_STRING_LENGTH (MD5_DIGEST_LENGTH * 2 + 1)
typedef struct MD5Context {
	uint32_t state[4];
	uint64_t count;
	uint8_t buffer[MD5_BLOCK_LENGTH];
} MD5_CTX;
void MD5Init(MD5_CTX *);
void MD5Update(MD5_CTX *, const uint8_t *, size_t);
void MD5Pad(MD5_CTX *);
void MD5Transform(uint32_t [4], const uint8_t [MD5_BLOCK_LENGTH]);
char *MD5End(MD5_CTX *, char *);
void MD5Final(uint8_t [MD5_DIGEST_LENGTH], MD5_CTX *);

/* Functions supplied by compats.c on systems without BSD libc. */
void *memmem(const void *, size_t, const void *, size_t);
void *memrchr(const void *, int, size_t);
void *reallocarray(void *, size_t, size_t);
size_t strlcat(char *, const char *, size_t);
size_t strlcpy(char *, const char *, size_t);
char *strndup(const char *, size_t);
long long strtonum(const char *, long long, long long, const char **);

/* Functions supplied by compat/msvc.c. */
int asprintf(char **, const char *, ...);
int vasprintf(char **, const char *, va_list);
int strcasecmp(const char *, const char *);
int strncasecmp(const char *, const char *, size_t);
size_t mbsnrtowcs(wchar_t *, const char **, size_t, size_t, mbstate_t *);
int wcswidth(const wchar_t *, size_t);
void *recallocarray(void *, size_t, size_t, size_t);

#endif /* _MSC_VER */

#endif /* !OCONFIGURE_CONFIG_H */
