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
# OSF Research Institute MK6.1 (unencumbered) 1/31/1995

VPATH			= ${TARGET_MACHINE}

PROGRAMS		= vm_perf

${PROGRAMS}: ../lib/libperf.a

ILIST			= vm_perf
IDIR			= /usr/mach3/bin/

USE_STATIC_LIBRARIES	= YES
DEPENDENCIES		= YES

CFLAGS			+= -pic-none
INCFLAGS                = -I. -I../include ${INCDIRS}
LIBDIRS			+= -L../lib
LDFLAGS			+= -e ___start -e _malloc -e ___start
LIBS			= -lperf -lmach -lm -lnetname

OFILES			= vm_perf.o ${${TARGET_MACHINE}_OFILES}

.if exists(${TARGET_MACHINE}/machdep.mk)
.include "${TARGET_MACHINE}/machdep.mk"
.endif

.include <${RULES_MK}>

.if exists(depend.mk)
.include "depend.mk"
.endif
