/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)wd.c	8.1 (Berkeley) 06/11/93
 */

/*  device driver for winchester disk  */

#include <sys/param.h>
#include <sys/dkbad.h>
#include <sys/disklabel.h>

#include <i386/isa/isa.h>
#include <i386/isa/wdreg.h>
#include <stand/saio.h>

#define	NWD		2	/* number of hard disk units supported, max 2 */
#define	RETRIES		5	/* number of retries before giving up */

int noretries, wdquiet;
/*#define WDDEBUG*/

#ifdef	SMALL
extern struct disklabel disklabel;
#else
struct disklabel wdsizes[NWD];
#endif

extern cyloffset ;		/* bootstrap's idea of cylinder for disklabel */

/*
 * Record for the bad block forwarding code.
 * This is initialized to be empty until the bad-sector table
 * is read from the disk.
 */
#define TRKSEC(trk,sec)	((trk << 8) + sec)

struct	dkbad	dkbad[NWD];
static wdcport;

wdopen(io)
	register struct iob *io;
{
        register struct disklabel *dd;

#ifdef WDDEBUG
	printf("wdopen ");
#endif
#ifdef SMALL
        dd = &disklabel;
#else
        dd = &wdsizes[io->i_unit];
	if (io->i_part > 8)
                _stop("Invalid partition number");
	if(io->i_ctlr > 1)
                _stop("Invalid controller number");
#endif
        if (wdinit(io))
                _stop("wd initialization error");
	io->i_boff = dd->d_partitions[io->i_part].p_offset ;
	return(0);
}

wdstrategy(io,func)
	register struct iob *io;
{
	register int iosize;    /* number of sectors to do IO for this loop */
	register daddr_t sector;
	int nblocks, cyloff;
	int unit, partition;
	char *address;
	register struct disklabel *dd;

	unit = io->i_unit;
	partition = io->i_part;
#ifdef WDDEBUG
	printf("wdstrat %d %d ", unit, partition);
#endif
#ifdef	SMALL
	dd = &disklabel;
#else
	dd = &wdsizes[unit];
#endif
        iosize = io->i_cc / dd->d_secsize;
	/*
	 * Convert PGSIZE "blocks" to sectors.
	 * Note: doing the conversions this way limits the partition size
	 * to about 8 million sectors (1-8 Gb).
	 */
	sector = (unsigned long) io->i_bn * DEV_BSIZE / dd->d_secsize;
	nblocks = dd->d_partitions[partition].p_size;
#ifndef SMALL
        if (iosize < 0 || sector + iosize > nblocks || sector < 0) {
#ifdef WDDEBUG
		printf("bn = %d; sectors = %d; partition = %d; fssize = %d\n",
			io->i_bn, iosize, partition, nblocks);
#endif
                printf("wdstrategy - I/O out of filesystem boundaries\n");
		return(-1);
	}
	if (io->i_bn * DEV_BSIZE % dd->d_secsize) {
		printf("wdstrategy - transfer starts in midsector\n");
		return(-1);
	}
        if (io->i_cc % dd->d_secsize) {
		printf("wd: transfer of partial sector\n");
		return(-1);
	}
#endif

	address = io->i_ma;
        while (iosize > 0) {
                if (wdio(func, unit, sector, address))
                        return(-1);
		iosize--;
		sector++;
                address += dd->d_secsize;
        }
        return(io->i_cc);
}

/* 
 * Routine to do a one-sector I/O operation, and wait for it
 * to complete.
 */
