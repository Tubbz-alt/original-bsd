/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: hilreg.h 1.10 92/01/21$
 *
 *	@(#)hilreg.h	7.5 (Berkeley) 10/11/92
 */

#include <hp/dev/iotypes.h>		/* XXX */

#ifdef hp300
struct	hil_dev {
	char	hil_pad0;
	vu_char	hil_data;
	char	hil_pad1;
	vu_char	hil_cmd;
};

#define	HILADDR			((struct hil_dev *)IIOV(0x428000))
#define	BBCADDR			((struct hil_dev *)IIOV(0x420000))
#endif

#ifdef hp800
#ifdef hp700
struct hil_dev {
	vu_char	hil_rsthold;	/* (WO) reset hold (and Serial #3) */
	vu_char	hil_resv1[2047];
	vu_char	hil_data;	/* send/receive data to/from 8042 */
	vu_char	hil_cmd;	/* status/control to/from 8042 */
	vu_char	hil_resv2[1022];
	vu_char	hil_rstrel;	/* (WO) reset release (and Serial #3) */

};
#else
struct	hil_dev {
	vu_int hil_data;
	vu_int hil_pad;
	vu_int hil_cmd;
};
#endif
#endif
#define hil_stat hil_cmd

#if defined(hp300) || defined(hp700)
#define READHILDATA(x)		((x)->hil_data)
#define READHILSTAT(x)		((x)->hil_stat)
#define READHILCMD(x)		((x)->hil_cmd)
#define WRITEHILDATA(x, y)	((x)->hil_data = (y))
#define WRITEHILSTAT(x, y)	((x)->hil_stat = (y))
#define WRITEHILCMD(x, y)	((x)->hil_cmd  = (y))
#else
#define READHILDATA(x)		((x)->hil_data >> 24)
#define READHILSTAT(x)		((x)->hil_stat >> 24)
#define READHILCMD(x)		((x)->hil_cmd  >> 24)
#define WRITEHILDATA(x, y)	((x)->hil_data = ((y) << 24))
#define WRITEHILSTAT(x, y)	((x)->hil_stat = ((y) << 24))
#define WRITEHILCMD(x, y)	((x)->hil_cmd  = ((y) << 24))
#endif

#ifdef hp300
#define splhil()		spl1()
#else
extern int hilspl;
#define splhil()		splx(hilspl)
#endif

#define	HIL_BUSY		0x02
#define	HIL_DATA_RDY		0x01

#define HILWAIT(hil_dev)	while ((READHILSTAT(hil_dev) & HIL_BUSY))
#define HILDATAWAIT(hil_dev)	while (!(READHILSTAT(hil_dev) & HIL_DATA_RDY))

/* HIL status bits */
#define	HIL_POLLDATA	0x10		/* HIL poll data follows */
#define	HIL_COMMAND	0x08		/* Start of original command */
#define	HIL_ERROR	0x080		/* HIL error */
#define	HIL_RECONFIG	0x080		/* HIL has reconfigured */
#define	HIL_STATMASK	(HIL_DATA | HIL_COMMAND)

#define	HIL_SSHIFT	4		/* Bits to shift status over */
#define	HIL_SMASK	0xF		/* Service request status mask */
#define	HIL_DEVMASK	0x07

/* HIL status types */
#define	HIL_STATUS	0x5		/* HIL status in data register */
#define	HIL_DATA	0x6		/* HIL data in data register */
#define	HIL_CTRLSHIFT	0x8		/* key + CTRL + SHIFT */
#define	HIL_CTRL	0x9		/* key + CTRL */
#define	HIL_SHIFT	0xA		/* key + SHIFT */
#define	HIL_KEY		0xB		/* key only */
#define	HIL_68K		0x4		/* Data from the 68k is ready */

/* HIL commands */
#define	HIL_SETARD	0xA0		/* set auto-repeat delay */
#define	HIL_SETARR	0xA2		/* set auto-repeat rate */
#define	HIL_SETTONE	0xA3		/* set tone generator */
#define	HIL_CNMT	0xB2		/* clear nmi */
#define	HIL_INTON	0x5C		/* Turn on interrupts. */
#define	HIL_INTOFF	0x5D		/* Turn off interrupts. */
#define	HIL_TRIGGER	0xC5		/* trigger command */
#define	HIL_STARTCMD	0xE0		/* start loop command */
#define	HIL_TIMEOUT	0xFE		/* timeout */
#define	HIL_READTIME	0x13		/* Read real time register */

/* Read/write various registers on the 8042. */
#define	HIL_READBUSY		0x02	/* internal "busy" register */
#define	HIL_READKBDLANG		0x12	/* read keyboard language code */
#define	HIL_READKBDSADR	 	0xF9
#define	HIL_WRITEKBDSADR 	0xE9
#define	HIL_READLPSTAT  	0xFA
#define	HIL_WRITELPSTAT 	0xEA
#define	HIL_READLPCTRL  	0xFB
#define	HIL_WRITELPCTRL 	0xEB

/* BUSY bits */
#define	BSY_LOOPBUSY	0x04

/* LPCTRL bits */
#define	LPC_AUTOPOLL	0x01	/* enable auto-polling */
#define	LPC_NOERROR	0x02	/* don't report errors */
#define	LPC_NORECONF	0x04	/* don't report reconfigure */
#define	LPC_KBDCOOK	0x10	/* cook all keyboards */
#define	LPC_RECONF	0x80	/* reconfigure the loop */

/* LPSTAT bits */
#define	LPS_DEVMASK	0x07	/* number of loop devices */
#define	LPS_CONFGOOD	0x08	/* reconfiguration worked */
#define	LPS_CONFFAIL	0x80	/* reconfiguration failed */

/* HIL packet headers */
#define	HIL_MOUSEDATA   0x2
#define	HIL_KBDDATA     0x40
  
#define	HIL_MOUSEMOTION	0x02	/* mouse movement event */
#define	HIL_KBDBUTTON	0x40	/* keyboard button event */
#define	HIL_MOUSEBUTTON 0x40	/* mouse button event */
#define	HIL_BUTTONBOX   0x60	/* button box event */
#define	HIL_TABLET      0x02	/* tablet motion event */
#define	HIL_KNOBBOX     0x03	/* knob box motion data */

/* For setting auto repeat on the keyboard */
#define	ar_format(x)	~((x - 10) / 10)
#define	KBD_ARD		400	/* initial delay in msec (10 - 2560) */
#define	KBD_ARR		60	/* rate (10 - 2550 msec, 2551 == off) */

#ifdef hp300
/* Magic */
#define	KBDNMISTAT	((volatile char *)IIOV(0x478005))
#define	KBDNMI		0x04
#endif
