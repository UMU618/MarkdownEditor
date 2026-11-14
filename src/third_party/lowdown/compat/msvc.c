/*
 * POSIX/BSD functions missing from the Microsoft CRT that lowdown uses.
 * These are compiled only into the MSVC build.
 */

#include "config.h"

#ifdef _MSC_VER

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>

int
vasprintf(char **ret, const char *fmt, va_list ap)
{
	va_list	 ap2;
	int	 len;
	char	*buf;

	*ret = NULL;
	va_copy(ap2, ap);
	len = _vscprintf(fmt, ap2);
	va_end(ap2);
	if (len < 0)
		return -1;
	if ((buf = malloc((size_t)len + 1)) == NULL)
		return -1;
	vsnprintf(buf, (size_t)len + 1, fmt, ap);
	*ret = buf;
	return len;
}

int
asprintf(char **ret, const char *fmt, ...)
{
	va_list	 ap;
	int	 len;

	va_start(ap, fmt);
	len = vasprintf(ret, fmt, ap);
	va_end(ap);
	return len;
}

void *
recallocarray(void *ptr, size_t oldnmemb, size_t newnmemb, size_t size)
{
	size_t	 oldsize, newsize;
	void	*newptr;

	if (ptr == NULL)
		return calloc(newnmemb, size);

	if (size != 0 && (newnmemb > SIZE_MAX / size ||
	    oldnmemb > SIZE_MAX / size)) {
		errno = ENOMEM;
		return NULL;
	}
	oldsize = oldnmemb * size;
	newsize = newnmemb * size;

	if (newsize <= oldsize && newsize >= oldsize / 2 &&
	    oldsize - newsize < 4096) {
		memset((char *)ptr + newsize, 0, oldsize - newsize);
		return ptr;
	}

	if ((newptr = malloc(newsize)) == NULL)
		return NULL;
	if (newsize > oldsize) {
		memcpy(newptr, ptr, oldsize);
		memset((char *)newptr + oldsize, 0, newsize - oldsize);
	} else
		memcpy(newptr, ptr, newsize);
	free(ptr);
	return newptr;
}

int
strcasecmp(const char *s1, const char *s2)
{
	unsigned char c1, c2;

	do {
		c1 = (unsigned char)*s1++;
		c2 = (unsigned char)*s2++;
		c1 = (unsigned char)tolower(c1);
		c2 = (unsigned char)tolower(c2);
	} while (c1 != 0 && c1 == c2);
	return c1 - c2;
}

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
	unsigned char c1, c2;

	if (n == 0)
		return 0;
	do {
		c1 = (unsigned char)*s1++;
		c2 = (unsigned char)*s2++;
		c1 = (unsigned char)tolower(c1);
		c2 = (unsigned char)tolower(c2);
	} while (--n != 0 && c1 != 0 && c1 == c2);
	return c1 - c2;
}

size_t
mbsnrtowcs(wchar_t *dst, const char **src, size_t nms, size_t len,
    mbstate_t *ps)
{
	const char	*s = *src;
	size_t		 nchr = 0;
	mbstate_t	 st;

	if (ps != NULL)
		st = *ps;
	else
		memset(&st, 0, sizeof(st));

	while (nms > 0 && (dst == NULL || len > 0)) {
		wchar_t	 wc;
		size_t	 r;

		r = mbrtowc(&wc, s, nms, &st);
		if (r == (size_t)-1 || r == (size_t)-2) {
			if (ps != NULL)
				*ps = st;
			*src = s;
			errno = EILSEQ;
			return (size_t)-1;
		}
		if (r == 0) {
			if (dst != NULL) {
				*dst++ = L'\0';
				if (ps != NULL)
					*ps = st;
				*src = NULL;
				return nchr;
			}
			nchr++;
			s++;
			nms--;
			break;
		}
		if (dst != NULL) {
			*dst++ = wc;
			len--;
		}
		nchr++;
		s += r;
		nms -= r;
	}

	if (ps != NULL)
		*ps = st;
	*src = s;
	return nchr;
}

/*
 * Minimal wcwidth(3): widths for ASCII, combining marks and the common
 * East Asian wide ranges.  Returns -1 for non-printable characters.
 */
static int
wcwidth_approx(wchar_t wc)
{
	if (wc == 0)
		return 0;
	if (wc < 0x20 || (wc >= 0x7f && wc < 0xa0))
		return -1;
	if ((wc >= 0x0300 && wc <= 0x036f) ||
	    (wc >= 0x1ab0 && wc <= 0x1aff) ||
	    (wc >= 0x1dc0 && wc <= 0x1dff) ||
	    (wc >= 0x20d0 && wc <= 0x20ff) ||
	    (wc >= 0xfe20 && wc <= 0xfe2f))
		return 0;
	if ((wc >= 0x1100 && wc <= 0x115f) ||
	    wc == 0x2329 || wc == 0x232a ||
	    (wc >= 0x2e80 && wc <= 0xa4cf && wc != 0x303f) ||
	    (wc >= 0xac00 && wc <= 0xd7a3) ||
	    (wc >= 0xf900 && wc <= 0xfaff) ||
	    (wc >= 0xfe10 && wc <= 0xfe19) ||
	    (wc >= 0xfe30 && wc <= 0xfe6f) ||
	    (wc >= 0xff00 && wc <= 0xff60) ||
	    (wc >= 0xffe0 && wc <= 0xffe6) ||
	    (wc >= 0x1f300 && wc <= 0x1f64f) ||
	    (wc >= 0x1f900 && wc <= 0x1f9ff) ||
	    (wc >= 0x20000 && wc <= 0x3fffd))
		return 2;
	return 1;
}

int
wcswidth(const wchar_t *pwcs, size_t n)
{
	int	 width = 0;
	size_t	 i;

	for (i = 0; i < n && pwcs[i] != L'\0'; i++) {
		int w = wcwidth_approx(pwcs[i]);
		if (w < 0)
			return -1;
		width += w;
	}
	return width;
}

#endif /* _MSC_VER */
