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

int 	mem_use = MEM_USE_DEFAULT;	/* % of free memory used */
int  	use_opt = 0;
int	debug_ressources = 0;

ipc_ressources(task, ipc_space)
mach_port_t task;
struct ipc_space *ipc_space;
{
	mach_port_array_t names;
	mach_port_type_array_t types;
	mach_msg_type_number_t count;
	register i;
	mach_port_type_t port_type;

	MACH_CALL(mach_port_names, (task,
				    &names,
				    &count,
				    &types,
				    &count));

	bzero((char *)ipc_space, sizeof(*ipc_space));
	ipc_space->total = count;
	for (i=0 ; i<count; i++) {
		port_type = *(types + i);
		if (port_type & MACH_PORT_TYPE_SEND)
			ipc_space->send++;
		if (port_type & MACH_PORT_TYPE_RECEIVE)
			ipc_space->receive++;
		if (port_type & MACH_PORT_TYPE_SEND_ONCE)
			ipc_space->send_once++;
		if (port_type & MACH_PORT_TYPE_PORT_SET)
			ipc_space->port_set++;
		if (port_type & MACH_PORT_TYPE_DEAD_NAME)
			ipc_space->dead++;
		if (!(port_type & (MACH_PORT_TYPE_SEND|
			      MACH_PORT_TYPE_RECEIVE|
			      MACH_PORT_TYPE_SEND_ONCE|
			      MACH_PORT_TYPE_PORT_SET|
			      MACH_PORT_TYPE_DEAD_NAME)))
			ipc_space->other++;
		if (debug_ressources)
			printf("port name %x, type %x\n", *(names+i), port_type);
	}

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t) names,
				  count * sizeof(*names)));

	MACH_CALL(vm_deallocate, (mach_task_self(),
				  (vm_offset_t) types,
				  count * sizeof(*types)));

	if (debug_ressources) {
	  printf("ipc space:\n");
	  printf("Names	Send	Receive	Sonce	Pset	dead	other\n");
	  printf("%d	%d	%d	%d	%d	%d	%d\n",
		 ipc_space->total,
		 ipc_space->send,
		 ipc_space->receive,
		 ipc_space->send_once,
		 ipc_space->port_set,
		 ipc_space->dead,
		 ipc_space->other);
	}
}

struct ipc_space ipc_space_before;	
struct ipc_space ipc_space_after;

ipc_space_use(before, after)
struct ipc_space *before, *after;
{
	if (bcmp((char *)after, (char *)before, sizeof(struct ipc_space))) {
	  printf("consumed:\n");
	  printf("Names	Send	Receive	Sonce	Pset	dead	other\n");
	  printf("%d	%d	%d	%d	%d	%d	%d\n",
		 after->total - before->total,
		 after->send - before->send,
		 after->receive - before->receive,
		 after->send_once - before->send_once,
		 after->port_set - before->port_set,
		 after->dead - before->dead,
		 after->other - before->other);
	  return(1);
	}
	return(0);
}

vm_ressources(task, vm_space)
mach_port_t task;
struct vm_space *vm_space;
{
	vm_offset_t	addr;
	vm_size_t	size;
	vm_region_basic_info_data_t vm_info;
	mach_port_t	name;
	int		page_count;
	unsigned int	count = sizeof(vm_info);

	bzero((char *)vm_space, sizeof(*vm_space));
	for (addr = VM_MIN_ADDRESS;
	     addr < VM_MAX_ADDRESS;
	     addr += size) {
		if (vm_region (mach_task_self(),
				      &addr,
				      &size,
			              VM_REGION_BASIC_INFO,
				      (vm_region_info_t) &vm_info,
			              &count,
				      &name) != KERN_SUCCESS)
			break;
		vm_space->regions++;
		page_count = size/vm_page_size;
		vm_space->pages += page_count;
		
		if (vm_info.protection & VM_PROT_WRITE)
			vm_space->rw++;
		else if (vm_info.protection & VM_PROT_READ)
			vm_space->r++;
		if (vm_info.inheritance & VM_INHERIT_SHARE)
			vm_space->inh_share++;
		if (debug_ressources > 1)
			printf("address %x, %d pages , prot %x\n",
			       addr, page_count, vm_info.protection);
		mach_port_deallocate (mach_task_self(), name);
	}

	if (debug_ressources) {
	  printf("vm space:\n");
	  printf("vm_regs	Pages	R/W	RO	Inh_share\n");
	  printf("%d	%d	%d	%d	%d\n",
		 vm_space->regions,
		 vm_space->pages,
		 vm_space->rw,
		 vm_space->r,
		 vm_space->inh_share);
	}
}

struct vm_space vm_space_before;	
struct vm_space vm_space_after;

vm_space_use(before, after)
struct vm_space *before, *after;
{
	if (bcmp((char *)after, (char *)before, sizeof(struct vm_space))) {
		printf("consumed:\n");
		printf("vm_regs	Pages	R/W	RO	Inh_share\n");
		printf("%d	%d	%d	%d	%d\n",
		       after->regions - before->regions,
		       after->pages - before->pages,
		       after->rw - before->rw,
		       after->r - before->r,
		       after->inh_share - before->inh_share);
	  return(1);
	}
	return(0);
}

ressources_start()
{
	ipc_ressources(mach_task_self(), &ipc_space_before);
	vm_ressources(mach_task_self(), &vm_space_before);
}
 
ressources_stop()
{
	ipc_ressources(mach_task_self(), &ipc_space_after);
	vm_ressources(mach_task_self(), &vm_space_after);
}

