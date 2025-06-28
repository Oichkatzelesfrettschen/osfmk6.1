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


#
#	Makefile for netmem example.  
#

#CFLAGS			+= -DDEBUG -g

VPATH			= ${MAKETOP}mach_services/servers/netmem
PROGRAMS		= netmem_pager netmem_client

ILIST			= netmem_pager
IDIR			= ${MACH3_SERVERS_IDIR}

netmem_pager_OFILES	= netmem_msg_server.o netmem_pager.o \
			  netmem_msg_reply_user.o 
netmem_client_OFILES	= netmem_msg_user.o netmem_client.o

LIBS			= -lnetname -lmach

MIG_DEFS		= netmem_msg.defs netmem_msg_reply.defs

DATAFILES		= netmem_msg.h netmem_msg_reply.h ${MIG_DEFS}
INCLUDES		= ${DATAFILES}

EXPDIR			= ${MACH3_INCLUDE_IDIR}servers/


.include <${RULES_MK}>


netmem_msg.h netmem_msg_user.c netmem_msg_server.c :      netmem_msg.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC}  -server netmem_msg_server.c \
				-user netmem_msg_user.c

netmem_msg_reply_user.c : 	netmem_msg_reply.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -user netmem_msg_reply_user.c \
				-server /dev/null  


