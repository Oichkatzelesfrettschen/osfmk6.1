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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */
/*
 *	File:	sbrk.c
 *	Author: Avadis Tevanian, Carnegie Mellon University
 *	Date:	June 1986
 *
 *	Unix compatibility for sbrk system call.
 */

#include <mach.h>		/* for vm_allocate, vm_offset_t */
#include <stdio.h>		/* for stderr */
#include <mach_init.h>		/* for vm_page_size */

#define caddr_t	Caddr_t

typedef	char * caddr_t;

#if defined(i860)
#define __asm__ asm
#endif /* defined(i860) */

#if defined(luna88k)
__asm__("		global _curbrk");
__asm__("		global _minbrk");
__asm__("		global _end");
__asm__("		data");
__asm__("_minbrk:	word _end");
__asm__("_curbrk:	word _end");
#endif

#if	(defined(vax) || defined(ibmrt) || defined(ns32000) || defined(sun) || defined(i386) || defined(i860))
#if (defined(multimax) || defined(i860))
DEF_FUNC()
{
#endif	/* defined(multimax) || defined(i860) */
__asm__(".data");
__asm__(".globl	curbrk");
__asm__(".globl	minbrk");
__asm__(".globl	_curbrk");
__asm__(".globl	_minbrk");
__asm__(".globl	_end");
#if	multimax
__asm__("_minbrk:");
__asm__("minbrk:	.double	_end");
__asm__("_curbrk:");
__asm__("curbrk:	.double	_end");
__asm__(".text");
#else	/* multimax */
__asm__("_minbrk:");
__asm__("minbrk:	.long	_end");
__asm__("_curbrk:");
__asm__("curbrk:	.long	_end");
__asm__(".text");
#endif	/* multimax */
#if	defined(multimax) || defined(i860)
}
#endif	/* defined(multimax) || defined(i860) */
#else	/* (defined(vax) || defined(ibmrt) || defined(ns32000) || defined(sun) || defined (i386) || defined(i860) */

/* Will not find get "assembler" forms of cubrk, minbrk. */

#ifdef	mips
extern char end;
#define curbrk _curbrk
#define minbrk _minbrk
caddr_t curbrk = &end;
caddr_t minbrk = &end;
#endif	/* mips */

#endif	/* (defined(vax) || defined(ibmrt) || defined(ns32000) || defined(sun) || defined(i386) || defined(i860) */

#ifdef lint
   /* lint doesn't see asm stuff */
caddr_t	curbrk;
caddr_t	minbrk;
#else /* lint */
extern caddr_t curbrk;
extern caddr_t minbrk;
#endif /* lint */

#define	roundup(a,b)	((((a) + (b) - 1) / (b)) * (b))

static int sbrk_needs_init = FALSE;

caddr_t sbrk(size)
	int	size;
{
	vm_offset_t	addr;
	kern_return_t	ret;
	caddr_t		ocurbrk;

	if (sbrk_needs_init) {
		sbrk_needs_init = FALSE;
		/*
		 *	Get "curbrk"
		 */

	}
	
	if (size <= 0)
		return(curbrk);
	addr = (vm_offset_t) roundup((int)curbrk,vm_page_size);
	ocurbrk = curbrk;
	if (((int)curbrk+size) > addr)
	{	ret = vm_allocate(mach_task_self(), &addr, 
			    (vm_size_t) size -((int)addr-(int)curbrk), FALSE);
		if (ret == KERN_NO_SPACE) {
			ret = vm_allocate(mach_task_self(), &addr, (vm_size_t) size, TRUE);
			ocurbrk = (caddr_t)addr;
		}
		if (ret != KERN_SUCCESS) 
			return((caddr_t) -1);
	}

	curbrk = (caddr_t)ocurbrk + size;
	return(ocurbrk);

}

int brk(x)
	caddr_t x;
{
	printf("brk: not implemented\n");
	return(-1);
}




