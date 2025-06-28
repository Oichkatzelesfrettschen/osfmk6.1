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
 * File : i386at.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains i386at specific procedures of the Network bootstrap.
 */

#include "boot.h"

#define	TICKS_PER_10SEC	182

static unsigned	i386at_timeout;
static unsigned	i386at_random;

void
timerinit()
{
	biosrtc(&i386at_random);
}

void
settimeout(unsigned msecs)
{
	biosstc(0);
	i386at_timeout = TICKS_PER_10SEC * msecs;
	if (debug)
		printf("Set new timeout: 0x%x\n", i386at_timeout);
}

unsigned
getelapsed()
{
	unsigned val;

	biosrtc(&val);
	return ((val * 10000) / TICKS_PER_10SEC);
}

int
isatimeout()
{
	unsigned val;

	biosrtc(&val);
	return (val * 10000 >= i386at_timeout);
}

unsigned
rand()
{
	i386at_random = i386at_random * 1103515245 + 12345;
	return (i386at_random);
}
