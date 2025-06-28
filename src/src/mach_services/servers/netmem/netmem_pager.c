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
 *      File:   netmem_pager.c
 *      Author: David L. Black
 *
 *      The actual netmem_msg pager.  This is a simple pager that provides
 *	one page of memory, accessible either via mapping or a message
 *	based interface.  It only works with one kernel.
 */

#include <mach.h>
#include <mach_init.h>
#include <mig_errors.h>
#include <servers/netname.h>

#include <stdio.h>

#include <mach/message.h>

#include "netmem_defs.h"
#include "netmem_obj.h"
#include "netmem_msg_reply.h"

netmem_obj_data_t	master_obj;


/*
 *	Initialization routine for the server.
 */
void
init()
{
	kern_return_t	r;

	/*
	 *	Initialize the object.  This had better work.
	 */
	r = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
			       &master_obj.object_port);
	if (r != KERN_SUCCESS) {
		printf("Can't allocate object/service port\n");
		exit(-1);
	}

	master_obj.control_port = MACH_PORT_NULL;
	master_obj.state = NETMEM_OBJ_NONE;
	master_obj.data = (vm_address_t) 0;
	master_obj.pending_op_queue = PENDING_OP_NULL;
	master_obj.pending_kernel_access = VM_PROT_NONE;

	/*
	 *	Now check it into the name service.  No signature port
	 *	means this is unprotected.
	 */
	r = netname_check_in(name_server_port, NETMEM_NAME, MACH_PORT_NULL,
			master_obj.object_port);
	if (r != NETNAME_SUCCESS) {
		mach_error("Check In:", r);
		exit(-1);
	}

#ifdef 	DEBUG
	printf("netname_check_in %s 0x%x\n", NETMEM_NAME, 
		master_obj.object_port);
#endif
}


/*
 *	Conversion routine from port to object.
 *	Very simple because there's only one object.
 */
netmem_obj_t
netmem_lookup(port)
memory_object_t	port;
{
	if (port != master_obj.object_port) {
		printf("object mismatch\n");
		return(NETMEM_OBJ_NULL);  /* Can't happen */
	}

	return(&master_obj);
}

#define		SEND_RIGHT_MAX	10000

/*
 *	Routine for lazy discarding of send rights to control port.
 *	Add one more right to the number we have.  If over limit, get
 *	rid of all but one.  We retain one right to ensure that the port's
 *	name doesn't change.
 */
void
netmem_control_cleanup(obj)
netmem_obj_t	obj;
{
	obj->send_rights++;

	if (obj->send_rights > SEND_RIGHT_MAX) {
		(void) mach_port_mod_refs(mach_task_self(), obj->control_port,
				   MACH_PORT_RIGHT_SEND, -(SEND_RIGHT_MAX));
		obj->send_rights -= SEND_RIGHT_MAX;
	}
}
				   

/*
 *	Routines for handling pending operations.
 */
void
pending_op_queue(obj, op, offset, value, reply_to)
netmem_obj_t	obj;
int		op;
vm_address_t	offset;
int		value;
mach_port_t	reply_to;
{
	pending_op_t	pend, *pend_ptr;

	/*
	 *	Using malloc() here is not particularly efficient.
	 */
	pend = (pending_op_t) malloc(sizeof(pending_op_data_t));

	/*
	 *	Initialize pending op and add to queue.  This results in
	 *	LIFO pending operation order.
	 */
	pend->op = op;
	pend->offset = offset;
	pend->value = value;
	pend->reply_port = reply_to;
	pend->next = obj->pending_op_queue;
	obj->pending_op_queue = pend;
}

/*
 *	Execute operations from pending op queue.  This routine
 *	checks the object state to find out what is allowed, so any
 *	state transition must occur BEFORE calling this routine.
 *
 *	Processing stops when the queue is empty or a write is found
 *	that can't be executed because the server only has read permission.
 *
 *	Caller is responsible for handling pending kernel access.
 */

