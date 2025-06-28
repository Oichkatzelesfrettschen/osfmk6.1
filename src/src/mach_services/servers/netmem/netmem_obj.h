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
 *      File:   netmem_obj.h
 *      Author: David L. Black
 *
 *      Internal definitions for netmem_msg server.
 */

#include <mach.h>

/*
 *	Possible object states
 */
#define	NETMEM_OBJ_NONE		1	/* No memory exists for the object */
#define NETMEM_OBJ_READ 	2	/* Server has read only copy, kernel
				   	   may have read only copy */
#define NETMEM_OBJ_SERVER_WRITE	3	/* Server has writable copy, no
					   kernel copy */
#define NETMEM_OBJ_KERNEL_WRITE 4	/* Kernel may have writable copy,
					   server has no-access copy */
#define NETMEM_OBJ_CLEANING	5	/* Pending request to kernel to
					   clean page and set read-only */
#define NETMEM_OBJ_FLUSHING	6	/* Pending request to kernel to
					   clean and flush page */
/*
 *	Structure for a pending message operation
 */
#define PENDING_READ	1
#define PENDING_WRITE	2

struct	pending_op {
	struct pending_op	*next;
	int			op;		/* read or write */
	vm_address_t		offset;
	int			value;
	mach_port_t		reply_port;
};

typedef struct pending_op	*pending_op_t, pending_op_data_t;

#define PENDING_OP_NULL	((pending_op_t) 0)

/*
 *	Object data structure
 */
struct netmem_obj {
	mach_port_t		object_port;	/* represents object */
	mach_port_t		control_port;   /* kernel control port */
	int			send_rights;    /* control send right count */
	int			state;		/* who can access how */
	vm_address_t		data;		/* pointer to contents */

	/*
	 *	Next two fields used only in cleaning and flushing states.
	 */
	pending_op_t		pending_op_queue;	/* pending msg ops */
	vm_prot_t		pending_kernel_access;  /* pending kernel op */
	int			pending_kernel_type;    /* which kernel op */
#define PENDING_TYPE_LOCK	1
#define PENDING_TYPE_DATA	2
};

typedef struct netmem_obj	*netmem_obj_t, netmem_obj_data_t;

#define NETMEM_OBJ_NULL		((netmem_obj_t) 0)
