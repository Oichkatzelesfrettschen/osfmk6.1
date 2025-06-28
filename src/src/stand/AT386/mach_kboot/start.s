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

#include	"machine/asm.h"

BOOTSTACK	=	0xF000	/ boot stack
BOOTOFFSET	=	0x1000	/ boot offset

	.text

/*
 * start() -- indirect bootstrap start
 *
 * Environment:
 *	protected mode, flat 32-bit address space,
 *	all segments in the GDTR table, no interrupts.
 *	(Code/data/stack segments have base == 0, limit == 4G)
 */

start:
	/*
	 * Save prgram arguments.
	 */
	movl	0xC(%esp), %ebx		/ boothowto
	andl	$0xFFFF0000, %ebx	/ boot address

	leal	bootdev, %eax
	addl	%ebx, %eax
	movl	0x0(%esp), %ecx		/ get bootdev
	movl	%ecx, 0(%eax)		/ save bootdev

	leal	extmem, %eax
	addl	%ebx, %eax
	movl	0x4(%esp), %ecx		/ get extmem
	movl	%ecx, 0(%eax)		/ save extmem

	leal	cnvmem, %eax
	addl	%ebx, %eax
	movl	0x8(%esp), %ecx		/ get cnvmem
	movl	%ecx, 0(%eax)		/ save cnvmem

	leal	howto, %eax
	addl	%ebx, %eax
	movl	0xC(%esp), %ecx		/ get howto
	movl	%ecx, 0(%eax)		/ save howto

	/*
	 * Copy code to original bootstrap place. 
	 */
	movl	%ebx, %esi		/ source
	movl	$BOOTOFFSET, %edi	/ destination
	movl	$end, %ecx		/ get size of program
	rep
	movsb

	/*
	 * Rebuild a GDTR at the end of the program
	 */
	lea	BOOTOFFSET+EXT(Old_Gdtr), %eax
	sgdt	(%eax)			/ copy current GDTR

	movw	(%eax), %cx		/ get size
	addw	$1, %cx
	andw	$0xFFF8, %cx
	movl	2(%eax), %esi		/ source
	lea	BOOTOFFSET+end, %edx
	movl	%edx, %edi		/ destination
	rep
	movsb				/ copy current GDT

	/*
	 * Complement current GDT by an additional descriptor.
	 */
	movl	$8, %ecx
	lea	BOOTOFFSET+EXT(Add_Gdt), %esi
	rep
	movsb

	/*
	 * Switch to the new GDTR.
	 */
	addw	$8, (%eax)		/ new limit
	andw	$0xFFF8, (%eax)
	movl	%edx, 2(%eax)		/ new base
	lgdt	(%eax)

	/*
	 * Set up new_cs to the new restart segment.
	 */
	movw	(%eax), %dx
	subw	$8, %dx
	movw	%dx, BOOTOFFSET+new_cs

	/*
	 * Restart program at the correct place.
	 */
	ljmp	BOOTOFFSET+addr

restart:
	/*
	 * Enter Real Address mode under the additional segment.
	 */
	mov	%cr0, %eax
	and 	$0xFFFFFFFE, %eax
	mov	%eax, %cr0

	/*
	 * Restart program at the correct place.
	 */
	ljmp	$0x100, $restart16

restart16:
	/*
	 * Reload correct ds segment
	 */
	movw	%cs, %ax
	movw	%ax, %ds

	/*
	 * Reload GDTR.
	 */
	lea	EXT(Gdtr), %eax
	movl	$BOOTOFFSET+EXT(Gdt), 2(%eax)
	lgdt	(%eax)

	/ set the PE bit of CR0
	mov	%cr0, %eax
	or	$0x1, %eax
	mov	%eax, %cr0 

	/*
	 * Enter Protected mode with new GDTR.
	 */
	ljmp	$0x08, $restart32

restart32:
	movl	$0x10, %eax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	/*
	 * Load end_of_data pointer (for dynamic memory allocation).
	 */
	movl	$end, EXT(end_of_data);

	/*
	 * Reload stack address.
	 */
	movl	$BOOTSTACK, %esp

	/*
	 * Start executing bootstrap.
	 */
	pushl	extmem
	pushl	cnvmem
	pushl	bootdev
	pushl	howto
	call	EXT(boot)
	ret

		.align 4
bootdev	:	.long 0
extmem	:	.long 0
cnvmem	:	.long 0
howto	:	.long 0
addr	:	.long restart
new_cs	:	.word 0
