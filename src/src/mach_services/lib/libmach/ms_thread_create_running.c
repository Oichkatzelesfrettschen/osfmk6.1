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
 *	File:		ms_thread_create_running.c
 *	Purpose:
 *		Routine to implement thread_create_running as a 
 *		syscall trap call.
 */

#include <mach.h>
#include <mach/message.h>

kern_return_t 
thread_create_running(
	task_port_t		parent_task,
	thread_state_flavor_t	flavor,
	thread_state_t		new_state,
	mach_msg_type_number_t	new_state_count,
	thread_port_t		*child_thread)		/* OUT */
{
	kern_return_t result;

	result = syscall_thread_create_running(parent_task, flavor, new_state,
					       new_state_count, child_thread);
	if (result == MACH_SEND_INTERRUPTED)
		result = mig_thread_create_running(parent_task, flavor, 
				new_state, new_state_count, child_thread);
	return(result);
}





