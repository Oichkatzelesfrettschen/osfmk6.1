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
 *	File:		clock_res.c
 *	Purpose:
 *		Routines to implement old clock_get_res() and
 *		clock_set_res() calls.
 */

#include <mach.h>
#include <mach/message.h>

kern_return_t
clock_get_res(clock, cur_res)
	mach_port_t	clock;
	clock_res_t	*cur_res;
{
	kern_return_t		result;
	clock_attr_t		attr;
	mach_msg_type_number_t	aCnt;

	attr = (clock_attr_t) cur_res; aCnt = 1;
	result = clock_get_attributes(clock, CLOCK_ALARM_CURRES, attr, &aCnt);
	return(result);
}

kern_return_t
clock_set_res(clock, new_res)
	mach_port_t	clock;
	clock_res_t	new_res;
{
	kern_return_t		result;
	clock_attr_t		attr;
	mach_msg_type_number_t	aCnt;

	attr = (clock_attr_t) &new_res; aCnt = 1;
	result = clock_set_attributes(clock, CLOCK_ALARM_CURRES, attr, aCnt);
	return(result);
}
