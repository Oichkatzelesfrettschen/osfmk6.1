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
 * File : udpip.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains UDP/IP functions used for Network bootstrap.
 */

#include "boot.h"
#include "udpip.h"
#include "arp.h"
#include "dlink.h"
#include "endian.h"
#include "tftp.h"

static u16bits ip_id;		/* IP id counter */

u32bits	udpip_laddr;		/* Local IP address */
u32bits	udpip_raddr;		/* Remote IP address */
u8bits	udpip_buffer[IP_MSS];	/* Output buffer */
u16bits udpip_tftp;		/* tftp server port */
u16bits	udpip_port;		/* first udp port available */

void
udpip_init()
{
	udpip_laddr = 0;
	udpip_tftp = 0;
	udpip_port = IPPORT_START;
	tftp_port = 0;
}

void
udpip_abort(u16bits port)
{
	if (udpip_tftp != 0 && port == udpip_tftp)
		udpip_tftp = 0;
}

static u16bits
udpip_cksum(u16bits *addr,
	    u32bits count)
{
        u32bits sum = 0;

        while (count > 1) {
                sum += *addr++;
                count -= 2;
        }
        if (count > 0)
                sum += *(u8bits *)addr;
        while (sum >> 16)
                sum = (sum & 0xFFFF) + (sum >> 16);
        return (~sum);
}

int
udpip_output(struct udpip_output *udpip_output)
{
	struct frame_ip *ip;
	struct frame_udp *up;
	struct pseudo_ip *pp;
	unsigned len;

	ip = (struct frame_ip *)&udpip_output->udpip_buffer[dlink.dl_hlen];
	up = (struct frame_udp *)&udpip_output->udpip_buffer[dlink.dl_hlen +
							     sizeof (struct frame_ip)];
	pp = (struct pseudo_ip *)&udpip_output->udpip_buffer[dlink.dl_hlen +
							     sizeof (struct frame_ip) -
							     sizeof (struct pseudo_ip)];
	pp->ps_src = htonl(udpip_output->udpip_src);
	pp->ps_dst = htonl(udpip_output->udpip_dst);
	pp->ps_zero = 0;
	pp->ps_protocol = IPPROTO_UDP;
	pp->ps_len = htons(udpip_output->udpip_len + sizeof (struct frame_udp));
	up->udp_sport = htons(udpip_output->udpip_sport);
	up->udp_dport = htons(udpip_output->udpip_dport);
	up->udp_len = htons(udpip_output->udpip_len + sizeof (struct frame_udp));
	up->udp_cksum = 0;
	up->udp_cksum = udpip_cksum((u16bits *)pp, udpip_output->udpip_len +
				    sizeof (struct frame_udp) +
				    sizeof (struct pseudo_ip));
	if (up->udp_cksum == 0)
		up->udp_cksum = 0xFFFF;

	len = udpip_output->udpip_len + sizeof (struct frame_ip) +
		sizeof (struct frame_udp);
	ip->ip_version = IP_VERSION;
	ip->ip_hlen = sizeof (struct frame_ip) >> 2;
	ip->ip_tos = IP_TOS;
	ip->ip_len = htons(len);
	ip->ip_id = htons(ip_id++);
	ip->ip_offset = 0;
	ip->ip_ttl = 0xFF;
	ip->ip_protocol = IPPROTO_UDP;
	ip->ip_cksum = 0;
	ip->ip_src = htonl(udpip_output->udpip_src);
	ip->ip_dst = htonl(udpip_output->udpip_dst);
	ip->ip_cksum = udpip_cksum((u16bits *)ip, sizeof (struct frame_ip));

	if (debug)
		printf("End of udpip_output\n");

	switch (dlink.dl_type) {
	case ARPHRD_ETHERNET:
		return (ether_output(udpip_output->udpip_buffer,
				     len,
				     udpip_output->udpip_dst == 0xFFFFFFFF ?
				     ether_broadcast : dlink.dl_raddr,
				     ETHERNET_TYPE_IP));
		/*NOTREACHED*/
	}
	return (1);
}

