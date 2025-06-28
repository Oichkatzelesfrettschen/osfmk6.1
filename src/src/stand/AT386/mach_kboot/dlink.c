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
 * File : dlink.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains all data link definitions used for Network bootstrap.
 */

#include "boot.h"
#include "dlink.h"
#include "endian.h"

struct dlink dlink;	/* local data link informations */

u8bits ether_broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

int
ether_output(void *addr,
	     unsigned len,
	     u8bits dst[6],
	     u16bits type)
{
	struct frame_ether *ep;
	unsigned i;

	if (debug)
		printf("%x (0x%x, 0x%x, %d:%d:%d:%d:%d:%d, 0x%x)\n",
		       "Start ether_output", addr, len, dst[0], dst[1],
		       dst[2], dst[3], dst[4], dst[5], type);

	ep = (struct frame_ether *)addr;
	for (i = 0; i < 6; i++)
		ep->ether_dst[i] = dst[i];
	for (i = 0; i < 6; i++)
		ep->ether_src[i] = dlink.dl_laddr[i];
	ep->ether_type = htons(type);

	if (debug) {
		printf("Packet dump: ");
		for (i = 0; i < 42; i++) {
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
	return ((*dlink.dl_output)(addr, len + dlink.dl_hlen));
}

void
ether_input(void *addr,
	    unsigned len)
{
	struct frame_ether *ep;
	unsigned i;

	if (debug)
		printf("Start ether_input(0x%x, 0x%x)\n", addr, len);

	ep = (struct frame_ether *)addr;
	if (len < sizeof (struct frame_ether))
		return;
	ep->ether_type = ntohs(ep->ether_type);
	switch (ep->ether_type) {
	case ETHERNET_TYPE_IP:
		udpip_input(&((char *)addr)[ETHERNET_HLEN],
			    len - ETHERNET_HLEN);
		break;
	case ETHERNET_TYPE_ARP:
		arp_input(&((char *)addr)[ETHERNET_HLEN], len - ETHERNET_HLEN);
		break;
	default:
		if (debug)
			printf("Unknown Ethernet Type : %x\n", ep->ether_type);
		break;
	}
}
