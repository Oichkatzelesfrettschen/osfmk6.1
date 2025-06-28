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
/* CMU_HIST */
/*
 * Revision 2.1.2.1  92/04/30  11:53:47  bernadat
 * 	Copied from main line
 * 	[92/04/21            bernadat]
 * 
 * Revision 2.2  92/04/04  11:35:57  rpd
 * 	Fixed for IBM L40's A20 initialization.
 * 	[92/03/30            rvb]
 * 
 * 	Created.
 * 	[92/03/30            mg32]
 * 
 */
/* CMU_ENDHIST */
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

#include "bios.h"

#define K_RDWR 		0x60		/* keyboard data & cmds (read/write) */
#define K_STATUS 	0x64		/* keyboard status */
#define K_CMD	 	0x64		/* keybd ctlr command (write-only) */

#define K_OBUF_FUL 	0x01		/* output buffer full */
#define K_IBUF_FUL 	0x02		/* input buffer full */

#define KC_CMD_WIN	0xd0		/* read  output port */
#define KC_CMD_WOUT	0xd1		/* write output port */
#define KB_A20		0x9f		/* enable A20,
					   enable output buffer full interrupt
					   enable data line
					   disable clock line */

#define CNTRL(x)	(x & 037)
#define INTR_CHAR 	CNTRL('c')
#define BELL_CHAR 	CNTRL('g')

/*
 * Gate A20 for high memory
 */
unsigned char	x_20 = KB_A20;
gateA20()
{
#ifdef	IBM_L40
	outb(0x92, 0x2);
#else	IBM_L40
	while (inb(K_STATUS) & K_IBUF_FUL);
	while (inb(K_STATUS) & K_OBUF_FUL)
		(void)inb(K_RDWR);

	outb(K_CMD, KC_CMD_WOUT);
	while (inb(K_STATUS) & K_IBUF_FUL);
	outb(K_RDWR, x_20);
	while (inb(K_STATUS) & K_IBUF_FUL);
#endif	IBM_L40
}

/* printf - only handles %d as decimal, %c as char, %s as string */

printf(format,data)
     char *format;
     int data;
{
	int *dataptr = &data;
	char c;
	while (c = *format++)
		if (c != '%')
			putchar(c);
		else
			switch (c = *format++) {
			      case 'd': {
				      int num = *dataptr++;
				      char buf[10], *ptr = buf;
				      if (num<0) {
					      num = -num;
					      putchar('-');
				      }
				      do
					      *ptr++ = '0'+num%10;
				      while (num /= 10);
				      do
					      putchar(*--ptr);
				      while (ptr != buf);
				      break;
			      }
			      case 'x': {
				      int num = *dataptr++, dig;
				      char buf[8], *ptr = buf;
				      do
					      *ptr++ = (dig=(num&0xf)) > 9?
							'a' + dig - 10 :
							'0' + dig;
				      while (num >>= 4);
				      do
					      putchar(*--ptr);
				      while (ptr != buf);
				      break;
			      }
			      case 'c': putchar((*dataptr++)&0xff); break;
			      case 's': {
				      char *ptr = (char *)*dataptr++;
				      while (c = *ptr++)
					      putchar(c);
				      break;
			      }
			}
}

extern int remote;

int cons_is_com = 0;
int com_initialized = 0;

do_putc(c)
{
	putc(c);
	if (remote) {
		if (!com_initialized ) {
	  		com_initialized = 1;
			com_init(COM_CS7|COM_PAREVEN|COM_1STOP|COM_9600); 
		}
		com_putc(c);
	}
}

do_getc()
{
	while(1)
		if (ischar()) {
			cons_is_com = 0;
			return(getc());
		} else if (remote && is_com_char()) {
			cons_is_com = 1;
			return(com_getc());
		}
}

is_char()
{
	return(ischar() || (remote && is_com_char()));
}

is_com_char() 
{
	return(com_getstat() & 0x100);	/* data ready */
}

putchar(c)
{
	if (c == '\n')
		do_putc('\r');
	do_putc(c);
}

getchar()
{
	int c;

	if ((c=do_getc()) == '\r')
		c = '\n';
	if (c == '\b') {
		putchar('\b');
		putchar(' ');
	}
	putchar(c);
	return(c);
}

flush_char()
{
	while(is_char())
		getchar();
	if (remote) {
		com_putc(BELL_CHAR);
		com_putc(BELL_CHAR);
	}
	  	
}

/*
 * detect INTR_CHAR. If console is not remote, do not check remote input
 */

is_intr_char()
{
	if ((ischar() && getc() == INTR_CHAR) ||
	    (cons_is_com && is_com_char() && com_getc() == INTR_CHAR)) {
		printf("*** INTERRUPT ***");
		return(1);
	}
	return(0);
}

gets(buf)
char *buf;
{
	int	i;
	char *ptr=buf;

	if (remote)
		cons_is_com++;	/* Default console is COM1 */
	for (i = 240000; i>0; i--)
		if (is_char())
			for (;;)
				switch(*ptr = getchar() & 0xff) {
				      case '\n':
				      case '\r':
					*ptr = '\0';
					return 1;
				      case '\b':
					if (ptr > buf) ptr--;
					continue;
				      default:
					ptr++;
				}
	return 0;
}

strcmp(s1, s2)
char *s1, *s2;
{
	while (*s1 == *s2) {
		if (!*s1++)
			return 0;
		s2++;
	}
	return 1;
}

bcopy(from, to, len)
char *from, *to;
int len;
{
	while (len-- > 0)
		*to++ = *from++;
}
