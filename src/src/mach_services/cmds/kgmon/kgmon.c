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
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: kgmon.c,v $ $Revision: 1.1.2.2 $ (OSF) $Date: 1993/07/08 17:51:10 $";
#endif

#include <sys/param.h>
#include <sys/file.h>
#include <sys/vm.h>
#include <sys/gprof.h>
#include <stdio.h>
#include <nlist.h>
#include <ctype.h>
#include <paths.h>

#define	PROFILING_ON	0
#define	PROFILING_OFF	3

u_long	s_textsize;
off_t	sbuf, klseek(), lseek();
int	ssiz;
int	kopen(), kread(), kwrite();

struct nlist nl[] = {
#define N_HZ		0
	{ "_hz" },
#define N_FROMS		1
	{ "_froms" },
#define	N_PROFILING	2
	{ "_profiling" },
#define	N_S_LOWPC	3
	{ "_s_lowpc" },
#define	N_S_TEXTSIZE	4
	{ "_s_textsize" },
#define	N_SBUF		5
	{ "_sbuf" },
#define N_SSIZ		6
	{ "_ssiz" },
#define	N_TOS		7
	{ "_tos" },
#if	mips
#define N_PHDR		8
	{ "_phdr" },
#endif
	0,
};

int	kmem;
int	bflag, hflag, rflag, pflag;
int	mkflag;
int	debug = 0;

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	int ch, mode, disp, openmode;
	char *system, *kmemf, *malloc();

	while ((ch = getopt(argc, argv, "bhprmd")) != EOF)
		switch((char)ch) {
		case 'b':
			bflag++;
			break;
		case 'h':
			hflag++;
			break;
		case 'p':
			pflag++;
			break;
		case 'r':
			rflag++;
			break;
		case 'm':
			mkflag++;
			break;
		case 'd':
			debug++;
			break;
		default:
			(void)fprintf(stderr,
			    "usage: kgmon [-bhrpm] [system]\n");
			exit(1);
		}
	argc -= optind;
	argv += optind;

	kmemf = _PATH_KMEM;
	if (*argv) {
		system = *argv;
	}
	else
		system = _PATH_UNIX;

	if (nlist(system, nl) < 0 || nl[0].n_type == 0) {
		(void)fprintf(stderr, "kgmon: %s: no namelist\n", system);
		exit(2);
	}
	if (!nl[N_PROFILING].n_value) {
		(void)fprintf(stderr,
		    "kgmon: profiling: not defined in kernel.\n");
		exit(10);
	}

	openmode = (bflag || hflag || pflag || rflag) ? O_RDWR : O_RDONLY;
	kmem = kopen(kmemf, openmode);
	if (kmem < 0) {
		openmode = O_RDONLY;
		kmem = kopen(kmemf, openmode);
		if (kmem < 0) {
			perror(kmemf);
			exit(3);
		}
		(void)fprintf(stderr, "%s opened read-only\n", kmemf);
		if (rflag)
			(void)fprintf(stderr, "-r supressed\n");
		if (bflag)
			(void)fprintf(stderr, "-b supressed\n");
		if (hflag)
			(void)fprintf(stderr, "-h supressed\n");
		rflag = bflag = hflag = 0;
	}
	mode = kfetch(N_PROFILING);
	if (hflag)
		disp = PROFILING_OFF;
	else if (bflag)
		disp = PROFILING_ON;
	else
		disp = mode;
	if (pflag) {
		if (openmode == O_RDONLY && mode == PROFILING_ON)
			(void)fprintf(stderr, "data may be inconsistent\n");
		dumpstate();
	}
	if (rflag)
		resetstate();
	turnonoff(disp);
	(void)fprintf(stdout,
	    "kernel profiling is %s.\n", disp ? "off" : "running");
}

