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

mach_port_t 	mach_fork_child_task;
volatile int	mach_fork_sync;

int		norma_node = NORMA_NODE_NULL;

#define	    FORK_INIT 			0
#define     FORK_PARENT_IDLE		1
#define	    FORK_PARENT_SUSPENDED	2
#define	    FORK_CHILD_READY		3

int	    is_master_task = 1;

fork_slave(parent_thread)
mach_port_t parent_thread;
{
	thread_state_t state;
	mach_msg_type_number_t count;
	mach_port_t child_thread;

	if (debug > 1) {
		printf("fork_slave()\n");
	}

	while (mach_fork_sync != FORK_PARENT_IDLE)
		thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
	MACH_CALL( thread_suspend, (parent_thread));
	mach_fork_sync = FORK_PARENT_SUSPENDED;
	if (norma_node != NORMA_NODE_NULL) {
		MACH_CALL( norma_task_create, (mach_task_self(),
					       TRUE, 
					       norma_node, 
					       &mach_fork_child_task));
	} else {
		MACH_CALL( task_create, (mach_task_self(),
					 TRUE, 
					 &mach_fork_child_task));
	}
	if (!standalone) {
	  	/*
		 * OSF/1 server might panic otherwise
		 */

		MACH_CALL( task_set_exception_ports,
			  (mach_fork_child_task,
			   EXC_MASK_ALL,
			   MACH_PORT_NULL,
			   EXCEPTION_DEFAULT,
			   0));
	}

	/*
	 * insert parent thread port so that child does not get same
	 * port name for thread_self(). 
	 */

	MACH_CALL( mach_port_insert_right, (mach_fork_child_task,
					    parent_thread, parent_thread,
					    MACH_MSG_TYPE_COPY_SEND));

	MACH_CALL( thread_create, (mach_fork_child_task, &child_thread));

	MACH_CALL( vm_allocate, (mach_task_self(),
				  (vm_offset_t *)&state,
				  THREAD_STATE_COUNT,
				  TRUE));

	if (debug > 1)
		printf("get state parent\n");
	count = THREAD_STATE_COUNT;
	MACH_CALL( thread_get_state, (parent_thread, THREAD_STATE,
				       state, &count));


	if (debug > 1) {
		register i;
		for (i=0; i<THREAD_STATE_COUNT; i++)
			printf("%x/", state[i]);
		printf("\n");
	}
	if (debug > 1)
		printf("thread_set_state (task %x thread %x\n",
		       mach_fork_child_task,
		       child_thread);
	MACH_CALL( thread_set_state, (child_thread, THREAD_STATE,
			       state, count));

	if (debug > 1) {
	  	register i;
		for (i=0; i<THREAD_STATE_COUNT; i++)
			printf("%x/", state[i]);
		printf("\n");
	}
	printf_enable(mach_fork_child_task);

	MACH_CALL( thread_resume, (parent_thread));
	MACH_CALL( thread_resume, (child_thread));
	MACH_CALL( vm_deallocate, (mach_task_self(),
				  (vm_offset_t)state,
				  THREAD_STATE_COUNT));
	MACH_CALL( mach_port_deallocate, (mach_task_self(), child_thread));
	mach_fork_sync = FORK_CHILD_READY;
	while(1)
		thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
}

mach_port_t
mach_fork()
{
	register i;
	mach_port_t parent_task = mach_task_self();
	mach_port_t parent_thread;
	mach_port_t port;
	mach_thread_t slave;

	if (debug > 1)
		printf("mach_fork()\n");
	mach_fork_sync = FORK_INIT;
	parent_thread = mach_thread_self();
	mach_fork_child_task = 0;
	new_thread(&slave, fork_slave, parent_thread);
	mach_fork_sync = FORK_PARENT_IDLE;
  	while(mach_fork_sync == FORK_PARENT_IDLE)
		continue;
	if (mach_thread_self() == parent_thread) {
		while(mach_fork_sync != FORK_CHILD_READY)
			thread_switch(MACH_PORT_NULL, SWITCH_OPTION_NONE, 0);
		kill_thread(slave);
		return(mach_fork_child_task);
	} else {
		is_master_task = 0;
		mach_init();
	   	child_threads();
		MACH_CALL( mach_port_destroy, (mach_task_self(),
					       parent_thread));
		return(MACH_PORT_NULL);
	}
}

