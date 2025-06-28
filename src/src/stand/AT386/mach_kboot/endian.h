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
 * File : endian.h
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains all declarations depending on machine endianness.
 */

#ifndef	__ENDIAN_H__
#define	__ENDIAN_H__

#include "boot.h"

#define	BIG_ENDIAN	1234
#define	LITTLE_ENDIAN	4321

#if	defined(i386)
#define	BYTE_ORDER	LITTLE_ENDIAN
#endif

#define	ntohs(a)	htons(a)
#define	ntohl(a)	htonl(a)

#if	BYTE_ORDER == BIG_ENDIAN
#define	htons(a)	a
#define	htonl(a)	a
#endif

#if	BYTE_ORDER == LITTLE_ENDIAN
extern u16bits htons(u16bits);
extern u32bits htonl(u32bits);
#endif

#endif	/* __ENDIAN_H__ */
