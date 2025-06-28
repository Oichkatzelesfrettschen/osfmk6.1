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
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#include <mach.h>
#include <machine/ndr_def.h>
#include <mach/machine/rpc.h>

extern void mig_init();
extern void mach_init_ports();

mach_port_t	mach_task_self_ = MACH_PORT_NULL;

vm_size_t	vm_page_size;

static struct rpc_glue_vector _rpc_glue_vector_data;
rpc_glue_vector_t _rpc_glue_vector;

/*
 * Forward internal declarations for automatic mach_init during
 * fork() implementation.
 */
/* fork() calls through atfork_child_routine */
extern void (*_atfork_child_routine)();

static void mach_atfork_child_routine();
static boolean_t first = TRUE;
static void (*previous_atfork_child_routine)();

static void mach_atfork_child_routine()
{
	/*
	 * If an (*_atfork_child_routine)() was registered when
	 * mach_init was first called, then call that routine
	 * prior to performing our re-initialization. This ensures
	 * that the post-fork handlers are called in exactly the
	 * same order as the crt0 (exec) handlers. Any library 
	 * that makes use of the _atfork_child_routine must follow
	 * the same technique.
	 */
	if (previous_atfork_child_routine) {
		(*previous_atfork_child_routine)();
	}
	mach_init();
}

int		mach_init()
{
	kern_return_t		kr;
	task_user_data_data_t	user_data;
	unsigned int 		user_data_count = TASK_USER_DATA_COUNT;

	if (first) {
		/*
		 * Set up the post-fork child handler in the libc stub
		 * to invoke this routine if this process forks. Save the
		 * previous value in order that we can call that handler
		 * prior to performing our postfork work.
		 */
		first = FALSE;
		previous_atfork_child_routine = _atfork_child_routine;
		_atfork_child_routine = mach_atfork_child_routine;
	}

#undef	mach_task_self

	/*
	 *	Get the important ports into the cached values,
	 *	as required by "mach_init.h".
	 */
	 
	mach_task_self_ = mach_task_self();

	/*
	 *	Initialize the single mig reply port
	 */

	mig_init(0);

	/*
	 *	Cache some other valuable system constants
	 */

	(void)host_page_size(mach_host_self(), &vm_page_size);

	mach_init_ports();

	/*
	 * Initialize glue vector needed for direct RPC.
	 */
	user_data.user_data = 0;
	(void)task_info(mach_task_self_, TASK_USER_DATA,
		(task_info_t)&user_data, &user_data_count);
	if (user_data.user_data == 0) {
		user_data.user_data = &_rpc_glue_vector_data;
	    (void)task_set_info(mach_task_self_, TASK_USER_DATA,
		    (task_info_t)&user_data, user_data_count);
	}
	_rpc_glue_vector = (rpc_glue_vector_t)user_data.user_data;
	return(0);
}

int	(*_mach_init_routine)() = mach_init;
