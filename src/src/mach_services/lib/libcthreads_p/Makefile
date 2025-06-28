#
# Copyright (c) 1995, 1994, 1993, 1992, 1991, 1990  
# Open Software Foundation, Inc. 
#  
# Permission to use, copy, modify, and distribute this software and 
# its documentation for any purpose and without fee is hereby granted, 
# provided that the above copyright notice appears in all copies and 
# that both the copyright notice and this permission notice appear in 
# supporting documentation, and that the name of ("OSF") or Open Software 
# Foundation not be used in advertising or publicity pertaining to 
# distribution of the software without specific, written prior permission. 
#  
# OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL OSF BE LIABLE FOR ANY 
# SPECIAL, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
# ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING 
# OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE 
#
#
# OSF Research Institute MK6.1 (unencumbered) 1/31/1995

VPATH			= ../libcthreads:${${TARGET_MACHINE}_VPATH}

INCFLAGS		= -I../libcthreads ${${TARGET_MACHINE}_INCFLAGS}

LIBRARIES		= libcthreads_p.a

EXPLIB_TARGETS		= export_libcthreads_p.a

CFLAGS			= -DMACH_IPC_COMPAT=0 ${${TARGET_MACHINE}_CFLAGS} \
			  -DGPROF -pg
OFILES			= cthreads.o malloc.o mig_support.o \
			  stack.o sync.o exp_malloc.o structs.o\
			  ${${TARGET_MACHINE}_OFILES}

ILIST			= libcthreads_p.a
IDIR			= ${MACH3_LIBRARY_IDIR}
IMODE			= 644

.include "${TARGET_MACHINE}/machdep.mk"
.include <${RULES_MK}>
