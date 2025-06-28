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

VPATH                   = ../libmach:../libmach/${${TARGET_MACHINE}_VPATH}:${MAKETOP}mach_kernel/mach:${MAKETOP}mach_kernel/mach_debug:${MAKETOP}mach_kernel/device:${MAKETOP}default_pager/mach

LIBRARIES		= libmach_sa.a
EXPLIB_TARGETS		= export_libmach_sa.a

INCFLAGS                = -I../libmach -I../libmach/${${TARGET_MACHINE}_INCDIRS}
ILIST			= libmach_sa.a
IDIR			= ${MACH3_LIBRARY_IDIR}
IMODE			= 644

DEFINES			= -DMACH_IPC_COMPAT=0 -DSTANDALONE -DTypeCheck=0
CFLAGS			= ${DEFINES}
MIGFLAGS		= ${DEFINES}
OFILES			= _setjmp.o bcopy.o bzero.o \
			  default_pager_object_server.o \
			  default_pager_object_user.o device_reply_server.o \
			  device_request_user.o device_server.o \
			  device_user.o doprnt.o error_codes.o \
			  exc_server.o exc_user.o exit.o \
			  gcc.o get_privileged_ports.o \
			  htg_vm_map.o mach_debug_server.o \
			  mach_debug_user.o mach_error_string.o \
			  mach_host_server.o mach_host_user.o \
			  mach_init_sa.o mach_msg.o mach_msg_overwrite.o \
			  mach_msg_destroy.o mach_msg_receive.o \
			  mach_msg_send.o mach_msg_server.o \
			  mach_norma_server.o mach_norma_user.o \
			  mach_port_server.o mach_port_user.o \
			  mach_server.o mach_user.o memcpy.o \
			  memory_object_server.o memory_object_user.o \
			  Smem_serv.o Smem_def_serv.o \
			  Bserver.o Bpriv_ports.o \
			  mig_allocate.o \
			  mig_deallocate.o mig_reply_setup.o \
			  mig_strncpy.o mig_support.o \
			  ms_mach_port_allocate.o ms_mach_port_allocate_name.o \
			  ms_mach_port_deallocate.o \
			  ms_mach_port_insert_right.o \
			  ms_task_create.o ms_task_set_special_port.o \
			  ms_task_suspend.o ms_task_terminate.o \
			  ms_thread_depress_abort.o ms_vm_allocate.o \
			  ms_vm_deallocate.o ms_vm_map.o \
			  ms_clock_get_time.o \
			  ms_device_read.o ms_device_write.o \
			  ms_device_read_request.o ms_device_write_request.o \
			  ms_device_read_overwrite_request.o \
			  ms_device_read_overwrite.o \
			  ms_vm_read_overwrite.o ms_vm_write.o \
			  notify_server.o notify_user.o \
			  panic.o printf.o slot_name.o \
			  sprintf.o strcat.o strcmp.o strcpy.o \
			  strlen.o strncpy.o \
			  ${${TARGET_MACHINE}COBJS} \
			  ${${TARGET_MACHINE}SOBJS}

.if exists(${TARGET_MACHINE}/machdep.mk)
.include "${TARGET_MACHINE}/machdep.mk"
.endif
.include <${RULES_MK}>

Smem_serv.c: memory_object.defs
	${_MIG_} ${_MIGFLAGS_} -DSEQNOS ${.ALLSRC} -server Smem_serv.c 

Smem_def_serv.c: memory_object_default.defs
	${_MIG_} ${_MIGFLAGS_} -DSEQNOS ${.ALLSRC} -server Smem_def_serv.c

memory_object_server.c memory_object_user.c:	memory_object.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server memory_object_server.c \
				 -user   memory_object_user.c

default_pager_object_server.c default_pager_object_user.c: \
						default_pager_object.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server default_pager_object_server.c \
				 -user   default_pager_object_user.c

mach_interface.h mach_server.c : mach.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server mach_server.c -user /dev/null \
				 -header mach_interface.h

