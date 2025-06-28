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
 * File : ns8390.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains ns8390-based boards functions used for Network bootstrap.
 */

#include "boot.h"
#include "ns8390.h"
#include "smc.h"
#include "udpip.h"
#include "arp.h"
#include "dlink.h"
#include "endian.h"

ns8390_reset()
{
	u8bits temp;
	unsigned i;

	(*ns8390.ns_reset)();
	outb(ns8390.ns_nicbase + NS8390_CR, NS8390_STP|NS8390_RD4|NS8390_PS0);
	outb(ns8390.ns_nicbase + NS8390_RBCR0, 0);
	outb(ns8390.ns_nicbase + NS8390_RBCR1, 0);
	while ((inb(ns8390.ns_nicbase + NS8390_ISR) & NS8390_RST) == 0)
		continue;

	if (ns8390.ns_flags & NS8390_FLAGS_SLOT_16BITS) {
#if	BYTE_ORDER == LITTLE_ENDIAN
		temp = NS8390_WTS|NS8390_BOS|NS8390_FT2;
#endif
#if	BYTE_ORDER == BIG_ENDIAN
		temp = NS8390_WTS|NS8390_FT2;
#endif
	} else
		temp = NS8390_FT2;

	outb(ns8390.ns_nicbase + NS8390_DCR, temp);
	outb(ns8390.ns_nicbase + NS8390_RCR, NS8390_MON);
	outb(ns8390.ns_nicbase + NS8390_TCR, NS8390_LB2);
	outb(ns8390.ns_nicbase + NS8390_PSTART, ns8390.ns_pstart);
	outb(ns8390.ns_nicbase + NS8390_PSTOP, ns8390.ns_pstop);
	outb(ns8390.ns_nicbase + NS8390_BNRY, ns8390.ns_pstart);
	outb(ns8390.ns_nicbase + NS8390_ISR, 0xFF);
	outb(ns8390.ns_nicbase + NS8390_IMR, 0);

	outb(ns8390.ns_nicbase + NS8390_CR, NS8390_STP|NS8390_RD4|NS8390_PS1);
	for (i = 0; i < 6; i++)
		outb(ns8390.ns_nicbase + NS8390_PAR + i, dlink.dl_laddr[i]);
	for (i = 0; i < 8; i++)
		outb(ns8390.ns_nicbase + NS8390_MAR + i, 0);

	ns8390.ns_packet = ns8390.ns_pstart + 1;
	outb(ns8390.ns_nicbase + NS8390_CURR, ns8390.ns_packet);
	outb(ns8390.ns_nicbase + NS8390_CR, NS8390_STA|NS8390_RD4|NS8390_PS0);
	outb(ns8390.ns_nicbase + NS8390_RCR, NS8390_AB);
	outb(ns8390.ns_nicbase + NS8390_TCR, 0);

	if (debug) {
		printf("RESET : pstart = %x, pstop = %x, packet = %x",
		       ns8390.ns_pstart, ns8390.ns_pstop, ns8390.ns_packet);
		temp = inb(ns8390.ns_nicbase + NS8390_CR) & NS8390_PS_MASK;
		outb(ns8390.ns_nicbase + NS8390_CR, temp|NS8390_PS1);
		printf(", curr = %x)\n", inb(ns8390.ns_nicbase + NS8390_CURR));
		outb(ns8390.ns_nicbase + NS8390_CR, temp|NS8390_PS0);
	}
}

