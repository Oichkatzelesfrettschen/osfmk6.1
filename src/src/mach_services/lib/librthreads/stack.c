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
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */
/*
 * 	File: 	stack.c
 *	Author:	Eric Cooper, Carnegie Mellon University
 *	Date:	Dec, 1987
 *
 * 	Real-Time thread stack allocation.
 *
 */

#include <rthreads.h>
#include "rthread_internals.h"
#include <mach/vm_region.h>
#include <mach/task_special_ports.h>
#include <mach/machine/vm_param.h>

#define	BYTES_TO_PAGES(b)	(((b) + vm_page_size - 1) / vm_page_size)

#define	WIRE_IN_KERNEL_STACKS
#ifdef	WIRE_IN_KERNEL_STACKS

#ifdef __i386
#define IN_KERNEL(addr)	(VM_MAX_ADDRESS < (unsigned)(addr))
#define IN_KERNEL_STACK_SIZE	(32 * 1024)
#else
Need an IN_KERNEL definition for current target.
#endif
boolean_t in_kernel;

#endif	/* WIRE_IN_KERNEL_STACKS */

vm_size_t 		rthread_stack_size;
int 			rthread_stack_chunk_count = 8;
private vm_address_t 	next_stack_base;
private vm_address_t 	stack_pool;
private boolean_t 	red_zone = FALSE;

/*
 *  Forward
 */

vm_offset_t 		rthread_stack_base(rthread_t, int);
void	 		alloc_stack(rthread_t);

/*
 * 	Set up a stack segment for a thread.
 * 	Segment has a red zone (invalid page)
 * 	for early detection of stack overflow.
 * 	The _rthread_self pointer is stored at the top.
 *
 *
 *	--------- (high address)
 *	| self	|
 *	|  ...	|
 *	|	|
 *	| stack	|
 *	|	|
 *	|  ...	|
 *	|	|
 *	---------
 *	|	|
 *	|invalid|
 *	|	|
 *	--------- (stack base)
 *	--------- (low address)
 *
 * 	or the reverse, if the stack grows up.
 */

private void
setup_stack(register rthread_t		th,
	    register vm_address_t 	base)
{
	th->stack_base = base;

	/* Stack size is segment size minus size of self pointer */
	th->stack_size = rthread_control.stack_size;

	/* Store self pointer */
	*(rthread_t *)&_rthread_ptr(base) = th;
}

vm_offset_t
addr_range_check(vm_offset_t	start_addr,
		 vm_offset_t 	end_addr,
		 vm_prot_t   	desired_protection)
{
	register vm_offset_t		addr;
	vm_offset_t			r_addr;
	vm_size_t			r_size;
	kern_return_t			kr;
	unsigned int			count;
	memory_object_name_t		r_object_name;
	vm_region_basic_info_data_t	info;

	addr = start_addr;

	while (addr < end_addr) {
		r_addr = addr;
		count  = VM_REGION_BASIC_INFO_COUNT;

		kr = vm_region(mach_task_self(),
			       &r_addr,
			       &r_size,
			       VM_REGION_BASIC_INFO,
			       (vm_region_info_t) &info, 
			       &count,
			       &r_object_name);

		if ((kr == KERN_SUCCESS) && MACH_PORT_VALID(r_object_name))
			(void) mach_port_deallocate(mach_task_self(),
						    r_object_name);

		if ((kr != KERN_SUCCESS) || (r_addr > addr) ||
		((info.protection & desired_protection) != desired_protection))
			return (0);

		addr = r_addr + r_size;
	}
	return (addr);
}

/*
 *  ROUTINE:	prob_stack
 *
 *  FUNCTION:	Probe for bottom and top of stack.
 *
 *  ASSUME:	1. stack grows DOWN
 * 		2. There is an unallocated region below the stack.
 */

