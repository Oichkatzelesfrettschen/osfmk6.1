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

EXPINC_SUBDIRS		= servers ${${TARGET_MACHINE}_SUBDIRS}
COMP_SUBDIRS		=
SUBDIRS			= servers ${${TARGET_MACHINE}_SUBDIRS}

DATAFILES		= cthreads.h cthread_filter.h \
			  rthreads.h rthread_filter.h \
			  errorlib.h mach.h mach_error.h  \
			  mach_init.h mig_errors.h msg_type.h \
			  port_obj.h ${${TARGET_MACHINE}_DATAFILES}
LINKS			= machine ${${TARGET_MACHINE}_LINKS}
OTHERS			= ${LINKS} ${${TARGET_MACHINE}_OTHERS}

INCLUDES		= ${DATAFILES} ${OTHERS}

ILIST			= ${INCLUDES}
IDIR			= ${MACH3_INCLUDE_IDIR}
IMODE			= 644

.if exists(${TARGET_MACHINE}/machdep.mk)
.include "${TARGET_MACHINE}/machdep.mk"
.endif
.include <${RULES_MK}>

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	ln -s ${MACHINE} machine
