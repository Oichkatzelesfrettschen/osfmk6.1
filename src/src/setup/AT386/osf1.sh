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
# Setup variables describing the current environment
#
MACHINE="i386"
TARGET_MACHINE="AT386"
target_machine="at386"
CPUTYPE="I386"
cputype="i386"
OS_TYPE="OSF1"
os_type="osf1"


export MACHINE TARGET_MACHINE target_machine CPUTYPE cputype

#
# Project definitions
#
PROJECT_NAME=OSC
project_name=osc
RULES_MK=osf.rules.mk

export PROJECT_NAME project_name RULES_MK


#
# Environment variables for the host compilers
#
OS_PATH="/usr/bin"

export OS_PATH


#
# Use native object or macho version of gcc
#
HOST_OBJECT_FORMAT=A_OUT
CMD_OBJECT_FORMAT=MACHO
KERNEL_OBJECT_FORMAT=MACHO

ARCHIVE_FORMAT=OSFARCH

export ARCHIVE_FORMAT

#
# Define these if you do not have them
#
#NO_VFPRINTF=NO_VFPRINTF
#export NO_VFPRINTF
#NO_SETENV=NO_SETENV
#export NO_SETENV
#NO_PW_STAYOPEN=NO_PW_STAYOPEN
#export NO_PW_STAYOPEN
#NO_DIRENT=NO_DIRENT
#export NO_DIRENT
#NO_UUDECODE=NO_UUDECODE
#export NO_UUDECODE
#NO_SHARED_LIBRARIES=NO_SHARED_LIBRARIES
#export NO_SHARED_LIBRARIES
