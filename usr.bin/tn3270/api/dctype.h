/*-
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)dctype.h	8.1 (Berkeley) 06/06/93
 */

#define	INCLUDED_ECTYPE

#define	D_UPPER	0x01
#define	D_LOWER	0x02
#define	D_DIGIT	0x04
#define	D_SPACE	0x08
#define	D_PUNCT	0x10
#define	D_PRINT 0x20

#define	Disalpha(c)	(dctype[(c)]&(D_UPPER|D_LOWER))
#define	Disupper(c)	(dctype[(c)]&D_UPPER)
#define	Dislower(c)	(dctype[(c)]&D_LOWER)
#define	Disdigit(c)	(dctype[(c)]&D_DIGIT)
#define	Disalnum(c)	(dctype[(c)]&(D_UPPER|D_LOWER|D_DIGIT))
#define	Disspace(c)	(dctype[(c)]&D_SPACE)	/* blank or null */
#define	Dispunct(c)	(dctype[(c)]&D_PUNCT)
#define	Disprint(c)	(dctype[(c)]&D_PRINT)

extern unsigned char dctype[192];