int
ns8390_rcv(void *addr,
	   unsigned len)
{
	volatile u8bits *ptr;
	u8bits cr;
	unsigned hbc;
	unsigned nic_overcount;
	unsigned pkt_size;
	unsigned wrap_size;
	unsigned ret;
	struct ns8390_input input;

	ptr = ((u8bits *)ns8390.ns_rambase) + (ns8390.ns_packet << 8);
	pcpy_in(ptr, &input, 4);

	if (debug) {
		printf("ns8390_rcv (ptr = 0x%x, stat = 0x%x, next = 0x%x, ",
		       ptr, input.ns_status, input.ns_next);
		cr = inb(ns8390.ns_nicbase + NS8390_CR) & NS8390_PS_MASK;
		outb(ns8390.ns_nicbase + NS8390_CR, cr|NS8390_PS1);
		printf("rbc0 = 0x%x, rbc1 = 0x%x, curr = 0x%x\n",
		       input.ns_lbc, input.ns_hbc,
		       inb(ns8390.ns_nicbase + NS8390_CURR));
		outb(ns8390.ns_nicbase + NS8390_CR, cr|NS8390_PS0);
	}

	if (input.ns_next < ns8390.ns_pstart ||
	    input.ns_next >= ns8390.ns_pstop) {
		ns8390_reset();
		return(1);
	}

	if ((input.ns_lbc + NS8390_HEADER_SIZE) > NS8390_PAGE_SIZE)
		nic_overcount = 2;
	else
		nic_overcount = 1;
	if (input.ns_next > ns8390.ns_packet) {
		wrap_size = 0;
		hbc = input.ns_next - ns8390.ns_packet - nic_overcount;
	} else {
		wrap_size = ((unsigned) (ns8390.ns_pstop -
					 ns8390.ns_packet) << 8) -
						 NS8390_HEADER_SIZE;
		hbc = ns8390.ns_pstop - ns8390.ns_packet +
			input.ns_next - ns8390.ns_pstart - nic_overcount;
	}
	pkt_size = (hbc << 8) | (input.ns_lbc & 0xFF);

	if (debug)
		printf("ns8390_rcv (len = %x, pkt_size = %x, wrap_size = %x)\n",
		       len, pkt_size, wrap_size);

	if (pkt_size < 60) {
		ns8390_reset();
		return (1);
	}

	if (wrap_size > pkt_size)
		wrap_size = 0;

	ptr += NS8390_HEADER_SIZE;
	if (pkt_size <= len)
		if (ns8390.ns_flags & NS8390_FLAGS_SLOT_16BITS) {
			if (ns8390.ns_en16bits)
				(*ns8390.ns_en16bits)();
			if (wrap_size) {
				pcpy16_in(ptr, addr, wrap_size);
				ptr = ((u8bits *)ns8390.ns_rambase) +
					(ns8390.ns_pstart << 8);
			}
			pcpy16_in(ptr, ((char *)addr) + wrap_size,
				  pkt_size - wrap_size);
			if (ns8390.ns_dis16bits)
				(*ns8390.ns_dis16bits)();
		} else {
			if (wrap_size) {
				pcpy_in(ptr, addr, wrap_size);
				ptr = ((u8bits *)ns8390.ns_rambase) +
					(ns8390.ns_pstart << 8);
			}
			pcpy_in(ptr, ((char *)addr) + wrap_size,
				pkt_size - wrap_size);
		}

	if ((ns8390.ns_packet = input.ns_next) == ns8390.ns_pstart) {
		outb(ns8390.ns_nicbase + NS8390_BNRY, ns8390.ns_pstop - 1);
	} else {
		outb(ns8390.ns_nicbase + NS8390_BNRY, input.ns_next - 1);
	}
	if (pkt_size <= len)
		ether_input(addr, pkt_size);

	cr = inb(ns8390.ns_nicbase + NS8390_CR) & NS8390_PS_MASK;
	outb(ns8390.ns_nicbase + NS8390_CR, cr | NS8390_PS1);
	ret = ns8390.ns_packet == inb(ns8390.ns_nicbase + NS8390_CURR);
	outb(ns8390.ns_nicbase + NS8390_CR, cr | NS8390_PS0);
	return (ret);
}

void
ns8390_overwrite(char *addr,
		 unsigned len)
{
	u8bits cr;

	if (ns8390.ns_flags & NS8390_FLAGS_WD83C690) {
		cr = inb(ns8390.ns_nicbase + NS8390_CR) &
			NS8390_PS_MASK & ~NS8390_TXP;
		outb(ns8390.ns_nicbase + NS8390_CR, cr | NS8390_PS1);
		if (ns8390.ns_packet != inb(ns8390.ns_nicbase + NS8390_CURR)) {
			outb(ns8390.ns_nicbase + NS8390_CR, cr | NS8390_PS0);
			while (!ns8390_rcv((char *)0, 0))
				continue;
		} else {
			outb(ns8390.ns_nicbase + NS8390_CR, cr | NS8390_PS0);
			outb(ns8390.ns_nicbase + NS8390_BNRY,
			     inb(ns8390.ns_nicbase + NS8390_BNRY));
		}
	} else {
		outb(ns8390.ns_nicbase + NS8390_CR,
		     NS8390_STP|NS8390_RD4|NS8390_PS0);
		outb(ns8390.ns_nicbase + NS8390_RBCR0, 0);
		outb(ns8390.ns_nicbase + NS8390_RBCR1, 0);
		while ((inb(ns8390.ns_nicbase + NS8390_ISR) & NS8390_RST) == 0)
			continue;
		outb(ns8390.ns_nicbase + NS8390_TCR, NS8390_LB1);
		while (!ns8390_rcv((char *)0, 0))
			continue;
		outb(ns8390.ns_nicbase + NS8390_RCR, NS8390_AB);
		outb(ns8390.ns_nicbase + NS8390_TCR, 0);
	}
}