void
pending_op_execute(obj)
netmem_obj_t	obj;
{
	pending_op_t	pend, new_pend;
	kern_return_t	result;

	pend = obj->pending_op_queue;

	while (pend != PENDING_OP_NULL) {
	    if (pend->op == PENDING_READ) {

		/*
		 *  Can always do read.  Must send reply ourselves.
		 */
	    	result = netmem_read(obj->object_port, MACH_PORT_NULL,
				     pend->offset, &pend->value);
		printf("read_reply\n");
		result = netmem_read_reply(pend->reply_port, result,
					   pend->value);
	    }
	    else {

		/*
		 *	Write case.  Stop if read only.
		 */
		if (obj->state == NETMEM_OBJ_READ)
			break;

		result = netmem_write(obj->object_port, MACH_PORT_NULL,
				      pend->offset, pend->value);
		printf("write_reply\n");
		result = netmem_write_reply(pend->reply_port, result);
	    }

	    /*
	     *	Reply might fail if client died.  INVALID_DEST is the only
	     *	possible error.
	     */

	    if (result == MACH_SEND_INVALID_DEST) {
		mach_port_deallocate(mach_task_self(), pend->reply_port);
	    }

	    /*
	     *	Onward to next pending operation.
	     */

	    new_pend = pend->next;
	    free(pend);
	    pend = new_pend;
	}

	/*
	 *	Reset pending op queue.  This sets it to null if
	 *	we ran it completely, else to the first write that
	 *	stopped us.
	 */
	obj->pending_op_queue = pend;
}


/*
 *	Routines for handling messages from clients
 */

mach_error_t
netmem_read(object, reply_to, offset, value)
mach_port_t	object;
mach_port_t	reply_to;
vm_address_t	offset;
int		*value;
{
	netmem_obj_t	my_obj;

	printf("read\n");

	/*
	 *	Check arguments.
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL)
		return (NETMEM_BAD_OBJECT);

	if (offset < 0 || offset >= (vm_page_size/sizeof(int)))
		return (NETMEM_BAD_OFFSET);

	/*
	 *	Execute the read.
	 */
	switch(my_obj->state) {

	    case NETMEM_OBJ_NONE:

		/*
		 *	Need to initialize.  Server has exclusive access
		 *	(write) when we're done.
		 */
       		(void) vm_allocate(mach_task_self(), &my_obj->data,
			    vm_page_size, TRUE);
		my_obj->state = NETMEM_OBJ_SERVER_WRITE;

		/*
		 *  Fall through ...
		 */
	    case NETMEM_OBJ_READ:
	    case NETMEM_OBJ_SERVER_WRITE:

		/*
		 *	Read the value.
		 */
		*value = *(((int *) (my_obj->data)) + offset);

		return(ERR_SUCCESS);

	    case NETMEM_OBJ_KERNEL_WRITE:

		/*
		 *	Request kernel to give up write access to page.
		 *	Ask for a reply when it's finished.
		 */
		printf("lock_request: clean, lock %d\n", VM_PROT_WRITE);
		(void) memory_object_lock_request(my_obj->control_port,
					   (vm_address_t)0, vm_page_size,
		           		   TRUE, FALSE, VM_PROT_WRITE,
					   my_obj->object_port);
		my_obj->state = NETMEM_OBJ_CLEANING;

		/*
		 *  Fall through ...
		 */

	    case NETMEM_OBJ_CLEANING:
	    case NETMEM_OBJ_FLUSHING:

		/*
		 *	Wait for this change to complete.  Enqueue
		 *	this operation to be handled later.
		 */
		pending_op_queue(my_obj, PENDING_READ, offset, 0, reply_to);

		/*
		 *	Reply will happen when lock_request completes.
		 */
		return(MIG_NO_REPLY);

	    default:
		printf("Bad state %d\n", my_obj->state);
		return(NETMEM_FAILURE);		
	}
	/*NOTREACHED*/
}


