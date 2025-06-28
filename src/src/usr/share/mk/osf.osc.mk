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

.if !defined(_OSF_OSC_MK_)
_OSF_OSC_MK_=

#
#  Define MAILSYSTEM (used by rcs, etc. to send mail)
#
MAILSYSTEM?=SENDMAIL

#
#  Security flags
#
_ACL_SWARE_DEFS=-DSEC_ACL_SWARE
_ACL_POSIX_DEFS=-DSEC_ACL_POSIX
_ACL_DEFS_=${ACL_POLICY:D${_${ACL_POLICY}_DEFS}:}
_SEC_C2_DEFS_=-DSEC_BASE ${_ACL_DEFS_}
_SEC_B1_DEFS_=-DSEC_BASE ${_ACL_DEFS_} -DSEC_PRIV -DSEC_MAC_OB


.if !defined(SEC_DEFS) && defined(SEC_LEVEL)
SEC_DEFS=${_SEC_${SEC_LEVEL}_DEFS_}
.endif

.if defined(SEC_DEFS)
SEC_LIBS=-lsecurity
.else
SEC_LIBS=
.endif
PROGRAMS_C2=${PROGRAMS_BASE} ${PROGRAMS_ACL} ${PROGRAMS_${ACL_POLICY}}
PROGRAMS_B1=${PROGRAMS_C2} ${PROGRAMS_PRIV} ${PROGRAMS_MAC}

.if defined(SEC_LEVEL) && !defined(ACL_POLICY)
ACL_POLICY=ACL_POSIX
.endif

#
#  Default locale/language information
#
LOCALE?=C
LOCALEPATH=${MAKETOP}usr/lib/nls/loc
MSGLANG?=en_US.ISO8859-1
MSGLANGPATH=${MAKETOP}usr/lib/nls/msg/${MSGLANG}

#
#  Flags to keep ${MAKEPATH} program quiet on success
#
MAKEPATHFLAGS=-quiet

#
#  Flags to get the loader to strip symbols
#
_MACHO_LDSTRIP_	= -%ld," -x" -%ld," -S"
_ELF_LDSTRIP_	= -%ld," -x" -%ld," -S"

#
#  Strip options when invoking 'cc'
#
_MACHO_SHLDSTRIP_ = -x -S
_ELF_SHLDSTRIP_	  = -x -S

#
#  Strip options when invoking 'ld' directly
#
LDSTRIP	   = ${_${OBJECT_FORMAT}_LDSTRIP_}
SHLDSTRIP  = ${_${OBJECT_FORMAT}_SHLDSTRIP_}

#
#  Format-dependent lorder rules 
#
.if defined(ORDER_LIBRARIES)
COFF_LORDER=`lorder *.o | tsort`
ELF_LORDER=${_OFILES_}
A_OUT_LORDER=${_OFILES_}
MACHO_LORDER=`${LORDER} ${_OFILES_} | ${TSORT}`
.endif

#
#  Shared libraries definitions
#
#  First, see if we have specified an object format that does not
#  support shared libraries.
#
.if !defined(${OBJECT_FORMAT}_SHARED_LIBRARIES)
NO_SHARED_LIBRARIES=
.endif

#
#  Now see if anyone has specified STATIC.
#
.if defined(NO_SHARED_LIBRARIES) || defined(USE_STATIC_LIBRARIES)
USE_SHARED_LIBRARIES=0
.else
USE_SHARED_LIBRARIES=1
.endif

#
#  C compiler variations
#
_GCC_EXEC_PREFIX_=${${OBJECT_FORMAT}_GCC_EXEC_PREFIX}

BASE_CC?=${_GCC_EXEC_PREFIX_}gcc -B${_GCC_EXEC_PREFIX_} ${GLINE}
ANSI_CC?=${BASE_CC} ${${.TARGET}_NO_STRICT_ANSI:D:U${NO_STRICT_ANSI:D:U-pedantic -Wpointer-arith}}
TRADITIONAL_CC?=${BASE_CC} -traditional
WRITABLE_STRINGS_CC?=${BASE_CC} -fwritable-strings

