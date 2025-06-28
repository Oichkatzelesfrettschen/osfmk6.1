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
# Mach Operating System
# Copyright (c) 1994 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
# 
# Carnegie Mellon requests users of this software to return to
# 
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they make and grant Carnegie Mellon
# the rights to redistribute these changes.
#
#
# OSF Research Institute MK6.1 (unencumbered) 1/31/1995
#

VPATH = ../../../mach_kernel/${MACHINE}

USE_STATIC_LIBRARIES =
OBJECT_FORMAT = ELF

OTHERS = mach_kboot

ILIST = mach_kboot
IDIR = /stand/mach_kboot

# start.o should be first
OBJS = start.o 3c503.o arp.o asm.o bios.o boot.o bootp.o io.o dlink.o\
	endian.o gcc.o i386at.o inflate.o ns8390.o smc.o table.o tftp.o\
	udpip.o unzip.o
mach_kboot_OFILES = ${OBJS}
SOBJS = asm.o bios.o gcc.o start.o
COBJS = 3c503.o arp.o boot.o bootp.o io.o dlink.o endian.o inflate.o\
	i386at.o ns8390.o smc.o table.o tftp.o udpip.o unzip.o

NO_STRICT_ANSI =

.include <${RULES_MK}>

${OTHERS}: $${$${.TARGET}_OFILES}
	${_LD_} -Ttext 0 -o ${.TARGET}.X ${${.TARGET}_OFILES}

${SOBJS:.o=.S}: $${.TARGET:.S=.s}
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${${.TARGET:.S=.s}:P} ${.TARGET}

${COBJS:.o=.S}: $${.TARGET:.S=.c}
	${RM} ${_RMFLAGS_} ${.TARGET}
	${_CC_} ${_CCFLAGS_} -S ${${.TARGET:.S=.c}:P} 
	sed -e "s/\\.data/.text/"  ${.TARGET:.S=.s} > ${.TARGET}

${OBJS}: $${.TARGET:.o=.S}
	${_CC_} ${_CCFLAGS_} -traditional -E ${.TARGET:.o=.S} > ${.TARGET:.o=.i}
	sed '/^\#/d' ${.TARGET:.o=.i} > X${.TARGET:.o=.s}
	${_CC_} -c X${.TARGET:.o=.s} -o ${.TARGET}
