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

#define TEST_NAME	pager

#include <mach_perf.h>
#include <pager_types.h>

extern	pager_server_main();
int	vm_size;

char *private_options = "\n\
\t[-mem <% memory_use>]    Percentage of free+active memory to use.\n\
\t                         (default is 60 %)\n\
\t[-show_server]           Show server side timings.\n\
\t[-client]                Only start client side task.\n\
\t[-server]                Only start server side task.\n\
\t[-norma <node>]          Start server task on node <node>\n\
";

int vm_map_test(boolean_t);
int pager_rw_test();

#define DEALLOCATE 1

#define READ 0
#define WRITE 1
#define LOCK 2

#define PRECIOUS 1
#define TEMPORARY 1

struct test tests[] = {
#if 1
"vm_map",			0, vm_map_test, 0, 0, 0, 0,
"vm_map/deallocate",		0, vm_map_test, DEALLOCATE, 0, 0, 0,
#endif
"page read",			0, pager_rw_test, READ, 0, TEMPORARY, 0,
"page write",			0, pager_rw_test, WRITE, 0, TEMPORARY, 0,
"read/write with RO lock",	0, pager_rw_test, LOCK, 0, TEMPORARY, 0,
"tests with precious pages (pager gives away pages)",	0, 0, 0, 0, 0, 0,
"page read",			0, pager_rw_test, READ, PRECIOUS, TEMPORARY, 0,
"page write",			0, pager_rw_test, WRITE, PRECIOUS, TEMPORARY, 0,
"read/write with RO lock",	0, pager_rw_test, LOCK, PRECIOUS, TEMPORARY, 0,
"tests with permanent objects (not temporary)",	0, 0, 0, 0, 0, 0,
"page read",			0, pager_rw_test, READ, 0, 0, 0,
"page write",			0, pager_rw_test, WRITE, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0
};

main(argc, argv)
int argc;
char *argv[];
{
	int i;

	test_init();

	for (i = 1; i < argc; i++)
	       if (!strcmp(argv[i], "-client")) {
                        client_only = 1;
                } else if (!strcmp(argv[i], "-server")) {
                        server_only = 1;
                } else if (!strcmp(argv[i], "-show_server")) {
                        server_times = 1;
		} else if (!strcmp(argv[i], "-mem")) {
			if (++i >= argc || *argv[i] == '-')
				usage();
		  	mem_use = atoi(argv[i]); 
			use_opt = 1;
		} else if (!strcmp(argv[i], "-norma")) {
			if (++i >= argc || *argv[i] == '-')
				usage();
			if (!atod(argv[i], &norma_node))
				usage();
		} else if (!is_gen_opt(argc, argv, &i, tests, private_options))
                        usage();

	if (!client_only)
        	run_server(pager_server_main);
	else {
		server = server_lookup(PAGER_SERVER_NAME);
		MACH_CALL(set_printer, (server, printf_port));
	}

        if (server_only)
                return(0);

	vm_size = mem_size();
	run_tests(tests);
	
	if (!client_only)
                kill_server(server);
}

vm_map_test(boolean_t deallocate)
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t *mobjs, *mobj;
	vm_offset_t *addrs, *addr;
	mach_msg_type_number_t	count;

	MACH_CALL(vm_allocate, (mach_task_self(),
				(vm_offset_t *)&addrs,
				loops*sizeof(vm_offset_t),
				TRUE));

	MACH_CALL(create_memory_objects, (server,
					  loops,
					  &mobjs,
					  &count,
					  vm_page_size,
					  FALSE,
					  VM_PROT_ALL,
					  FALSE, 	/* precious */
					  TRUE		/* temporary */
					  ));
					 
	if (debug > 1)
		printf("vm_map_test, count %d, mobjs (%x) = %x\n",
		       count, mobjs, *mobjs);
	
	START_TIME();

	for (i= 0, mobj = mobjs, addr = addrs;
	     i < loops;
	     i++, mobj++, addr++)  {
		MACH_CALL(vm_map, (task,
				   addr,
				   vm_page_size,
				   0,		/* mask */
				   TRUE,	/* anywhere */
				   *mobj,
				   0,		/* offset */
				   FALSE,	/* copy */
				   VM_PROT_READ,
				   VM_PROT_ALL,
				   VM_INHERIT_COPY));
		if (deallocate) {
			MACH_CALL(vm_deallocate, (task,
						  (vm_offset_t)*addr,
						  vm_page_size));
		}

	}
	STOP_TIME();

	if (!deallocate)
		for (i=loops, addr = addrs; i--; addr++)  {
			MACH_CALL(vm_deallocate, (task,
						  *addr,
						  vm_page_size));
		}
	MACH_CALL(vm_deallocate, (task,
				  (vm_offset_t)addrs,
				  loops*sizeof(vm_offset_t)));

	if (debug > 1)
		printf("vm_map_test, mobjs (%x) = %x\n", mobjs, *mobjs);
	MACH_CALL(destroy_memory_objects, (server,
					  mobjs,
					  loops));
					 
}

pager_rw_test(op, precious, temporary)
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t *mobj;
	vm_offset_t map_addr, addr;
	mach_msg_type_number_t	count;
	vm_prot_t prot = VM_PROT_NONE;

	set_max_loops(precious ? vm_size : vm_size/2);

	switch(op) {
	case WRITE:
		prot |= VM_PROT_WRITE;
	case READ:
	case LOCK:
		prot |= VM_PROT_READ;
	}

	MACH_CALL(create_memory_objects, (server,
					  1,
					  &mobj,
					  &count,
					  vm_page_size*loops,
					  TRUE,
					  prot,
					  precious,
					  temporary));
					 
#define RW_MAP()					\
	map_addr = 0;					\
	MACH_CALL(vm_map, (task,			\
			   &map_addr,			\
			   vm_page_size*loops,		\
			   0,		/* mask */	\
			   TRUE,	/* anywhere */	\
			   *mobj,			\
			   0,		/* offset */	\
			   FALSE,	/* copy */	\
			   VM_PROT_ALL,			\
			   VM_PROT_ALL,			\
			   VM_INHERIT_COPY));


#define RW_TEST()							\
	for (i = 0, addr = map_addr; 					\
	     i < loops;							\
	     i++, addr+= vm_page_size)  {				\
		if (op != WRITE) {					\
		    if (*(int *)addr != READ_PATTERN)			\
			printf("read: invalid content (%x instead of %x)\n",\
			       *(int *)addr, READ_PATTERN);		\
	        }							\
		if (op != READ)						\
		    *(int *)addr = WRITE_PATTERN;			\
	}

	if (!temporary) {
		RW_MAP();
		RW_TEST();
		MACH_CALL(vm_deallocate, (task, map_addr, vm_page_size*loops));
	}

	RW_MAP();

	if (debug)
		printf("map addr %x\n", map_addr);
	
	if (temporary) {
		START_TIME();
		RW_TEST();
		STOP_TIME();
	} else {
		start_time();
		RW_TEST();
		stop_time();
	}

	MACH_CALL(vm_deallocate, (task, map_addr, vm_page_size*loops));
	MACH_CALL(destroy_memory_objects, (server, mobj, 1));
}