dumpstate()
{
	extern int errno;
	struct rawarc rawarc;
	struct tostruct *tos;
	u_long frompc;
	off_t kfroms, ktos;
	u_short *froms;		/* froms is a bunch of u_shorts indexing tos */
	int i, fd, fromindex, endfrom, fromssize, tossize, toindex;
	char buf[BUFSIZ], *s_lowpc, *malloc(), *strerror();
#if	mips
        struct phdr phdr;
        int hdr;
#endif

	turnonoff(PROFILING_OFF);
	fd = creat("gmon.out", 0666);
	if (fd < 0) {
		perror("gmon.out");
		return;
	}
#if	mips
	ksteal(N_PHDR, &phdr, sizeof(phdr));
        kwrite(fd, &phdr.lowpc, sizeof(phdr.lowpc));
        kwrite(fd, &phdr.highpc, sizeof(phdr.highpc));
        kwrite(fd, &phdr.pc_bytes, sizeof(phdr.pc_bytes));

        ssiz = phdr.pc_bytes;
        sbuf = (off_t)phdr.pc_buf;
#else
	ssiz = kfetch(N_SSIZ);
	sbuf = kfetch(N_SBUF);
#endif
	(void)klseek(kmem, (off_t)sbuf, L_SET);
	for (i = ssiz; i > 0; i -= BUFSIZ) {
		kread(kmem, buf, i < BUFSIZ ? i : BUFSIZ);
		kwrite(fd, buf, i < BUFSIZ ? i : BUFSIZ);
	}
	s_textsize = kfetch(N_S_TEXTSIZE);
	fromssize = s_textsize / HASHFRACTION;
	froms = (u_short *)malloc((u_int)fromssize);
	kfroms = kfetch(N_FROMS);
	(void)klseek(kmem, kfroms, L_SET);
	i = kread(kmem, ((char *)(froms)), fromssize);
	if (i != fromssize) {
		(void)fprintf(stderr, "read kmem: request %d, got %d: %s",
		    fromssize, i, strerror(errno));
		exit(5);
	}
	tossize = (s_textsize * ARCDENSITY / 100) * sizeof(struct tostruct);
	tos = (struct tostruct *)malloc((u_int)tossize);
	ktos = kfetch(N_TOS);
	(void)klseek(kmem, ktos, L_SET);
	i = kread(kmem, ((char *)(tos)), tossize);
	if (i != tossize) {
		(void)fprintf(stderr, "read kmem: request %d, got %d: %s",
		    tossize, i, strerror(errno));
		exit(6);
	}
	s_lowpc = (char *)kfetch(N_S_LOWPC);
	if (debug)
		(void)fprintf(stderr, "s_lowpc 0x%x, s_textsize 0x%x\n",
		    s_lowpc, s_textsize);
	endfrom = fromssize / sizeof(*froms);
	for (fromindex = 0; fromindex < endfrom; fromindex++) {
		if (froms[fromindex] == 0)
			continue;
		frompc = (u_long)s_lowpc +
		    (fromindex * HASHFRACTION * sizeof(*froms));
		for (toindex = froms[fromindex]; toindex != 0;
		   toindex = tos[toindex].link) {
			if (debug)
			    (void)fprintf(stderr,
			    "[mcleanup] frompc 0x%x selfpc 0x%x count %d\n" ,
			    frompc, tos[toindex].selfpc, tos[toindex].count);
			rawarc.raw_frompc = frompc;
			rawarc.raw_selfpc = (u_long)tos[toindex].selfpc;
			rawarc.raw_count = tos[toindex].count;
			kwrite(fd, (char *)&rawarc, sizeof (rawarc));
		}
	}
	close(fd);
}

resetstate()
{
	off_t kfroms, ktos;
	int i, fromssize, tossize;
	char buf[BUFSIZ];
#if	mips
        struct phdr phdr;
#endif
        
	turnonoff(PROFILING_OFF);
	bzero(buf, BUFSIZ);
#if	mips
	ksteal(N_PHDR, &phdr, sizeof(phdr));
        ssiz = phdr.pc_bytes;
        sbuf = (off_t)phdr.pc_buf;
#else
	ssiz = kfetch(N_SSIZ);
	sbuf = kfetch(N_SBUF);
	ssiz -= sizeof(struct phdr);
	sbuf += sizeof(struct phdr);
#endif
	(void)klseek(kmem, (off_t)sbuf, L_SET);
	for (i = ssiz; i > 0; i -= BUFSIZ)
		if (kwrite(kmem, buf, i < BUFSIZ ? i : BUFSIZ) < 0) {
			perror("sbuf write");
			exit(7);
		}
	s_textsize = kfetch(N_S_TEXTSIZE);
	fromssize = s_textsize / HASHFRACTION;
	kfroms = kfetch(N_FROMS);
	(void)klseek(kmem, kfroms, L_SET);
	for (i = fromssize; i > 0; i -= BUFSIZ)
		if (kwrite(kmem, buf, i < BUFSIZ ? i : BUFSIZ) < 0) {
			perror("kforms write");
			exit(8);
		}
	tossize = (s_textsize * ARCDENSITY / 100) * sizeof(struct tostruct);
	ktos = kfetch(N_TOS);
	(void)klseek(kmem, ktos, L_SET);
	for (i = tossize; i > 0; i -= BUFSIZ)
		if (kwrite(kmem, buf, i < BUFSIZ ? i : BUFSIZ) < 0) {
			perror("ktos write");
			exit(9);
		}
}

turnonoff(onoff)
	int onoff;
{
	(void)klseek(kmem, (long)nl[N_PROFILING].n_value, L_SET);
	(void)kwrite(kmem, (char *)&onoff, sizeof (onoff));
}

kfetch(index)
	int index;
{
	off_t off;
	int value;

	if ((off = nl[index].n_value) == 0) {
		printf("%s: not defined in kernel\n", nl[index].n_name);
		exit(11);
	}
	if (klseek(kmem, off, L_SET) == -1) {
		perror("lseek");
		exit(12);
	}
	if (kread(kmem, (char *)&value, sizeof (value)) != sizeof (value)) {
		perror("read");
		exit(13);
	}
	return (value);
}

