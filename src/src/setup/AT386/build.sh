#!/bin/sh
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


#
# Setup immediate directory heirarchy
#
srcdir=`pwd`
base=`expr "${srcdir}" : "\(.*\)/.*"`
if [ `basename "${srcdir}"` != "src" ]
then
    echo "Please move source tree into a \"src\" subdirectory"
    exit 1
fi

#
# Read the configuration information for this host
#
if [ $# -ne 4 ]
then
  USAGE="usage: build.sh <TARGET_MACHINE> <os> <subdirpath> <target>"
  echo "ERROR: improper number of arguments"
  echo $USAGE
  exit 1
fi

if [ -r setup/$1/$2.sh ]
then
    . setup/$1/$2.sh
else
    echo "ERROR: file 'setup/$1/$2.sh' not readable"
    exit 1
fi

#
# Setup immediate directory heirarchy
#
obj=${base}/obj
if [ -d ${obj} ]
then
    true
else
    echo "Please create the path for ${obj}"
    exit 1
fi
objdir=${obj}/${target_machine}
if [ -d ${objdir} ]
then
    true
else
    echo "Please create the path for ${objdir}"
    exit 1
fi
exp=${base}/export
if [ -d ${exp} ]
then
    true
else
    echo "Please create the path for ${exp}"
    exit 1
fi
expdir=${exp}/${target_machine}/
if [ -d ${expdir} ]
then
    true
else
    echo "Please create the path for ${expdir}"
    exit 1
fi
tools=${base}/tools
if [ -d ${tools} ]
then
    true
else
    echo "Please create the path for ${tools}"
    exit 1
fi
toolsdir=${tools}/${target_machine}
if [ -d ${toolsdir} ]
then
    true
else
    echo "Please create the path for ${toolsdir}"
    exit 1
fi
hostbindir=${toolsdir}/hostbin
if [ -d ${hostbindir} ]
then
    true
else
    echo "Please create the path for ${hostbindir}"
    exit 1
fi

#
# Constrain search paths
#
PATH="${hostbindir}:${bindir}:${OS_PATH}"
# LEXER="${libdir}/ncform"
# YACCPAR="${libdir}/yaccpar"

LEXER="${srcdir}/usr/ccs/lib/ncform"
YACCPAR="${srcdir}/usr/ccs/lib/yaccpar"
FLEXSKEL="${srcdir}/usr/ccs/lib/flex.skel"

export PATH LEXER YACCPAR

CCTYPE=ansi

export CCTYPE

#
# Site/Environment stuff
#
CENV="-UMSG -D_BLD"
OWNER="bin"
GROUP="bin"

export CENV OWNER GROUP

#
# New build environment definitions
#
MAKESYSPATH="${srcdir}/usr/share/mk"
EXPORTBASE="${expdir}"
SOURCEBASE="${srcdir}"
SOURCEDIR="${srcdir}"
BACKED_SOURCEDIR="${srcdir}"
OBJECTDIR=../obj/${target_machine}

export MAKESYSPATH EXPORTBASE SOURCEBASE SOURCEDIR BACKED_SOURCEDIR OBJECTDIR

INCDIRS="-I${expdir}usr/include"
LIBDIRS="-L${expdir}usr/ccs/lib"
SHLIBDIRS="-L${expdir}usr/shlib"
MACHO_SHLIBDIRS="-L${expdir}usr/shlib"
ELF_SHLIBDIRS="-L${expdir}usr/lib"
SHLIBPATH="${expdir}usr/lib"
DEF_ARFLAGS=" cr"

export INCDIRS LIBDIRS SHLIBDIRS MACHO_SHLIBDIRS ELF_SHLIBDIRS SHLIBPATH DEF_ARFLAGS

# target prefixes

MACHO_SHARED_LIBRARIES=""
ELF_SHARED_LIBRARIES=""
ELF_GCC_EXEC_PREFIX="${toolsdir}/elf/"
MACHO_GCC_EXEC_PREFIX="${toolsdir}/macho/"
COFF_GCC_EXEC_PREFIX="${toolsdir}/coff/"
A_OUT_GCC_EXEC_PREFIX="${toolsdir}/a_out/"
ARCHIVE_FORMAT="OSFARCH"
TARGET_EXEC_PREFIX="${toolsdir}/hostbin/"

export MACHO_SHARED_LIBRARIES ELF_SHARED_LIBRARIES 
export ELF_GCC_EXEC_PREFIX MACHO_GCC_EXEC_PREFIX COFF_GCC_EXEC_PREFIX A_OUT_GCC_EXEC_PREFIX
export ARCHIVE_FORMAT TARGET_EXEC_PREFIX

# Add Definitions for Mig

IPC_FORMAT="untyped"
MIGCOM="${hostbindir}/migcom"
MIGCOM25="${hostbindir}/migcom2.5"
MACH3_MIG_IDIR="/usr/ccs/bin/"
MIGCOM_TYPED="${hostbindir}/migcom_typed"
MIGCOM_UNTYPED="${hostbindir}/migcom"

export IPC_FORMAT MIGCOM MIGCOM25 MACH3_MIG_IDIR MIGCOM_TYPED MIGCOM_UNTYPED

# mach 3 defs

MACH3_ROOT_SERVERS_IDIR=/mach_servers/
MACH3_SERVERS_IDIR=/usr/sbin/
MACH3_MIG_IDIR=/usr/ccs/bin/
MACH3_CMD_IDIR=/usr/bin/
MACH3_LIBRARY_IDIR=/usr/ccs/lib/
MACH3_INCLUDE_IDIR=/usr/include/

export MACH3_ROOT_SERVERS_IDIR MACH3_SERVERS_IDIR MACH3_MIG_IDIR MACH3_CMD_IDIR MACH3_LIBRARY_IDIR MACH3_INCLUDE_IDIR

# make -version
# which mig

# (cd $3; make -cEF $4)
(cd $3; make -r $4)

exit 0