mach_error_t
netmem_write(object, reply_to, offset, value)
mach_port_t	object;
mach_port_t	reply_to;
vm_address_t	offset;
int		value;
{
	netmem_obj_t	my_obj;

	printf("write\n");

	/*
	 *	Check arguments. 
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL)
		return (NETMEM_BAD_OBJECT);

	if (offset < 0 || offset >= (vm_page_size/sizeof(int)))
		return (NETMEM_BAD_OFFSET);  

	/*
	 *	Execute the write.
	 */
	switch(my_obj->state) {
	    case NETMEM_OBJ_NONE:

		/*
		 *	Need to initialize.  Server has exclusive access
		 *	(write) when we're done.
		 */
       		(void) vm_allocate(mach_task_self(), &my_obj->data,
			    vm_page_size, TRUE);
		my_obj->state = NETMEM_OBJ_SERVER_WRITE;

		/*
		 *  Fall through ...
		 */
	    case NETMEM_OBJ_SERVER_WRITE:

		/*
		 *	Write the value.
		 */
		*((int *) (my_obj->data) + offset) = value;

		return(ERR_SUCCESS);

	    case NETMEM_OBJ_READ:
	    case NETMEM_OBJ_KERNEL_WRITE:

		/*
		 *	Request kernel to give up all access to page.
		 *	Ask for a reply when it's finished.
		 */
		printf("lock_request: clean, flush, lock %d\n",
		       VM_PROT_ALL);
		(void) memory_object_lock_request(my_obj->control_port,
					   (vm_address_t)0, vm_page_size,
		           		   TRUE, TRUE, VM_PROT_ALL,
					   my_obj->object_port);
		my_obj->state = NETMEM_OBJ_FLUSHING;

		/*
		 *  Fall through ...
		 */
	    case NETMEM_OBJ_CLEANING:
	    case NETMEM_OBJ_FLUSHING:

		/*
		 *	Wait for this change to complete.  Enqueue
		 *	this operation to be handled later.
		 */
		pending_op_queue(my_obj, PENDING_WRITE, offset, value,
				 reply_to);

		/*
		 *	Reply will happen when lock_request completes.
		 */
		return(MIG_NO_REPLY);

	    default:
		printf("Bad state %d\n", my_obj->state);
		return(NETMEM_FAILURE);		
	}
	/*NOTREACHED*/
}


/*
 *	Routines for handling paging messages from the kernel.
 */

/*
 *	Kernel request to initialize object.
 */
kern_return_t
memory_object_init(object, control, name, obj_page_size)
memory_object_t		object;
memory_object_control_t	control;
memory_object_name_t	name;
vm_size_t	        obj_page_size;
{
	netmem_obj_t	my_obj;

	printf("init\n");
	
	/*
	 *	Check the object's page size.
	 */
	if (obj_page_size != vm_page_size) {
		printf("page size mismatch\n");
		return(KERN_FAILURE);
	}

	/*
	 *	Check that it's our object.
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL) {
		return(KERN_FAILURE);
	}

	/*
	 *	Set up control port.  If it's not NULL, we've just
	 *	won an init/terminate race.  This code does not cope
	 *	with that race.  The right thing to do is allocate a
	 *	new data structure to represent the object.  This
	 *	race will be removed from future Mach kernels.
	 */
	if (my_obj->control_port != MACH_PORT_NULL) {
		printf("init before terminate\n");
		if (my_obj->control_port == control) {
			printf("control port match: I'm confused\n");
			return(KERN_FAILURE);
		}

		/*
		 *	Get rid of all the send rights on that port.
		 */
		(void) mach_port_mod_refs(mach_task_self(),
					  my_obj->control_port,
					  MACH_PORT_RIGHT_SEND,
					  -(my_obj->send_rights));
	}

	my_obj->control_port = control;
	my_obj->send_rights = 1;

	/*
	 *	Reply: the object is ready.  Not cacheable, no special
	 *	copy strategy.
	 */
	printf("change_attributes\n");
	{
	memory_object_attr_info_data_t info;

	info.object_ready = TRUE;
	info.may_cache = FALSE;
	info.copy_strategy = MEMORY_OBJECT_COPY_NONE;
	
	(void) memory_object_change_attributes(control,
				MEMORY_OBJECT_ATTRIBUTE_INFO,
				(memory_object_info_t)&info,
				MEMORY_OBJECT_ATTR_INFO_COUNT,
				MACH_PORT_NULL);
	}

	/*
	 *	Send right on control port is held by object data
	 *	structure until termination.  Get rid of name port right.
	 *	mach_port_deallocate could also be used here.
	 */
	(void) mach_port_mod_refs(mach_task_self(), name,
				  MACH_PORT_RIGHT_SEND, -1);
	return(KERN_SUCCESS);
}


/*
 *	Kernel object termination.
 */
