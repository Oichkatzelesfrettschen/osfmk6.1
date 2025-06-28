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
 * File : unzip.c
 *
 * Author : Eric PAIRE (O.S.F. Research Institute)
 *
 * This file contains the declaration for uncompressing [g]zip files.
 */

/*
 * GZIP compression methods.
 */
#define	GZIP_DEFLATED		8

/*
 * GZIP flags.
 */
#define	GZIP_ASCII		0x01 /* File probably ascii text */
#define	GZIP_CONTINUATION	0x02 /* Continuation of multi-part gzip file */
#define	GZIP_EXTRA		0x04 /* Extra field present */
#define	GZIP_FILENAME		0x08 /* Original file name present */
#define	GZIP_COMMENT		0x10 /* file comment present */
#define	GZIP_ENCRYPTED		0x20 /* File is encrypted */
#define	GZIP_RESERVED		0xC0 /* Reserved bits */

extern int unzip(void **, void *, unsigned , int (*)(unsigned, void *,
						     void **, unsigned));
extern int unzip_getc(unsigned short *);
extern int unzip_flush(unsigned);
extern void *unzip_malloc(unsigned);
extern void unzip_free(void *);


/*
 * Miscellaneous needed by inflate.c
 */
typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;

#define	MACH_KBOOT 1	/* Enable MACH_KBOOT compilation variable */
#define	NULL	   0	/* Standard NULL definition */
#define	NOMEMCPY	/* Don't use memcpy */
#define	OF(x)	   x	/* Allow function prototypes */
#define	WSIZE	0x8000  /* window size--must be a power of two, and at least
                           32K for zip's deflate method */
extern uch *slide;
extern unsigned mask_bits[];

#define	ReadByte(p)	unzip_getc(p)
#define	malloc(u)	unzip_malloc(u)
#define	free(p)		unzip_free(p)
#define	flush(w)	unzip_flush(w)
