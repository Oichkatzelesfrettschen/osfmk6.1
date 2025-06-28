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

.if !defined(_OSF_LIB_MK_)
_OSF_LIB_MK_=


#  Build rules
#
.if defined(LIBRARIES)

#${LIBRARIES}: $${_LIBRARY_OFILES_}

${LIBRARIES}: $${.TARGET}($${_OFILES_})
	${_AR_} ${DEF_ARFLAGS} ${.TARGET} ${.OODATE}
.if defined(ORDER_LIBRARIES)
	${RM} -rf tmp
	mkdir tmp
	cd tmp && { \
	    ${_AR_} x ../${.TARGET}; \
	    ${RM} -f __.SYMDEF __________ELELX; \
	    ${_AR_} cr ${.TARGET} ${${OBJECT_FORMAT}_LORDER}; \
	}
	${MV} -f tmp/${.TARGET} .
	${RM} -rf tmp
.endif
	${_RANLIB_} ${.TARGET}
	${RM} -f ${.OODATE}

.endif

.if defined(SHARED_LIBRARIES)

${SHARED_LIBRARIES}: $${_OFILES_}
.if defined(ORDER_LIBRARIES)
	${_LD_} ${_SHLDFLAGS_} -o ${.TARGET}.X ${${OBJECT_FORMAT}_LORDER} ${_LIBS_}
.else
	${_LD_} ${_SHLDFLAGS_} -o ${.TARGET}.X ${_OFILES_} ${_LIBS_}
.endif
	${MV} ${.TARGET}.X ${.TARGET}

.endif

.endif