kern_return_t
memory_object_terminate(object, control, name)
memory_object_t		object;
memory_object_control_t	control;
memory_object_name_t	name;
{
	netmem_obj_t	my_obj;

	printf("terminate\n");
	
	/*
	 *	Check that it's our object.
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL)
		return(KERN_FAILURE);

	/*
	 *	Clear control port, and remove any send rights we have.
	 */
	if (my_obj->control_port == control) {
		my_obj->control_port = MACH_PORT_NULL;
		(void) mach_port_mod_refs(mach_task_self(), control,
			   MACH_PORT_RIGHT_SEND,-(my_obj->send_rights));
		my_obj->send_rights = 0;
	}

	/*
	 *	Reset object state: no kernel copy, so server write ok.
	 */
	my_obj->state = NETMEM_OBJ_SERVER_WRITE;

	/*
	 *	Now get rid of the receive rights that came in this message.
	 */
	(void) mach_port_mod_refs(mach_task_self(), control,
			   MACH_PORT_RIGHT_RECEIVE,-1);
	(void) mach_port_mod_refs(mach_task_self(), name,
			   MACH_PORT_RIGHT_RECEIVE,-1);

	return(KERN_SUCCESS);
}


/*
 *	Pagein
 */
kern_return_t
memory_object_data_request(object, control, offset, length, access)
memory_object_t 	object;
memory_object_control_t	control;
vm_address_t		offset;
vm_size_t		length;
vm_prot_t		access;
{
	netmem_obj_t	my_obj;
	vm_prot_t	lock_value;

	printf("data_request %d\n", access);

	/*
	 *	Check that it's our object.
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL)
		return(KERN_FAILURE);

	/*
	 *	Check offset and length arguments.
	 */
	if (offset > 0 || offset + length > vm_page_size) {

		/*
		 *	Accessing this region of the object is an error.
		 *
		 *	NOTE: This code assumes that there will not be
		 *	multi-page requests that include both valid
		 *	and invalid regions.  Current kernels do not
		 *	make such requests.  A request like that would
		 *	be split into its components here.
		 */
		printf("data_error\n");
		(void) memory_object_data_error(control, offset, length,
					 NETMEM_BAD_OFFSET);
		return(KERN_FAILURE);
	}

	/*
	 *	Now do something about it.
	 */
	switch (my_obj->state) {
	    case NETMEM_OBJ_NONE:

		/*
		 *	Need to initialize.  Initialize kernel to zeros
		 *	with data_unavailable.  Kernel gets write access.
		 */
       		(void) vm_allocate(mach_task_self(), &my_obj->data,
			    vm_page_size, TRUE);
		printf("data_unavailable\n");
		(void) memory_object_data_unavailable(control, offset, length);
		my_obj->state = NETMEM_OBJ_KERNEL_WRITE;
		break;

	    case NETMEM_OBJ_READ:
	    case NETMEM_OBJ_SERVER_WRITE:
	    case NETMEM_OBJ_KERNEL_WRITE:

		/*
		 *	Kernel does not have a copy.  Give it one.
		 *	If write permission wasn't requested, don't
		 *	grant it.
		 */
		if (access & VM_PROT_WRITE) {
			lock_value = VM_PROT_NONE;
			my_obj->state = NETMEM_OBJ_KERNEL_WRITE;
		}
		else {
			lock_value = VM_PROT_WRITE;
			my_obj->state = NETMEM_OBJ_READ;
		}			
		printf("data_provided: lock %d\n", lock_value);
		(void) memory_object_data_provided(control, offset,
					    my_obj->data, length, lock_value);
		break;

	    case NETMEM_OBJ_CLEANING:
	    case NETMEM_OBJ_FLUSHING:

		/*
		 *	This will be handled when the cleaning or flushing
		 *	completes.
		 */
		my_obj->pending_kernel_access |= access;
		my_obj->pending_kernel_type = PENDING_TYPE_DATA;
		break;

	    default:
		printf("Bad state %d\n", my_obj->state);
		break;
	}

	/*
	 *	Get rid of send right on control port.
	 */
	netmem_control_cleanup(my_obj);
	return(KERN_SUCCESS);
}


/*
 *      Unlock request.  The kernel has a read only page and wants to write.
 */