void
probe_stack(vm_offset_t	*stack_bottom,
	    vm_offset_t *stack_top)
{
	/*
	 * Since vm_region returns the region starting at
	 * or ABOVE the given address, we cannot use it
	 * directly to search downwards.  However, we
	 * also want a size that is the closest power of
	 * 2 to the stack size (so we can mask off the stack
	 * address and get the stack base).  So we probe
	 * in increasing powers of 2 until we find a gap
	 * in the stack.
	 */

	vm_offset_t	start_addr, end_addr;
	vm_offset_t	last_start_addr, last_end_addr;
	vm_size_t	stack_size;

	/*
	 * Start with a page
	 */
	start_addr = rthread_sp() & ~(vm_page_size - 1);
	end_addr   = start_addr + vm_page_size;
	stack_size = vm_page_size;

	/*
	 * Increase the tentative stack size, by doubling each
	 * time, until we have exceeded the stack (some of the
	 * range is not valid).
	 */
	do {
		/*
		 * Save last addresses
		 */
		last_start_addr = start_addr;
		last_end_addr   = end_addr;

		/*
		 * Double the stack size
		 */
		stack_size <<= 1;
		start_addr = end_addr - stack_size;

		/*
		 * Check that the entire range exists and is writable
		 */
	} while ((end_addr = (addr_range_check(start_addr,
					       end_addr,
					       (VM_PROT_READ|VM_PROT_WRITE)))));
	/*
	 * Back off to previous power of 2.
	 */

	*stack_bottom = last_start_addr;
	*stack_top    = last_end_addr;
}

/*
 *  ROUTINE:	stack_init
 *
 *  FUNCTION:	Initializes the stack of a specified rthread.
 */ 

vm_offset_t
stack_init(rthread_t p)
{
	extern char	etext, end;

	vm_offset_t	stack_bottom,
			stack_top,
			start;
	vm_size_t	size;
	kern_return_t	r;

	in_kernel = IN_KERNEL(&etext);

	/*
	 * If we're running in a kernel-loaded application, force thread
	 * stacks to be small (but don't overrule our client).
	 */
	if (in_kernel && rthread_stack_size == 0)
		rthread_stack_size = IN_KERNEL_STACK_SIZE;

	/*
	 * Probe for bottom and top of stack, as a power-of-2 size.
	 */
	probe_stack(&stack_bottom, &stack_top);

	/*
	 * Use the stack size found for the rthread stack size,
	 * if not already specified.
	 */
	if (rthread_stack_size == 0)
		rthread_control.stack_size = stack_top - stack_bottom;
	else
		rthread_control.stack_size = rthread_stack_size;
#ifdef	RED_ZONE
	/*
	 * If not in a kernel-loaded server, allocate room for a protected
	 * red zone at end of stack that will cause an exception if touched.
	 * Make it equal to stack size, to have a good chance to catch large
	 * arrays that overflow the stack:
	 */
#ifdef	WIRE_IN_KERNEL_STACKS
	if (!in_kernel)
		red_zone = TRUE;
#else   /*WIRE_IN_KERNEL_STACKS*/
	red_zone = TRUE;
#endif  /*WIRE_IN_KERNEL_STACKS*/
	if (red_zone)
		rthread_control.stack_size *= 2;
#endif	/*RED_ZONE*/
#ifdef	STACK_GROWTH_UP
	rthread_control.stack_mask = ~(rthread_control.stack_size - 1);
#else	/*STACK_GROWTH_UP*/
	rthread_control.stack_mask = rthread_control.stack_size - 1;
#endif	/*STACK_GROWTH_UP*/

	if (rthread_stack_chunk_count <= 0)
		rthread_stack_chunk_count = 1;
	/*
	 * Guess at first available region for stack.
	 */
	next_stack_base = rthread_control.stack_size;
	if (in_kernel)
#ifdef	STACK_GROWTH_UP
		next_stack_base = (unsigned)(&end + next_stack_base) &
			rthread_control.stack_mask;
#else	/* STACK_GROWTH_UP */
	next_stack_base = (unsigned)(&end + next_stack_base) &
		~rthread_control.stack_mask;
#endif	/* STACK_GROWTH_UP */

	/*
	 * Set up stack for main thread.
	 */
	alloc_stack(p);

	/*
	 * Delete rest of old stack.
	 */

#ifdef	STACK_GROWTH_UP
	start = (rthread_sp() | (vm_page_size - 1)) + 1 + vm_page_size;
	size = stack_top - start;
#else	/*STACK_GROWTH_UP*/
	start = stack_bottom;
	size = (rthread_sp() & ~(vm_page_size - 1)) - stack_bottom - 
		vm_page_size;
#endif	/*STACK_GROWTH_UP*/
	MACH_CALL(vm_deallocate(mach_task_self(),start,size),r);

	/*
	 * Return new stack; it gets passed back to the caller
	 * of rthread_init who must switch to it.
	 */
	return rthread_stack_base(p, RTHREAD_STACK_OFFSET);
}

#ifdef	WIRE_IN_KERNEL_STACKS

/*
 *  ROUTINE:	stack_wire
 *
 *  FUNCTION:   Wire down a stack, on behalf of a kernel-loaded client (who
 * 		can't tolerate kernel exception handling on a pageable stack).
 */

