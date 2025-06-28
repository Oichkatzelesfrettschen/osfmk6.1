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
 * 	File: rpc_defs.h
 *	Author: Peter Watkins
 *
 *	Common definitions for sample RPC server.
 */

#ifndef	RPC_DEFS_INCL
#define	RPC_DEFS_INCL

#include	<mach/error.h>


/*
 *	Name of RPC server for the nameservice.
 */

#define RPC_NAME		"RpcS"


/*
 *	Control optional functionality.
 */

#define	DEBUG 				1

/*
 *	Define three sizes for fixed arrays: fits on kernel stack (small),
 *	must kalloc space (medium), can use virtual copy or cow (large). This
 *	is for testing different kernel code paths during rpc processing.
 */

#define FIXED_ARRAY_SIZE_SMALL		8		/* kernel stack */
#define FIXED_ARRAY_SIZE_MEDIUM		128		/* kalloc */
#define FIXED_ARRAY_SIZE_LARGE		1024		/* vcopy/cow */

#define CHAR_ARRAY_SIZE			83		/* char array */
#define STRG_ARRAY_SIZE			80		/* string array */

/*
 *	C types corresponding to MiG types.
 */

typedef int			fixed_array_small_t[FIXED_ARRAY_SIZE_SMALL];
typedef int			fixed_array_medium_t[FIXED_ARRAY_SIZE_MEDIUM];
typedef int			fixed_array_large_t[FIXED_ARRAY_SIZE_LARGE];
typedef int			*fixed_array_pointer_t;

typedef int			*bva_small_t;		/* bnd var array */
typedef int			*bva_medium_t;	
typedef int			*bva_large_t;
typedef int			*bva_pointer_t;

typedef int			*uva_t;			/* uva */
typedef int			*uva_pointer_t;		/* uva int  ptr */
typedef char			*uva_char_pointer_t;	/* uva char ptr */

typedef char			string_t[STRG_ARRAY_SIZE];

typedef	mach_port_t		*port_array_t;	/* unbnd var array ptr ports */
typedef mach_port_t		*uvpap_t;	/* unbnd var array ptr ports */
typedef mach_port_t		*uvpa_t;	/* unbnd var array ports */
typedef mach_port_t		*bvpa_t;	/* bnded var array ports */


#endif	/* RPC_DEFS_INCL */


