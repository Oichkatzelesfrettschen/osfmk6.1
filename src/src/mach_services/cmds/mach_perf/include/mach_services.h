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

#ifndef _MACH_SERVICES_H
#define _MACH_SERVICES_H

#include <mach.h>

#define MACH_CALL(call, args) {						\
	register kern_return_t kern_ret;				\
	DEBUG_RESSOURCES_RESET();					\
	kern_ret = call args ;						\
	if (kern_ret != KERN_SUCCESS) {					\
		test_exit(						\
			"%s() in %s at line %d: %s\n",			\
			 # call,					\
			__FILE__,					\
			__LINE__,					\
			mach_error_string(kern_ret));			\
	}								\
	DEBUG_RESSOURCES_TEST();					\
}

#define MACH_FUNC(ret, func, args) {					\
	DEBUG_RESSOURCES_RESET();					\
	ret = func args ;						\
	if (!ret) {							\
		test_exit(						\
			"%s() in %s at line %d: returns NULL value\n",\
			# func,						\
			__FILE__,					\
			__LINE__);					\
	}								\
	DEBUG_RESSOURCES_TEST();					\
}

mach_port_t 			mach_fork();
processor_set_control_port_t 	get_processor_set();
extern int 			is_master_task;
extern int			norma_node;

#define		NORMA_NODE_NULL -1

#define MAX_THREADS 10
#define MAX_MALLOCS 3
#define STACK_SIZE  (64*1024)

struct malloc {
	vm_offset_t   addr;
	vm_size_t     size;
	int	      seq;
};

typedef struct malloc malloc_t;
typedef int thread_malloc_state_t;

struct mach_thread {
	thread_port_t thread;
	vm_offset_t   stack;
	mach_port_t   mig_reply_port;
	int	      malloc_seq;
	struct malloc mallocs[MAX_MALLOCS];
};

typedef struct mach_thread *mach_thread_t;

struct mach_thread threads[MAX_THREADS];

#define thread_stack(addr)	((unsigned)addr & ~(STACK_SIZE-1)) 

extern mach_thread_t thread_self();
extern boolean_t threads_initialized;

#endif /* _MACH_SERVICES_H */
	














