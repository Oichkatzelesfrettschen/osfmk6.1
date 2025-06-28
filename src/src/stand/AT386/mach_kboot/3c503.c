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
 * File : 3c503.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains the boards functions of 3c503 used for Network bootstrap.
 */

#include "boot.h"
#include "ns8390.h"
#include "3c503.h"
#include "udpip.h"
#include "arp.h"
#include "dlink.h"

#define	_3C503_XSEL	_3C503_CR_XSEL		/* Default Transciever == BNC */

i386_ioport_t _3c503_addr[] = {
	0x2E0,
	0x2A0,
	0x280,
	0x250,
	0x350,
	0x330,
	0x310,
	0x300,
	0,
};

void
_3c503_reset()
{
	ns8390.ns_pstart = 0x26;
	ns8390.ns_pstop = 0x40;
	ns8390.ns_txt = 0x20;

	if (debug)
		printf("Start _3c503_reset()\n");

	outb(ns8390.ns_iobase + _3C503_GA_CR, _3C503_CR_XSEL | _3C503_CR_RST);
	outb(ns8390.ns_iobase + _3C503_GA_CR, _3C503_CR_XSEL);
	while (inb(ns8390.ns_iobase + _3C503_GA_CR) !=
	       (_3C503_CR_XSEL | _3C503_CR_EAHI))
		continue;

	outb(ns8390.ns_iobase + _3C503_GA_CR, _3C503_CR_XSEL);
	outb(ns8390.ns_iobase + _3C503_GA_PSTR, ns8390.ns_pstart);
	outb(ns8390.ns_iobase + _3C503_GA_PSPR, ns8390.ns_pstop);
	outb(ns8390.ns_iobase + _3C503_GA_GACFR, _3C503_GACFR_NIM |
	     _3C503_GACFR_TCM | _3C503_GACFR_RSEL | _3C503_GACFR_MSB0);
}

int
probe_3c503()
{
	unsigned i;
	unsigned rev;

	for (i = 0; _3c503_addr[i] != 0; i++) {
		if (debug)
			printf("Started looking for a 3C503 board @ 0x%x\n",
			       _3c503_addr[i]);

		if (inb(_3c503_addr[i] + _3C503_GA_BCFR) == (1 << i)) {
			switch(inb(_3c503_addr[i] + _3C503_GA_PCFR)) {
			case 0x10:
				ns8390.ns_rambase = (void *)0xC8000;
				break;
			case 0x20:
				ns8390.ns_rambase = (void *)0xCC000;
				break;
			case 0x40:
				ns8390.ns_rambase = (void *)0xD8000;
				break;
			case 0x80:
				ns8390.ns_rambase = (void *)0xDC000;
				break;
			default:
				continue;
			}
			break;
		}
	}

	if (_3c503_addr[i] == 0)
		return (0);

	printf("Found 3C503 rev 0x%x, 8 kB ram\n",
	       inb(_3c503_addr[i] + _3C503_GA_STREG) & _3C503_STREG_REV);

	outb(_3c503_addr[i] + _3C503_GA_CR, _3C503_CR_XSEL | _3C503_CR_RST);
	outb(_3c503_addr[i] + _3C503_GA_CR, _3C503_CR_XSEL);
	while (inb(_3c503_addr[i] + _3C503_GA_CR) !=
	       (_3C503_CR_XSEL | _3C503_CR_EAHI))
		continue;

	outb(_3c503_addr[i] + _3C503_GA_CR, _3C503_CR_XSEL | _3C503_CR_EALO);
	dlink.dl_laddr[0] = inb(_3c503_addr[i] + _3C503_LAR_0);
	dlink.dl_laddr[1] = inb(_3c503_addr[i] + _3C503_LAR_1);
	dlink.dl_laddr[2] = inb(_3c503_addr[i] + _3C503_LAR_2);
	dlink.dl_laddr[3] = inb(_3c503_addr[i] + _3C503_LAR_3);
	dlink.dl_laddr[4] = inb(_3c503_addr[i] + _3C503_LAR_4);
	dlink.dl_laddr[5] = inb(_3c503_addr[i] + _3C503_LAR_5);
	dlink.dl_hlen = 14;
	dlink.dl_type = ARPHRD_ETHERNET;
	dlink.dl_len = 6;
	dlink.dl_output = ns8390_output;
	dlink.dl_input = ns8390_input;

	/*
	 * For 8K configuration, NIC page 20 is the first SRAM address.
	 */
	ns8390.ns_rambase = (void *)(((unsigned)ns8390.ns_rambase) - 0x2000);
	ns8390.ns_ramsize = 8 * 1024;
	ns8390.ns_flags = 0;

	/*
	 * Complete generic ns8390 initialization.
	 */
	ns8390.ns_iobase = _3c503_addr[i];
	ns8390.ns_nicbase = _3c503_addr[i];
	ns8390.ns_reset = _3c503_reset;
	ns8390.ns_en16bits = (void (*)(void))0;
	ns8390.ns_dis16bits = (void (*)(void))0;
	ns8390_reset();

	return (1);
}
