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

/*
  Copyright 1988, 1989, 1990, 1991, 1992 
   by Intel Corporation, Santa Clara, California.

                All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
 * File : boot.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains the automaton of the Network bootstrap.
 */

#include <i386/exec.h>
#include <sys/reboot.h>
#include "boot.h"
#include "smc.h"
#include "3c503.h"
#include "dlink.h"
#include "bootp.h"
#include "udpip.h"
#include "tftp.h"
#include "unzip.h"

void *end_of_data;	/* current end of data segment */
unsigned debug;		/* debug enabled */
struct exec head;
int a_str;		/* string table length */
unsigned a_strlen;	/* length of string table length */
int argv[10], esym;

#define	BOOT_UNCOMPRESSED	0	/* Kernel image uncompressed */
#define	BOOT_GZIP		1	/* Kernel image compressed via gzip */
unsigned compressed;

char *names[] = {
	"",		/* bootp default boot file */
	"mach.boot",
	"mach",
	"vmunix",
	"mach.old",
	(char *)0
};

int (*boot_probe[])(void) = {
	probe_3c503,
	probe_smc,
	(int (*)(void))0
};

#define	BOOT_COPY_REPORT	4
#define	BOOT_COPY_ACTIVITY(block)					\
	if (((block) & ((1 << BOOT_COPY_REPORT) - 1)) == 0) {		\
		switch ((block) % (1 << (BOOT_COPY_REPORT + 2))) {	\
		case 0:							\
			putchar('-');					\
			break;						\
		case (1 << BOOT_COPY_REPORT):				\
			putchar('\\');					\
			break;						\
		case (1 << BOOT_COPY_REPORT) * 2:			\
			putchar('|');					\
			break;						\
		case (1 << BOOT_COPY_REPORT) * 3:			\
			putchar('/');					\
			break;						\
		}							\
		putchar('\b');						\
	}

int
boot_copy(unsigned block,
	  void *inaddr,
	  void **outaddr,
	  unsigned len)
{
	unsigned rlen;
	unsigned i;
	unsigned next;

	if (block == 1) {
		a_strlen = sizeof (int);
		a_str = 0;
		head = *(struct exec *)inaddr;
		switch (head.a_info) {
		case 0407:
			/*
			 * Translate code to skip header.
			 */
			inaddr = inaddr + sizeof (struct exec);
			len -= sizeof (struct exec);
			compressed = BOOT_UNCOMPRESSED;
			break;

		case 0413:
			head.a_text += sizeof(struct exec);
			compressed = BOOT_UNCOMPRESSED;
			break;

		default:
			if (((char *)inaddr)[0] == '\037' &&
			    ((char *)inaddr)[1] == '\213') {
				compressed = BOOT_GZIP;
				printf("Loading gzip compressed image");
				break;
			}
			printf("Invalid format!\n");
			return (1);
		}
		if (compressed == BOOT_UNCOMPRESSED)
			printf("%d", head.a_text);
		next = 1;
	} else
		next = 0;

	if (compressed != BOOT_UNCOMPRESSED) {
		pcpy_out(inaddr, *outaddr, len);
		*outaddr = *(char **)outaddr + len;
		BOOT_COPY_ACTIVITY(block);
		if (len < 512) {
			printf(" (%d bytes)\n", (block - 1) * 512 + len);
			return (2);
		}
		return (0);
	}

	if (head.a_text > 0) {
		rlen = head.a_text >= len ? len : head.a_text;
		pcpy_out(inaddr, *outaddr, rlen);
		head.a_text -= rlen;
		*outaddr = *(char **)outaddr + rlen;
		if (head.a_text > 0) {
			BOOT_COPY_ACTIVITY(block);
			return (0);
		}
		len -= rlen;
		inaddr += rlen;
		next = 1;
	}

	if (next)
		printf("+%d", head.a_data);

	if (head.a_data > 0) {
		rlen = head.a_data >= len ? len : head.a_data;
		pcpy_out(inaddr, *outaddr, rlen);
		head.a_data -= rlen;
		*outaddr = *(char **)outaddr + rlen;
		if (head.a_data > 0) {
			BOOT_COPY_ACTIVITY(block);
			return (0);
		}
		len -= rlen;
		inaddr += rlen;
		next = 1;
	}

	if (next)
		printf("+%d", head.a_bss);

	if (head.a_bss > 0) {
		*outaddr = *(char **)outaddr + head.a_bss;
		argv[3] = (int)*outaddr;
		head.a_bss = 0;
		pcpy_out(&head.a_syms, *outaddr, sizeof (head.a_syms));
		*outaddr = *(char **)outaddr + sizeof (head.a_syms);
	}

	if (next)
		printf("[+%d", head.a_syms);

	if (head.a_syms > 0) {
		rlen = head.a_syms >= len ? len : head.a_syms;
		pcpy_out(inaddr, *outaddr, rlen);
		head.a_syms -= rlen;
		*outaddr = *(char **)outaddr + rlen;
		if (head.a_syms > 0) {
			BOOT_COPY_ACTIVITY(block);
			return (0);
		}
		inaddr += rlen;
		len -= rlen;
		next = 1;
	}
	
	if (a_strlen > 0) {
		rlen = a_strlen >= len ? len : a_strlen;
		for (i = 0; i < rlen; i++)
			a_str = (a_str << 8) | ((unsigned char *)inaddr)[i];
		a_strlen -= rlen;
		if (a_strlen > 0 && len > 0) {
			BOOT_COPY_ACTIVITY(block);
			return (0);
		}
		a_str = ntohl(a_str); /* i386 is a little endian */
		pcpy_out(&a_str, *outaddr, sizeof (int));
		*outaddr = *(char **)outaddr + sizeof (int);
		if (a_str >= sizeof(int))
			a_str -= sizeof(int);
		len -= rlen;
		inaddr += rlen;
		next = 1;
	}

	if (next)
		printf("+%d]", a_str);

	if (a_str > 0) {
		rlen = a_str >= len ? len : a_str;
		pcpy_out(inaddr, *outaddr, rlen);
		a_str -= rlen;
		*outaddr = *(char **)outaddr + rlen;
		if (a_str > 0) {
			BOOT_COPY_ACTIVITY(block);
			return (0);
		}
		len -= rlen;
		inaddr += rlen;
	}
	printf(" \n");
	if (len > 0)
		printf("Warning: %d bytes remain at the end of file (block = %x)\n", len, block);
	return (2);
}

