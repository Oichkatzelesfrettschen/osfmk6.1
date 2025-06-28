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
#include <mach/i386/thread_status.h>

extern thread_start();

kern_return_t
thread_set_regs(state, pc, arg0, sp, ssize)
struct i386_thread_state *state;
unsigned int pc, arg0, sp, ssize;
{
	state->eip = (unsigned int) thread_start;
	state->eax = pc;	
	state->ebx = arg0;
	state->uesp = sp + ssize;
}

/*
 * For i386, thread pointer is saved in lowest word of the stack
 */

#define mach_thread(addr)	(*(struct mach_thread **)thread_stack(addr))

mach_thread_t
thread_self()
{
	int i;

	if (threads_initialized)
	 	return(mach_thread(&i));
	else
		return(&threads[0]);
}

set_thread_self(thread)
mach_thread_t thread;
{
  	mach_thread_t *th_pt;

	th_pt = (mach_thread_t *)thread->stack;
	*th_pt = thread;
}