#if	mips
ksteal(index, buf, sz)
	int index;
        char *buf;
        int sz;
{
	off_t off;

	if ((off = nl[index].n_value) == 0) {
		printf("%s: not defined in kernel\n", nl[index].n_name);
		exit(11);
	}
	if (klseek(kmem, off, L_SET) == -1) {
		perror("lseek");
		exit(12);
	}
	if (kread(kmem, (char *)buf, sz) != sz) {
		perror("read");
		exit(13);
	}
}
#endif

#include <mach.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <mach/mig_errors.h>

#define task_by_pid(pid)	syscall(-33, pid)

off_t koffset;
mach_port_t devport;

off_t
klseek(fd, base, off)
	int fd, off;
	off_t base;
{
	if (mkflag && (fd == kmem)) {
		if (off != L_SET)
			return(-1);
		return (koffset = base);
	}
	else return (lseek(fd, base, off));
}

int
kopen(path, mode)
	char *path;
	int mode;
{
	if (mkflag && (strcmp(path, _PATH_KMEM) == 0)) {
		/* use special mach kernel access */

		mach_port_t	device_server_port;
		kern_return_t	result;

#ifdef notyet
		mach_port_t	bootstrap_port;
		mach_port_t	reply_port;
		struct imsg {
			mach_msg_header_t	hdr;
			mach_msg_type_t	port_desc_1;
			mach_port_t		port_1;
			mach_msg_type_t	port_desc_2;
			mach_port_t		port_2;
		} imsg;
#endif

		if (debug)
			printf("kopen: using mach device open\n");

		device_server_port = task_by_pid(-2);
		if (device_server_port == MACH_PORT_NULL)
			return(-1);

#ifdef notyet
		/*
		 * Get our bootstrap port
		 */
		result = task_get_bootstrap_port(mach_task_self(), &bootstrap_port);
		if (result != KERN_SUCCESS)
			return(-1);

		/*
		 * Allocate a reply port
		 */
		reply_port = mach_reply_port();
		if (reply_port == MACH_PORT_NULL)
			return(-1);

		printf("reply port %d\n", reply_port);

		/*
		 * Send a message to it, asking for the host and device ports
		 */
		imsg.hdr.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
						    MACH_MSG_TYPE_MAKE_SEND_ONCE);
		imsg.hdr.msgh_size = 0;
		imsg.hdr.msgh_remote_port = bootstrap_port;
		imsg.hdr.msgh_local_port = reply_port;
		imsg.hdr.msgh_kind = MACH_MSGH_KIND_NORMAL;
		imsg.hdr.msgh_id = 999999;

		result = mach_msg(&imsg.hdr, MACH_SEND_MSG|MACH_RCV_MSG,
				  sizeof imsg.hdr, sizeof imsg, reply_port,
				  MACH_MSG_TIMEOUT_NONE, NULL);
		if (result != MACH_MSG_SUCCESS)
			return(-1);

		device_server_port = imsg.port_2;
#endif
		result = device_open(device_server_port,
				 0,
				 "gprof",
				 &devport);
		if (result != KERN_SUCCESS) {
			printf("device_open status %d\n", result);
			return (-1);
		}

		return (44);
	}
	else
		return (open(path, mode));
}

int
kread(fd, buf, bufsiz)
	int fd;
	char *buf;
	size_t bufsiz;
{
	if (mkflag && (fd == kmem)) {
		/* use special mach kernel access */

		kern_return_t result;
		int count, len;
		vm_offset_t data;

		len = bufsiz;
		do {
			count = 0;
			result = device_read(devport,
					     0,	/* mode */
					     koffset,
					     len,
					     &data,
					     &count);


			if (result != KERN_SUCCESS)
				/* problems, bail out */
				break;

			/* copy data to buf and deallocate ool data area */
			bcopy((char *)data, buf, count);
			vm_deallocate(mach_task_self(), data, count);

			/* move seek pointer, buf addr and back off len */
			koffset += count;
			buf += count;
			len -= count;

		} while (len > 0);

		return (bufsiz - len);
	}
	else
		return (read(fd, buf, bufsiz));
}

int
kwrite(fd, buf, bufsiz)
	int fd;
	char *buf;
	size_t bufsiz;
{
	if (mkflag && (fd == kmem)) {
		/* use special mach kernel access */

		kern_return_t result;
		int count;

		result = device_write(devport,
				  0,	/* mode */
				  koffset,
				  buf,
				  bufsiz,
				  &count);

		if (result != KERN_SUCCESS)
			return(-1);

		/* move file offset */
		koffset += count;
		return(count);
	}
	else
		return (write(fd, buf, bufsiz));
}
