/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Fischbein.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)util.c	8.5 (Berkeley) 04/28/95";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls.h"
#include "extern.h"

void
prcopy(src, dest, len)
	char *src, *dest;
	int len;
{
	int ch;

	while (len--) {
		ch = *src++;
		*dest++ = isprint(ch) ? ch : '?';
	}
}

void
usage()
{
	(void)fprintf(stderr,
	    "usage: ls [-1ACFLRTWacdfikloqrstu] [file ...]\n");
	exit(1);
}
