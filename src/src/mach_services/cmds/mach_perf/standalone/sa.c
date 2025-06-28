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

mach_port_t privileged_host_port;
mach_port_t master_device_port;
mach_port_t name_server_port;

jmp_buf sa_jmp_buf;

exit()
{
	for(;;)
		if (is_master_task)
			mach_longjmp(&sa_jmp_buf, 1);
		else 
			MACH_CALL(task_terminate, ( mach_task_self()));
}

atoi(s)
char *s;
{
	int i = 0;

	while(*s)
		i = i*10+(*s++ - '0');
	return(i);
}

_Seterrno() {}

rand()
{
	return(sa_random());
}

mach_port_t
task_by_pid(pid)
{
	if (pid == -1)
		return(privileged_host_port);
	else if (pid == -2)
		return(master_device_port);
	else
		return(0);
}

__main() {}	/* GCC 2 C++ init routine */

void  *
memchr(const void *s,
       int c,
       size_t n)
{
	unsigned char *res = (unsigned char *)s;
	for (; res, n--; res++)
		if (*res == c)
			return(res);
	return(0);
}

bcmp(
     const char *s1,
     const char *s2, 
     int n)
{
	while(n--)
		if (*s1++ != *s2++)
			return(1);
	return(0);
}

isdigit(c)
{
	if (c >= '0' && c <= '9')
		return(1);
	else
		return(0);
}

isascii(c)
{
	if (c >= 0 && c <= 0x3f)
		return(1);
	else
		return(0);
}

double copysign(double x,double y)
{
	if ((y < 0 && x < 0) || (y >= 0 && x >= 0))
			return(x);
		else
			return(-x);
}

extern int device();
extern int exc();
extern int hw();
extern int ipc();
extern int pager();
extern int port();
extern int sched();
extern int task();
extern int thread();
extern int vm();
extern int exc();
extern int console();
extern int reboot();
extern int interruptible_cmd();

struct test_dir test_dir [] = {
	TEST(device, 1),
	TEST(exc, 1),
	TEST(hw, 1),
	TEST(ipc, 1),
	TEST(pager, 1),
	TEST(port, 1),
	TEST(sched, 1),
	TEST(task, 1),
	TEST(thread, 1),
	TEST(vm, 1),
	TEST(console, 0),
	TEST(reboot, 0),
	0, 0, 0
};

extern struct test_dir machine_test_dir[];

struct test_dir *test_dirs[] = {
	test_dir,
	machine_test_dir,
	0
};

#define MAX_ARGS 32

char *argv[MAX_ARGS];
int argc = 1;

#undef main
#undef usage

main()
{
	mach_port_t bootstrap_port;
	register i;
	struct test_dir *td, **tds;
	int all;
	kern_return_t kr;
	boolean_t found;

	MACH_CALL(task_get_bootstrap_port, (mach_task_self(), &bootstrap_port));
	MACH_CALL(bootstrap_privileged_ports, (bootstrap_port,
					       &privileged_host_port,
					       &master_device_port));
	MACH_FUNC(host_port, mach_host_self, ());
	standalone = 1;
	threads_init();
	printf_init(master_device_port);
	_printf_init();
	printf("\n\n");
	version();
	kernel_version();
	vm_opt = 1;
	print_vm_stats();
	while(1) {
		mach_setjmp(&sa_jmp_buf);
		reset_options();
		printf("$ ");
		if (!(argc = read_cmd(argv)))
			continue;
		all = strcmp(argv[0],"*") ? 0 : 1;
		for (found = FALSE, tds = test_dirs; *tds && !found; tds++)
		    for (td = *tds; td->name && !found; td++)
			if ((all && td->is_a_test) 
			    || !strcmp(argv[0],td->name)) {
				printf("_______________________________________________________________________________\n");
			  	argv[0] = td->name;
				if (td->is_a_test)
					interruptible_cmd(td->func,
							  argc,
							  argv);
				else
					(*td->func) (argc, argv);
				if (!all)
					found = TRUE;
			}
		if ((!all) && (!found))
			usage();
		else
			print_vm_stats();
	}
	printf("done\n");
}

usage()
{
	register i;
	struct test_dir *td, **tds;
	printf("usage:\n* [...]");
	for (tds = test_dirs; *tds; tds++)
	    for (td = *tds; td->name; td++)
		printf("\n%s [...]", td->name);
	printf("\n");
}
 
#define LINE_LENGTH 100
char command_line[LINE_LENGTH];

read_cmd(argv)
char *argv [];
{
	char *arg;
	int no_arg;
	int argc; 
	safe_gets(command_line, LINE_LENGTH);

	for (arg = command_line, no_arg = 1, argc = 0;
	     *arg && argc < MAX_ARGS;
	     arg++) {
		if (*arg == ' ' || *arg == '\t') {
			if (!no_arg) {
				*arg = 0;
				no_arg = 1;
			}
		} else {
			if (no_arg) {
				argv[argc++] = arg;
				no_arg = 0;
			}
		}
	}
	return(argc);
}

netname_look_up()
{
	printf("not implemented yet\n");
	return(KERN_FAILURE);
}

