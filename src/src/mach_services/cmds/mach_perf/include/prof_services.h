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

#ifndef _PROF_SERVICES_H
#define _PROF_SERVICES_H

struct sample_buf {
	unsigned free_count;
	unsigned *next_sample;
	struct sample_buf *next;
	unsigned samples[1];
};

typedef struct sample_buf *sample_buf_t;

extern sample_buf_t 	sample_bufs;
extern int		sample_count;
extern int		samples_per_buf;
#define PROF_ALLOC_UNIT (4*vm_page_size)


struct symbol {
	vm_offset_t	offset;
	char		*name;
	unsigned int	count;
};

typedef struct symbol *symbol_t;

extern  time_value_t 	prof_time_start;
extern  time_value_t 	prof_time_stop;

extern float		prof_trunc;

extern	char		prof_command[];

#if	STANDALONE
extern	struct symbol 	kernel_symbols[];
extern	unsigned int	kernel_symbols_count;
char			*kernel_symbols_version;
#endif	/* STANDALONE */

#endif /* _PROF_SERVICES_H */
