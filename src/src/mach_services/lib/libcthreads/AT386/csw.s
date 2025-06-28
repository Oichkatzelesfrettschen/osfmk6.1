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
 * Revision 2.7.2.1  92/06/22  11:49:56  rwd
 * 	Modified for single lock version of cproc.c
 * 	[92/04/22            rwd]
 * 
 * Revision 2.7  91/07/31  18:36:32  dbg
 * 	Fix for ANSI C preprocessor.
 * 	[91/07/30  17:35:16  dbg]
 * 
 * Revision 2.6  91/05/14  17:56:56  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/08  13:35:49  dbg
 * 	Unlock lock with a locked instruction (xchg).
 * 	[91/03/20            dbg]
 * 
 * Revision 2.4  91/02/14  14:20:02  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/13  12:15:27  mrt]
 * 
 * Revision 2.3  91/01/08  16:46:20  rpd
 * 	Don't use Times - horta doesn't like it for some reason.
 * 	[91/01/06            rpd]
 * 
 * Revision 2.2  90/05/03  15:54:37  dbg
 * 	Created.
 * 	[90/02/05            dbg]
 * 
 */
#ifdef GPROF
#undef GPROF
#define GFUDGE 8
#define FRAMEP 12
#define RETURN 8
#else /*GPROF*/
#define GFUDGE 0
#endif /*GPROF*/

#include <i386/asm.h>
#define B_ARG4	24(%ebp)
#define B_ARG5	28(%ebp)
#define ROUTINE 4
#define ARGUMENT 0

/* This value must be updated if adding extra words to the context.  */
#define CONTEXT_SIZE 16
	.globl EXT(cthread_context_size)
	.align 2
LEXT(cthread_context_size)
	.long CONTEXT_SIZE

filter_switch:
	.long 0f
	.long 1f
	.long 2f
	.long 3f
	.long 4f
	.long 5f
	.long 6f
	.long 7f

/*
 * cthread_filter(con, type, a1, a2, a3, a4)
 */

ENTRY(cthread_filter)
	movl	8(%esp),%ecx	/* ARG1 */

	jmp	*filter_switch(,%ecx,4)

/* USER_BUILD */
#if GFUDGE > 0
0:	movl	(%esp),%eax	
	pushl	%ebx		
#else
0:	pushl	%ebx		
#endif
	pushl	%esi
	pushl	%edi
	pushl	%ebp
#if GFUDGE > 0
	pushl	%ebp
	pushl	%eax
#endif
	movl	20+GFUDGE(%esp),%ecx	/* ARG0 */
	subl	$8,%esp
	movl	%esp,(%ecx)
	movl	44+GFUDGE(%esp),%ecx	/* ARG4 */
	pushl	%ecx
	movl	44+GFUDGE(%esp),%ecx	/* ARG3 */
	pushl	%ecx
#if GFUDGE > 0
	pushl	%eax		/* Pretend return address */
	jmp	*48+GFUDGE(%esp)
#else
	call	*44(%esp)
#endif


/* INTERNAL_BUILD */
1:	movl	4(%esp),%ebx	/* COMPRESS */
	movl	(%ebx),%ebx
	movl	16(%esp),%eax	/* ARG3 */
	movl	%eax,ARGUMENT(%ebx)
	movl	12(%esp),%eax	/* ARG2 */
	movl	%eax,ROUTINE(%ebx)
#if GFUDGE > 0
	movl	%ebp,FRAMEP(%ebx)
#endif
	movl	20(%esp),%edx	/* ARG4 */
	movl	24(%esp),%ecx	/* ARG5 */
	movl	(%ecx),%esp
	xorl	%eax,%eax
	xchg	%eax,(%edx)
#if GFUDGE > 0
	movl	FRAMEP(%esp),%ebp
#endif
	movl	ARGUMENT(%esp),%esi
	pushl	%esi
#if GFUDGE > 0
	movl	RETURN+4(%esp),%eax
	pushl	%eax		/* Pretend return address */
	jmp	*ROUTINE+8(%esp)
#else
	call	*ROUTINE+4(%esp)
#endif


/* INTERNAL_INVOKE - not used */
2:	ret

/* USER_INVOKE */
3:	movl	4(%esp),%ecx	/* OUT */
	movl	8(%esp),%eax
	movl	(%ecx),%esp
	addl	$8,%esp
#if GFUDGE > 0
	popl	%edi		/* Not relevant */
	popl	%edi
#endif
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%ebx
	ret

/* PREPARE */
4:	pushl	%ebp
	movl	%esp, %ebp

	movl	B_ARG0,%ecx	/* PREPARE */
	movl	B_ARG2,%edx
	movl	B_ARG3,%eax
	movl	(%ecx),%ecx
#if GFUDGE > 0
	movl	$0,RETURN(%ecx)
	movl	$0,FRAMEP(%ecx)
#endif
	movl	%edx,ROUTINE(%ecx)
	movl	%eax,ARGUMENT(%ecx)
	leave
	ret

/* GET_STATE */
5:	
/*
 * This whole get_state/set_state thing on blocked threads is REAL
 * fuzzy.  Do we care beyond eip, ebp, and esp?  For now deal with
 * only these.  It might make sense to save efl across internal
 * context switching to allow single stepping to be specified
 * for blocked threads.  I REALLY don't know
 */

	movl	B_ARG0,%ecx	/* GET_STATE */
	movl	(%ecx),%ecx
	movl	B_ARG2,%edx
	movl	B_ARG5,%eax
	pushl	%ebx
	movl	ROUTINE(%ecx),%ebx /* eip will be the continuation */
	movl	%ebx,48(%edx)
	movl	%eax,60(%edx)	/* uesp is after the block */
	addl	$20,%eax
	movl	%eax,24(%edx)	/* point ebp to saved ebp */
	popl	%ebx
	leave
	ret

/* SET_STATE */
6:	movl	B_ARG0,%ecx	/* SET_STATE */
	movl	(%ecx),%ecx
	movl	B_ARG2,%edx
	movl	48(%edx),%eax
	movl	%eax, ROUTINE(%ecx) /* eip will be the continuation */
/*
 * I don't see any way to do other registers here that makes any sense
 */
	leave
	ret

/* INVOKE_SELF */
7:	movl	16(%esp),%esi	/* ARG3 (argument) */
	movl	12(%esp),%ebx	/* ARG2 (routine) */
	movl	4(%esp),%eax	/* ARG0 (&context) */
	movl	(%eax),%esp
	pushl	%esi
#if GFUDGE > 0
	pushl	$0		/* Pretend return address */
	jmp	*%ebx
#else
	call	*%ebx
#endif