ANSI_LD?=${_GCC_EXEC_PREFIX_}ld
TRADITIONAL_LD?=${ANSI_LD}
WRITABLE_STRINGS_LD?=${ANSI_LD}

ANSI_AR?=${_GCC_EXEC_PREFIX_}ar
TRADITIONAL_AR?=${ANSI_AR}
WRITABLE_STRINGS_AR?=${ANSI_AR}

ANSI_RANLIB?=${_GCC_EXEC_PREFIX_}ranlib
TRADITIONAL_RANLIB?=${ANSI_RANLIB}
WRITABLE_STRINGS_RANLIB?=${ANSI_RANLIB}

ANSI_CONFIG?=${TARGET_EXEC_PREFIX}config
TRADITIONAL_CONFIG?=${ANSI_CONFIG}
WRITABLE_STRINGS_CONFIG?=${ANSI_CONFIG}

ANSI_MIG?=${TARGET_EXEC_PREFIX}mig
TRADITIONAL_MIG?=${ANSI_MIG}
WRITABLE_STRINGS_MIG?=${ANSI_MIG}

_CCDEFS_= -D${MACHINE} -D__${MACHINE}__

MACHO_EXTRA_WARNINGS	= -Wall\
			  -Waggregate-return\
			  -Wcast-align\
			  -Wimplicit\
			  -Wmissing-prototypes\
			  -Wnested-externs\
			  -Wno-parentheses\
			  -Wshadow\
			  -Wstrict-prototypes\
			  -Wtraditional\
			  -Wwrite-strings

ELF_EXTRA_WARNINGS	= -Wall\
			  -Waggregate-return\
			  -Wcast-align\
			  -Wimplicit\
			  -Wmissing-prototypes\
			  -Wnested-externs\
			  -Wno-parentheses\
			  -Wshadow\
			  -Wstrict-prototypes\
			  -Wtraditional\
			  -Wwrite-strings

.if ${USE_SHARED_LIBRARIES}
_SHCCDEFS_=-D_SHARED_LIBRARIES
.else
_SHCCDEFS_=
.endif

_PMAX_MACHO_CFLAGS_=-G 0
_MMAX_MACHO_CFLAGS_=-mnosb
_O_F_CFLAGS_=${_${_T_M_}_${OBJECT_FORMAT}_CFLAGS_}

_host_NOSTDINC_=
_ansi_NOSTDINC_=-nostdinc
_traditional_NOSTDINC_=-nostdinc
_writable_strings_NOSTDINC_=-nostdinc
_CC_NOSTDINC_=${_${_CCTYPE_}_NOSTDINC_}

_host_GENINC_=
_ansi_GENINC_=${x:L:!${GENPATH} -I.!} -I-
_traditional_GENINC_=${x:L:!${GENPATH} -I.!} -I-
_writable_strings_GENINC_=${x:L:!${GENPATH} -I.!} -I-
_CC_GENINC_=${_${_CCTYPE_}_GENINC_}

#
#  C flags for turning on/off position independent code.
#
.if ${USE_SHARED_LIBRARIES}
_MACHO_PIC_=-pic-extern
_ELF_PIC_=-fpic
.else
_MACHO_PIC_=-pic-none
_ELF_PIC_=-pic-none
.endif
_host_PIC_=
_ansi_PIC_=${_${OBJECT_FORMAT}_PIC_}
_traditional_PIC_=${_${OBJECT_FORMAT}_PIC_}
_writable_strings_PIC_=${_${OBJECT_FORMAT}_PIC_}
_CC_PICLIB_=${_${_CCTYPE_}_PIC_}

#
#  Variables to send to linker when not generating
#  shared libraries.
#
_MACHO_NOSHRLIB_=-noshrlib
_ELF_NOSHRLIB_=-dn 

#
#  Any special linker options dealing with non-pic code
#  and shared libraries.
#
.if ${USE_SHARED_LIBRARIES}
_MACHO_GLUE_=-%ld," -warn_nopic -glue"
_ELF_GLUE_=
.else
_${OBJECT_FORMAT}_GLUE_=${_${OBJECT_FORMAT}_NOSHRLIB_}
.endif
_host_GLUE_=
_ansi_GLUE_=${_${OBJECT_FORMAT}_GLUE_}
_traditional_GLUE_=${_${OBJECT_FORMAT}_GLUE_}
_writable_strings_GLUE_=${_${OBJECT_FORMAT}_GLUE_}
_CC_GLUE_=${_${_CCTYPE_}_GLUE_}

