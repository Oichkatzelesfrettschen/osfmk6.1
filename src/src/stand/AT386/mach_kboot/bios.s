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

	.file	"bios.s"

#include "machine/asm.h"
	.text


/ biosrtc(&ticks)
/	gets Tick Counter and returns if Roll-over has taken place.
/ Tick counter increments 18.2 times per second.
/ BIOS call "INT 0x1A Function 0x0" to read tick counter
/	Call with	%ah = 0x0
/
/	Return		%cx = high portion of count
/			%dx = low portion of count
/			%al = Roll-over flag

ENTRY(biosrtc)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx
	push	%ecx
	push	%edx

	call	EXT(prot_to_real)	/ enter real mode
	movb	$0, %ah			/ subfunction

	sti
	int	$0x1a			/ call BIOS
	cli

	xorl	%ebx, %ebx
	movb	%al, %bl		/ real_to_prot uses %eax

	data16
	call	EXT(real_to_prot)

	movw	%cx, %ax
	shll	$0x10, %eax
	movw	%dx, %ax 		/ eax contains ticks value
	movl	0x8(%ebp), %ecx		/ where to store tick number
	movl	%eax, 0(%ecx)		/ store ticks number
	movl	%ebx, %eax		/ get back return value

	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret


/ biosstc(ticks)
/	sets Tick Counter.
/ BIOS call "INT 0x1A Function 0x1" to set tick counter
/	Call with	%ah = 0x1
/			%cx = high portion of count
/			%dx = low portion of count
/
/	Return		None

ENTRY(biosstc)
	push	%ebp
	mov	%esp, %ebp
	push	%ecx
	push	%edx

	movl	0x8(%ebp), %ecx
	shrl	$0x10, %ecx
	movl	0x8(%ebp), %edx
	call	EXT(prot_to_real)	/ enter real mode
	movb	$0x1, %ah		/ subfunction

	sti
	int	$0x1a			/ call BIOS
	cli

	data16
	call	EXT(real_to_prot)

	pop	%edx
	pop	%ecx
	pop	%ebp
	ret


/ putc(ch)
/ BIOS call "INT 10H Function 0Eh" to write character to console
/	Call with	%ah = 0x0e
/			%al = character
/			%bh = page
/			%bl = foreground color ( graphics modes)


ENTRY(putc)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx
	push	%ecx

	movb	0x8(%ebp), %cl

	call	EXT(prot_to_real)

	data16
	mov	$0x1, %ebx	/ %bh=0, %bl=1 (blue)
	movb	$0xe, %ah
	movb	%cl, %al
	sti
	int	$0x10		/ display a byte
	cli

	data16
	call	EXT(real_to_prot)

	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret


/ getc()
/ BIOS call "INT 16H Function 00H" to read character from keyboard
/	Call with	%ah = 0x0
/	Return:		%ah = keyboard scan code
/			%al = ASCII character

ENTRY(getc)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx		/ save %ebx

	call	EXT(prot_to_real)

	movb	$0x0, %ah
	sti
	int	$0x16
	cli

	movb	%al, %bl	/ real_to_prot uses %eax

	data16
	call	EXT(real_to_prot)

	xor	%eax, %eax
	movb	%bl, %al

	pop	%ebx
	pop	%ebp
	ret

/ ischar()
/       if there is a character pending, return it; otherwise return 0
/ BIOS call "INT 16H Function 01H" to check whether a character is pending
/	Call with	%ah = 0x1
/	Return:
/		If key waiting to be input:
/			%ah = keyboard scan code
/			%al = ASCII character
/			Zero flag = clear
/		else
/			Zero flag = set

ENTRY(ischar)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx

	call	EXT(prot_to_real)		/ enter real mode

	xor	%ebx, %ebx
	movb	$0x1, %ah
	sti
	int	$0x16
	cli
	data16
	jz	nochar
	movb	%al, %bl

nochar:
	data16
	call	EXT(real_to_prot)

	xor	%eax, %eax
	movb	%bl, %al

	pop	%ebx
	pop	%ebp
	ret

/ com_putc(ch)
/ BIOS call "INT 14H Function 01h" to write character to COM0
/	Call with	%ah = 0x01
/			%al = character
/			%dx = 0 for COM0


ENTRY(com_putc)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx
	push	%ecx

	movb	0x8(%ebp), %cl

	call	EXT(prot_to_real)

	data16
	mov	$0x0, %edx	/ COM0
	movb	$0x1, %ah
	movb	%cl, %al
	sti
	int	$0x14		/ display a byte
	cli

	data16
	call	EXT(real_to_prot)

	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret

/ com_getc()
/ BIOS call "INT 14H Function 02h" to write character to COM0
/	Call with	%ah = 0x02
/			%dx = 0 for COM0
/	Returns		%al = character


ENTRY(com_getc)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx
	push	%ecx

	call	EXT(prot_to_real)

	data16
	mov	$0x0, %edx	/ COM0
	movb	$0x2, %ah

	sti
	int	$0x14		/ read a byte
	cli

	mov	%eax, %ebx	/ real_to_prot uses %eax

	data16
	call	EXT(real_to_prot)

	xor	%eax, %eax
	mov	%ebx, %eax

	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret

/ com_init(param_byte)
/ BIOS call "INT 14H Function 00h" to read status  COM0
/	Call with	%ah = 0x00
/			%al = param_byte
/			%dx = 0 for COM0
/ param_byte: see bios.h



ENTRY(com_init)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx
	push	%ecx

	movb	0x8(%ebp), %cl

	call	EXT(prot_to_real)

	data16
	mov	$0x0, %edx	/ COM0
	movb	$0x0, %ah
	movb	%cl, %al
	sti
	int	$0x14		/ init
	cli
	mov	%ax, %cx

	data16
	call	EXT(real_to_prot)

	mov	%cx, %ax
	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret

/ com_getstat()
/ BIOS call "INT 14H Function 03h" to read status bytes from COM0
/	Call with	%ah = 0x03
/			%dx = 0 for COM0
/	Returns		%al = Modem Status
/			%ah = Line Status


ENTRY(com_getstat)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx
	push	%ecx


	call	EXT(prot_to_real)

	data16
	mov	$0x0, %edx	/ COM0
	movb	$0x3, %ah
	sti
	int	$0x14		/ Get status
	cli

	mov	%eax, %ebx	/ real_to_prot uses %eax

	data16
	call	EXT(real_to_prot)

	xor	%eax, %eax
	mov	%ebx, %eax

	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret
