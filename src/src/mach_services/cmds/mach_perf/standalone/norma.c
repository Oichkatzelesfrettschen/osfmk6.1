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

#include <mach_perf.h>
#include <norma_services.h>
#include <mach/norma_special_ports.h>

get_def_pager(argc, argv)
char *argv[];
{
	mach_port_t port, node_priv_port, vm_port;
	int node;

	if (argc != 2 || (!atod(argv[1], &node))) {
		printf("syntax: get_def_pager <node>\n");
		return;
	}

	if (!kernel_is_norma()) {
		printf("NORMA not configured\n");
		return;
	}

	MACH_CALL(norma_get_host_paging_port, (privileged_host_port,
					      node,
					      &port));

	MACH_CALL(norma_get_host_priv_port, (privileged_host_port,
					      node,
					      &node_priv_port));

	vm_port = MACH_PORT_NULL;
	MACH_CALL(vm_set_default_memory_manager, (node_priv_port,
						  &vm_port));

	printf("Default pager port for node %d: %d\n", node, port);
	if (vm_port != port)
		printf("WARNING: vm_set_default_memory_manager reports %d\n",
		       vm_port);
}

set_def_pager(argc, argv)
char *argv[];
{
	mach_port_t port, node_priv_port, old_port;
	int node, my_node;

	if (argc != 3 || (!atod(argv[1], &node)) || (!atod(argv[2], &port))) {
		printf("syntax: set_def_pager <node> <port name>\n");
		return;
	}


	if (!kernel_is_norma()) {
		printf("NORMA not configured\n");
		return;
	}

	MACH_CALL(norma_get_host_paging_port, (privileged_host_port,
					      node,
					      &old_port));

	MACH_CALL(norma_get_host_priv_port, (privileged_host_port,
					      node,
					      &node_priv_port));

	MACH_CALL(norma_set_host_paging_port, (node_priv_port,
					      port));

	MACH_CALL(norma_node_self, (host_port, &my_node));

	printf("Default pager port for node %d set to %d, was %d\n",
	       node, port, old_port);

}

node()
{
  	int *my_node;

	if (norma_node_self(host_port, &my_node) == KERN_SUCCESS)
		printf("node %d\n", my_node);
	else
		printf("NORMA not configured\n");
}

kernel_is_norma()
{
  	int *my_node;
	if (norma_node_self(host_port, &my_node) == KERN_SUCCESS)
		return 1;
	return 0;
}





