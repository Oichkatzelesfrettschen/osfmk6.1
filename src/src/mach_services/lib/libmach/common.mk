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

VPATH                   = ${MAKETOP}mach_services/lib/libmach:${${TARGET_MACHINE}_VPATH}:${MAKETOP}mach_kernel/mach:${MAKETOP}mach_kernel/mach_debug:${MAKETOP}mach_kernel/device:${MAKETOP}default_pager/mach

LIBRARIES		= ${LOCAL_LIBMACH}

EXPLIB_TARGETS		= export_${LOCAL_LIBMACH}

ILIST			= ${LIBRARIES}
IMODE			= 644

INCFLAGS                = -I${MAKETOP}mach_services/lib/libmach \
			  -I${MAKETOP}mach_services/lib/libmach/${TARGET_MACHINE}
mach_user.c_MIGFLAGS	= -DMIG_USER -DMOR=1 ${LOCAL_MIGFLAGS}
mach_debug.c_MIGFLAGS	= -DMIG_USER ${LOCAL_MIGFLAGS}
mach_user.o_CFLAGS	+= -DTypeCheck=0 ${CFLAGS}
DEFINES			= -DMACH_IPC_COMPAT=0 -DTypeCheck=0 -DMOR=1
CFLAGS			= ${DEFINES} ${LOCAL_CFLAGS}
MIGFLAGS		= ${DEFINES} ${LOCAL_MIGFLAGS}
MIG_DEFS		= bootstrap.defs \
			  clock.defs \
			  clock_reply.defs \
			  default_pager_object.defs \
			  device.defs \
			  device_reply.defs \
			  device_request.defs \
			  exc.defs \
			  mach.defs \
			  mach_debug.defs \
			  mach_host.defs \
			  mach_norma.defs \
			  mach_port.defs \
			  memory_object.defs \
			  notify.defs \
			  sync.defs	

TRAPS			= clock_get_time device_read device_read_overwrite \
			  device_read_overwrite_request device_read_request \
			  device_write device_write_request \
			  kernel_task_create mach_port_allocate \
			  mach_port_allocate_qos \
			  mach_port_allocate_name \
			  mach_port_allocate_full \
			  mach_port_allocate_subsystem mach_port_deallocate \
			  mach_port_destroy mach_port_insert_right \
			  mach_port_move_member mach_port_rename \
			  task_create task_info task_set_exception_ports \
			  task_set_special_port task_suspend task_terminate \
			  task_set_port_space \
			  thread_create_running thread_depress_abort \
			  thread_set_exception_ports thread_switch \
			  vm_allocate vm_deallocate vm_map vm_msync \
			  vm_protect vm_read_overwrite vm_region vm_write \
			  vm_remap

# Considerably complexify this, on behalf of brain-dead System V archiver
# (which imposes 15-character maximum on object filenames).  Basically,
# there should be a name in either FULLTRAPS or TRUNCTRAPS for each of
# names in TRAPS.  If a name is longer than 10 characters (15 chars
# - "MS_" - ".o"), it has to be truncated in TRUNCTRAPS.  Yum!
#
FULLTRAPS		= task_info \
			  vm_map \
			  vm_msync \
			  vm_protect \
			  vm_region \
			  vm_remap \
			  vm_write
TRUNCTRAPS		= CLK_get_tm \
			  DEV_r_o_rq \
			  DEV_r_over \
			  DEV_r_req \
			  DEV_read \
			  DEV_w_req \
			  DEV_write \
			  K_TK_creat \
			  M_pt_al_nm \
			  M_pt_al_qs \
			  M_pt_al_sb \
			  M_pt_alloc \
			  M_pt_deall \
			  M_pt_dest \
			  M_pt_in_rt \
			  M_pt_mv_m \
			  M_pt_renam \
			  TH_creat_r \
			  TH_dep_abt \
			  TH_s_ex_ps \
			  TH_switch \
			  TK_create \
			  TK_s_ex_ps \
			  TK_s_p_s \
			  TK_s_sp_pt \
			  TK_suspend \
			  TK_termine \
			  VM_alloc \
			  VM_dealloc \
			  VM_r_over

