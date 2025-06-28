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
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */
/*
 * 	File: 	exp_malloc.c
 *	Author: Francois Barbou des Places, Open Software Foundation
 *	Date:	February, 1993
 *
 * 	Standard interface to the internal memory allocation package
 * 	in malloc.c.
 */

#include <cthreads.h>
#include "cthread_internals.h"

char *
malloc(unsigned int size)
{
    extern char *cthreads_malloc(unsigned int);
	return cthreads_malloc(size);
}

free(base)
	char *base;
{
	return cthreads_free(base);
}

char *
realloc(old_base, new_size)
	char *old_base;
	unsigned int new_size;
{
	return (char *) cthreads_realloc(old_base, new_size);
}

#ifdef	DEBUG
void
print_malloc_free_list()
{
	cthreads_print_malloc_free_list();
}
#endif	/* DEBUG */

void malloc_fork_prepare()
/*
 * Prepare the malloc module for a fork by insuring that no thread is in a
 * malloc critical section.
 */
{
	cthreads_malloc_fork_prepare();
}

void malloc_fork_parent()
/*
 * Called in the parent process after a fork() to resume normal operation.
 */
{
	cthreads_malloc_fork_parent();
}

void malloc_fork_child()
/*
 * Called in the child process after a fork() to resume normal operation.
 */
{
	cthreads_malloc_fork_child();
}
