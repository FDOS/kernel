#
# makefile that is included in all other makefiles for configuration
#

################################################################
#  NOTICE!  You must edit and rename this file to CONFIG.MAK!  #
################################################################

# These are generic definitions
RM=..\utils\rm -f
NASM=nasm


# Use these for Turbo C 2.01
#COMPILER=TC2
#COMPILERPATH=c:\tc201
#CC=$(COMPILERPATH)\tcc
#LINK=$(COMPILERPATH)\tlink
#LIBUTIL=$(COMPILERPATH)\tlib
#LIBPATH=$(COMPILERPATH)\lib
#CLIB=$(COMPILERPATH)\lib\cs.lib
#INCLUDEPATH=$(COMPILERPATH)\include
#MATH_EXTRACT=*LDIV *LLSH *LURSH *LXMUL *LRSH *SPUSH *SCOPY
#MATH_INSERT=+LDIV +LLSH +LURSH +LXMUL +LRSH +SPUSH +SCOPY


# Use these for Turbo C 3.0
#COMPILER=TC3
#COMPILERPATH=c:\tc
#CC=$(COMPILERPATH)\bin\tcc
#LINK=$(COMPILERPATH)\bin\tlink
#LIBUTIL=$(COMPILERPATH)\bin\tlib
#LIBPATH=$(COMPILERPATH)\lib
#CLIB=$(COMPILERPATH)\lib\cs.lib
#INCLUDEPATH=$(COMPILERPATH)\include
#MATH_EXTRACT=*H_LDIV *H_LLSH *H_LURSH *N_LXMUL *F_LXMUL *H_LRSH *H_SPUSH *N_SCOPY *F_SCOPY
#MATH_INSERT=+H_LDIV +H_LLSH +H_LURSH +N_LXMUL +F_LXMUL +H_LRSH +H_SPUSH +N_SCOPY +F_SCOPY


# Use these for Borland C++
#COMPILER=BC5
#COMPILERPATH=c:\bc5
#CC=$(COMPILERPATH)\bin\tcc
#LINK=$(COMPILERPATH)\bin\tlink
#LIBUTIL=$(COMPILERPATH)\bin\tlib
#LIBPATH=$(COMPILERPATH)\lib
#CLIB=$(COMPILERPATH)\lib\cs.lib
#INCLUDEPATH=$(COMPILERPATH)\include
#MATH_EXTRACT=*H_LDIV *H_LLSH *H_LURSH *N_LXMUL *F_LXMUL *H_LRSH *H_SPUSH *N_SCOPY *F_SCOPY
#MATH_INSERT=+H_LDIV +H_LLSH +H_LURSH +N_LXMUL +F_LXMUL +H_LRSH +H_SPUSH +N_SCOPY +F_SCOPY


#
# $Id$
#
# $Log$
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