# Same deal here as for trap names, except that here the length limit
# is 13 characters (no "MS_" prefix).  Any object file with a longer
# name has to be truncated in TRUNCOFILES.
#
TRUNCOFILES		= BTServer.o \
			  BTUser.o \
			  CK_rp_server.o \
			  DEV_r_rp_over.o \
			  DEV_req_user.o \
			  DEV_rp_server.o \
			  DP_obj_server.o \
			  DP_obj_user.o \
			  MG_deallocate.o \
			  MG_rp_setup.o \
			  MO_server.o \
			  MO_user.o \
			  M_debug_serve.o \
			  M_debug_user.o \
			  M_error_strin.o \
			  M_host_user.o \
			  M_init_ports.o \
			  M_msg_destroy.o \
			  M_msg_receive.o \
			  M_msg_server.o \
			  M_norma_user.o \
			  M_port_user.o

OFILES =		  clock_res.o \
			  clock_sleep.o \
			  clock_user.o \
			  device_user.o \
			  error_codes.o \
			  exc_server.o \
			  exc_user.o \
			  mach_error.o \
			  mach_init.o \
			  mach_msg.o \
			  mach_msg_send.o \
			  mach_user.o \
			  mig_allocate.o \
			  mig_strncpy.o \
			  mig_support.o \
			  notify_server.o \
			  notify_user.o \
			  panic.o \
			  port_obj.o \
			  prof_server.o \
			  prof_user.o \
			  safe_gets.o \
			  sbrk.o \
			  slot_name.o \
			  sync_user.o \
			  Smem_serv.o \
			  Smem_def_serv.o \
			  ${TRUNCOFILES} \
			  ${TRUNCTRAPS:S/^/MS_/g:S/$/.o/g} \
			  ${FULLTRAPS:S/^/ms_/g:S/$/.o/g} \
			  ${${TARGET_MACHINE}_OFILES}

.if exists(${TARGET_MACHINE}/machdep.mk)
.include "${TARGET_MACHINE}/machdep.mk"
.endif
.include <${RULES_MK}>

# Sources for raw traps are taken from kernel include files.
# Routines that shadow libc versions are ignored.
# .h files are used for mig generated interfaces instead of the .c files

migsubst.sed: common.mk
	for i in ${TRAPS}; do \
		echo "s/kern_return_t $$i"'$$'"/kern_return_t mig_$$i/"; \
	done > migsubst.sed

prof.h prof_user.c prof_server.c :	prof.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server prof_server.c \
				-user prof_user.c

memory_object_server.c memory_object_user.c:	memory_object.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server memory_object_server.c \
				 -user   memory_object_user.c

Smem_serv.c: memory_object.defs
	${_MIG_} ${_MIGFLAGS_} -DSEQNOS ${.ALLSRC:M*.defs} -server Smem_serv.c 

Smem_def_serv.c: memory_object_default.defs
	${_MIG_} ${_MIGFLAGS_} -DSEQNOS ${.ALLSRC:M*.defs} -server Smem_def_serv.c

default_pager_object_server.c default_pager_object_user.c: \
						default_pager_object.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server default_pager_object_server.c \
				 -user   default_pager_object_user.c

mach_user.c: mach.defs migsubst.sed
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET}.X -server /dev/null -header mach_interface.h
	sed -f migsubst.sed < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

mach_port_user.c: mach_port.defs migsubst.sed
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET}.X -server /dev/null -header mach_port.h
	sed -f migsubst.sed < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

exc_server.c exc_user.c: exc.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server exc_server.c -user exc_user.c

clock_user.c: clock.defs migsubst.sed
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET}.X -header clock.h -server /dev/null
	sed -f migsubst.sed < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

clock_reply_server.c: clock_reply.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server ${.TARGET} -user /dev/null