netname_check_in()
{
	printf("not implemented yet\n");
	return(KERN_FAILURE);
}

console(argc, argv)
char *argv[];
{
	extern mach_port_t console_port;
	mach_port_t port;

	if (argc != 2) {
		printf("syntax: console <device_name>\n");
		return;
	}

	MACH_CALL(device_open, (master_device_port, 0, argv[1], &port));
	MACH_CALL(device_close, (console_port));
	if (console_port != port) {
	  	vm_opt = 1; /* get vm stats printed again */
		console_port = port;
		kernel_version();
		print_vm_stats();
	}
}

reboot()
{
	MACH_CALL(host_reboot, (privileged_host_port, 0));
}

debugger()
{
	MACH_CALL(host_reboot, (privileged_host_port, 0x1000));
}


/*
 * iob[], setvbuf(): Just to satisfy unresolved symbols from
 * shared modules. Related code is never executed();
 */

int	__iob[1];

setvbuf() {
	debugger();	/* shouldnt happen */
}

#define CNTRL(x)	(x & 037)
#define INTR_CHAR 	CNTRL('c')

static jmp_buf 		intr_jmp_buf;

interruptible_cmd(func, argc, argv) 
char *argv[];
int			(*func)();
{
	mach_port_t 		task;
	mach_port_t 		notify_port;
	extern mach_port_t 	console_port;
	mach_port_t 		port_set;
	mach_port_t 		bootstrap_port;
	mach_port_t 		reply_port;
	mach_port_t 		reply_port_once;
	mach_port_t 		prev_port;
	extern	boolean_t	sa_demux();
	mach_port_t		*tasks_before, *tasks_after;
	int			ntask_before, ntask_after;
	register 		i, j;
	mach_port_t 		*tp;

	
	ressources_start();

	ntask_before = get_tasks(&tasks_before);

	/*
	 * We need to:
	 *	- Create a task to run the test
	 *	- Pass bootstrap ports to the new task. (As a side
	 *	  effect, the new task waits until all setups are
	 *	  done on our side, preventing noise on time stats)
	 *	- Wait for test to complete (slave task to terminate)
	 *	- Check for INTERRUPT
 	 */


	/*
	 * Create port set. 
	 * We will insert:
	 *	- a port to wait dead name notification on the created task
	 *	- a device reply port for character input.
	 *	- a bootstrap port on which the created task syncs to wait
	 *	  before it starts
	 */

	MACH_CALL(mach_port_allocate, 
		  (mach_task_self(),
		   MACH_PORT_RIGHT_PORT_SET,
		   &port_set));

	/*
	 * Set bootstrap port so that slave task can communicate
	 */ 

	MACH_CALL( mach_port_allocate, (mach_task_self(),
				 MACH_PORT_RIGHT_RECEIVE,
				 &bootstrap_port));
	MACH_CALL( mach_port_insert_right, (mach_task_self(),
					    bootstrap_port,
					    bootstrap_port,
					    MACH_MSG_TYPE_MAKE_SEND));
	MACH_CALL(task_set_bootstrap_port, (mach_task_self(),
					    bootstrap_port));
	MACH_CALL(mach_port_move_member, (mach_task_self(),
					   bootstrap_port, port_set));

	/*
	 * Create a device reply port for the console.
	 */

	MACH_CALL( mach_port_allocate, (mach_task_self(),
				 MACH_PORT_RIGHT_RECEIVE,
				 &reply_port));
	MACH_CALL( mach_port_insert_right, (mach_task_self(),
					    reply_port,
					    reply_port,
					    MACH_MSG_TYPE_MAKE_SEND));
	MACH_CALL(mach_port_move_member, (mach_task_self(),
					   reply_port, port_set));
	MACH_CALL(device_read_request_inband, (console_port,
					       reply_port,
					       0, 0, 0));

	/*
	 * Catch dead name notifications on task port.
	 */

	MACH_CALL(mach_port_allocate, (mach_task_self(),
				 MACH_PORT_RIGHT_RECEIVE,
				 &notify_port));
	MACH_CALL(mach_port_move_member, (mach_task_self(),
					   notify_port, port_set));

	if (task = mach_fork()) {
		thread_malloc_state_t mallocs = save_mallocs(thread_self());

		if (debug > 1)
			printf("created task %x\n", task);

		MACH_CALL (mach_port_request_notification,
			   (mach_task_self(),
			    task,
			    MACH_NOTIFY_DEAD_NAME,
			    0,
			    notify_port,
			    MACH_MSG_TYPE_MAKE_SEND_ONCE,
			    &prev_port));

		if (mach_setjmp(intr_jmp_buf) == 0) {
			MACH_CALL( mach_msg_server, (sa_demux,
						     256,
						     port_set,
						     MACH_MSG_OPTION_NONE));
		} else {	
			restore_mallocs(thread_self(), mallocs);
		}

	} else {
		MACH_CALL(task_get_bootstrap_port, (mach_task_self(),
						  &bootstrap_port));	
		MACH_CALL(bootstrap_privileged_ports, (bootstrap_port,
						       &privileged_host_port,
						       &master_device_port));
		(*func) (argc, argv);
		MACH_CALL( task_terminate, (mach_task_self())); 
	}

	ntask_after = get_tasks(&tasks_after);

	MACH_CALL( mach_port_destroy, (mach_task_self(), bootstrap_port));
	MACH_CALL( mach_port_destroy, (mach_task_self(), reply_port));
	MACH_CALL( mach_port_destroy, (mach_task_self(), notify_port));
	MACH_CALL( mach_port_destroy, (mach_task_self(), port_set));

	if (ntask_after != ntask_before) {
		for (i=0, tp = tasks_after; i < ntask_after; i++, tp++) {
			for (j = 0; j < ntask_before; j++)
				if (tasks_before[j] == *tp) {
				  	MACH_CALL(mach_port_deallocate,
						  (mach_task_self(),
						   *tp));
					*tp = 0;
					break;
				}
			if (*tp) {
				if (debug > 1)
					printf("killing task %x\n", *tp);
				task_terminate (*tp); /* Dont test ret code
							 task can terminate
							 itself */
				MACH_CALL( mach_port_deallocate,
					  (mach_task_self(), *tp));
				*tp = 0;
			}
		}
	}

	for (i=0, tp = tasks_before; i < ntask_before; i++, tp++)
		if (*tp) {
			MACH_CALL(mach_port_deallocate, (mach_task_self(),
							 *tp));
		}

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t) tasks_before,
				  sizeof(*tasks_before)*ntask_before));

	for (i=0, tp = tasks_after; i < ntask_after; i++, tp++)
		if (*tp) {
			MACH_CALL(mach_port_deallocate, (mach_task_self(),
							 *tp));
		}

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t) tasks_after,
				  sizeof(*tasks_after)*ntask_after));
	
	MACH_CALL( mach_port_destroy, (mach_task_self(), task));

	ressources_stop();
	ressources_use("Standalone ");       
}

