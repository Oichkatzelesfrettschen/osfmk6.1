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
#include <pager_types.h>

#include <mach/memory_object.h>


extern int banner;

struct pager_object {
	vm_size_t				size;
	boolean_t				filled;
	struct memory_object_behave_info	behavior;
	struct memory_object_attr_info		attributes;
	vm_prot_t				lock;
	boolean_t				precious;
	boolean_t				temporary;
	mach_port_t				control;
	mach_port_t				name;
	vm_offset_t				datas[1]; /* 1 per page */
};

typedef struct pager_object *pager_object_t;

boolean_t       pager_server();
boolean_t       memory_object_server();
boolean_t       pager_demux();

mach_port_t server_port;
mach_port_t port_set;
jmp_buf saved_state;

pager_server_main(service_port)
mach_port_t service_port;
{
        char **argv;
        int argc = 0;

        test_init();

	if (service_port) {
		server_port = service_port;
	} else {
		MACH_CALL( mach_port_allocate, (mach_task_self(),
						MACH_PORT_RIGHT_RECEIVE,
						&server_port));
		if (debug)
			printf("server port %x\n", server_port);
		MACH_CALL (netname_check_in, (name_server_port,
					      PAGER_SERVER_NAME,
					      mach_task_self(),
					      server_port));
	}
        MACH_CALL( mach_port_allocate, (mach_task_self(),
					MACH_PORT_RIGHT_PORT_SET,
					&port_set));
        MACH_CALL( mach_port_move_member, (mach_task_self(),
					   server_port,
					   port_set));

        if (debug)
                printf("calling mach_msg_server: port %x\n",
                       port_set);
	while (1) {
		thread_malloc_state_t mallocs;

		mallocs = save_mallocs(thread_self());
		if (mach_setjmp(saved_state))
			restore_mallocs(thread_self(), mallocs);
		else {
			MACH_CALL( mach_msg_server,
				  (pager_demux,
				   PAGER_MSG_BUF_SIZE,
				   port_set,
				   MACH_MSG_OPTION_NONE));
		}
	}
}

boolean_t
pager_demux(in, out)
{
	if (memory_object_server(in, out))
		return(TRUE);
	else if (pager_server(in, out))
		return(TRUE);
	else
		return(server_server(in, out));
}

kern_return_t
do_create_memory_objects(
	mach_port_t		server,
	int			nobjs,
	mach_port_array_t	*obj_ports,
	mach_msg_type_number_t	*count,
	vm_size_t		size,
	boolean_t		fill,
	vm_prot_t      		prot,
	boolean_t		precious,
	boolean_t		temporary)
{
	mach_port_array_t	ports;
	pager_object_t		objects, obj;
	mach_port_t		port;
	vm_offset_t		data;
	int			i;

	if (debug) {
		printf("create_memory_objects: nobjs %d size %d fill %d\n",
			      nobjs, size, fill);
		printf("                       prot %d precious %d\n",
			      prot, precious);
	}

	MACH_CALL(vm_allocate, (mach_task_self(),
				(vm_offset_t *)&ports,
				nobjs*sizeof(mach_port_t),
				TRUE));

	MACH_CALL(vm_allocate,
		  (mach_task_self(),
		   (vm_offset_t *)&objects,
		   nobjs*(sizeof(struct pager_object)
			  +(size/vm_page_size) * sizeof(vm_offset_t)),
		   TRUE));

	if (fill)
		MACH_CALL(vm_allocate, (mach_task_self(),
					&data,
					size*nobjs,
					TRUE));

	for (i = 0; i < nobjs; i++) {
	  	obj = objects+i;
		port = (mach_port_t)obj;
		MACH_CALL(mach_port_allocate_name, (mach_task_self(),
					      MACH_PORT_RIGHT_RECEIVE,
					      port));
		*(ports+i) = port;
		obj->filled = fill;
		obj->size = size;
		obj->lock = VM_PROT_ALL & ~prot;
		obj->precious = precious;
		obj->temporary = temporary;
		obj->control = MACH_PORT_NULL;
		obj->name = MACH_PORT_NULL;
		obj->attributes.object_ready = TRUE;
		obj->attributes.may_cache = !temporary;
		obj->attributes.copy_strategy = MEMORY_OBJECT_COPY_TEMPORARY;
		obj->behavior.copy_strategy = MEMORY_OBJECT_COPY_TEMPORARY;
		obj->behavior.temporary = temporary;
		obj->behavior.invalidate = FALSE;
		obj->behavior.write_completions = FALSE;
		if (fill) {
			int j;
			int npages = size/vm_page_size;

			for (j = 0; j < npages; j ++) {
				obj->datas[j] = data + j*vm_page_size;
				*(int *)(obj->datas[j]) = READ_PATTERN;
			}
		}
		data += size;
		MACH_CALL( mach_port_move_member, (mach_task_self(),
						   port,
						   port_set));
	}		

	if (debug)
		printf("create_memory_objects: ports (%x) = %x\n",
			      ports, *ports);
	*count = nobjs;
	*obj_ports = ports;
	return(KERN_SUCCESS);
}

