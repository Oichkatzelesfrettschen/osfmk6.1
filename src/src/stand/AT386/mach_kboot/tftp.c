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
 * This file contains TFTP functions used for Network bootstrap.
 */

#include "boot.h"
#include "tftp.h"
#include "udpip.h"
#include "dlink.h"
#include "endian.h"

u16bits tftp_port;			/* current TFTP client port */

static u16bits tftp_waitfor;		/* packet waited for */
static unsigned tftp_retransmit;	/* current TFTP retransmission timer */
static void *tftp_addr;			/* address where copying next data */
static enum tftp_state tftp_state;	/* current TFTP state */
static (*tftp_copy)(unsigned, void *, void **, unsigned);
					/* current copying procedure */
static u8bits tftp_buffer[DLINK_MAXHLEN +
			  sizeof (struct frame_udp) +
			  sizeof (struct frame_ip) +
			  sizeof (struct frame_tftp_4)]; /* Ack buffer */

int
tftp_init()
{
	tftp_port = 0;
}

static int
tftp_output_req(char *file)
{
	union frame_tftp *tp;
	struct udpip_output udpip;
	char *p;
	char *q;

	if (debug)
		printf("Start tftp_output_req('%s')\n", file);

	tp = (union frame_tftp *)&udpip_buffer[dlink.dl_hlen +
					       sizeof (struct frame_ip) +
					       sizeof (struct frame_udp)];

	tp->u1.tftp_opcode = htons(TFTP_OPCODE_READ);
	q = tp->u1.tftp_request;
	p = file;
	while (*q++ = *p++)
		continue;
	p = TFTP_MODE;
	while (*q++ = *p++)
		continue;

	udpip.udpip_sport = tftp_port;
	udpip.udpip_dport = IPPORT_TFTP;
	udpip.udpip_src = udpip_laddr;
	udpip.udpip_dst = udpip_raddr;
	udpip.udpip_len = (q - tp->u1.tftp_request) + sizeof (u16bits);
	udpip.udpip_buffer = udpip_buffer;

	return udpip_output(&udpip);
}

static int
tftp_output_ack(u16bits block)
{
	union frame_tftp *tp;
	struct udpip_output udpip;

	if (debug)
		printf("Start tftp_output_ack(0x%x)\n", block);

	tp = (union frame_tftp *)&tftp_buffer[dlink.dl_hlen +
					      sizeof (struct frame_ip) +
					      sizeof (struct frame_udp)];

	tp->u4.tftp_opcode = htons(TFTP_OPCODE_ACK);
	tp->u4.tftp_block = htons(block);

	udpip.udpip_sport = tftp_port;
	udpip.udpip_dport = udpip_tftp;
	udpip.udpip_src = udpip_laddr;
	udpip.udpip_dst = udpip_raddr;
	udpip.udpip_len = 2 * sizeof (u16bits);
	udpip.udpip_buffer = tftp_buffer;

	return udpip_output(&udpip);
}

static int
tftp_output_error(u16bits error,
		  char *msg)
{
	union frame_tftp *tp;
	struct udpip_output udpip;
	unsigned i;

	if (debug)
		printf("Start tftp_output_error(0x%x, '%s')\n", error, msg);

	tp = (union frame_tftp *)&tftp_buffer[dlink.dl_hlen +
					      sizeof (struct frame_ip) +
					      sizeof (struct frame_udp)];

	tp->u5.tftp_opcode = htons(TFTP_OPCODE_ERROR);
	tp->u5.tftp_errorcode = htons(error);
	for (i = 0; i < 511 && msg[i] != '\0'; i++)
		tp->u5.tftp_errormsg[i] = msg[i];
	tp->u5.tftp_errormsg[i++] = '\0';

	udpip.udpip_sport = tftp_port;
	udpip.udpip_dport = udpip_tftp;
	udpip.udpip_src = udpip_laddr;
	udpip.udpip_dst = udpip_raddr;
	udpip.udpip_len = 2 * sizeof (u16bits) + i;
	udpip.udpip_buffer = tftp_buffer;

	return udpip_output(&udpip);
}