_host_GENLIB_=
_ansi_GENLIB_=
_traditional_GENLIB_=
_writable_strings_GENLIB_=
_CC_GENLIB_=${_${_CCTYPE_}_GENLIB_}

_host_LFLAGS_=
_ansi_LFLAGS_=${FLEXSKEL:D-S${FLEXSKEL}}
_traditional_LFLAGS_=${FLEXSKEL:D-S${FLEXSKEL}}
_writable_strings_LFLAGS_=${FLEXSKEL:D-S${FLEXSKEL}}
_LEX_LFLAGS_=${_${_CCTYPE_}_LFLAGS_}
#
#  Project rules
#
MIG?=mig
MIGCPP?=${_GCC_EXEC_PREFIX_}cpp

_MIGFLAGS_=\
	-cpp ${MIGCPP}\
	${${.TARGET}_MIGENV:U${MIGENV}}\
	${${.TARGET}_MIGFLAGS:U${MIGFLAGS}}\
	${${.TARGET}_MIGARGS:U${MIGARGS}}\
	${_CC_NOSTDINC_} ${_GENINC_} ${_CC_INCDIRS_}

.if defined(MIG_DEFS)
_MIG_HDRS_=${MIG_DEFS:.defs=.h}
_MIG_USRS_=${MIG_DEFS:.defs=User.c}
_MIG_SRVS_=${MIG_DEFS:.defs=Server.c}

${_MIG_HDRS_}: $${.TARGET:.h=.defs}
	${_MIG_} ${_MIGFLAGS_} ${${.TARGET:.h=.defs}:P} \
		-server /dev/null -user /dev/null

${_MIG_USRS_}: $${.TARGET:S/User.c$$/.defs/}
	${_MIG_} ${_MIGFLAGS_} ${${.TARGET:S/User.c$/.defs/}:P} \
		 -server /dev/null

${_MIG_SRVS_}: $${.TARGET:S/Server.c$$/.defs/}
	${_MIG_} ${_MIGFLAGS_} ${${.TARGET:S/Server.c$/.defs/}:P} \
		 -user /dev/null
.endif

.if defined(MSGHDRS)
${MSGHDRS}: $${.TARGET:_msg.h=.msg}
	${MKCATDEFS} ${.TARGET:_msg.h=} ${.ALLSRC} > ${.TARGET:_msg.h=.cat.in}
	-${GENCAT} ${.TARGET:_msg.h=.cat} ${.TARGET:_msg.h=.cat.in}
	-${RM} ${_RMFLAGS_} ${.TARGET:_msg.h=.cat.in}
.if empty(.TARGETS:Msetup_*)
	-${RM} ${_RMFLAGS_} ${MSGLANGPATH}/${.TARGET:_msg.h=.cat}
	${MAKEPATH} ${MAKEPATHFLAGS} ${MSGLANGPATH}/${.TARGET:_msg.h=.cat}
	${CP} ${.TARGET:_msg.h=.cat} ${MSGLANGPATH}/${.TARGET:_msg.h=.cat}
.endif
.endif

.if defined(CATFILES)
${CATFILES}: $${.TARGET:.cat=.msg}
	${MKCATDEFS} ${.TARGET:.cat=} ${.ALLSRC} > ${.TARGET}.in
	-${GENCAT} ${.TARGET} ${.TARGET}.in
	-${RM} ${_RMFLAGS_} ${.TARGET}.in
.if empty(.TARGETS:Msetup_*)
	-${RM} ${_RMFLAGS_} ${MSGLANGPATH}/${.TARGET}
	${MAKEPATH} ${MAKEPATHFLAGS} ${MSGLANGPATH}/${.TARGET}
	${CP} ${.TARGET} ${MSGLANGPATH}/${.TARGET}
.endif
.endif

.endif