kern_return_t
do_destroy_memory_objects(
	mach_port_t		server,
	mach_port_array_t	obj_ports,
	mach_msg_type_number_t	count)
{
	/* 
	 * Assumes the ports to be detroyed have
	 * been allocated together at once with
	 * create_pager_objects
	 */  

	pager_object_t		obj;
	mach_port_t		port;
	int			i;

	for (i = 0; i < count; i++) {
	  	port = *(obj_ports+i);
		obj = (pager_object_t) port;
		if (debug)
			printf("destroy_memory_objects: port %d: %x\n",
			       i, port);
		MACH_CALL( mach_port_move_member, (mach_task_self(),
						   port,
						   MACH_PORT_NULL));
		MACH_CALL(mach_port_destroy, (mach_task_self(),
					      port));
		if (obj->filled) {
			int j;
			int npages = obj->size/vm_page_size;

			for (j = 0; j < npages; j++) {
				if (obj->datas[j]) {
					MACH_CALL(vm_deallocate,
						  (mach_task_self(),
						   obj->datas[j],
						   vm_page_size));
				}
			}
		}
		/* 
		 * We do not use MACH_CALL as these call might fail
		 * if memory_object_terminate comes first
		 * 
		 * We shouldn't destroy ports already destroyed 
		 * as the name might have been recycled and already in
		 * use for another purpose
		 */
		mach_port_destroy (mach_task_self(), obj->control);
		mach_port_destroy (mach_task_self(), obj->name);
	}

	obj = (pager_object_t) *obj_ports;


	MACH_CALL(vm_deallocate,
		  (mach_task_self(),
		   (vm_offset_t)obj,
		   count*(sizeof(struct pager_object)
			  +(obj->size/vm_page_size) * sizeof(vm_offset_t))));

	MACH_CALL(vm_deallocate, (mach_task_self(),
				(vm_offset_t)obj_ports,
				count*sizeof(mach_port_t)));

	return(KERN_SUCCESS);
}

kern_return_t
memory_object_init(
	mach_port_t     pager,
	mach_port_t     control,
	mach_port_t     name,
	vm_size_t       size)
{
	pager_object_t obj;

	if (debug)
		printf("memory object init\n");

	obj = (pager_object_t) pager;

	MACH_CALL (memory_object_change_attributes,
		   (control,
		    MEMORY_OBJECT_ATTRIBUTE_INFO,
		    (memory_object_info_t) &obj->attributes,
		    MEMORY_OBJECT_ATTR_INFO_COUNT,
		    MACH_PORT_NULL));

	obj->control = control;
	obj->name = name;
	server_count--;
	return(KERN_SUCCESS);
}

kern_return_t
memory_object_terminate(
	mach_port_t     pager,
	mach_port_t     control,
	mach_port_t     name)
{
	pager_object_t	obj = (pager_object_t) pager;

	if (debug)
		printf("memory object terminate\n");

	/*
	 * We have to avoid extra mach_port_destroy for 
	 * the port name we are destroying here
	 */
	if (obj->control == control && obj->name == name) {
		obj->control = MACH_PORT_NULL;
		obj->name = MACH_PORT_NULL;
	} else
		printf("m_o_terminate: pager_object table is broken\n");

	MACH_CALL(mach_port_destroy, (mach_task_self(), control));
	MACH_CALL(mach_port_destroy, (mach_task_self(), name));
	return(KERN_SUCCESS);
}

