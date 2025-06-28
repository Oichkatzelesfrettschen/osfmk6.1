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
 * File : bootp.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains BOOTP functions used for Network bootstrap.
 */

#include "boot.h"
#include "bootp.h"
#include "udpip.h"
#include "dlink.h"
#include "endian.h"
#include "arp.h"

static u32bits bootp_xid;
static char bootp_file[128];
static char bootp_vend[64];
static u32bits bootp_giaddr;
static u8bits bootp_hops;
static char *bootp_rootpath;
static u32bits bootp_waiting;

void
bootp_init()
{
	bootp_waiting = 0;
}

static int
bootp_output(char *file)
{
	struct frame_bootp *bp;
	struct udpip_output udpip;
	unsigned i;

	if (debug)
		printf("Start bootp_output('%s')\n", file);

	bp = (struct frame_bootp *)&udpip_buffer[dlink.dl_hlen +
						 sizeof (struct frame_ip) +
						 sizeof (struct frame_udp)];

	bp->bootp_op = BOOTP_OP_REQUEST;
	bp->bootp_htype = dlink.dl_type;
	bp->bootp_hlen = dlink.dl_len;
	bp->bootp_hops = 0;
	bp->bootp_xid = htonl(bootp_xid);
	bp->bootp_secs = 0;
	bp->bootp_filler = 0;
	bp->bootp_ciaddr = 0;
	bp->bootp_yiaddr = 0;
	bp->bootp_siaddr = 0;
	bp->bootp_giaddr = 0;
	for (i = 0; i < dlink.dl_len; i++)
		bp->bootp_chaddr[i] = dlink.dl_laddr[i];
	for (i = 0; i < 64; i++)
		bp->bootp_sname[i] = '\0';
	for (i = 0; i < 128 && file[i] != '\0'; i++)
		bp->bootp_file[i] = file[i];
	while (i < 128)
		bp->bootp_file[i++] = '\0';
	bp->bootp_cookie = htonl(BOOTP_COOKIE);
	bp->bootp_vend[0] = BOOTP_TAG_END;
	for (i = 1; i < 64; i++)
		bp->bootp_vend[i] = '\0';

	udpip.udpip_sport = IPPORT_BOOTPC;
	udpip.udpip_dport = IPPORT_BOOTPS;
	udpip.udpip_src = 0;
	udpip.udpip_dst = 0xFFFFFFFF;
	udpip.udpip_len = sizeof (struct frame_bootp);
	udpip.udpip_buffer = udpip_buffer;

	return udpip_output(&udpip);
}

int
bootp_input(struct udpip_input *udpip_input)
{
	struct frame_bootp *bp;
	unsigned i;

	if (debug)
		printf("Start bootp_input(0x%x)\n", udpip_input);

	if (bootp_waiting == 0) {
		if (debug)
			printf("bootp_input: received unexpected packet\n");
		return (0);
	}
	bp = (struct frame_bootp *)udpip_input->udpip_addr;
	if (bp->bootp_op != BOOTP_OP_REPLY ||
	    bp->bootp_htype != dlink.dl_type ||
	    bp->bootp_hlen != dlink.dl_len) {
		if (debug)
			printf("bootp_input: Bad operation\n");
		return (0);
	}
	bp->bootp_xid = ntohl(bp->bootp_xid);
	if (bp->bootp_xid != bootp_xid)
		return (0);
	bp->bootp_ciaddr = ntohl(bp->bootp_ciaddr);
	bp->bootp_yiaddr = ntohl(bp->bootp_yiaddr);
	if (bp->bootp_ciaddr && (bp->bootp_ciaddr != bp->bootp_yiaddr ||
				 udpip_laddr != bp->bootp_yiaddr)) {
		if (debug)
			printf("bootp_input: Bad address\n");
		return (0);
	}

	for (i = 0; i < 128; i++)
		bootp_file[i] = bp->bootp_file[i];
	for (i = 0; i < 64; i++)
		bootp_vend[i] =  bp->bootp_vend[i];
	if (udpip_laddr == 0)
		udpip_laddr = bp->bootp_yiaddr;
	udpip_raddr = ntohl(bp->bootp_siaddr);
	if ((bootp_hops = bp->bootp_hops) > 0 && bp->bootp_giaddr != 0)
		bootp_giaddr = ntohl(bp->bootp_giaddr);
	else
		bootp_giaddr = udpip_raddr;
	bootp_waiting = 0;
	return (1);
}

int
bootp_copy(unsigned block,
	   void *inaddr,
	   void **outaddr,
	   unsigned len)
{
	pcpy_out(inaddr, *outaddr, len);
	*outaddr = *(char **)outaddr + len;
}

