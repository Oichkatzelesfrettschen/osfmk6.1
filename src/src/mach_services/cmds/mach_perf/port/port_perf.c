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

#define TEST_NAME	port

#include <mach_perf.h>

int port_allocate_and_destroy_test();
int port_allocate_or_destroy_test();
int name_allocate_and_destroy_test();
int name_allocate_or_destroy_test();
int reply_port_test();
int reply_port_and_destroy_test();
int port_insert_right_and_deallocate_test();
int port_insert_right_or_deallocate_test();

char *private_options = "\n\
\t[-first_port <name>]     When using non generic port names\n\
\t                         the value <name> is used as the first name\n\
\t                         (default is 50000)\n\
\t[-max_port <count>]      Maximum port names count for IPC space\n\
\t                         is <count> (default is 20000)\n\
";

#define ALLOCATE	0
#define DEALLOCATE	1
mach_port_t port_first_port_name = 50000;	/* !! update help message */
int port_max_port_names = 20000; 		/* !! update help message */

struct test tests[] = {
"port_allocate",		0, port_allocate_or_destroy_test,
				ALLOCATE, 0, 0, 0,
"port_destroy",			0, port_allocate_or_destroy_test,
				DEALLOCATE, 0, 0, 0,
"port_allocate/destroy",	0, port_allocate_and_destroy_test,
				0, 0, 0, 0,
"port_allocate_name",		0, name_allocate_or_destroy_test,
				ALLOCATE, 0, 0, 0,
"port_allocate_name/destroy",	0, name_allocate_and_destroy_test,
				0, 0, 0, 0,
"reply_port",			0, reply_port_test, 0, 0, 0, 0,
"reply_port/destroy",		0, reply_port_and_destroy_test,
				0, 0, 0, 0,
"port_insert_right/deallocate",	0, port_insert_right_and_deallocate_test,
				0, 0, 0, 0,
"port_insert_right",		0, port_insert_right_or_deallocate_test,
				ALLOCATE, 0, 0, 0,
"port_deallocate",		0, port_insert_right_or_deallocate_test,
				DEALLOCATE, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0
};

main(argc, argv)
	int argc;
	char *argv[];
{
	int i;
	mach_port_t port;
	struct test *test = tests;
	int oloops;

	test_init();
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-first_port")) {
			if (++i >= argc || *argv[i] == '-')
				usage();
		  	port_first_port_name = atoi(argv[i]); 
		} else if (!strcmp(argv[i], "-max_port")) {
			if (++i >= argc || *argv[i] == '-')
				usage();
		  	port_max_port_names = atoi(argv[i]); 
		} else if (!is_gen_opt(argc, argv, &i, tests, private_options))
			usage();

	run_tests(tests);
}

port_allocate_and_destroy_test()
{
	register i;
	mach_port_t port;
	mach_port_t task = mach_task_self();

	start_time();

	for (i=loops; i--;)  {
		MACH_CALL(mach_port_allocate, (task,
					 MACH_PORT_RIGHT_RECEIVE,
					 &port));
		MACH_CALL(mach_port_destroy, (task, port));
	}
	stop_time();
}

port_allocate_or_destroy_test(op)
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t *ports, *port;

	set_max_loops(port_max_port_names);
	MACH_CALL(vm_allocate, (mach_task_self(), (vm_offset_t *)&ports,
				loops*sizeof(mach_port_t), TRUE));

	if (op == ALLOCATE)
		start_time();

	for (i=loops, port = ports; i--; port++)  {
		MACH_CALL(mach_port_allocate, (task,
					       MACH_PORT_RIGHT_RECEIVE,
					       port));
	}

	if (op == ALLOCATE)
		stop_time();
	if (op == DEALLOCATE)
		start_time();

	for (i=loops, port = ports; i--; port++) 
		MACH_CALL(mach_port_destroy, (task, *port));

	if (op == DEALLOCATE)
		stop_time();
	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t )ports,
				  loops*sizeof(mach_port_t)));
	
	test_cache(port_allocate_or_destroy_test, (op));
}

