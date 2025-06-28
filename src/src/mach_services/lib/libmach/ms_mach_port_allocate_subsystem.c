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

static mach_port_qos_t		qos_template;

kern_return_t
mach_port_allocate_subsystem(
	task_port_t		task,
	mach_port_name_t	subsystem,
	mach_port_name_t	*namep)
{
	kern_return_t		kr;
	mach_port_qos_t		qos = qos_template;

	kr = syscall_mach_port_allocate_full(task, MACH_PORT_RIGHT_RECEIVE,
					subsystem, &qos, namep);
	if (kr == MACH_SEND_INTERRUPTED)
	    kr = mig_mach_port_allocate_full(task, MACH_PORT_RIGHT_RECEIVE,
					subsystem, &qos, namep);

	return(kr);
}

