/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)krcmd.c	1.6.1.1 (Berkeley) 10/21/90";
#endif /* not lint */

/*
 *	$Source: /mit/kerberos/ucb/mit/kcmd/RCS/krcmd.c,v $
 *	$Header: /mit/kerberos/ucb/mit/kcmd/RCS/krcmd.c,v 5.1
 *		89/07/25 15:38:44 kfall Exp Locker: kfall $
 * static char *rcsid_kcmd_c =
 * "$Header: /mit/kerberos/ucb/mit/kcmd/RCS/krcmd.c,v 5.1 89/07/25 15:38:44
 *	kfall Exp Locker: kfall $";
 */

#ifdef KERBEROS
#include <sys/types.h>
#include <stdio.h>
#include <kerberosIV/des.h>
#include <kerberosIV/krb.h>

#define	SERVICE_NAME	"rcmd"

/*
 * krcmd: simplified version of Athena's "kcmd"
 *	returns a socket attached to the destination, -1 or krb error on error 
 *	if fd2p is non-NULL, another socket is filled in for it
 */

int
krcmd(ahost, rport, remuser, cmd, fd2p, realm)
	char	**ahost;
	u_short	rport;
	char	*remuser, *cmd;
	int	*fd2p;
	char	*realm;
{
	int		sock = -1, err = 0;
	KTEXT_ST	ticket;
	long		authopts = 0L;

	err = kcmd(
		&sock,
		ahost,
		rport,
		NULL,	/* locuser not used */
		remuser,
		cmd,
		fd2p,
		&ticket,
		SERVICE_NAME,
		realm,
		(CREDENTIALS *)  NULL,		/* credentials not used */
		(bit_64 *) NULL,		/* key schedule not used */
		(MSG_DAT *) NULL,		/* MSG_DAT not used */
		(struct sockaddr_in *) NULL,	/* local addr not used */
		(struct sockaddr_in *) NULL,	/* foreign addr not used */
		authopts
	);

	if (err > KSUCCESS && err < MAX_KRB_ERRORS) {
		fprintf(stderr, "krcmd: %s\n", krb_err_txt[err]);
		return(-1);
	}
	if (err < 0)
		return(-1);
	return(sock);
}

#endif /* KERBEROS */
