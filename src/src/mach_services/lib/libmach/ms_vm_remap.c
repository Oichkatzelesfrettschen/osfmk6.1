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
 * 
 */
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */

#include <mach.h>
#include <mach/message.h>

kern_return_t
vm_remap(
	task_port_t	target_map,
	vm_address_t	*address,
	vm_size_t	size,
	vm_address_t	mask,
	boolean_t	anywhere,
	task_port_t	src_map,
	vm_offset_t	memory_address,
	boolean_t	copy,
	vm_prot_t	*cur_protection,
	vm_prot_t	*max_protection,
	vm_inherit_t	inheritance)
{
	kern_return_t kr;

	kr = syscall_vm_remap(target_map, address, size, mask,
			anywhere, src_map, memory_address, copy,
			cur_protection, max_protection, inheritance);

	if (kr == MACH_SEND_INTERRUPTED)
		kr = mig_vm_remap(target_map, address, size, mask,
				anywhere, src_map, memory_address, copy,
				cur_protection, max_protection, inheritance);

	return kr;
}