int
bootp_vendor(unsigned char *addr,
	     void *load)
{
	unsigned char c;
	unsigned i;
	unsigned char buffer[257];

	if (debug)
		printf("Start bootp_vendor(0x%x, 0x%x)\n", addr, load);

	for (;;) {
		if (load)
			c = *addr++;
		else
			pcpy_in(addr, &c, 1);

		switch(c) {
		case BOOTP_TAG_PAD:
			continue;
		case BOOTP_TAG_END:
			return (0);
		default:
			if (load)
				i = *addr++;
			else
				pcpy_in(addr, &i, 1);
			break;
		}

		switch (c) {
		case BOOTP_TAG_ROOTPATH:
			if (bootp_rootpath != (char *)0)
				printf("Duplicate root path : '%s'\n", addr);
			else {
				bootp_rootpath = end_of_data;
				end_of_data = &bootp_rootpath[i+1];
				if (load)
					bcopy(addr, bootp_rootpath, i);
				else
					pcpy_in(addr, bootp_rootpath, i);
				bootp_rootpath[i] = '\0';
			}
			break;

		case BOOTP_TAG_EXTENSIONS:
			if (load) {
				bcopy(addr, buffer, i);
				buffer[i] = '\0';
				printf("%s from %d.%d.%d.%d",
				       "Bootp extension downloaded from",
				       (udpip_raddr >> 24) & 0xFF,
				       (udpip_raddr >> 16) & 0xFF,
				       (udpip_raddr >> 8) & 0xFF,
				       udpip_raddr & 0xFF, buffer);
				if (bootp_hops > 0 && bootp_giaddr != 0)
					printf(" via %d.%d.%d.%d", 
					       (bootp_giaddr >> 24) & 0xFF,
					       (bootp_giaddr >> 16) & 0xFF,
					       (bootp_giaddr >> 8) & 0xFF,
					       bootp_giaddr & 0xFF);
				putchar('\n');
				switch (tftp_main(buffer, load, bootp_copy)) {
				case 0:
					bootp_vendor(load, (void *)0);
					break;
				case 1:
					printf("Extension file not found\n");
					return (1);
				case 2:
					return (2);
				}
			} else
				printf("Recursive extension path is ignored\n");
			break;
		}
		addr += i;
	}
	return (0);
}

bootp_main(char **file,
	   unsigned *root_device,
	   int rdev,
	   unsigned bootflags,
	   void *addr)
{
	unsigned bootp_retransmit;
	unsigned num;
	char *devname;
	int ret;
	unsigned i;
	unsigned waiting;

	if (debug)
		printf("Start bootp_main(0x%x, 0x%x, 0x%x, 0x%x)\n",
		       file, root_device, bootflags, addr);

	for (i = 0; (*file)[i] != '\0'; i++)
		continue;
	if (i >= 128) {
		printf("Boot file name too long\n");
		return (1);
	}

	bootp_rootpath = (char *)0;
	printf("Send bootp request for '%s'", **file ? *file : BOOTP_DEFAULT);

	bootp_waiting = 1;
	(void)bootp_output(*file);
	settimeout(BOOTP_RETRANSMIT);
	waiting = BOOTP_RETRANSMIT;
	bootp_retransmit = BOOTP_RETRANSMIT << 1;
	while (bootp_waiting) {
		if (isatimeout()) {
			if ((waiting += bootp_retransmit) > 60000)
				return (1);
			putchar('.');
			(void)bootp_output(*file);
			settimeout(bootp_retransmit);
			bootp_retransmit <<= 1;
		}
		if (is_intr_char()) {
			putchar('\n');
			return (2);
		}
		(*dlink.dl_input)(udpip_buffer, IP_MSS);
	}
	*file = bootp_file;
	if (bootp_file[0] == '\0')
		return (0);

	printf("\nBooting from %d.%d.%d.%d",
	       (udpip_raddr >> 24) & 0xFF, (udpip_raddr >> 16) & 0xFF,
	       (udpip_raddr >> 8) & 0xFF, udpip_raddr & 0xFF);
	if (bootp_hops > 0 && bootp_giaddr != 0)
		printf(" via %d.%d.%d.%d", 
		       (bootp_giaddr >> 24) & 0xFF, (bootp_giaddr >> 16) & 0xFF,
		       (bootp_giaddr >> 8) & 0xFF, bootp_giaddr & 0xFF);

	switch (arp_main(bootp_giaddr)) {
	case 0:
		break;
	case 1:
		printf("\nImpossible to get ARP answer for %d.%d.%d.%d\n",
		       (bootp_giaddr >> 24) & 0xFF, (bootp_giaddr >> 16) & 0xFF,
		       (bootp_giaddr >> 8) & 0xFF, bootp_giaddr & 0xFF);
		return (1);
	case 2:
		return (2);
	}
	if (ret = bootp_vendor(bootp_vend, addr))
		return (ret);

	if (rdev == 0 || bootp_rootpath == (char *)0) {
		if (devnum_to_name(*root_device, &devname))
			printf(" : %s", devname);
		else
			printf(" : '0x%x'", *root_device);
		printf("%s", bootp_file);
		printflags(bootflags);

	} else if (devname_to_num(bootp_rootpath, &num) == (char *)0) {
		if (devnum_to_name(*root_device, &devname))
			printf(" : %s", devname);
		else
			printf(" : '0x%x'", *root_device);
		printf("%s", bootp_file);
		printflags(bootflags);
		if (debug)
			printf("\n%s '%s' (default to '%s')",
			       "Warning: BOOTP gave an unknown root device",
			       bootp_rootpath, devname);

	} else {
		printf(" : %s%s", bootp_rootpath, bootp_file);
		printflags(bootflags);
		*root_device = num;
	}
	putchar('\n');
	return (0);
}
