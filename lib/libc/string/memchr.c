/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)memchr.c	8.1 (Berkeley) 06/04/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/cdefs.h>
#include <string.h>

void *
memchr(s, c, n)
	const void *s;
	register unsigned char c;
	register size_t n;
{
	if (n != 0) {
		register const unsigned char *p = s;

		do {
			if (*p++ == c)
				return ((void *)(p - 1));
		} while (--n != 0);
	}
	return (NULL);
}