name_allocate_and_destroy_test()
{
	register i;
	mach_port_t port = port_first_port_name;
	mach_port_t task = mach_task_self();

	start_time();

	for (i=loops; i--;)  {
		MACH_CALL(mach_port_allocate_name, (task,
					      MACH_PORT_RIGHT_RECEIVE,
					      port));
		MACH_CALL(mach_port_destroy, (task, port));
	}
	stop_time();
}

name_allocate_or_destroy_test(op)
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t *ports, *port;

	set_max_loops(port_max_port_names);

	MACH_CALL(vm_allocate, (mach_task_self(),
				  (vm_offset_t *)&ports,
				  loops*sizeof(mach_port_t),
				  TRUE));

	for (i=0; i<loops; i++)
		*(ports+i) = port_first_port_name + i;

	if (op == ALLOCATE)
		start_time();

	for (i=loops, port = ports; i--; port++)
		MACH_CALL(mach_port_allocate_name, (task,
					 MACH_PORT_RIGHT_RECEIVE,
					 *port));

	if (op == ALLOCATE)
		stop_time();
	if (op == DEALLOCATE)
		start_time();

	for (i=loops, port = ports; i--; port++) 
		MACH_CALL(mach_port_destroy, (task, *port));

	if (op == DEALLOCATE)
		stop_time();

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t )ports,
				  loops*sizeof(mach_port_t)));
	
	test_cache(name_allocate_or_destroy_test, (op));
}

reply_port_test()
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t *ports, *port;

	set_max_loops(port_max_port_names);

	MACH_CALL(vm_allocate, (mach_task_self(),
				  (vm_offset_t *)&ports,
				  loops*sizeof(mach_port_t),
				  TRUE));

	start_time();

	for (i=loops, port = ports; i--; port++)
		MACH_FUNC(*port, mach_reply_port, ());

	stop_time();


	for (i=loops, port = ports; i--; port++) 
		MACH_CALL(mach_port_destroy, (task, *port));

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t )ports,
				  loops*sizeof(mach_port_t)));
	
	test_cache(reply_port_test, ());
}

reply_port_and_destroy_test()
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t port;

	start_time();

	for (i=loops; i--;)  {
		MACH_FUNC(port, mach_reply_port, ());
		MACH_CALL(mach_port_destroy, (task, port));
	}

	stop_time();
}


port_insert_right_and_deallocate_test()
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t port;

	MACH_FUNC(port, mach_reply_port, ());

	start_time();

	for (i=loops; i--;)  {
		MACH_CALL(mach_port_insert_right, (task, port, port,
					     MACH_MSG_TYPE_MAKE_SEND));
		MACH_CALL(mach_port_deallocate, (task, port));
	}

	stop_time();

	MACH_CALL(mach_port_destroy, (task, port));

}
		
port_insert_right_or_deallocate_test(op)
{
	register i;
	mach_port_t task = mach_task_self();
	mach_port_t *ports, *port;

	max_loops = port_max_port_names;	
	MACH_CALL(vm_allocate, (mach_task_self(),
				  (vm_offset_t *)&ports,
				  loops*sizeof(mach_port_t),
				  TRUE));

	for (i=loops, port = ports; i--; port++) 
		MACH_FUNC(*port, mach_reply_port, ());

	if (op == ALLOCATE)
		start_time();

	for (i=loops, port = ports; i--; port++)
		MACH_CALL(mach_port_insert_right, (task, *port, *port,
					     MACH_MSG_TYPE_MAKE_SEND));

	if (op == ALLOCATE)
		stop_time();
	if (op == DEALLOCATE)
		start_time();

	for (i=loops, port = ports; i--; port++)
		MACH_CALL(mach_port_deallocate, (task, *port));

	if (op == DEALLOCATE)
		stop_time();


	for (i=loops, port = ports; i--; port++) 
		MACH_CALL(mach_port_destroy, (task, *port));

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t )ports,
				  loops*sizeof(mach_port_t)));
}
