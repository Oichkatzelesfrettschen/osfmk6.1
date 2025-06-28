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

SETUP_SUBDIRS		= mach_kernel osf1_server usr mach_services stand

EXPINC_SUBDIRS		= mach_kernel mach_services default_pager \
			  osf1_server usr
EXPLIB_SUBDIRS		= mach_services usr
EXPSHLIB_SUBDIRS	= sbin usr

SEC_SUBDIRS		= usr etc sbin seccmd tcb
SUBDIRS			= mach_kernel mach_services bootstrap default_pager \
			  osf1_server usr dev etc sbin setup stand \
			  ${SEC_DEFS:Dseccmd tcb}

IDIR			= /
ILIST			= mach

mach:
	${TARGET_EXEC_PREFIX}makeboot -o mach ${TOSTAGE}/mach_kernel \
		${TOSTAGE}/mach_servers/bootstrap

.include <${RULES_MK}>
