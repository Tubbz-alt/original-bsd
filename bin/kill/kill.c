/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)kill.c	4.7 (Berkeley) 02/25/91";
#endif /* not lint */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *signals[] = {
	"hup", "int", "quit", "ill", "trap", "iot",		/*  1 - 6  */
	"emt", "fpe", "kill", "bus", "segv", "sys",		/*  7 - 12 */
	"pipe", "alrm",  "term", "urg", "stop", "tstp",		/* 13 - 18 */
	"cont", "chld", "ttin", "ttou", "io", "xcpu",		/* 19 - 24 */
	"xfsz", "vtalrm", "prof", "winch", "29", "usr1",	/* 25 - 30 */
	"usr2", NULL,						/* 31 - 32 */
	};

main(argc, argv)
	int argc;
	char **argv;
{
	register int numsig;
	register char **p;
	int errors;

	if (argc < 2)
		usage();

	if (!strcmp(*++argv, "-l")) {
		printsig();
		exit(0);
	}

	numsig = SIGTERM;
	if (**argv == '-') {
		++*argv;
		if (isalpha(**argv)) {
			if (!strncasecmp(*argv, "sig", 3))
				*argv += 3;
			for (p = signals;; ++p) {
				if (!*p)
					goto error;
				if (!strcasecmp(*p, *argv)) {
					numsig = p - signals + 1;
					break;
				}
			}
		}
		else if (isdigit(**argv)) {
			numsig = atoi(*argv);
			if (numsig <= 0 || numsig > NSIG)
				goto error;
		}
		else {
error:			printf("kill: unknown signal %s; valid signals:\n", *argv);
			printsig();
			exit(1);
		}
		++argv;
	}

	if (!*argv)
		usage();

	for (errors = 0; *argv; ++argv) {
		if (!isdigit(**argv))
			usage();
		if (kill(atoi(*argv), numsig) == -1) {
			perror(*argv);
			errors = 1;
		}
	}
	exit(errors);
}

printsig()
{
	register char **p;

	for (p = signals; *p; ++p) {
		printf("%s ", *p);
		if ((p - signals) == NSIG / 2 - 1)
			printf("\n");
	}
	printf("\n");
}

usage()
{
	printf("usage: kill [-l] [-sig] pid ...\n");
	exit(2);
}
