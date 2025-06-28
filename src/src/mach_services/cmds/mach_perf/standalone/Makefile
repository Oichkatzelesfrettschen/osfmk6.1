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

VPATH 			= ../device:../exc:../hw:../ipc:../pager:../port:../sched:../task:../thread:../vm:${TARGET_MACHINE}

PROGRAMS		= mach_perf

${PROGRAMS}: ../lib/libperf.a

ILIST			= mach_perf
IDIR			= /mach_servers
CFLAGS			+= -DSTANDALONE -pic-none -DTHREAD_STATE=${MACHINE}_THREAD_STATE -DTHREAD_STATE_COUNT=${MACHINE}_THREAD_STATE_COUNT
USE_STATIC_LIBRARIES	=
DEPENDENCIES		= YES

INCFLAGS                =  -I../include -I../device -I../exc -I../hw -I../ipc -I../pager -I../port -I../sched -I../task -I../thread -I../vm ${INCDIRS}
LIBDIRS			+= -L../lib
LIBS			=  -lperf -lsa_mach -lmach -lsa_mach -lm

OFILES			= sa.o \
			  device.o \
			  random.o \
			  device_perf.o \
		          exc_perf.o \
			  ../exc/exc_server_main.o \
		          hw_perf.o \
			  ipc_perf.o \
			  ../ipc/ipc_server_main.o \
			  ../ipc/ipc_server.o \
			  ../ipc/ipc_user.o \
			  pager_perf.o \
			  ../pager/pager_server_main.o \
			  ../pager/pager_server.o \
			  ../pager/pager_user.o \
		          port_perf.o \
			  sched_perf.o \
		          task_perf.o \
		          thread_perf.o \
			  vm_perf.o \
			  ${${TARGET_MACHINE}_OFILES}

device.c: device.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server device.c \
	-user /dev/null -header /dev/null

.if exists(${TARGET_MACHINE}/machdep.mk)
.include "${TARGET_MACHINE}/machdep.mk"
.endif

.include <${RULES_MK}>

.if exists(depend.mk)
.include "depend.mk"
.endif


