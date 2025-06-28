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

AT386COBJS	= bcopy.o fixunsdfsi.o fp_const.o frexp.o index.o\
		  memccpy.o memchr.o memset.o msem_init.o msem_lock.o\
		  msem_remove.o msem_unlock.o rindex.o strcat.o\
		  strchr.o strcpyn.o strncat.o strncmp.o strrchr.o

PROTO_SYSCALLS	= accept.o access.o acct.o adjtime.o async_daemon.o\
		  bind.o chdir.o chmod.o chown.o chroot.o close.o\
		  connect.o dup.o dup2.o exec_with_loader.o execve.o\
		  fchdir.o fchmod.o fchown.o fcntl.o flock.o fstat.o\
		  fstatfs.o fsync.o ftruncate.o getaddressconf.o\
		  getdirentries.o getdomainname.o getfh.o getfsstat.o\
		  getgid.o getgroups.o gethostid.o gethostname.o\
		  getitimer.o getpagesize.o getpeername.o getpgid.o\
		  getpgrp.o getpid.o getpriority.o getprocthreads.o\
		  getrlimit.o getrusage.o getsid.o getsockname.o getsockopt.o\
		  gettimeofday.o getuid.o ioctl.o kill.o kernel_execve.o kloadcall.o \
		  lchown.o link.o listen.o lseek.o lstat.o madvise.o\
		  mkdir.o mknod.o mmap.o mprotect.o msgctl.o msgget.o\
		  msgrcv.o msgsnd.o msleep.o msync.o munmap.o mvalid.o\
		  mwakeup.o naccept.o nfssvc.o ngetpeername.o\
		  ngetsockname.o nrecvfrom.o nrecvmsg.o nsendmsg.o\
		  open.o plock.o poll.o profil.o quotactl.o read.o\
		  readlink.o readv.o reboot.o recv.o recvfrom.o\
		  recvmsg.o rename.o revoke.o rmdir.o select.o semctl.o\
		  semget.o semop.o send.o sendmsg.o sendto.o\
		  setdomainname.o setgid.o setgroups.o sethostid.o\
		  sethostname.o setitimer.o setlogin.o setpgid.o\
		  setpriority.o setregid.o setreuid.o setrlimit.o\
		  setsid.o setsockopt.o settimeofday.o setuid.o shmat.o\
		  shmctl.o shmdt.o shmget.o shutdown.o sigsend.o\
		  sigsendset.o sigstack.o socket.o socketpair.o stat.o\
		  statfs.o subsys_info.o swapon.o symlink.o sync.o\
		  syscall_num.o table.o truncate.o umask.o umount.o\
		  uname.o unlink.o utimes.o wait4.o write.o writev.o

PSEUDO_SYSCALLS	= _exit.o _getlogin.o _set_program_attributes.o\
		  _sigpending.o _sigprocmask.o _sigsuspend.o

ASSEM_OBJS	= __divsi3.o __udivsi3.o _fork.o _setjmp.o\
		  abs.o alloca.o bcmp.o bzero.o cerror.o execl.o\
		  execle.o exect.o execv.o fabs.o ffs.o fixdfsi.o\
		  fpinit.o getegid.o geteuid.o getppid.o htonl.o\
		  htons.o insque.o mcount.o memcmp.o memcpy.o\
		  memmove.o modf.o mount.o msem_tas.o ntohl.o ntohs.o ptrace.o\
		  remque.o setjmp.o sigaction.o sigreturn.o\
		  socketpipe.o strcmp.o strcpy.o strlen.o strncpy.o\
		  syscall.o write_rnd.o

.if	!defined(CREATING_SHARED_LIBC)
ASSEM_OBJS     += brk.o sbrk.o _ldr_jump.o
.endif

AT386SOBJS	= ${PROTO_SYSCALLS} ${PSEUDO_SYSCALLS} ${ASSEM_OBJS}

fpinit.o_CFLAGS		= ${CFLAGS} -D__ASSEMBLER__
write_rnd.o_CFLAGS	= ${CFLAGS} -D__ASSEMBLER__

SED_SYSCALL='s/SYSCALL_NAME/${.TARGET:.S=}/'

${PROTO_SYSCALLS:.o=.S}: syscall_tmpl.s
	${SED} -e ${SED_SYSCALL} ${syscall_tmpl.s:P} > ${.TARGET}.X
	${MV} ${.TARGET}.X ${.TARGET}

SED_PSEUDO='s/SYSCALL_NAME/${.TARGET:S/^_//:.S=}/'

${PSEUDO_SYSCALLS:.o=.S}: pseudo_tmpl.s
	${SED} -e ${SED_PSEUDO} ${pseudo_tmpl.s:P} > ${.TARGET}.X
	${MV} ${.TARGET}.X ${.TARGET}

${ASSEM_OBJS:.o=.S}: $${.TARGET:.S=.s}
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

${AT386SOBJS}: $${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -traditional -O -c ${.TARGET:.o=.S}