void
printflags(unsigned flags)
{
	if (flags & 0xFF00)
		printf(" -%d", (flags & 0xFF00) >> 8);

	if (flags & (RB_ASKNAME|RB_KDB|RB_HALT|RB_SINGLE))
		printf(" -");

	if (flags & RB_ASKNAME)
		putchar('a');
	if (flags & RB_KDB)
		putchar('d');
	if (flags & RB_HALT)
		putchar('h');
	if (flags & RB_SINGLE)
		putchar('s');
}

getbootstr(unsigned	*howto,
	   char		**bootstring,
	   char		**kernel)
{
	char c;
	char *ptr;
	char *old_ptr;
	char *q; 
	unsigned name_found;
	unsigned val;

	ptr = end_of_data;
	for (;;) {
		flush_char();
		printf("boot string: ");
		if (gets(end_of_data) == 0) {
			*bootstring = "";
			return (0);
		}
		if (*ptr != '?')
			break;
		printf("usage: %s -adhs -<#cpu> server_args\n",
		       "[<root_device>(<drive>,<part>)][<kernel>][:<server>]");
	}

	c = *ptr;
	while (c == ' ' || c == '\t')
		c = *++ptr;
		
	name_found = 0;
	*howto = 0;
	*bootstring = ptr;
	while (*ptr++ != '\0')
		continue;
	end_of_data = ptr;
	ptr = *bootstring;

	if (c != '\0' && c != '-') {
		/*
		 * look for <root_dev><kernel>:<server>
		 * Copy <root_dev><kernel> part only to *kernel
		 */
		if (c != ':') {
			q = *kernel = end_of_data;
			do
				*q++ = *ptr++;
			while ((c = *ptr) != '\0' &&
			       c != ':' && c != ' ' && c != '\t');
			*q = '\0';
			end_of_data = q + 1;
		}
		while (c != ' ' && c != '\t')
			c = *++ptr;
	}

	while (c != '\0') {
		if (c != '-') {
			/* must be a server argument without '-' */
			break;
		}

		while ((c = ptr[1]) && c != ' ' && c != '\t') {
			old_ptr = ptr++;
			switch (c) {
			case 'a':
				*howto |= RB_ASKNAME;
				break;
			case 's':
				*howto |= RB_SINGLE;
				break;
			case 'd':
				*howto |= RB_KDB;
				break;
			case 'b':
			case 'h':
				*howto |= RB_HALT;
				break;
			default:
				/*
				 * check if digit, in this case it is
				 * the number of cpus to boot
				 */
				if (c <= '0' || c > '9') {
					/* 
					 * this is not a boot arg pass it to
					 * the server and assume the rest of
					 * the line is server arguments
					 */
					return (1);
				}
				val = c - '0';
				while ((c = *++ptr) >= '0' && c <= '9') {
					if ((val = val * 10 + (c - '0')) > 255)
						/* 
						 * #cpus too large ==> assume
						 * the rest of the line is
						 * server arguments
						 */
						return (1);
				}
				*howto = (*howto & 0xFF) | (val << 8);
				break;
			}
			/*
			 * since this is a boot arg, clear
			 * it for the server, if it must
			 * be passed to the server it is
			 * done via the howto argument
			 */
			while (ptr >= old_ptr)
				*old_ptr++ = ' ';
		}

		while (c == ' ' || c == '\t')
			c = *++ptr;
	}
	return (1);
}