device_user.c: device.defs migsubst.sed
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET}.X -header device.h -server /dev/null
	sed -f migsubst.sed < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

device_request_user.c: device_request.defs migsubst.sed
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET}.X -server /dev/null
	sed -f migsubst.sed < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

device_reply_server.c: device_reply.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server ${.TARGET} -user /dev/null

device_read_reply_overwrite.c: device_reply.defs migsubst.sed
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET}.X -server /dev/null
	sed -f migsubst.sed < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

mach_host_user.c: mach_host.defs migsubst.sed
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET}.X -server /dev/null
	sed -f migsubst.sed < ${.TARGET}.X > ${.TARGET}
	${RM} ${_RMFLAGS_} ${.TARGET}.X

mach_debug_server.c mach_debug_user.c: mach_debug.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server mach_debug_server.c \
				 -user   mach_debug_user.c

mach_norma_user.c: mach_norma.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -user ${.TARGET} -server /dev/null

notify_server.c notify_user.c: notify.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC:M*.defs}  -server notify_server.c -user notify_user.c

sync_user.c: sync.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -user sync_user.c -server /dev/null

#
# Rules to generate truncated object filenames
#
BTServer.o: bootstrapServer.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

BTUser.o: bootstrapUser.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

CK_rp_server.o: clock_reply_server.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

DEV_r_rp_over.o: device_read_reply_overwrite.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

DEV_req_user.o: device_request_user.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

DEV_rp_server.o: device_reply_server.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

DP_obj_server.o: default_pager_object_server.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

DP_obj_user.o: default_pager_object_user.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MG_deallocate.o: mig_deallocate.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MG_rp_setup.o: mig_reply_setup.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MO_server.o: memory_object_server.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MO_user.o: memory_object_user.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_CLK_get_tm.o: ms_clock_get_time.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_DEV_r_o_rq.o: ms_device_read_overwrite_request.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_DEV_r_over.o: ms_device_read_overwrite.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_DEV_r_req.o: ms_device_read_request.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_DEV_read.o: ms_device_read.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_DEV_w_req.o: ms_device_write_request.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_DEV_write.o: ms_device_write.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_K_TK_creat.o: ms_kernel_task_create.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_al_nm.o: ms_mach_port_allocate_name.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_al_qs.o: ms_mach_port_allocate_qos.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_al_fl.o: ms_mach_port_allocate_full.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_al_sb.o: ms_mach_port_allocate_subsystem.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_alloc.o: ms_mach_port_allocate.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_deall.o: ms_mach_port_deallocate.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_dest.o: ms_mach_port_destroy.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_in_rt.o: ms_mach_port_insert_right.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_mv_m.o: ms_mach_port_move_member.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_M_pt_renam.o: ms_mach_port_rename.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TH_creat_r.o: ms_thread_create_running.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TH_dep_abt.o: ms_thread_depress_abort.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TH_s_ex_ps.o: ms_thread_set_exception_ports.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TH_switch.o: ms_thread_switch.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TK_create.o: ms_task_create.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TK_s_ex_ps.o: ms_task_set_exception_ports.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TK_s_p_s.o: ms_task_set_port_space.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TK_s_sp_pt.o: ms_task_set_special_port.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TK_suspend.o: ms_task_suspend.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_TK_termine.o: ms_task_terminate.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_VM_alloc.o: ms_vm_allocate.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_VM_dealloc.o: ms_vm_deallocate.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

MS_VM_r_over.o: ms_vm_read_overwrite.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_debug_serve.o: mach_debug_server.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_debug_user.o: mach_debug_user.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_error_strin.o: mach_error_string.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_host_user.o: mach_host_user.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_init_ports.o: mach_init_ports.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_msg_destroy.o: mach_msg_destroy.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_msg_receive.o: mach_msg_receive.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_msg_server.o: mach_msg_server.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_norma_user.o: mach_norma_user.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@

M_port_user.o: mach_port_user.o
	${RM} ${_RMFLAGS_} $@
	${CP} $> $@
