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
#include "printf_types.h"
#include <stdarg.h>
#include <stdio.h>

mach_port_t	printf_port = 0;

extern int gen_printf(const char *format, ...);

printf_thread_body(port)
mach_port_t port;
{
	extern boolean_t printf_server();

	if (debug)
		printf("printf_thread_body\n");
	MACH_CALL(mach_msg_server, (printf_server,
				    512,
				    port,
				    MACH_MSG_OPTION_NONE));
}

mach_thread_t	printf_thread;

_printf_init()
{
	if (debug)
		printf("_printf_init\n");

	if (is_master_task && !printf_port) {
		if (is_master_task && !standalone)
			setvbuf(stdout, 0, _IONBF, 0);
		MACH_CALL( mach_port_allocate, (mach_task_self(),
				 MACH_PORT_RIGHT_RECEIVE,
				 &printf_port));
		if (debug > 1)
			printf("printf_port %x\n", printf_port);
		new_thread(&printf_thread,
			   printf_thread_body,
			   printf_port);
		printf_enable(mach_task_self());
	}
}

printf_enable(task)
mach_port_t task;
{
	if (is_master_task) {
		MACH_CALL( mach_port_insert_right, (task,
						    printf_port,
						    printf_port,
						    MACH_MSG_TYPE_MAKE_SEND));
	} else {
		MACH_CALL( mach_port_insert_right, (task,
						    printf_port,
						    printf_port,
						    MACH_MSG_TYPE_COPY_SEND));
	}
}

do_remote_printf(port, string, count) 
mach_port_t port;
char *string;
{
	printf("%s", string);
	return(KERN_SUCCESS);
}

#undef printf

/*
 * - We cant use printf from remote tasks.
 * - gen_printf() sends a message to the printf_thread in the case
 *   the current task is not the master one.
 * - The standalone library does not include float format, so we
 *   use our own version of doprnt via do_sprintf().
 * - The libmach_sa lib assumes printf strings are less than 128 bytes,
 *   so we must use a for() loop.
 */

int
gen_printf(const char *format, ...)
{
	va_list	args;

	char string[4096], *s, c;
	int length, l;
	boolean_t remote;

	va_start(args, format);
	do_sprintf(string, format, args);
	va_end(args);

	remote = ((!is_master_task) ||
			    (printf_thread && 
			     thread_self(&remote) != printf_thread));

	for (length = strlen(string)+1, s = string;
	     length;
	     length -= l, s += l) {
		l = (length < PRINTF_LENGTH-1) ? length : PRINTF_LENGTH-1;
		c = *(s+l);
		*(s+l) = 0;
		if (!remote) {
			printf("%s", s);
		} else { 
			remote_printf(printf_port, s, l+1);
		}
		*(s+l) = c;
	}
}