ressources_use(message)
char *message;
{
	if (ipc_space_diff(&ipc_space_before, &ipc_space_after)) {
		printf("%s", message);
		ipc_space_use(&ipc_space_before, &ipc_space_after);
	}
	if (vm_space_diff(&vm_space_before, &vm_space_after)) {
		printf("%s", message);
		vm_space_use(&vm_space_before, &vm_space_after);
	}
}

struct vm_space d_vm_space_before, d_vm_space_after;
struct ipc_space d_ipc_space_before, d_ipc_space_after;
int debug_ressources_initialized;

debug_ressources_reset()
{
	if (!debug_ressources_initialized) {
		ipc_ressources(mach_task_self(), &d_ipc_space_before);
	   	vm_ressources(mach_task_self(), &d_vm_space_before);
	   	debug_ressources_initialized= 1;
	}					
}

debug_ressources_test(file, line)
char *file;
int line;
{	
	ipc_ressources(mach_task_self(), &d_ipc_space_after);
	vm_ressources(mach_task_self(), &d_vm_space_after);
	if (ipc_space_diff(&d_ipc_space_before, &d_ipc_space_after)) {
		printf("At line %d in file %s ", line, file);
		ipc_space_use(&d_ipc_space_before, &d_ipc_space_after);
	}
	if (vm_space_diff(&d_vm_space_before, &d_vm_space_after)) {
		printf("At line %d in file %s ", line, file);
		vm_space_use(&d_vm_space_before, &d_vm_space_after);
	}
	bcopy((char *)&d_vm_space_after,
	      (char *)&d_vm_space_before,
	      sizeof(struct vm_space));
	bcopy((char *)&d_ipc_space_after,
	      (char *)&d_ipc_space_before,
		 sizeof(struct ipc_space));
}

#define vm_assign(vm_stat, vms)					\
		(vms)->free = (vm_stat)->free_count;		\
		(vms)->active = (vm_stat)->active_count;	\
		(vms)->inactive = (vm_stat)->inactive_count;	\
		(vms)->wired = (vm_stat)->wire_count;		\
		(vms)->zero_fill = (vm_stat)->zero_fill_count;	\
		(vms)->pagein = (vm_stat)->pageins;		\
		(vms)->pageout = (vm_stat)->pageouts;		\
		(vms)->fault = (vm_stat)->faults;		\
		(vms)->cow = (vm_stat)->cow_faults;		\
		(vms)->lookup = (vm_stat)->lookups;

struct vm_stat vm_stat_before, vm_stat_after;

get_vm_stats(vms)
struct vm_stat *vms;
{
	vm_statistics_data_t vm_stats;
	struct host_basic_info basic_info;
	int count;

	bzero((char *)vms, sizeof(*vms));

	count = sizeof(basic_info);
	MACH_CALL(host_info, (mach_host_self(),
			      HOST_BASIC_INFO,
			      &basic_info,
			      &count));
	vms->real = basic_info.memory_size/vm_page_size;

	count = sizeof(vm_stats);
	MACH_CALL( host_statistics, (privileged_host_port,
				     HOST_VM_INFO,
				     &vm_stats,
				     &count));
	vm_assign(&vm_stats, vms);      
}

/*
012345678012345678012345678012345678012345678012345678012345678012345678012345678
vm real   free    inact      act    wired   faults   cows lookups     in/out
12345671234567 +1234567  1234567  1234567  12345671234567123456781234567/1234567
       1234567          +1234567 +1234567
1234567
*/

print_vm_stats()
{
	bcopy((char *)&vm_stat_after,
	      (char *)&vm_stat_before,
	      sizeof(struct vm_stat));
	get_vm_stats(&vm_stat_after);
	if (!vm_opt)
		return;
	printf("vm real   free    inact      act    wired   faults   cows lookups     in/out\n");
	printf("%7d%7d +%7d  %7d  %7d  %7d%7d%8d%7d/%d\n",
	       vm_stat_after.real,
	       vm_stat_after.free, vm_stat_after.inactive,
	       vm_stat_after.active, vm_stat_after.wired,
	       vm_stat_after.fault - vm_stat_before.fault,
	       vm_stat_after.cow - vm_stat_before.cow,
	       vm_stat_after.lookup - vm_stat_before.lookup,
	       vm_stat_after.pagein - vm_stat_before.pagein,
	       vm_stat_after.pageout - vm_stat_before.pageout);
	printf("       %7d          +%7d +%7d\n",
	       vm_stat_after.free + vm_stat_after.inactive,
	       vm_stat_after.active, vm_stat_after.wired);
	printf("%7d\n",
	       vm_stat_after.free + vm_stat_after.inactive +
	       vm_stat_after.active + vm_stat_after.wired);

}

int mem_size()
{
	struct vm_stat vm_stats;
	int free;

	get_vm_stats(&vm_stats);
	free = vm_stats.free + vm_stats.inactive;
	free = (free * mem_use) / 100;
	if (!free) {
		free = vm_stats.real *  mem_use/100;
		printf("No free memory, use %d %% of total memory = %d pages\n\n",
		       mem_use, free);
	} else if (free > vm_stats.free && standalone && !use_opt) {
		printf("%d %% of free+inactive memory (%d) > free memory (%d)\n",
		       mem_use, free, vm_stats.free);
		free = (vm_stats.free * 90)/100;
		printf("Instead use 90 %% of free memory = %d pages\n\n", free);
	} else {
		printf("Use %d %% of free+inactive memory = %d pages\n\n",
		       mem_use, free);
	}
	
	return(free);
}



