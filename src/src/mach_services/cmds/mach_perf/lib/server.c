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

struct test server_test = {
"             server side:", 0, 0, 0, 0, 0, 0,
};

static jmp_buf 		server_jmp_buf;
boolean_t		server_server();
int 			server_times = 0;
int			server_count = 0;
boolean_t 		server_only = FALSE;
boolean_t 		client_only = FALSE;
mach_port_t		server = MACH_PORT_NULL;
mach_port_t		server_task = MACH_PORT_NULL;


/*
 * Variables used when server thread is created in same task
 */

boolean_t		server_in_same_task = FALSE;
mach_port_t		saved_server;	/* server thread of server task */
mach_port_t		server_thread;

/*
 * Create a server task, and establish server/client channel
 * with it.
 *	create_server(func, serv_task, serv_port)
 *		func: entry point for server
 *	   out	serv_task: mach_port_t for server task
 *	   out	serv_port: send right to server
 */

kern_return_t
create_server(func, serv_task, serv_port)
void		(*func)();
mach_port_t	*serv_task, *serv_port;
{
	mach_port_t bootstrap_port;

	if (debug)
		printf("create_server\n");

	MACH_CALL( mach_port_allocate, (mach_task_self(),
				 MACH_PORT_RIGHT_RECEIVE,
				 &bootstrap_port));
	MACH_CALL( mach_port_insert_right, (mach_task_self(),
					    bootstrap_port,
					    bootstrap_port,
					    MACH_MSG_TYPE_MAKE_SEND));
	MACH_CALL(task_set_bootstrap_port, (mach_task_self(),
					    bootstrap_port));

	if (server_task = mach_fork()) {
		server_wait(bootstrap_port);
		*serv_port = server;
		*serv_task = server_task;
		MACH_CALL(set_timer, (server, timer));
	} else {
		MACH_CALL(task_get_bootstrap_port, (mach_task_self(),
						  &bootstrap_port));	
		MACH_CALL(mach_port_allocate, (mach_task_self(),
						MACH_PORT_RIGHT_RECEIVE,
						&server));
		MACH_CALL(send_server_port, (bootstrap_port,
					   server));
		need_banner = FALSE;
		prof_opt = 0;
		(*func)(server);
	}
	return(KERN_SUCCESS);
}	

server_wait(bootstrap_port)
mach_port_t bootstrap_port;
{
	thread_malloc_state_t mallocs = save_mallocs(thread_self());
	if (debug > 1)
		printf("bootstrap_server()\n");
	if (mach_setjmp(server_jmp_buf) == 0) {
		MACH_CALL( mach_msg_server, (server_server,
				     256,
				     bootstrap_port,
				     MACH_MSG_OPTION_NONE));
	} else {
		restore_mallocs(thread_self(), mallocs);
		MACH_CALL(mach_port_destroy, (mach_task_self(),
					      bootstrap_port));
	}
}

do_send_server_port(boostrap_port, server_port)
mach_port_t boostrap_port;
mach_port_t server_port;
{
	server = server_port;
	mach_longjmp(server_jmp_buf, 1);
}

/*
 * Services used between client and server for test statistics and
 * synchronization
 */

do_server_start_test(server, show_time, _verbose)
mach_port_t server;
{
	server_stats.thread = mach_thread_self();
	if (!server_in_same_task)
		ressources_start();
	server_times = show_time;
	verbose = _verbose;
	return(0);
}

do_server_stop_test(server)
mach_port_t server;
{
	if (!server_in_same_task) {
		ressources_stop();
		ressources_use("Server side ");   
	}
	return(0);
}

do_server_print_time(server)
mach_port_t	server;
{

	print_header(&server_test);
	print_results(&server_stats);
	reset_time_stats(&server_stats);
	return(0);
}

do_server_collect(server)
mach_port_t	server;
{
	if (!server_times)
		return(0);
	loops -= server_count;
	collect_stats(&server_stats, loops);
	if (verbose)
		print_one_sample(&server_test, &server_stats);
	return(0);
}

