#
# makefile that is included in all other makefiles for configuration
#

################################################################
#  NOTICE!  You must edit and rename this file to CONFIG.MAK!  #
################################################################

#*********************************************************************
# determine your compiler settings
# 
# you have to
#   search for NASM     - and set the path for NASM
#   search for ??_BASE  - and set the path to your compiler
#   search for LINK     - and set the path to your linker
# 
#*********************************************************************

#**********************************************************************
#- define where to find NASM - remember - it should not be protected
#  mode DJGPP version if you're using Windows NT/2k/XP to compile
#**********************************************************************

NASM=c:\bin\nasm16

#**********************************************************************
#- where is the BASE dir of your compiler(s) ??
#**********************************************************************
						
WC_BASE=C:\watcom
MS_BASE=C:\msvc
TC2_BASE=C:\tc201
TP1_BASE=C:\tcpp
TC3_BASE=C:\tc3
BC5_BASE=C:\bc5

#**********************************************************************
#- select your default target: required CPU and what FAT system to support
#**********************************************************************

#XCPU=86
#XCPU=186
#XCPU=386

#XFAT=16
#XFAT=32

# Give extra Turbo C compiler flags here
# such as -DDEBUG : extra DEBUG output
#         -DDOSEMU : printf output goes to dosemu log
#         -DWITHFAT32 : compile with FAT32 support
#ALLCFLAGS=-DDEBUG

!include "..\mkfiles\generic.mak"

#**********************************************************************
#- which linker to use: WATCOM wlink is not suitable for linking
#**********************************************************************

# Turbo Link
#LINK=$(TC2_BASE)\tlink /m/c
LINK=d:\util\tlink /m/c
# Microsoft Link
#LINK=$(COMPILERBIN)\link /ONERROR:NOEXE /ma
# VAL: you need VAL95, NOT the one in LANG1.ZIP (yet); 
# look at the software list on www.freedos.org.
# VAL complains about MODEND record missing for Watcom compiled objects!
# LINK=c:\bin\val /MP /NCI

# use a ; to end the LINK command line?
LINKTERM=;
						

#
# $Id$
#
# $Log$
# Revision 1.9  2001/11/04 19:47:37  bartoldeman
# kernel 2025a changes: see history.txt
#
# Revision 1.8  2001/09/23 20:39:43  bartoldeman
# FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
#
# Revision 1.7  2001/04/16 14:36:56  bartoldeman
# Added ALLCFLAGS for compiler option configuration.
#
# Revision 1.6  2001/04/15 03:21:49  bartoldeman
# See history.txt for the list of fixes.
#
# Revision 1.5  2001/03/22 10:51:04  bartoldeman
# Suggest to extract F_SCOPY into libm.lib for Borland C++.
#
# Revision 1.4  2001/03/19 04:50:56  bartoldeman
# See history.txt for overview: put kernel 2022beo1 into CVS
#
# Revision 1.3  2000/05/25 20:56:19  jimtabor
# Fixed project history
#
# Revision 1.2  2000/05/14 17:07:07  jimtabor
# Cleanup CRs
#
# Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
# The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
# MS-DOS.  Distributed under the GNU GPL.
#
# Revision 1.3  1999/09/13 20:40:17  jprice
# Added COMPILER variable
#
# Revision 1.2  1999/08/25 03:59:14  jprice
# New build batch files.
#
# Revision 1.1  1999/08/25 03:20:39  jprice
# ror4 patches to allow TC 2.01 compile.
#
#