int
tftp_input(struct udpip_input *udpip_input)
{
	union frame_tftp *tp;
	u16bits block;
	unsigned datalen;
	u16bits error;

	if (udpip_input->udpip_len < sizeof (u16bits))
		return (0);
	tp = (union frame_tftp *)udpip_input->udpip_addr;

	if (debug)
		printf("Enter tftp_input(0x%x, 0x%x)\n",
		       tftp_state, tp->u1.tftp_opcode);

	if (tftp_state != TFTP_STATE_RUNNING && tftp_state != TFTP_STATE_START)
		return (0);

	switch (ntohs(tp->u1.tftp_opcode)) {
	case TFTP_OPCODE_DATA:
		if (udpip_input->udpip_len < 2 * sizeof (u16bits))
			return (0);
		block = ntohs(tp->u3.tftp_block);
		if (block != tftp_waitfor)
			return (0);
		(void)tftp_output_ack(block);
		tftp_retransmit = ((tftp_retransmit * 9) + getelapsed()) / 10;
		if (tftp_retransmit == 0)
			tftp_retransmit = 1;
		settimeout(tftp_retransmit);
		tftp_waitfor++;
		datalen = udpip_input->udpip_len - 2 * sizeof (u16bits);
		switch ((*tftp_copy)(block,
				     tp->u3.tftp_data, &tftp_addr, datalen)) {
		case 0:
			if (datalen < TFTP_DATA_MAX) {
				printf("\nError: Kernel Image is too short\n");
				tftp_state = TFTP_STATE_ERROR;
			} else
				tftp_state = TFTP_STATE_RUNNING;
			break;
		case 1:
			tftp_state = TFTP_STATE_ERROR;
			break;
		case 2:
			tftp_state = TFTP_STATE_FINISHED;
			break;
		}
		break;

	case TFTP_OPCODE_ERROR:
		if (udpip_input->udpip_len < 2 * sizeof (u16bits) + 1)
			return (0);
		error = ntohs(tp->u5.tftp_errorcode);
		switch (tftp_state) {
		case TFTP_STATE_START:
			printf("\n%s (error %d: '%s')\n",
			       "Can't receive kernel by TFTP",
			       error, tp->u5.tftp_errormsg);
			break;
		case TFTP_STATE_RUNNING:
			printf("\n%s #%d by TFTP (error %d: '%s')\n",
			       "Can't receive block",
			       tftp_waitfor, error, tp->u5.tftp_errormsg);
			break;
		default:
			printf("\nTFTP error detected (error %d: '%s')\n",
			       error, tp->u5.tftp_errormsg);
			break;
		}
		tftp_state = TFTP_STATE_ERROR;
		break;

	default:
		return (0);
	}
	return (1);
}

int
tftp_main(char *file,
	  void **addr,
	  int (*boot_copy)(unsigned, void *, void **, unsigned))
{
	u16bits waitfor;
	unsigned i;

	if (debug)
		printf("Started tftp_main('%s', 0x%x, 0x%x)\n",
		       file, addr, boot_copy);

	tftp_copy = boot_copy;
	tftp_addr = *addr;
	tftp_waitfor = 1;
	tftp_retransmit = TFTP_RETRANSMIT;
	tftp_state = TFTP_STATE_FINISHED;
	tftp_port = udpip_port++;
	i = 0;
	while (i < TFTP_RETRANSMIT_MAX) {
		if (tftp_state == TFTP_STATE_FINISHED || isatimeout()) {
			tftp_state = TFTP_STATE_START;
			if (debug)
				printf("TFTP initial request\n");
			(void)tftp_output_req(file);
			settimeout(TFTP_RETRANSMIT);
			i++;
		}
		if (tftp_state != TFTP_STATE_START)
			break;
		if (is_intr_char()) {
			tftp_port = 0;
			udpip_abort(udpip_tftp);
			return (2);
		}
		(*dlink.dl_input)(udpip_buffer, IP_MSS);
	}

	switch (tftp_state) {
	case TFTP_STATE_START:
		printf("\nKernel reception by TFTP aborted: %s\n",
		       "Server unreachable");
		break;

	case TFTP_STATE_RUNNING:
		if (debug)
			printf("OK to receive NMK by TFTP:\n");
		waitfor = tftp_waitfor;
		i = 0;
		while (i < TFTP_RETRANSMIT_MAX) {
			if (tftp_state != TFTP_STATE_RUNNING)
				break;
			if (isatimeout()) {
				(void)tftp_output_ack(tftp_waitfor - 1);
				tftp_retransmit *= 2;
				if (tftp_retransmit > TFTP_RETRANSMIT) {
					tftp_retransmit = TFTP_RETRANSMIT;
					i++;
				}
				settimeout(tftp_retransmit);
			}
			if (waitfor != tftp_waitfor) {
				i = 0;
				waitfor = tftp_waitfor;
			}
			if (is_intr_char()) {
				tftp_output_error(0, "Transfert aborted");
				printf(" \n");
				tftp_port = 0;
				udpip_abort(udpip_tftp);
				return (2);
			}
			(*dlink.dl_input)(udpip_buffer, IP_MSS);
		}
		if (i == TFTP_RETRANSMIT_MAX)
			printf("\nKernel reception by TFTP aborted: %s\n",
			       "Connection timed out");
		break;

	default:
		break;
	}

	tftp_port = 0;
	udpip_abort(udpip_tftp);

	if (tftp_state != TFTP_STATE_FINISHED)
		return (1);

	*addr = tftp_addr;
	return (0);
}