do_set_printer(server, printer)
mach_port_t	server;
mach_port_t printer;
{
	if (server_only && !debug) {
		printf_port = printer;
		is_master_task = 0;
	}
	return(0);
}

do_set_timer(server, _timer)
mach_port_t	server;
int		_timer;
{
	timer = _timer;
	if (timer != -1)
		measure_timer(timer);
	return(0);
}

do_stop_server(server)
mach_port_t	server;
{
	if (server_times) {
		_stop_time(server_stats, 0 /* No prof */);
	}
	if (debug > 1)
		printf("stop_server()\n");
	if (server_count)
		printf("Invalid number of loops for server: %d\n", server_count);
	return(0);
}

do_start_server(server, new_count)
mach_port_t	server;
int new_count;
{
	loops = server_count = new_count;
	if (debug > 1)
		printf("start_server(%d)\n", new_count);
	if (server_times) {
		_start_time(server_stats, 0 /* No prof */);
	}
	return(0);
}

do_kill_server(server)
mach_port_t	server;
{
	if (debug > 1)
		printf("kill_server\n");
	MACH_CALL(task_terminate, (mach_task_self())); 
	MACH_CALL(mach_port_destroy, (mach_task_self(),
					      server));
	server = MACH_PORT_NULL;
	return (KERN_SUCCESS);
}

server_time(run)
{
	if ((!server_times) || (server == MACH_PORT_NULL))
		return;
	if (run) {
	  	if (server_in_same_task) { 
			do_server_collect(MACH_PORT_NULL);
		} else
			MACH_CALL(server_collect, (server));
	} else {
	  	if (server_in_same_task) { 
			do_server_print_time(MACH_PORT_NULL);
		} else
			MACH_CALL(server_print_time, (server));
	}
}

mach_port_t lookup(name)
	char *name;
{
	mach_port_t	port;
	kern_return_t	ret;

	MACH_CALL(netname_look_up, (name_server_port, "*", name, &port));
	return(port);
}

mach_port_t
server_lookup(name)
char *name;
{
	if (client_only) {
		server = lookup(name);
		if (server == MACH_PORT_NULL) {
			printf("unable to find server.\n");
			leave(2);
		}
		MACH_CALL(set_printer, (server, printf_port));
		MACH_CALL(set_timer, (server, timer));
		return(server);
	}
	return(MACH_PORT_NULL);
}

start_server_test()
{
	if (server != MACH_PORT_NULL) {
		MACH_CALL(server_start_test, (server, server_times, verbose));
	}
}

stop_server_test()
{
	if (server != MACH_PORT_NULL && !server_in_same_task) {
		MACH_CALL(server_stop_test, (server));
	}
}

run_server(func)
void		(*func)();
{
	if (server_only && !standalone) {
		(*func)(0);
		printf("server died\n");
		leave(1);
	}
	MACH_CALL(create_server, (func,
				  &server_task,
				  &server));
}

/*
 * Start a server thread in the same task
 */

start_server_thread(func)
void (*func)();
{
	mach_port_t port;

	if (debug)
		printf("start server thread(%x)\n", func);
	
	MACH_CALL( mach_port_allocate, (mach_task_self(),
					MACH_PORT_RIGHT_RECEIVE,
					&port));
	MACH_CALL( mach_port_insert_right, (mach_task_self(),
					    port, port,
					    MACH_MSG_TYPE_MAKE_SEND));
	MACH_CALL(new_thread, (&server_thread, func, port));
	saved_server = server;
	server = port;
	start_server_test();
	server_in_same_task = TRUE;
}

stop_server_thread()
{
	mach_port_t port;

	if (debug)
		printf("terminate server thread\n");

	if (saved_server == MACH_PORT_NULL) {
		printf("No active server thread in current task\n");
		return;
	}
	port = server;
	server = saved_server;
	saved_server = MACH_PORT_NULL;

	if (server_count)
		printf("Invalid number of loops for server: %d\n",
		       loops-server_count);
	kill_thread(server_thread);
	MACH_CALL( mach_port_destroy, (mach_task_self(), port));

	/* Do not reset server_in_same_task now, it is used by
	   server_time() */
	 
}

