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


# Use these for Turbo C 3.0
#COMPILER=TC3
#COMPILERPATH=c:\tc
#CC=$(COMPILERPATH)\bin\tcc
#LINK=$(COMPILERPATH)\bin\tlink
#LIBUTIL=$(COMPILERPATH)\bin\tlib
#LIBPATH=$(COMPILERPATH)\lib
#CLIB=$(COMPILERPATH)\lib\cs.lib
#INCLUDEPATH=$(COMPILERPATH)\include


# Use these for Borland C++
#COMPILER=BC5
#COMPILERPATH=c:\bc5
#CC=$(COMPILERPATH)\bin\tcc
#LINK=$(COMPILERPATH)\bin\tlink
#LIBUTIL=$(COMPILERPATH)\bin\tlib
#LIBPATH=$(COMPILERPATH)\lib
#CLIB=$(COMPILERPATH)\lib\cs.lib
#INCLUDEPATH=$(COMPILERPATH)\include


#
# $Id$
#
# $Log$
# Revision 1.1  2000/05/06 19:34:02  jhall1
# Initial revision
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