mach_user.c: mach.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -user ${.TARGET}.X -server /dev/null -header /dev/null
	sed -e 's/vm_allocate/mig_vm_allocate/' \
	    -e 's/vm_map/mig_vm_map/' \
	    -e 's/vm_write/mig_vm_write/' \
	    -e 's/vm_deallocate/mig_vm_deallocate/' \
	    -e 's/task_create/mig_task_create/' \
	    -e 's/task_terminate/mig_task_terminate/' \
	    -e 's/task_suspend/mig_task_suspend/' \
	    -e 's/task_set_special_port/mig_task_set_special_port/' \
	    < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

# This is a funny one.  We do the mig into a temporary directory and
# then pick out the one file that we actually want.  We use a
# temporary directory to avoid interfering with the files normally
# generated from mach.defs.  We use htg_vm_map.defs instead of
# using mach.defs directly so that the dependencies show up in
# htg_vm_map.defs.d instead of overwriting mach.defs.d.

htg_vm_map.c : htg_vm_map.defs
	${RM} -rf tmp_htg_vm_map
	-mkdir tmp_htg_vm_map
	${_MIG_} ${_MIGFLAGS_} -DEMULATOR -i tmp_htg_vm_map/ \
		-user htg_vm_map.c \
		-header tmp_htg_vm_map/mach_interface.h \
		-server /dev/null htg_vm_map.defs
	${RM} ${_RMFLAGS_} ${.TARGET}
	mv tmp_htg_vm_map/htg_vm_map.c ${.TARGET}

htg_vm_map.defs : mach.defs
	${RM} ${_RMFLAGS_} ${.TARGET}
	ln -s $? ${.TARGET}

htg_vm_map.o : mach_interface.h

mach_port_server.c : mach_port.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server mach_port_server.c -user /dev/null

mach_port_user.c: mach_port.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -user ${.TARGET}.X -server /dev/null
	sed -e 's/mach_port_allocate/mig_mach_port_allocate/' \
	    -e 's/mach_port_deallocate/mig_mach_port_deallocate/' \
	    -e 's/mach_port_insert_right/mig_mach_port_insert_right/' \
	    < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

Bserver.c Bpriv_ports.c: bootstrap.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server Bserver.c -user Bpriv_ports.c

exc_server.c exc_user.c: exc.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server exc_server.c -user exc_user.c

clock_user.c: clock.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -user ${.TARGET} -header clock.h -server /dev/null

clock_reply_server.c: clock_reply.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -server ${.TARGET} -user /dev/null

device_server.c: device.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -user /dev/null -server ${.TARGET}

device_user.c: device.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -user ${.TARGET}.X -header device.h -server /dev/null
	sed -e 's/device_read$$/mig_device_read/' \
	    -e 's/device_write$$/mig_device_write/' \
	    < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

device_request_user.c: device_request.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -user ${.TARGET}.X -server /dev/null
	sed -e 's/device_read_request$$/mig_device_read_request/' \
	    -e 's/device_write_request$$/mig_device_write_request/' \
	    < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

device_reply_server.c: device_reply.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -server ${.TARGET} -user /dev/null

device_read_reply_overwrite.c: device_reply.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -user ${.TARGET}.X -server /dev/null
	sed -e 's/device_read_reply_overwrite/mig_device_read_reply_overwrite/' \
	    < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

mach_host_server.c: mach_host.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server ${.TARGET} -user /dev/null

mach_host_user.c: mach_host.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -user ${.TARGET}.X -server /dev/null
	sed -e 's/thread_depress_abort/mig_thread_depress_abort/' \
	    < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

mach_debug_server.c mach_debug_user.c: mach_debug.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server mach_debug_server.c \
				 -user   mach_debug_user.c

mach_norma_server.c mach_norma_user.c: mach_norma.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server mach_norma_server.c \
				 -user   mach_norma_user.c

notify_server.c notify_user.c: notify.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server notify_server.c -user notify_user.c
