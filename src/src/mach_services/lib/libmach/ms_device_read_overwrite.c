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

#include <mach.h>
#include <mach/message.h>
#include <device/device.h>

kern_return_t device_read_overwrite(device, mode, recnum, bytes_wanted, data, data_count)
	mach_port_t		device;
	dev_mode_t		mode;
	recnum_t		recnum;
	int			bytes_wanted;
	vm_address_t		data;
	int			*data_count;
{
	kern_return_t result;

	result = syscall_device_read_overwrite(device, mode, recnum, 
					       bytes_wanted, data, data_count);
	if (result == MACH_SEND_INTERRUPTED) {
		io_buf_ptr_t		buf;
		mach_msg_type_number_t	count;
		
		result = mig_device_read(device, mode, recnum, bytes_wanted, 
					 &buf, &count);
		if (result == KERN_SUCCESS) {
			bcopy((char *)buf, (char *)data, count);
			(void) vm_deallocate(mach_task_self(), 
					     (vm_offset_t) buf, count);
			*data_count = count;
		}
	}
	return(result);
}
