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
 *	File: netmem_client.c
 *	Author: David L. Black
 *
 *	Simple client to demonstrate netmem server.
 */

#include <mach.h>
#include <mach_init.h>
#include <servers/netname.h>
#include <stdio.h>

#include "netmem_defs.h"

main()
{
	kern_return_t	r;
	mach_port_t	server_port;
	vm_address_t	netmem_address;

	r = netname_look_up(name_server_port, "", NETMEM_NAME, &server_port);
	if (r != NETNAME_SUCCESS) {
		mach_error("Server Look Up", r);
		exit(-1);
	}

#ifdef	DEBUG
	printf("server_port %s 0x%x\n", NETMEM_NAME, server_port);
#endif

	/*
	 *	Deliberately map in an extra page to exercise error path.
	 */
	netmem_address = 0;
	r = vm_map(mach_task_self(), &netmem_address,
		   2*vm_page_size, 0, TRUE, server_port, 0, FALSE,
		   VM_PROT_ALL, VM_PROT_ALL, VM_INHERIT_NONE);
	if (r != KERN_SUCCESS) {
		mach_error("Object vm_map", r);
		exit(-1);
	}
	printf("Mapped at 0x%x\n", netmem_address);

	/*
	 *	Simple command format:
	 *		Letter offset data
	 *	Letter: p[ut], g[et], r[ead], w[rite]
	 *	put and get are messages, read and write are memory.
	 */
	printf("Server contacted.  pgrw = put/get/read/write x = exit\n");

	for (;;) {

		char cmd;
		int  c, offset, data;

		printf("pgrw> ");
		scanf("%c %d", &cmd, &offset);
		switch (cmd) {
		    case 'p':
			scanf("%d", &data);
			printf("put %d = %d", offset, data);
			r = netmem_write(server_port, offset, data);
			printf("     result %d\n", r);
			break;

		    case 'g':
			printf("get %d", offset);
			r = netmem_read(server_port, offset, &data);
			printf(" = %d     result %d\n", data, r);
			break;

		    case 'r':
		    	data = *( ((int *) netmem_address) + offset);
			printf("read %d = %d\n", offset, data);
			break;

		   case 'w':
			scanf("%d", &data);
		        *( ((int *)netmem_address) + offset) = data;
    			printf("write %d = %d\n", offset, data);
			break;

		    case 'x':
		    	exit(1);

		    default:
		        printf("???\n");
	        }

		while ((c = getc(stdin)) != '\n');
	}
	/*NOTREACHED*/
}
