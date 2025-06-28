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

LIBRARIES		= libperf.a
USE_STATIC_LIBRARIES	= YES
DEPENDENCIES		= YES

INCDIRS			:= ${INC3DIRS} ${INCDIRS}
INCFLAGS                = -I../include

DEFINES			= -pic-none  -DTHREAD_STATE=${MACHINE}_THREAD_STATE -DTHREAD_STATE_COUNT=${MACHINE}_THREAD_STATE_COUNT
CFLAGS			= ${DEFINES}
MIGFLAGS		= ${DEFINES} ${INCFLAGS}
MIG_DEFS		= printf.defs prof_ctrl.defs
OFILES			= doprnt.o \
			  fork.o \
			  mach.o \
			  printf_server.o \
			  printf_user.o \
			  printf.o \
			  prof.o \
			  prof_ctrl_server.o \
			  prof_ctrl_user.o \
			  ressources.o \
			  server.o \
			  server_server.o \
			  server_user.o \
		          stat.o \
			  test.o \
			  thread.o \
			  time.o \
			  ux_prof.o \
			  ${${TARGET_MACHINE}_OFILES}

printf_server.c printf_user.c printf.h: printf.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server printf_server.c \
	-user printf_user.c -header printf.h

prof_ctrl_server.c prof_ctrl_user.c: prof_ctrl.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server prof_ctrl_server.c \
	-user prof_ctrl_user.c -header /dev/null

server_server.c server_user.c: server.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server server_server.c \
	-user server_user.c -header /dev/null

.include <${RULES_MK}>
.if exists(${TARGET_MACHINE}/machdep.mk)
.include "${TARGET_MACHINE}/machdep.mk"
.endif

.if exists(depend.mk)
.include "depend.mk"
.endif