kern_return_t
memory_object_data_request(
	memory_object_t pager,
	mach_port_t     reply,
	vm_offset_t     offset,
	vm_size_t       length,
	vm_prot_t       prot)
{
	pager_object_t obj;
	vm_offset_t data;

	if  (debug)
		printf("memory_object_data_request(%x, %x, %x, %x, %x)\n",
		       pager, reply, offset, length, prot);
	obj = (pager_object_t) pager;
	data = obj->datas[offset/vm_page_size];
	if (obj->precious)
		obj->datas[offset/vm_page_size] = 0;
	if  (debug)
		printf("supplies %x lock %x precious %x\n",
		       data, obj->lock, obj->precious);

	MACH_CALL(memory_object_data_supply, (reply,
					      offset,
					      data,
					      length,
					      obj->precious, /* deallocate */
					      obj->lock,
					      obj->precious,
					      MACH_PORT_NULL));

	server_count--;
	return(KERN_SUCCESS);
}

kern_return_t
memory_object_data_unlock(
        memory_object_t pager,
        mach_port_t     control,
        vm_offset_t     offset,
        vm_size_t       size,
	vm_prot_t	prot)
{
	if (debug) 
		printf("memory object unlock off : %d size : %d prot %x\n",
		       offset, size, prot);

	MACH_CALL(memory_object_lock_request, (control,
					       offset,
					       size,
					       MEMORY_OBJECT_RETURN_ALL,
					       FALSE, /* should flush */
					       VM_PROT_ALL & ~prot,
					       MACH_PORT_NULL));

        return(KERN_SUCCESS);
}

kern_return_t
memory_object_copy(
        memory_object_t 	old_memory_object,
        memory_object_control_t old_memory_control,
        vm_offset_t     	offset,
        vm_size_t       	length,
        memory_object_t 	new_memory_object)
{
	if (debug) 
	  printf("memory object copy\n");

        return(KERN_FAILURE);
}
kern_return_t
memory_object_lock_completed(
        memory_object_t pager,
        mach_port_t     pager_request,
        vm_offset_t     offset,
        vm_size_t       length)
{
	if (debug) 
		printf("memory object lock completed\n");

        return(KERN_SUCCESS);
}

kern_return_t
memory_object_supply_completed(
        memory_object_t pager,
        mach_port_t     pager_request,
        vm_offset_t     offset,
        vm_size_t       length,
        kern_return_t   result,
        vm_offset_t     error_offset)
{
	if (debug)
		printf("memory object supply completed\n");

        return(KERN_FAILURE);
}
kern_return_t
memory_object_data_return(
        memory_object_t pager,
        mach_port_t     control,
        vm_offset_t     offset,
        pointer_t       addr,
        vm_size_t       size,
        boolean_t       dirty,
        boolean_t       copy)
{
	pager_object_t obj;

	if (debug)
		printf("memory object data return: offset :%x size :%x\n",
		       offset,size);

	obj = (pager_object_t) pager;
	if (dirty) {
		if (*(int *)addr != WRITE_PATTERN)
			printf("data_return: invalid content (%x instead of %x)\n",
			       *(int *)addr, WRITE_PATTERN);
	} else {
		if (*(int *)addr != READ_PATTERN)
			printf("data_return: invalid content (%x instead of %x)\n",
			       *(int *)addr, READ_PATTERN);
	}
	if (obj->precious && !obj->datas[offset/vm_page_size])
		obj->datas[offset/vm_page_size] = addr;
	else {
		MACH_CALL(vm_deallocate, (mach_task_self(),
					  addr,
					  vm_page_size));
	}
	return(KERN_SUCCESS);
}

kern_return_t
memory_object_synchronize(
        mach_port_t             pager,
        mach_port_t             control,
        vm_offset_t             offset,
        vm_offset_t             length,
        vm_sync_t               flags )
{
	if (debug) 
		printf("memory object synchronize\n");

	MACH_CALL(memory_object_synchronize_completed,(control,
						       offset,
						       length));
	return(KERN_SUCCESS);
}

kern_return_t
memory_object_notify(
        mach_port_t             pager,
        mach_port_t             pager_request,
        mach_port_t		host,
        vm_size_t               size )
{
	if (debug) 
		printf("memory object notify\n");

	return(KERN_FAILURE);
}

kern_return_t
memory_object_rejected(
        mach_port_t             pager,
        mach_port_t             pager_request,
        kern_return_t           reason)
{
	if (debug) 
		printf("memory object rejected\n");

	return(KERN_FAILURE);
}

kern_return_t
memory_object_change_completed(
        mach_port_t             reply,
        mach_port_t             pager_request,
        memory_object_flavor_t  flavor)
{
	if (debug) 
		printf("memory object cange completed\n");

	return(KERN_FAILURE);
}