extern int remote;

int
boot(unsigned howto,
     unsigned device,
     unsigned cnvmem,
     unsigned extmem)
{
	int (**probe)(void);
	char *name;
	char *namep;
	char *p;
	char *new_name;
	unsigned i;
	unsigned loadflags;
	char *bootstr;
	unsigned boot_device;
	unsigned root_device;
	unsigned wait;
	int rdev;
	int rboot;
	int ret;
	void *cur_end;
	void *copy_addr;
	void *load_addr;
	static int (*x_entry)() = 0;

	remote = (howto & RB_ALTBOOT) != 0;
	debug = (howto >> 8) & 0xFF;

	printf("\n>> EXTENDED MACH BOOT: %d/%d k of memory  [07/19/93]\n",
	       cnvmem, extmem);

	for (probe = boot_probe; *probe != (int (*)(void))0; probe++)
		if ((*probe)())
			break;

	timerinit();
	udpip_init();
	arp_init();
	tftp_init();
	bootp_init();

 restart:
	boot_device = device;
	rboot = *probe != (int (*)(void))0 ? 0 : -1;
	wait = getbootdev(&boot_device, &rboot);

	i = 0;
	name = (char *)0;
	
	cur_end = end_of_data;
	for (;;) {
		if (name == names[i])
			if (names[++i] == (char *)0)
				i = 0;
		name = names[i];
		if (rboot == -1 && *name == '\0')
			continue; /* skip bootp entry for local boot */

		if (wait) {
			wait = getbootstr(&loadflags, &bootstr, &name);
			loadflags |= howto & ~RB_HALT & 0xFFFF;
			if (name == names[i] ||
			    (new_name =
			     devname_to_num(name, &root_device)) == (char *)0) {
				rdev = 1;
				new_name = name;
				root_device = boot_device;
			} else
				rdev = 0;
		} else {
			rdev = 1;
			new_name = name;
			root_device = boot_device;
		}

		if (!wait)
			putchar('\n');

		if (rboot == -1) { /* Local device */
			if (name != names[i] && *new_name == '\0')
				name = new_name = names[i];
#if	0
			if (openrd() == 0) {
				if (inode.i_mode & IEXEC)
					loadflags |= RB_KDB;
				loadprog(loadflags);
				break;
			} else
				printf("Can't find %s\n", new_name);
#else
			printf("Local bootstrap not yet ready\n");
			goto restart;
#endif
		} else { /* Remote device */
			load_addr = (void *)(howto & ~0xFFFF);
			switch (ret = bootp_main(&new_name, &root_device, rdev,
						 loadflags, load_addr)) {
			case 0:
				if (*new_name == '\0') {
					printf("\nNo bootp access for '%s'\n",
					       *name ? name : BOOTP_DEFAULT);
					ret = 1;
					break;
				}
				if ((ret = tftp_main(new_name, &load_addr,
						     boot_copy)) == 2) {
					end_of_data = cur_end;
					goto restart;
				}
				break;
			case 1:
				printf("\nNo bootp answer for '%s'\n",
				       *name ? name : BOOTP_DEFAULT);
				break;
			case 2:
				end_of_data = cur_end;
				goto restart;
			}
		}

		/*
		 * Handle compressed kernel images.
		 */
		if (ret == 0 && compressed != BOOT_UNCOMPRESSED) {
			/*
			 * Copy image at the end of extended memory.
			 */
			i = (unsigned)load_addr - (howto & ~0xFFFF);
			copy_addr = (void *)(((cnvmem + extmem) * 1024 -
					      (i + BOOTOFFSET)) & 0xFFFFF000);
			if (copy_addr < load_addr) {
				printf("Not enough memory to uncompress\n");
				goto restart;
			}
			load_addr = (void *)(howto & ~0xFFFF);
			pcpy_in(load_addr, copy_addr, i);

			end_of_data = cur_end;
			switch (compressed) {
			case BOOT_GZIP:
				if ((ret = unzip(&load_addr, copy_addr,
						 i, boot_copy)) == 2) {
					end_of_data = cur_end;
					goto restart;
				}
				break;
			}
		}

		if (ret == 0)
			break;
		if (name != names[i])
			name = (char *)0;
		end_of_data = cur_end;
	}

	/*
	 * Copy bootstring after kernel image.
	 */
	argv[4] = ((int)load_addr + sizeof (int) - 1) & ~(sizeof (int) - 1);
	load_addr = (void *)argv[4];

	if (rdev && boot_device != root_device) {
		if (devnum_to_name(root_device, &namep) == 0) {
			printf("Fatal error: unknown root_device 0x%x\n",
			       root_device);
			for (;;)
				continue;
		}
		for (ret = 0; namep[ret] != '\0'; ret++)
			continue;
		pcpy_out(namep, load_addr, ret);
		load_addr += ret;
		if (name == names[i])
			pcpy_out(" ", load_addr, 1);
	}
	for (ret = 0; bootstr[ret] != '\0'; ret++)
		continue;
	pcpy_out(bootstr, load_addr, ret + 1);

	/*
	 *  We now pass the various bootstrap parameters to the loaded
	 *  image via the argument list
	 *
	 *  arg1 = boot flags
	 *  arg2 = boot device
	 *  arg3 = start of symbol table (0 if not loaded)
	 *  arg4 = end of symbol table (0 if not loaded)
	 *  arg5 = transfer address from image
	 *  arg6 = transfer address for next image pointer
	 */
	argv[1] = loadflags;
	argv[2] = boot_device;
	argv[5] = (head.a_entry &= 0xffffff);
	argv[6] = (int) &x_entry;
	argv[7] = cnvmem;
	argv[8] = extmem;
	argv[0] = 8;
	startprog(head.a_entry, argv);
}