wdio(func, unit, blknm, addr)
        short *addr;
{
	struct disklabel *dd;
	register wdc = wdcport;
	struct bt_bad *bt_ptr;
        int    i;
	int retries = 0;
        long    cylin, head, sector;
        u_char opcode, erro;

#ifdef	SMALL
	dd = &disklabel;
#else
	dd = &wdsizes[unit];
#endif
        if (func == F_WRITE)
                opcode = WDCC_WRITE;
        else
                opcode = WDCC_READ;

        /* Calculate data for output.           */
        cylin = blknm / dd->d_secpercyl;
        head = (blknm % dd->d_secpercyl) / dd->d_nsectors;
        sector = blknm % dd->d_nsectors;

	/* 
	 * See if the current block is in the bad block list.
	 */
	if (blknm > BBSIZE/DEV_BSIZE)	/* should be BBSIZE */
	    for (bt_ptr = dkbad[unit].bt_bad; bt_ptr->bt_cyl != -1; bt_ptr++) {
		if (bt_ptr->bt_cyl > cylin)
			/* Sorted list, and we passed our cylinder. quit. */
			break;
		if (bt_ptr->bt_cyl == cylin &&
			bt_ptr->bt_trksec == (head << 8) + sector) {
			/*
			 * Found bad block.  Calculate new block addr.
			 * This starts at the end of the disk (skip the
			 * last track which is used for the bad block list),
			 * and works backwards to the front of the disk.
			 */
#ifdef WDDEBUG
			    printf("--- badblock code -> Old = %d; ",
				blknm);
#endif
			blknm = dd->d_secperunit - dd->d_nsectors
				- (bt_ptr - dkbad[unit].bt_bad) - 1;
			cylin = blknm / dd->d_secpercyl;
			head = (blknm % dd->d_secpercyl) / dd->d_nsectors;
			sector = blknm % dd->d_nsectors;
#ifdef WDDEBUG
			    printf("new = %d\n", blknm);
#endif
			break;
		}
	}

        sector += 1;
retry:
#ifdef WDDEBUG
	printf("sec %d sdh %x cylin %d ", sector,
		WDSD_IBM | (unit<<4) | (head & 0xf), cylin);
#endif
	outb(wdc+wd_precomp, 0xff);
	outb(wdc+wd_seccnt, 1);
	outb(wdc+wd_sector, sector);
	outb(wdc+wd_cyl_lo, cylin);
	outb(wdc+wd_cyl_hi, cylin >> 8);

	/* Set up the SDH register (select drive).     */
	outb(wdc+wd_sdh, WDSD_IBM | (unit<<4) | (head & 0xf));
	while ((inb(wdc+wd_status) & WDCS_READY) == 0) ;

	outb(wdc+wd_command, opcode);
	while (opcode == WDCC_READ && (inb(wdc+wd_status) & WDCS_BUSY))
		;
	/* Did we get an error?         */
	if (opcode == WDCC_READ && (inb(wdc+wd_status) & WDCS_ERR))
		goto error;

	/* Ready to remove data?        */
	while ((inb(wdc+wd_status) & WDCS_DRQ) == 0) ;

	if (opcode == WDCC_READ)
		insw(wdc+wd_data,addr,256);
	else	outsw(wdc+wd_data,addr,256);

	/* Check data request (should be done).         */
	if (inb(wdc+wd_status) & WDCS_DRQ) goto error;

	while (opcode == WDCC_WRITE && (inb(wdc+wd_status) & WDCS_BUSY)) ;

	if (inb(wdc+wd_status) & WDCS_ERR) goto error;

#ifdef WDDEBUG
printf("+");
#endif
        return (0);
error:
	erro = inb(wdc+wd_error);
	if (++retries < RETRIES)
		goto retry;
	if (!wdquiet)
#ifdef	SMALL
	    printf("wd%d: hard error: sector %d status %x error %x\n", unit,
		blknm, inb(wdc+wd_status), erro);
#else
	    printf("wd%d: hard %s error: sector %d status %b error %b\n", unit,
		opcode == WDCC_READ? "read" : "write", blknm, 
		inb(wdc+wd_status), WDCS_BITS, erro, WDERR_BITS);
#endif
	return (-1);
}

wdinit(io)
	struct iob *io;
{
	register wdc;
	struct disklabel *dd;
        unsigned int   unit;
	struct dkbad *db;
	int i, errcnt = 0;
	char buf[512];
	static open[NWD];

	unit = io->i_unit;
	if (open[unit]) return(0);

	wdcport = io->i_ctlr ? IO_WD2 : IO_WD1;
	wdc = wdcport;

#ifdef	SMALL
	dd = &disklabel;
#else
	/* reset controller */
	outb(wdc+wd_ctlr, 12); wait(10); outb(wdc+wd_ctlr, 8);
	wdwait();

	dd = &wdsizes[unit];

tryagainrecal:
	/* set SDH, step rate, do restore to recalibrate drive */
	outb(wdc+wd_sdh, WDSD_IBM | (unit << 4));
	wdwait();
	outb(wdc+wd_command, WDCC_RESTORE | WD_STEP);
	wdwait();
	if ((i = inb(wdc+wd_status)) & WDCS_ERR) {
/*#ifdef SMALL
		printf("wd%d: recal status %x error %x\n",
			unit, i, inb(wdc+wd_error));
#else*/
		printf("wd%d: recal status %b error %b\n",
			unit, i, WDCS_BITS, inb(wdc+wd_error), WDERR_BITS);
/*#endif*/
		if (++errcnt < 10)
			goto tryagainrecal;
		return(-1);
	}

