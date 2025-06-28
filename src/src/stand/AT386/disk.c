/*
 * Copyright (c) 1995, 1994, 1993, 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of ("OSF") or Open Software 
 * Foundation not be used in advertising or publicity pertaining to 
 * distribution of the software without specific, written prior permission. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL OSF BE LIABLE FOR ANY 
 * SPECIAL, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
 * ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE 
 */
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */
/* CMU_HIST */
/*
 * Revision 2.1.2.1  92/04/30  11:53:17  bernadat
 * 	Fixed code to use mk includes only (not sys/inode.h ....).
 * 	Copied from main line
 * 	[92/03/19            bernadat]
 * 
 * Revision 2.2  92/04/04  11:35:49  rpd
 * 	Fabricated from 3.0 bootstrap and 2.5 boot disk.c:
 * 	with support for scsi
 * 	[92/03/30            mg32]
 * 
 */
/* CMU_ENDHIST */
/*
 * Mach Operating System
 * Copyright (c) 1992, 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/*
 */

#include "boot.h"
#include <i386/disk.h>

#define	BIOS_DEV_FLOPPY	0x0
#define	BIOS_DEV_WIN	0x80

#define BPS		512
#define	SPT(di)		((di)&0xff)
#define	HEADS(di)	((((di)>>8)&0xff)+1)
#define	BDEV(dev,unit)	(((dev)==1 ? BIOS_DEV_FLOPPY:BIOS_DEV_WIN)+(unit))

char *devs[] = {"hd", "fd", "wt", "sd", "ha", 0};

struct alt_info alt_info;
int spt, spc;

char *iodest;
struct fs *fs;
struct file inode;
int unit, part, maj, boff, poff, bnum, cnt;

devopen()
{
	char *altptr;
	struct ipart *iptr;
	int dev = inode.f_dev;
	int i, sector, di = get_diskinfo(BDEV(dev,unit));
	spc = (spt = SPT(di)) * HEADS(di);
	if (dev == 1) {
		/*
		 * Check if installed floppy has this density
		 */
	  	for (i = spt;
		     biosread(BDEV(dev,unit), 0, 0, i-1) != 0;
		     i /= 2);
                for (i = i + 1;
		     i <= spt && biosread(BDEV(dev,unit), 0, 0, i-1) == 0;
		     i++);
		if (--i != spt) {
			spt = i;
			spc = spt * HEADS(di);
	    	}
		boff = 0;
		part = (spt == 15 ? 3 : 1);
	}
	else {
		if (maj == 0 && spt == 32 && spc == 32*64) /* SCSI */
			maj = 3;
		Bread(dev, unit, 0);
		iptr = (struct ipart *)((struct mboot *)0)->parts;
		for (i = 0; i < FD_NUMPART; i++, iptr++)
			if (iptr->systid == UNIXOS)
				break;
		sector = iptr->relsect + HDPDLOC;
		if (Bread(dev, unit, sector++))
			return 1;
		if (((struct evtoc *)0)->sanity != VTOC_SANE) {
			printf("vtoc insane");
			return 1;
		}
		boff = ((struct evtoc *)0)->part[part].p_start;
		altptr = (char *)(&alt_info);
		for (i = 0; i++ < 4; altptr += BPS, sector++) {
			if (Bread(dev, unit, sector))
				return 1;
			bcopy(0, altptr, BPS);
		}
		if (alt_info.alt_sanity != ALT_SANITY) {
			printf("Bad alt_sanity\n");
			return 1;
		}
	}
	return 0;
}

devread()
{
	int offset, sector = bnum;
	int dev = inode.f_dev;
	for (offset = 0; offset < cnt; offset += BPS) {
		if (Bread(dev, unit, badsect(dev, sector++)))
			return 1;
		bcopy(0, iodest+offset, BPS);
	}
	return 0;
}

Bread(dev,unit,sector)
     int dev,unit,sector;
{
	int retry = 3;
	int cyl = sector/spc, head = (sector%spc)/spt, secnum = sector%spt;
	while (retry--  && !is_intr_char() ) {
		if (!biosread(BDEV(dev,unit), cyl,head,secnum))
			return 0;
		printf("Error: C:%d H:%d S:%d\n",cyl,head,secnum);
	}
	return(1);
}

badsect(dev, sector)
     int dev, sector;
{
	int i;
	if (!dev) {
		for (i = 0; i < alt_info.alt_trk.alt_used; i++)
			if (sector/spt == alt_info.alt_trk.alt_bad[i])
				return (alt_info.alt_trk.alt_base + 
					i*spt + sector%spt);
		for (i = 0; i < alt_info.alt_sec.alt_used; i++)
			if (sector == alt_info.alt_sec.alt_bad[i])
				return (alt_info.alt_sec.alt_base + i);
	}
	return sector;
}





