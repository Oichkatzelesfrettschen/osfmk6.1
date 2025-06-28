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
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * rthread_internals.h
 *
 * Private definitions for the Real-Time Threads implementation.
 *
 */

#include "options.h"
#include "queue.h"
#include <mach.h>
#include <mach/thread_switch.h>


#define RTHREADS_VERSION 1


/*
 * Low-level Rthread implementation.
 */
struct rthread {
	struct rthread  *next;
	int 		status;
	rthread_fn_t	func;
	void 		*arg;
	int 		result;
	const char 	*name;
	void 		*data;
	policy_t	policy;
	int		priority;
	int		quantum;
	int 		context;
	struct rthread 	*list;		/* for master rthread list  */
	mach_port_t 	wired;		/* kernel thread port       */
	mach_port_t 	reply_port;	/* for mig_get_reply_port() */
	vm_offset_t	stack_base;
	vm_size_t	stack_size;
};

/*
 * Thread status bits.
 */
#define	T_MAIN		0x1
#define T_ACT		0x2
#define T_CHILD		0x4

/*
 * Rthread control block definition
 */
typedef struct rthread_control_struct {
	lock_set_port_t	locks;		   /* rthread control block locks    */
	int 		version;	   /* rthread version		     */
	int 	  	active_rthreads;   /* number of active rthreads      */
	int 	  	alloc_rthreads;    /* number of allocated rthreads   */
	int		activation_count;  /* number of created activations  */	
	rthread_t  	rthread_list;	   /* list of all rthreads           */
	vm_size_t  	stack_size;	   /* size of user stacks 	     */
	vm_offset_t	stack_mask;	   /* mask to begining               */
	policy_t	default_policy;	   /* default child policy	     */
	int		base_priority;
	int		max_priority;
	int		default_quantum;
	boolean_t	main_waiting;      /* T_MAIN wait flag  	     */
	mach_port_t 	main_wait;	   /* semaphore T_MAIN blocks on if  */
	                                   /* waiting for its children to    */
					   /* complete.			     */
	struct rthread_queue  free_list;   /* exited rthread list: cached    */
					   /* for reuse 		     */
} *rthread_control_t;

extern struct rthread_control_struct rthread_control;

/*
 * Rthread control block lock definitions
 */

#define RTHREAD_CONTROL_LOCK			0
#define RTHREAD_CONTROL_POLICY_LOCK		1
#define RTHREAD_CONTROL_THREAD_LOCK		2
#define RTHREAD_CONTROL_TRACE_LOCK		3
#define RTHREAD_CONTROL_LOCK_GRANULARITY	4
 	
/*
 * Rthread control block locking MACROS
 */

#ifdef	LOCKS

#define rthread_control_create_locks()  			\
	MACH_CALL(						\
	lock_set_create(mach_task_self(),			\
			&rthread_control.locks,			\
			RTHREAD_CONTROL_LOCK_GRANULARITY,	\
			SYNC_POLICY_FIXED_PRIORITY),r)

#define rthread_control_destroy_locks()				\
	MACH_CALL(						\
	lock_set_destroy(mach_task_self(),			\
			 rthread_control.locks),r)

#define rthread_control_lock()		\
	MACH_CALL(			\
	lock_acquire(rthread_control.locks, RTHREAD_CONTROL_LOCK),r)
		  
#define rthread_control_unlock()	\
	MACH_CALL(			\
	lock_release(rthread_control.locks, RTHREAD_CONTROL_LOCK),r)

#define rthread_control_policy_lock()	\
	MACH_CALL(			\
	lock_acquire(rthread_control.locks, RTHREAD_CONTROL_POLICY_LOCK),r)

#define rthread_control_policy_unlock()	\
	MACH_CALL(			\
	lock_release(rthread_control.locks, RTHREAD_CONTROL_POLICY_LOCK),r)

#define rthread_lock()			\
	MACH_CALL(			\
	lock_acquire(rthread_control.locks, RTHREAD_CONTROL_THREAD_LOCK),r)

#define rthread_unlock()		\
	MACH_CALL(			\
	lock_release(rthread_control.locks, RTHREAD_CONTROL_THREAD_LOCK),r)

#define trace_lock()			\
	MACH_CALL(			\
	lock_acquire(rthread_control.locks, RTHREAD_CONTROL_TRACE_LOCK),r)

#define trace_try_lock()		\
	lock_try(rthread_control.locks, RTHREAD_CONTROL_TRACE_LOCK)

#define trace_unlock()			\
	MACH_CALL(			\
	lock_release(rthread_control.locks, RTHREAD_CONTROL_TRACE_LOCK),r)

#else	/* LOCKS */
#define rthread_control_create_locks()
#define rthread_control_destroy_locks()
#define rthread_control_lock()
#define rthread_control_unlock()
#define rthread_control_policy_lock()
#define rthread_control_policy_unlock()
#define rthread_lock()
#define rthread_unlock()
#define trace_lock()
#define trace_try_lock()
#define trace_unlock()
#endif 	/* LOCKS */


#ifdef	STACK_GROWTH_UP
#define	_rthread_ptr(sp) (*(rthread_t_t *) ((sp) & rthread_control.stack_mask))
#else	/*STACK_GROWTH_UP*/
#define	_rthread_ptr(sp) 						\
	(* (rthread_t *) ( ((sp) | rthread_control.stack_mask) + 1 	\
			      - sizeof(rthread_t *)) )
#endif	/*STACK_GROWTH_UP*/

#define	_rthread_self()		((rthread_t) _rthread_ptr(rthread_sp()))

#define RTHREAD_FILTER(a, b, c, d, e, f) rthread_filter(  		\
 		       (int *)(&(a)->context), (int)(b), (void *)(c), 	\
		       (void *)(d), (void *)(e), (void *)(f))

/*
 * Macro for MACH kernel calls.
 */
#ifdef CHECK_STATUS
#define	MACH_CALL(expr, ret)						\
	if (((ret) = (expr)) != KERN_SUCCESS)				\
		mach_error("Rthreads kernel return error:", ret)
#else /*CHECK_STATUS*/
#define MACH_CALL(expr, ret) (ret) = (expr)
#endif /*CHECK_STATUS*/

#ifdef GPROF
#define private
#else /*GPROF*/
#define private static
#endif

#define yield() \
	MACH_CALL(thread_switch(MACH_PORT_NULL, SWITCH_OPTION_DEPRESS, 10), r)
