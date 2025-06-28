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
 * This is the Mach standalone startup.  It has been written from
 * scratch to only depend upon interfaces present in a standalone
 * Mach environment.  It contains no OS personality code.
 *
 * General flow structure:
 *
 *	call mach library initialization (if present)
 *	call thread library initialization (if present)
 *	call main - zero argc, argv pointer to null args
 *	call thread library exit (if present) - called with main return value
 *	call exit - called with main return value
 */

void (*_mach_init_routine)(void);
void (*_threadlib_init_routine)(void);
void (*_threadlib_exit_routine)(int);

extern int main(int, char **);
extern void exit(int);

void __start(void);

void
__start(void)
{
	char *arg = (char *)0;
	int retval;

	if (*_mach_init_routine)
		(*_mach_init_routine)();

	if (*_threadlib_init_routine)
		(*_threadlib_init_routine)();

	retval = main(0, &arg);

	if (*_threadlib_exit_routine)
		(*_threadlib_exit_routine)(retval);

	exit(retval);
}
