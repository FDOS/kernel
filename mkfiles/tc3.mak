#
# TC3.MAK - kernel copiler options for Turbo C 3.0
#

# Use these for Turbo C 3.0

COMPILERPATH=$(TC3_BASE)
COMPILERBIN=$(COMPILERPATH)\bin
CC=$(COMPILERBIN)\tcc -c
CL=$(COMPILERBIN)\tcc
INCLUDEPATH=$(COMPILERPATH)\include
LIBUTIL=$(COMPILERBIN)\tlib
LIBPATH=$(COMPILERPATH)\lib
LIBTERM=  
LIBPLUS=+

TINY=-lt
CFLAGST=-L$(LIBPATH) -mt -a- -k- -f- -ff- -O -Z -d
CFLAGSC=-L$(LIBPATH) -a- -mc

TARGET=KT3

# used for building the library

CLIB=$(COMPILERPATH)\lib\cs.lib
MATH_EXTRACT=*H_LDIV *H_LLSH *H_LURSH *N_LXMUL *F_LXMUL *H_LRSH
MATH_INSERT=+H_LDIV +H_LLSH +H_LURSH +N_LXMUL +F_LXMUL +H_LRSH

#
# heavy stuff - building the kernel
# Compiler and Options for Borland C++
# ------------------------------------
#
#  -zAname       ¦ ¦ Code class
#  -zBname       ¦ ¦ BSS class
#  -zCname       ¦ ¦ Code segment
#  -zDname       ¦ ¦ BSS segment
#  -zEname       ¦ ¦ Far segment
#  -zFname       ¦ ¦ Far class
#  -zGname       ¦ ¦ BSS group
#  -zHname       ¦ ¦ Far group
#  -zPname       ¦ ¦ Code group
#  -zRname       ¦ ¦ Data segment
#  -zSname       ¦ ¦ Data group
#  -zTname       ¦ ¦ Data class
#  -zX           ¦«¦ Use default name for "X"

#
# ALLCFLAGS specified by turbo.cfg and config.mak
#
ALLCFLAGS=$(TARGETOPT) $(ALLCFLAGS)
INITCFLAGS=$(ALLCFLAGS) -zAINIT -zCINIT_TEXT -zDIB -zRID -zTID -zPI_GROUP -zBIB -zGI_GROUP -zSI_GROUP
CFLAGS=$(ALLCFLAGS)