void
ns8390_input(void * addr,
	     unsigned len)
{
	u8bits status;

	status = inb(ns8390.ns_nicbase + NS8390_ISR);

	if (status & NS8390_RXE) {
		/*
		 * Error on packet received. Just clear it !
		 */
		if (debug &&
		    (inb(ns8390.ns_nicbase + NS8390_RSR) & NS8390_PRX) == 0)
			printf("NS8390 input error (status = 0x%x)\n",
			       inb(ns8390.ns_nicbase + NS8390_RSR));
		outb(ns8390.ns_nicbase + NS8390_ISR, NS8390_RXE);
	}
	if (status & NS8390_OVW) {
		/*
		 * Received buffer overwritten.
		 */
		if (debug)
			printf("Start ns8390_overwrite(status = 0x%x)\n",
			       status);
		outb(ns8390.ns_nicbase + NS8390_ISR, NS8390_OVW);
		ns8390_overwrite(addr, len);
	}
	if (status & NS8390_PRX) {
		/*
		 * Packet has been received.
		 */
		if (debug)
			printf("Start ns8390_rcv(status = 0x%x)\n", status);
		if (ns8390_rcv(addr, len))
			outb(ns8390.ns_nicbase + NS8390_ISR, NS8390_PRX);
	}
}

int
ns8390_output(void *addr,
	      unsigned len)
{
	volatile u8bits *ptr;
	u8bits status;
	unsigned i;

	if (debug)
		printf("Start ns8390_output(0x%x, 0x%x)\n", addr, len);

	ptr =  (u8bits *)ns8390.ns_rambase + ((ns8390.ns_txt << 8) & 0xFFFF);

	if (ns8390.ns_flags & NS8390_FLAGS_SLOT_16BITS) {
		if (debug)
			printf("pcpy16 (0x%x, 0x%x, 0x%x)\n", addr, ptr, len);
		if (ns8390.ns_en16bits)
			(*ns8390.ns_en16bits)();
		pcpy16_out(addr, ptr, len);
		if (ns8390.ns_dis16bits)
			(*ns8390.ns_dis16bits)();
	} else {
		if (debug)
			printf("pcpy (0x%x, 0x%x, 0x%x)\n", addr, ptr, len);
		pcpy_out(addr, ptr, len);
	}
	if (len < NS8390_MIN_LENGTH)
		len = NS8390_MIN_LENGTH;

	if (debug)
		printf("Start packet transmission @ 0x%x\n", ns8390.ns_nicbase);
	outb(ns8390.ns_nicbase + NS8390_CR, NS8390_RD4|NS8390_STA);
	outb(ns8390.ns_nicbase + NS8390_TPSR, ns8390.ns_txt);
	outb(ns8390.ns_nicbase + NS8390_TBCR0, len & 0xFF);
	outb(ns8390.ns_nicbase + NS8390_TBCR1, len >> 8);
	outb(ns8390.ns_nicbase + NS8390_CR, NS8390_TXP|NS8390_RD4|NS8390_STA);

	for (;;) {
		status = inb(ns8390.ns_nicbase + NS8390_ISR);
		if (status & (NS8390_PTX|NS8390_TXE)) {
			if (debug)
				printf("Sent ns8390_output(status = %x)\n",
				       status);
			outb(ns8390.ns_nicbase + NS8390_ISR,
			     NS8390_PTX|NS8390_TXE);
			break;
		}
	}

	if (debug) {
		printf("Packet dump: ");
		for (i = 0; i < 128; i++) {
			u8bits val = ((char *)addr)[i];
			if ((val >> 4) >= 10)
				putchar((val >> 4) - 10 + 'A');
			else
				putchar((val >> 4) + '0');
			if ((val & 0xF) > 9)
				putchar((val & 0xF) - 10 + 'A');
			else
				putchar((val & 0xF) + '0');
			putchar(' ');
		}
		putchar('\n');
	}
}