kern_return_t
memory_object_data_unlock(object, control, offset, length, access)
memory_object_t 	object;
memory_object_control_t	control;
vm_address_t		offset;
vm_size_t		length;
vm_prot_t		access;
{
	netmem_obj_t	my_obj;
	vm_prot_t	lock_value;

	printf("data_unlock %d\n", access);

	/*
	 *	Check that it's our object.
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL)
		return(KERN_FAILURE);

	/*
	 *	Check offset and length arguments.
	 */
	if (offset > 0 || offset + length > vm_page_size) {
		return(KERN_FAILURE);
	}

	/*
	 *	Can handle in read state, otherwise mark as pending.
	 */
	if (my_obj->state == NETMEM_OBJ_READ) {
		/*
		 *	Drop the read page lock.
		 */
		printf("lock_request: lock %d\n", VM_PROT_NONE);
		(void) memory_object_lock_request(control, offset, length,
		           		   FALSE, FALSE, VM_PROT_NONE,
					   MACH_PORT_NULL);
		my_obj->state = NETMEM_OBJ_KERNEL_WRITE;
	}
	else if (my_obj->state == NETMEM_OBJ_CLEANING ||
		 my_obj->state == NETMEM_OBJ_FLUSHING) {

	  	my_obj->pending_kernel_access |= access;
		my_obj->pending_kernel_type = PENDING_TYPE_LOCK;
	}
	else {
		printf("Unexpected state %d\n", my_obj->state);
	}

	/*
	 *	Get rid of send right on control port.
	 */
	netmem_control_cleanup(my_obj);
	return(KERN_SUCCESS);	
}


/*
 *	Pageout
 */
kern_return_t
memory_object_data_return(object, control, offset, data, count, dirty, kernel_copy)
memory_object_t 	object;
memory_object_control_t	control;
vm_address_t		offset;
pointer_t		data;
vm_size_t		count;
boolean_t		dirty;
boolean_t		kernel_copy;
{
	netmem_obj_t	my_obj;

	printf("data_return\n");

	/*
	 *	Check that it's our object.
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL)
		return(KERN_FAILURE);

	/*
	 *	Check offset and length arguments.  Failure return
	 *	causes mach_msg_server to deallocate data for us.
	 */
	if (offset > 0 || offset + count > vm_page_size) {
		printf("wrong data\n");
		return(KERN_FAILURE);
	}

	/*
	 *	Handle the write.  Swap the data for our old copy.
	 */
	(void) vm_deallocate(mach_task_self(), my_obj->data, vm_page_size);
	my_obj->data = data;

	/*
	 *	Either in kernel write state, or middle of a clean or flush.
	 *	In latter case, state changes when that operation completes.
	 */
	if (my_obj->state == NETMEM_OBJ_KERNEL_WRITE) {
		my_obj->state = NETMEM_OBJ_SERVER_WRITE;
	}
	else if (my_obj->state != NETMEM_OBJ_CLEANING &&
		 my_obj->state != NETMEM_OBJ_FLUSHING) {
			printf("Unexpected state %d\n", my_obj->state);
	}

	/*
	 *	Get rid of send right on control port.
	 */
	netmem_control_cleanup(my_obj);
	return(KERN_SUCCESS);
}	


/*
 *	Completion of a clean or flush operation.
 */