char *devs[] = {"hd", "fd", "wt", "sd", "ha", (char *)0};

char *
devname_to_num(char *name,
	       unsigned *num)
{
	char *p, *q;
	unsigned i;
	unsigned ret;

	for (i = 0; devs[i] != (char *)0; i++) {
		p = devs[i];
		q = name;
		while (*p != '\0')
			if (*p != *q++)
				break;
			else
				p++;
		if (*p == '\0' && *q == '(')
			break;
	}
	if (devs[i] == (char *)0)
		return ((char *)0);
	ret = i << B_TYPESHIFT;

	*q++;
	if (*q < '0' || *q > '9')
		return ((char *)0);
	ret |= (*q++ - '0') << B_UNITSHIFT;

	if (*q++ != ',')
		return ((char *)0);
	if (*q >= 'A' && *q <= 'Z')
		ret |= (*q++ - 'A') << B_PARTITIONSHIFT;
	else if  (*q >= 'a' && *q <= 'z')
		ret |= (*q++ - 'a') << B_PARTITIONSHIFT;
	else
		return ((char *)0);

	if (*q != ')')
		return ((char *)0);

	*num = ret;
	return (q + 1);
}

unsigned
devnum_to_name(unsigned num,
	       char **name)
{
	char *p, *q;
	int type, unit, part;
	static char devname[8];

	type = ((num) >> B_TYPESHIFT) & B_TYPEMASK;
	unit = ((num) >> B_UNITSHIFT) & B_UNITMASK;
	part = ((num) >> B_PARTITIONSHIFT) & B_PARTITIONMASK;
	if (type >= sizeof (devs) / sizeof (*devs) || unit >= 10 || part >= 26)
		return 0;

	p = devs[type];
	q = devname;
	while (*q = *p++)
		q++;
	*q++ = '(';
	*q++ = unit + '0';
	*q++ = ',';
	*q++ = 'a' + part;
	*q++ = ')';
	*q = '\0';
	*name = devname;
	return 1;
}

getbootdev(unsigned *device,
	   unsigned *remote)
{
	char *ptr;

	ptr = end_of_data;
	for (;;) {
		flush_char();
		printf("\nboot device: ");
		if (gets(end_of_data) == 0)
			return (0);
		if (*ptr != '?')
			break;
		printf("usage: %s or %s\n",
		       "<device_name>(<drive>,<part>)",
		       "<remote_board>");
	}

	while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	if (*ptr == '\0')
		return (1);

	if (devname_to_num(ptr, device)) {
		*remote = -1;
		return (1);
	}

	/*
	 * Remote boot.
	 */
	*remote = 0;
	while (*ptr >= '0' && *ptr <= '9')
		*remote = *remote * 10 + (*ptr++ - '0');
	return (1);
}
