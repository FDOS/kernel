#
# TURBOCPP.MAK - kernel copiler options for TCPP 1.01
#

# Use these for Turbo CPP 1.01

COMPILERPATH=$(TP1_BASE)
COMPILERBIN=$(COMPILERPATH)\bin
CC=$(COMPILERBIN)\tcc
CFLAGST=-mt -lt -a- -k- -f- -ff- -O -Z -d
CFLAGSC=-a- -mc
INCLUDEPATH=$(COMPILERPATH)\include
LIBUTIL=$(COMPILERBIN)\tlib
LIBPATH=$(COMPILERPATH)\lib
LIBTERM=  
LIBPLUS=+

TARGET=KTP

# used for building the library

CLIB=$(COMPILERPATH)\lib\cs.lib
MATH_EXTRACT=*H_LDIV *F_LXMUL *H_LURSH *H_LLSH *H_LRSH
MATH_INSERT=+H_LDIV +F_LXMUL +H_LURSH +H_LLSH +H_LRSH

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
ALLCFLAGS = $(TARGETOPT) $(ALLCFLAGS)
INITCFLAGS = $(ALLCFLAGS) -zAINIT -zCINIT_TEXT -zDIB -zRID -zTID -zPIGROUP -zBIB -zGIGROUP -zSIGROUP
CFLAGS     = $(ALLCFLAGS) -zAHMA -zCHMA_TEXT
DYNCFLAGS = $(ALLCFLAGS) -zRDYN_DATA -zTDYN_DATA -zDDYN_DATA -zBDYN_DATA
IPRFCFLAGS = $(INITCFLAGS) -oiprf.obj
