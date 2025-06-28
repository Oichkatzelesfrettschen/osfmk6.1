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
 * Mach Operating System
 * Copyright (c) 1992, 1991 Carnegie Mellon University
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
 */

/*
  Copyright 1988, 1989, 1990, 1991, 1992 
   by Intel Corporation, Santa Clara, California.

                All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

	.file "asm.s"

#include "machine/asm.h"

BOOTSEG		=	0x100

CR0_PE_ON	=	0x1
CR0_PE_OFF	=	0xfffffffe

	.text

/
/ real_to_prot()
/ 	transfer from real mode to protected mode.
/

ENTRY(real_to_prot)
	/ guarantee that interrupt is disabled when in prot mode
	cli

	/ load the gdtr
	addr16
	data16
	lgdt	EXT(Gdtr)

	/ set the PE bit of CR0
	mov	%cr0, %eax

	data16
	or	$CR0_PE_ON, %eax
	mov	%eax, %cr0 

	/ make intrasegment jump to flush the processor pipeline and
	/ reload CS register
	data16
	ljmp	$0x08, $xprot

xprot:
	/ we are in USE32 mode now
	/ set up the protective mode segment registers : DS, SS, ES, FS, GS
	movl	$0x10, %eax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	ret

/
/ prot_to_real()
/ 	transfer from protected mode to real mode
/ 

ENTRY(prot_to_real)

	/ change to USE16 mode
	ljmp	$0x18, $x16

x16:
	/ clear the PE bit of CR0
	mov	%cr0, %eax
	data16
	and 	$CR0_PE_OFF, %eax
	mov	%eax, %cr0


	/ make intersegment jmp to flush the processor pipeline
	/ and reload CS register
	data16
	ljmp $BOOTSEG, $xreal

xreal:
	/ we are in real mode now
	/ set up the real mode segment registers : DS, SS, ES, FS, GS
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	data16
	ret

/
/ outb(port, byte)
/

ENTRY(outb)
	push	%ebp
	mov	%esp, %ebp

	movl	8(%ebp), %edx
	movl	12(%ebp), %eax
	outb	%al, %dx

	pop	%ebp
	ret

/
/ inb(port)
/

ENTRY(inb)
	push	%ebp
	mov	%esp, %ebp

	movl	8(%ebp), %edx
	xorl	%eax, %eax
	inb	%dx, %al

	pop	%ebp
	ret

/
/ startprog(phyaddr)
/	start the program on protected mode where phyaddr is the entry point
/

ENTRY(startprog)
	push	%ebp
	mov	%esp, %ebp

	/ push some number of args onto the stack
	movl	0x0c(%ebp), %eax	/ &argv
	push	0x10(%eax)		/ argv[4] = esym
	push	0x4(%eax)		/ argv[1] = howto
	push	0x1c(%eax)		/ argv[7] = cnvmem
	push	0x20(%eax)		/ argv[8] = extmem
	push	0x8(%eax)		/ argv[2] = bootdev
	
	mov	0x8(%ebp), %ecx		/ entry offset 
	mov	$0x28, %ebx		/ segment
	push	%ebx
	push	%ecx

	/ set up %ds and %es
	mov	$0x20, %ebx
	movw	%bx, %ds
	movw	%bx, %es

	lret

/
/ pcpy_out(src, dst, cnt)
/	where src is a virtual address and dst is a physical address
/

ENTRY(pcpy_out)
	push	%ebp
	mov	%esp, %ebp
	push	%es
	push	%esi
	push	%edi
	push	%ecx

	cld

	/ set %es to point at the flat segment
	mov	$0x20, %eax
	movw	%ax, %es

	mov	0x8(%ebp), %esi		/ source
	mov	0xc(%ebp), %edi		/ destination
	mov	0x10(%ebp), %ecx	/ count

	rep
	movsb

	pop	%ecx
	pop	%edi
	pop	%esi
	pop	%es
	pop	%ebp

	ret

/
/ pcpy_in(src, dst, cnt)
/	where dst is a virtual address and src is a physical address
/

ENTRY(pcpy_in)
	push	%ebp
	mov	%esp, %ebp
	push	%fs
	push	%esi
	push	%edi
	push	%ecx

	cld

	/ set %fs to point at the flat segment
	movl	$0x20, %eax
	movw	%ax, %fs

	mov	0x8(%ebp), %esi		/ source
	mov	0xc(%ebp), %edi		/ destination
	mov	0x10(%ebp), %ecx	/ count

	rep
	fs
	movsb

	pop	%ecx
	pop	%edi
	pop	%esi
	pop	%fs
	pop	%ebp

	ret

/
/ pcpy16_out(src, dst, cnt)
/	where src is a virtual address and dst is a physical address
/

ENTRY(pcpy16_out)
	push	%ebp
	mov	%esp, %ebp
	push	%es
	push	%esi
	push	%edi
	push	%ebx
	push	%ecx

	cld

	/ set %es to point at the flat segment
	mov	$0x20, %eax
	movw	%ax, %es

	mov	0x8(%ebp), %esi		/ source
	mov	0xc(%ebp), %edi		/ destination
	mov	0x10(%ebp), %ebx	/ count
	movl	%ebx, %ecx
	sarl	$1, %ecx		/ count/2 for words

	rep
	movsw

	andl	$1, %ebx
	je	end
	movb	0x0(%esi), %al
	movb	%al, %es:0x0(%edi)
end:

	pop	%ecx
	pop	%ebx
	pop	%edi
	pop	%esi
	pop	%es
	pop	%ebp

	ret

/
/ pcpy16_in(src, dst, cnt)
/	where dst is a virtual address and src is a physical address
/

ENTRY(pcpy16_in)
	push	%ebp
	mov	%esp, %ebp
	push	%fs
	push	%esi
	push	%edi
	push	%ebx
	push	%ecx

	cld

	/ set %fs to point at the flat segment
	mov	$0x20, %eax
	movw	%ax, %fs

	mov	0x8(%ebp), %esi		/ source
	mov	0xc(%ebp), %edi		/ destination
	mov	0x10(%ebp), %ebx	/ count
	movl	%ebx, %ecx
	sarl	$1, %ecx		/ count/2 for words

	rep
	fs
	movsw

	andl	$1, %ebx
	je	end16
	movb	%fs:(%esi), %al
	movb	%al, (%edi)
end16:

	pop	%ecx
	pop	%ebx
	pop	%edi
	pop	%esi
	pop	%fs
	pop	%ebp

	ret

/
/ pcopy(src, dst, cnt)
/	where src and dst are both physical addresses.
/

ENTRY(pcopy)
	pushl	%edi
	pushl	%esi
	movl	8 + 4(%esp), %edi	/* to */
	movl	8 + 8(%esp), %esi	/* from */
	movl	8 + 12(%esp), %edx	/* number of bytes */
	cld
/ move longs
	movl	%edx, %ecx
	sarl	$2, %ecx
	rep
	movsl
/ move bytes
	movl	%edx, %ecx
	andl	$3, %ecx
	rep
	movsb
	popl	%esi
	popl	%edi
	ret
