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

#
#	Build the sample RPC server and client
#

#MIGFLAGS	       += -migcom /afs/ri/project/sb/watkins/rt2/src/mach_services/servers/rpc/migcom -t 
MIGFLAGS	       += -t

USE_STATIC_LIBRARIES	=

PROGRAMS		= rpc_server rpc_client

ILIST			= rpc
IDIR			= ${MACH3_SERVERS_IDIR}

LIBS			= -lnetname -lmach
rpc_server_OFILES	= rpc_server.o rpc_sampleServer.o
rpc_client_OFILES	= rpc_client.o rpc_sampleUser.o

MIG_DEFS		= rpc_sample.defs  

DATAFILES		= rpc_sample_user.h 		\
			  rpc_sample_server.h ${MIG_DEFS}	
INCLUDES		= ${DATAFILES}

EXPDIR			= ${MACH3_INCLUDE_IDIR}servers/

.include <${RULES_MK}>


rpc_sampleServer.c rpc_sample_user.h rpc_sample_server.h :	rpc_sample.defs
	${_MIG_} ${_MIGFLAGS_} ${.ALLSRC} -server rpc_sampleServer.c 	\
	-user rpc_sampleUser.c -sheader rpc_sample_server.h 		\
	-header rpc_sample_user.h