int
udpip_input(void *addr,
	    unsigned len)
{
	struct frame_ip *ip;
	struct frame_udp *up;
	struct pseudo_ip *pp;
	struct udpip_input udpip_input;
	u16bits cksum;
	unsigned i;

	if (debug) {
		printf("Start udpip_input(0x%x, 0x%x\n", addr, len);
		printf("Packet dump: ");
		for (i = 0; i < 32; i++) {
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
	ip = (struct frame_ip *)addr;

	if (ip->ip_version != IP_VERSION ||
	    (ip->ip_hlen << 2) > len - dlink.dl_hlen ||
	    ip->ip_protocol != IPPROTO_UDP ||
	    udpip_cksum((u16bits *)ip, sizeof (struct frame_ip))) {
		if (debug)
			printf("udpip_input: bad IP header\n");
		return (0);
	}

	ip->ip_len = ntohs(ip->ip_len);
	if (ip->ip_len > len)
		return (0);
	if (ip->ip_len < len)
		udpip_input.udpip_len = ip->ip_len;
	else
		udpip_input.udpip_len = len;
	ip->ip_offset = ntohs(ip->ip_offset);
	if ((ip->ip_offset & IP_MF) || (ip->ip_offset & ~IP_FLAGS)) {
		if (debug)
			printf("udpip_input: IP fragmented\n");
		return (0);
	}
	udpip_input.udpip_dst = ntohl(ip->ip_dst);
	if (udpip_laddr != 0 && udpip_input.udpip_dst != udpip_laddr) {
		if (debug)
			printf("udpip_input: not for us (laddr = %x)\n",
			       udpip_laddr);
		return (0);
	}
	udpip_input.udpip_src = ntohl(ip->ip_src);
	udpip_input.udpip_addr = ((char *)addr) + (ip->ip_hlen << 2) +
		sizeof (struct frame_udp);

	up = (struct frame_udp *)&((char *)addr)[ip->ip_hlen << 2];
	pp = (struct pseudo_ip *)&((char *)addr)[(ip->ip_hlen << 2) -
						 sizeof (struct pseudo_ip)];
	if (up->udp_cksum != 0) {
		pp->ps_src = ip->ip_src;
		pp->ps_dst = ip->ip_dst;
		pp->ps_zero = 0;
		pp->ps_protocol = IPPROTO_UDP;
		pp->ps_len = up->udp_len;
		if (cksum = udpip_cksum((u16bits *)pp, ntohs(up->udp_len) +
				sizeof (struct pseudo_ip))) {
			if (debug)
				printf("udpip_input: bad udp checksum (%x)\n",
				       cksum);
			return (0);
		}
	}
	up->udp_dport = ntohs(up->udp_dport);
	up->udp_sport = ntohs(up->udp_sport);
	up->udp_len = ntohs(up->udp_len);
	if (up->udp_len - sizeof (struct frame_udp) < udpip_input.udpip_len)
		udpip_input.udpip_len = up->udp_len - sizeof (struct frame_udp);

	if (up->udp_dport == IPPORT_BOOTPC &&
	    up->udp_sport == IPPORT_BOOTPS)
		return (bootp_input(&udpip_input));

	if (tftp_port != 0 && tftp_port == up->udp_dport) {
		if (udpip_tftp == 0) {
			udpip_tftp = up->udp_sport;
			if (!tftp_input(&udpip_input))
				udpip_tftp = 0;
			return (1);
		}
		if (up->udp_sport == udpip_tftp)
			return (tftp_input(&udpip_input));
	}

	if (debug)
		printf("udpip_input: %s (sport = 0x%x, dport = 0x%x)\n",
		       "no destination", up->udp_sport, up->udp_dport);
	return (0);
}