void
stack_wire(vm_address_t base,
	   vm_size_t	length)
{
	mach_port_t 		bootstrap_port;
	static mach_port_t 	priv_host_port     = MACH_PORT_NULL;
	static mach_port_t 	unused_device_port = MACH_PORT_NULL;
	kern_return_t 		r;

	if (priv_host_port == MACH_PORT_NULL) {
		r = task_get_special_port(mach_task_self(),
					  TASK_BOOTSTRAP_PORT,
					  &bootstrap_port);

		if (r != KERN_SUCCESS) {
			printf("rthreads: task_get_special_port failed %x\n",r);
			exit(r);
		}

		r = bootstrap_privileged_ports(bootstrap_port,
					       &priv_host_port,
					       &unused_device_port);

		if (r != KERN_SUCCESS) {
		   printf("rthreads: bootstrap_privileged_ports failed %x\n",r);
		   exit(r);
		}
	}

	r = vm_wire(priv_host_port,
		    mach_task_self(),
		    base,
		    length,
		    (VM_PROT_READ|VM_PROT_WRITE));

	if (r != KERN_SUCCESS) {
		printf("rhreads: vm_wire failed %x\n", r);
		exit(r);
	}
}

#endif	/* WIRE_IN_KERNEL_STACKS */

/*
 *  ROUTINE:	alloc_stack
 *
 *  FUNCTION:	Allocate a stack segment for a thread.
 * 		Stacks are never deallocated.
 *
 *  NOTE:	The variable next_stack_base is used to align stacks.
 * 		It may be updated by several threads in parallel,
 * 		but mutual exclusion is unnecessary: at worst,
 * 		the vm_allocate will fail and the thread will try again.
 */

void
alloc_stack(rthread_t p)
{
	kern_return_t   r;
	vm_address_t	base = next_stack_base;

	if (stack_pool) {
		base = stack_pool;
		stack_pool = *(vm_address_t *)stack_pool;
		goto got_stack;
	}
		
	for (base = next_stack_base;
	     vm_allocate(mach_task_self(), 
			 &base, 
			 rthread_control.stack_size *rthread_stack_chunk_count, 
			 FALSE) != KERN_SUCCESS;
	     base += rthread_control.stack_size*rthread_stack_chunk_count);

	next_stack_base = base + 
		rthread_control.stack_size*rthread_stack_chunk_count;

#ifdef	WIRE_IN_KERNEL_STACKS
	/*
	 * If we're running in a kernel-loaded application, wire the stack
	 * we just allocated.
	 */
	if (in_kernel)
		stack_wire(base,
			  rthread_control.stack_size*rthread_stack_chunk_count);
#endif	/* WIRE_IN_KERNEL_STACKS */
        {
		int i;
		vm_address_t *loop = (vm_address_t *) base;
		for(i=1;i<rthread_stack_chunk_count;i++) {
			loop = (vm_address_t *)
				((int)loop + rthread_control.stack_size);
			*loop = (int)loop + rthread_control.stack_size;
		}
		if (loop != (vm_address_t *)base) {
			*loop = stack_pool;
			stack_pool = base + rthread_control.stack_size;
		}
	}
	
 got_stack:
#ifdef RED_ZONE
	/*
	 * Protect the red zone at far end of stack:
         */
	if (red_zone) {
		vm_address_t red_zone_base;
#ifdef	STACK_GROWTH_UP
		red_zone_base = base + rthread_control.stack_size/2;
#else	/*STACK_GROWTH_UP*/
		red_zone_base = base;
#endif	/*STACK_GROWTH_UP*/
		MACH_CALL(vm_protect(mach_task_self(),
				     red_zone_base,
				     rthread_control.stack_size / 2,
				     FALSE,
				     VM_PROT_NONE), r);
	}
#endif

	setup_stack(p, base);
}

/*
 *  ROUTINE:	rthread_stack_base
 *
 *  FUNCTION:	Returns the address of the specified offset from the
 *		rthread's stack base.
 */
 
vm_offset_t
rthread_stack_base(register rthread_t 	rthread,
		   register int 	offset)
{
#ifdef	STACK_GROWTH_UP
	return (rthread->stack_base + offset);
#else	/*STACK_GROWTH_UP*/
	return (rthread->stack_base + rthread->stack_size - offset);
#endif	/*STACK_GROWTH_UP*/

}

/*
 *  ROUTINE:	stack_fork_child
 *
 *  FUNCTION:	Called in the child after a fork().  Resets stack data
 *		structures to coincide with the reality that we now have
 *		a single rthread.
 */

void
stack_fork_child()
{
	next_stack_base = 0;
}
