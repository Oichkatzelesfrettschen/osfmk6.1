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
#include <mach/thread_switch.h>
#include <mach/thread_status.h>

mach_port_t privileged_host_port = MACH_PORT_NULL;
mach_port_t host_port = MACH_PORT_NULL;

mach_port_t bootstrap_port = MACH_PORT_NULL;
mach_port_t master_host_port = MACH_PORT_NULL;
mach_port_t master_device_port = MACH_PORT_NULL;

/*
 * Get kernel task port 
 */

task_port_t
get_kernel_task()
{
   	task_port_t *tasks_list;
    	unsigned int tasksCnt;
	task_port_t kernel_task;
	register i;

	tasksCnt = get_tasks(&tasks_list);

	kernel_task = tasks_list[0];
	for (i=1; i < tasksCnt; i++)
		MACH_CALL(mach_port_deallocate, (mach_task_self(),
						 tasks_list[i]));
	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t) tasks_list,
				  sizeof(*tasks_list)*tasksCnt));
	
	return(kernel_task);
}

processor_set_control_port_t
get_processor_set()
{
	mach_port_t proc_set;
    	unsigned int proc_setsCnt;
   	task_port_t *tasks_list;
    	unsigned int tasksCnt;
    	processor_set_name_port_t *proc_set_names;


	/* get processor sets */

	MACH_CALL(host_processor_sets, 
		  (privileged_host_port, &proc_set_names, &proc_setsCnt));

	/* convert processor set 0 name to send right privileged port */

	MACH_CALL(host_processor_set_priv,
		  (privileged_host_port, proc_set_names[0], &proc_set));

	return(proc_set);
}

mach_port_t
rename_to_gen_port(old_port)
mach_port_t old_port;
{
	mach_port_t port;
	MACH_CALL( mach_port_allocate, (mach_task_self(),
					MACH_PORT_RIGHT_RECEIVE,
					&port));
	MACH_CALL( mach_port_destroy, (mach_task_self(),
				       port));
	MACH_CALL( mach_port_rename, (mach_task_self(),
				      old_port, port));
	return(port);
}

/*
 * Same effect as vm_allocate(), but we want to be sure
 * that the underlying object is not shared with lower 
 * memory. Otherwise vm_deallocate will not free 
 * physical memory
 */
kern_return_t
vm_allocate_temporary(task, buf, size, anywhere)
mach_port_t task;
vm_offset_t *buf;
vm_size_t size;
boolean_t anywhere;
{
	kern_return_t ret;

	if (!anywhere)
		return(vm_allocate(task, buf, size, FALSE));
	ret = vm_allocate(task, buf, size + vm_page_size, TRUE);
	if (ret != KERN_SUCCESS)
		return ret;
	MACH_CALL( vm_deallocate, (task, *buf, size + vm_page_size));
	*buf += vm_page_size;
	return(vm_allocate (task, buf, size, FALSE));
}

privileged_user()
{
	if (is_master_task && privileged_host_port == MACH_PORT_NULL) {
		MACH_FUNC(host_port, mach_host_self, ());
		MACH_FUNC(privileged_host_port, mach_host_priv_self, ());
		MACH_CALL(task_get_special_port, (mach_task_self(),
						  TASK_BOOTSTRAP_PORT,
						  &bootstrap_port));
		MACH_CALL(bootstrap_privileged_ports, (bootstrap_port,
						       &master_host_port,
						       &master_device_port));
	}
}

/*
 * Get kernel task port 
 */

int
get_tasks(tasks)
task_port_t **tasks;
{
	processor_set_name_port_t proc_set;
    	unsigned int proc_setsCnt;
    	unsigned int tasksCnt;
    	processor_set_name_port_t *proc_set_names;
	register i;

	/* get processor sets */

	MACH_CALL(host_processor_sets, 
		  (privileged_host_port, &proc_set_names, &proc_setsCnt));

	/* convert processor set 0 name to send right privileged port */

	MACH_CALL(host_processor_set_priv,
		  (privileged_host_port, proc_set_names[0], &proc_set));


	for (i = 0; i < proc_setsCnt; i++) {
		MACH_CALL( mach_port_deallocate, (mach_task_self(),
						  proc_set_names[i]));
	}

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t) proc_set_names,
				  sizeof(*proc_set_names)*proc_setsCnt));
	/* get tasks */

	MACH_CALL(processor_set_tasks, 
		  (proc_set, tasks, &tasksCnt));

	MACH_CALL( mach_port_deallocate, (mach_task_self(),
					  proc_set));
	return(tasksCnt);
}