	/*
	 * Some controllers require this (after a recal they
	 * revert to a logical translation mode to compensate for
	 * dos limitation on 10-bit cylinders -- *shudder* -wfj)
	 * note: cylinders *must* be fewer than or equal to 8 to
	 * compensate for some IDE drives that latch this for all time.
	 */
	outb(wdc+wd_sdh, WDSD_IBM | (unit << 4) + 8 -1);
	outb(wdc+wd_seccnt, 35 );
	outb(wdc+wd_cyl_lo, 1224);
	outb(wdc+wd_cyl_hi, 1224/256);
	outb(wdc+wd_command, 0x91);
	while (inb(wdc+wd_status) & WDCS_BUSY) ;

	errcnt = 0;
retry:
	/*
	 * Read in LABELSECTOR to get the pack label and geometry.
	 */
	outb(wdc+wd_precomp, 0xff);	/* sometimes this is head bit 3 */
	outb(wdc+wd_seccnt, 1);
	outb(wdc+wd_sector, LABELSECTOR + 1);
	outb(wdc+wd_cyl_lo, (cyloffset & 0xff));
	outb(wdc+wd_cyl_hi, (cyloffset >> 8));
	outb(wdc+wd_sdh, WDSD_IBM | (unit << 4));
	wdwait();
	outb(wdc+wd_command, WDCC_READ);
	wdwait();
	if ((i = inb(wdc+wd_status)) & WDCS_ERR) {
		int err;

		err = inb(wdc+wd_error);
		if (++errcnt < RETRIES)
			goto retry;
		if (!wdquiet)
/*#ifdef SMALL
		    printf("wd%d: reading label, status %x error %x\n",
			unit, i, err);
#else*/
		    printf("wd%d: reading label, status %b error %b\n",
			unit, i, WDCS_BITS, err, WDERR_BITS);
/*#endif*/
		return(-1);
	}

	/* Ready to remove data?        */
	while ((inb(wdc+wd_status) & WDCS_DRQ) == 0) ;

	i = insw(wdc+wd_data, buf, 256);

#ifdef WDDEBUG
	printf("magic %x,insw %x, %x\n",
	((struct disklabel *) (buf + LABELOFFSET))->d_magic, i, buf);
#endif
	if (((struct disklabel *) (buf + LABELOFFSET))->d_magic == DISKMAGIC) {
		*dd = * (struct disklabel *) (buf + LABELOFFSET);
		open[unit] = 1;
	} else {
		if (!wdquiet)
			printf("wd%d: bad disk label\n", unit);
		if (io->i_flgs & F_FILE) return(-1);
		dkbad[unit].bt_bad[0].bt_cyl = -1;
		dd->d_secpercyl = 1999999 ; dd->d_nsectors = 17 ;
		dd->d_secsize = 512;
		outb(wdc+wd_precomp, 0xff);	/* force head 3 bit off */
		return (0) ;
	}
#endif SMALL
#ifdef SMALL
#ifdef WDDEBUG
	printf("magic %x sect %d\n", dd->d_magic, dd->d_nsectors);
#endif
#endif	SMALL


	/* now that we know the disk geometry, tell the controller */
	outb(wdc+wd_cyl_lo, dd->d_ncylinders);
	outb(wdc+wd_cyl_hi, (dd->d_ncylinders)>>8);
	outb(wdc+wd_sdh, WDSD_IBM | (unit << 4) + dd->d_ntracks-1);
	outb(wdc+wd_seccnt, dd->d_nsectors);
	outb(wdc+wd_command, 0x91);
	while (inb(wdc+wd_status) & WDCS_BUSY) ;

	dkbad[unit].bt_bad[0].bt_cyl = -1;
	outb(wdc+wd_precomp, dd->d_precompcyl / 4);

	/*
	 * Read bad sector table into memory.
	 */
	i = 0;
	do {
		int blknm = dd->d_secperunit - dd->d_nsectors + i;
		errcnt = wdio(F_READ, unit, blknm, buf);
	} while (errcnt && (i += 2) < 10 && i < dd->d_nsectors);
	db = (struct dkbad *)(buf);
#define DKBAD_MAGIC 0x4321
	if (errcnt == 0 && db->bt_mbz == 0 && db->bt_flag == DKBAD_MAGIC)
		dkbad[unit] = *db;
	else {
		if (!wdquiet)
			printf("wd%d: error in bad-sector file\n", unit);
		dkbad[unit].bt_bad[0].bt_cyl = -1;
	}
	return(0);
}

wdwait()
{
	register wdc = wdcport;
	register i = 0;
	
	while (inb(wdc+wd_status) & WDCS_BUSY)
		;
	while ((inb(wdc+wd_status) & WDCS_READY) == 0)
		if (i++ > 100000)
			return(-1);
	return(0);
}