kern_return_t
memory_object_lock_completed(object, control, offset, length)
memory_object_t 	object;
memory_object_control_t	control;
vm_address_t		offset;
vm_size_t		length;
{
	netmem_obj_t	my_obj;
	vm_prot_t	lock_value;

  	printf("lock_completed\n");

	/*
	 *	Check that it's our object.
	 */
	if ((my_obj = netmem_lookup(object)) == NETMEM_OBJ_NULL)
		return(KERN_FAILURE);

	/*
	 *	Check offset and length arguments.
	 */
	if (offset > 0 || offset + length > vm_page_size) {
		return(KERN_FAILURE);
	}
	
	/*
	 *	Action depends on what state we were in, but cleaning
	 *	and flushing are similar.
	 */
	if (my_obj->state == NETMEM_OBJ_CLEANING) {

	  	/*
		 *	Kernel may have read only copy of page.
		 */
		my_obj->state = NETMEM_OBJ_READ;
	}
	else if (my_obj->state == NETMEM_OBJ_FLUSHING) {

	  	/*
		 *	Kernel no longer has page.
		 */
		my_obj->state = NETMEM_OBJ_SERVER_WRITE;
		my_obj->pending_kernel_type = PENDING_TYPE_DATA;
	}
	else {
		printf("unexpected state: %d\n", my_obj->state);
		return(KERN_FAILURE);
	}

	/*
	 *	Handle pending client message operations.
	 */
	pending_op_execute(my_obj);

	/*
	 *	Pending message write takes priority (cleaning state only).
	 */
	if (my_obj->pending_op_queue != PENDING_OP_NULL) {

		/*
		 *	Request kernel to give up all access to page.
		 *	Ask for a reply when it's finished.
		 */
		printf("lock_request: clean, flush, lock %d\n",
		       VM_PROT_ALL);
		(void) memory_object_lock_request(control,
					   (vm_address_t)0, vm_page_size,
		           		   TRUE, TRUE, VM_PROT_ALL,
					   my_obj->object_port);
		my_obj->state = NETMEM_OBJ_FLUSHING;

	}

	/*
	 *	Does the kernel want the page?
	 */
 	else if (my_obj->pending_kernel_access != VM_PROT_NONE) {
		        
		if (my_obj->pending_kernel_access & VM_PROT_WRITE) {
			lock_value = VM_PROT_NONE;
			my_obj->state = NETMEM_OBJ_KERNEL_WRITE;
		}
		else {
			lock_value = VM_PROT_WRITE;
			my_obj->state = NETMEM_OBJ_READ;
		}			

		my_obj->pending_kernel_access = VM_PROT_NONE;

		if (my_obj->pending_kernel_type == PENDING_TYPE_LOCK) {
		        printf("lock_request: lock %d\n", lock_value);
			(void) memory_object_lock_request(control,
					   (vm_address_t) 0, vm_page_size,
					   FALSE, FALSE, lock_value,
					   MACH_PORT_NULL);
		}
		else {
			printf("data_provided: lock %d\n", lock_value);
			(void) memory_object_data_provided(control, 
					    (vm_address_t) 0, my_obj->data,
					    vm_page_size, lock_value);
		}
	}

	/*
	 *	Get rid of send right on control port.
	 */
	netmem_control_cleanup(my_obj);
	return(KERN_SUCCESS);
}


/*
 *	Not used by this pager
 */
kern_return_t
memory_object_copy(old_object, old_control, offset, length, new_object)
memory_object_t 	old_object;
memory_object_control_t	old_control;
vm_address_t		offset;
vm_size_t		length;
memory_object_t 	new_object;
{
  	printf("object_copy\n");
	return(KERN_FAILURE);
}

kern_return_t
memory_object_supply_completed(object, control, offset, length, result,
				error_offset)
memory_object_t 	object;
memory_object_control_t	control;
vm_address_t		offset;
vm_size_t		length;
kern_return_t		result;
vm_offset_t		error_offset;
{
  	printf("supply_completed\n");
	return(KERN_FAILURE);
}

kern_return_t
memory_object_change_completed(object, control, flavor)
memory_object_t		object;
memory_object_control_t control;
memory_object_flavor_t	flavor;
{
	printf("change_completed\n");
	return(KERN_FAILURE);
}

kern_return_t
memory_object_synchronize(object, control, offset, length, flags)
memory_object_t         object;
memory_object_control_t control;
vm_address_t            offset;
vm_address_t            length;
vm_sync_t		flags;

{
        printf("synchronize\n");
	(void) memory_object_synchronize_completed(control, offset, length);
        return(KERN_SUCCESS);
}

kern_return_t
memory_object_notify(rep, control, host, psize)
memory_object_rep_t             rep;
memory_object_control_t         control;
mach_port_t                     host;
vm_size_t                       psize;
{
	printf("notify\n");
        return(KERN_FAILURE);
}

kern_return_t
memory_object_rejected(object, control, reason)
memory_object_t         object;
memory_object_control_t control;
kern_return_t           reason;
{
        printf("rejected\n");
        return(KERN_FAILURE);
}



/*
 *	Demux routine for mach_msg_server.  This allows us to splice
 *	two interfaces into mach_msg_server().
 */
boolean_t
netmem_demux(inmsg, outmsg)
mach_msg_header_t	*inmsg, *outmsg;
{
	return (netmem_msg_server(inmsg,outmsg) ||
	        memory_object_server(inmsg,outmsg));
}

/*
 *	Size of max message we're willing to receive or send.  Have to
 *	allow enough space for memory_object_messages from the kernel.
 *	This number is on the high side.
 */

#define NETMEM_MAX_MSG_SIZE	512

/*
 *	Main routine: initialize and loop forever handling messages.
 *	Ignore error returns from mach_msg_server.
 */
main()
{
	(void) init();
	while (1) {
		(void) mach_msg_server(netmem_demux, NETMEM_MAX_MSG_SIZE,
				master_obj.object_port, MACH_MSG_OPTION_NONE);
	}
}
