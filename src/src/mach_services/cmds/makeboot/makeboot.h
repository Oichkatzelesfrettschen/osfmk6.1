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


#include <mach/boot_info.h>

typedef struct objfmt *objfmt_t;
typedef struct objfmt_switch *objfmt_switch_t;

struct objfmt {
    struct loader_info info;
    objfmt_switch_t fmt;
};

struct objfmt_switch {
    const char *name;
    int (*recog)(int, objfmt_t, char **);
    int (*load)(int, int, objfmt_t, char *);
    void (*symload)(int, int, objfmt_t);
};

extern objfmt_switch_t formats[];
extern off_t (*exec_header_size)(void);
extern void (*write_exec_header)(int, struct loader_info *, off_t);
extern vm_offset_t round_page(vm_offset_t);
extern vm_offset_t trunc_page(vm_offset_t);
extern void check_read(int, void *, size_t);
extern void file_copy(int, int, size_t);
extern void file_zero(int, size_t);

#if DEBUG
extern void _file_copy(int, int, size_t);
#define file_copy _file_copy

extern void _file_zero(int, size_t);
#define file_zero _file_zero

extern off_t _lseek __((int , off_t , int));
#define lseek _lseek

extern ssize_t _write __((int , const void *, size_t));
#define write _write

#endif /* DEBUG */

