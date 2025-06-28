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

#include <mach.h>		/* for mach_port_t */
#include <stdarg.h>		/* for va_list */
#include <stddef.h>		/* for size_t */

extern void bcopy(const char *, char *, unsigned int);
extern void _exit(int);
extern void exit(int);
extern void printf_init(mach_port_t);
extern void printf(const char *, ...);
extern void vprintf(const char *, va_list);
extern int getchar(void);
extern void vsprintf(char *, const char *, va_list);
extern void sprintf(char *, const char *, ...);
extern char *strcat(char *, const char *);
extern int strcmp(const char *, const char *);
extern size_t strlen(const char *);
extern int strncmp(const char *, const char *, size_t);
extern char *strncpy(char *, const char *, size_t);