boolean_t
sa_demux(in, out)
{
	if (notify_server(in, out))
		return(TRUE);
	else if (sa_device_reply_server(in, out))
		return(TRUE);
	else return(bootstrap_server(in, out));
}


do_mach_notify_port_deleted(port, name)
mach_port_t port;
{
	printf("mach_notify_port_deleted(), name = %x\n", name);
	return(KERN_SUCCESS);
}

do_mach_notify_port_destroyed(port, rcv_right)
mach_port_t port;
{
	printf("mach_notify_port_destroyed(), rcv_right = %x\n", rcv_right);
	return(KERN_SUCCESS);
}

do_mach_notify_no_senders(port, count)
mach_port_t port;
{
	printf("mach_notify_no_senders(), count = %x\n", count);
	return(KERN_SUCCESS);
}

do_mach_notify_send_once(port)
mach_port_t port;
{
	printf("mach_notify_send_once()\n");
	return(KERN_SUCCESS);
}

do_mach_notify_dead_name(port, name)
mach_port_t port;
{
	if (debug)
		printf("mach_notify_dead_name(), name = %x\n", name);
	mach_longjmp(intr_jmp_buf, 1);
	return(KERN_SUCCESS);
}

sa_device_read_reply_inband(reply_port, rc, data, data_count)
mach_port_t reply_port;
io_buf_ptr_inband_t data;
mach_msg_type_number_t data_count;
{
	extern mach_port_t console_port;

#if 0
	if (data_count >= 1 && *(char *)data == INTR_CHAR) {
#else
      	if (getchar() == INTR_CHAR) {
#endif
		printf("\nInterrupt\n");
		mach_longjmp(intr_jmp_buf, 1);
	} else {
		MACH_CALL(device_read_request_inband,
			  (console_port,
			   reply_port,
			   0, 0, 0));
	}
	return(KERN_SUCCESS);
}

sa_device_read_reply(reply_port, rc, data, data_count)
mach_port_t reply_port;
io_buf_ptr_t data;
mach_msg_type_number_t data_count;
{
	printf("ds_device_read_reply()\n");
	return(KERN_SUCCESS);
}

sa_device_read_reply_overwrite(reply_port, rc, data_count)
mach_port_t reply_port;
mach_msg_type_number_t data_count;
{
	printf("ds_device_read_reply_overwrite()\n");
	return(KERN_SUCCESS);
}

sa_device_write_reply(reply_port, rc, data_count)
mach_port_t reply_port;
int data_count;
{
	printf("ds_device_write_reply()\n");
	return(KERN_SUCCESS);
}

sa_device_write_reply_inband(reply_port, rc, data_count)
mach_port_t reply_port;
int data_count;
{
	printf("ds_device_write_reply_inband()\n");
	return(KERN_SUCCESS);
}

sa_device_open_reply(reply_port, rc, device)
mach_port_t reply_port, device;
{
	printf("ds_device_open_reply()\n");
	return(KERN_SUCCESS);
}

do_bootstrap_privileged_ports(bootstrap, priv_host, priv_device)
mach_port_t bootstrap;
mach_port_t *priv_host;
mach_port_t *priv_device;
{
	*priv_host = privileged_host_port;
	*priv_device = master_device_port;
	return(KERN_SUCCESS);
}
